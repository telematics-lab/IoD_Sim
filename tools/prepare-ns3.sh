#!/usr/bin/env bash

set -e

# Cd on the root IoD_Sim directory
cd $(dirname "$0")/..
ROOT_DIR="$(pwd)"

FORCE=false

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --force|-f) FORCE=true ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

check_availability() {
  PRG_NAME=$1

  command -v $PRG_NAME &> /dev/null
  if [ "$?" -ne 0 ]; then
    echo "The program \"${PRG_NAME}\" is not available in your system. \
          Please install it, then re-run this script."
    exit 1
  fi
}

check_availability git
check_availability patch

skip_ssh_id_and_signature() {
  local repo_path="$1"

  # Deactivate signature of commit if any setted globally on current global git config
  git -C "$repo_path" config --local commit.gpgsign false

  if [ -z "$(git -C "$repo_path" config user.name)" ]; then
    git -C "$repo_path" config --local user.name "IoD Sim"
  fi

  if [ -z "$(git -C "$repo_path" config user.email)" ]; then
    git -C "$repo_path" config --local user.email "iod_sim@no-reply.com"
  fi
}

# Impronta del set di patch di una directory, per accorgersi quando cambiano.
patches_fingerprint() {
  local patches_dir="$1"

  if [ -z "$patches_dir" ] || ! ls "$patches_dir"/*.patch > /dev/null 2>&1; then
    echo "no-patches"
  else
    cat "$patches_dir"/*.patch | git hash-object --stdin
  fi
}

# Risolve il ref pinnato preferendo il branch remoto, cosi' un branch locale
# rimasto indietro non viene usato al posto di quello di origin. I tag, che non
# esistono sotto refs/remotes/, ricadono sul ref cosi' com'e'.
resolve_ref() {
  local repo_path="$1"
  local checkout_ref="$2"

  if git -C "$repo_path" rev-parse --verify --quiet "refs/remotes/origin/${checkout_ref}" > /dev/null; then
    echo "origin/${checkout_ref}"
  else
    echo "$checkout_ref"
  fi
}

# Verifica se l'albero e' gia' esattamente nello stato che questo script produce:
# il ref pinnato come base, e sopra i commit delle patch, nello stesso ordine e
# con gli stessi subject. Serve ad adottare un albero preparato da una versione
# precedente dello script (o che ha perso il marker) senza rifare tutto da capo.
patches_already_applied() {
  local repo_path="$1"
  local base_ref="$2"
  local patches_dir="$3"

  git -C "$repo_path" merge-base --is-ancestor "$base_ref" HEAD > /dev/null 2>&1 || return 1

  local applied="$(git -C "$repo_path" log --reverse --format=%s "${base_ref}..HEAD")"
  local expected="$(
    if [ -n "$patches_dir" ] && ls "$patches_dir"/*.patch > /dev/null 2>&1; then
      for p in "$patches_dir"/*.patch; do
        # mailinfo normalizza il subject, che nei file patch puo' andare a capo.
        git mailinfo /dev/null /dev/null < "$p" 2> /dev/null | sed -n 's/^Subject: //p'
      done
    fi
  )"

  [ "$applied" = "$expected" ]
}

# Prepara un repository esterno: clone (se assente), checkout del ref pinnato e
# applicazione delle patch di IoD_Sim.
#
# E' idempotente: ri-eseguire lo script su un albero gia' preparato non fa nulla.
# Il lavoro viene rifatto solo se il ref o le patch sono cambiati, e in quel caso
# si riparte dal ref pinnato. Eventuali modifiche locali non committate bloccano
# l'operazione, a meno di --force, per non buttare via lavoro dell'utente.
prepare_repo() {
  local repo_path="$1"
  local repo_url="$2"
  local checkout_ref="$3"
  local patches_dir="${4:-}"

  local expected="${checkout_ref}|$(patches_fingerprint "$patches_dir")"

  if [ ! -d "$repo_path" ]; then
    echo ">> Cloning ${repo_url} into ${repo_path}"
    mkdir -p "$(dirname "$repo_path")"
    git clone "$repo_url" "$repo_path"
    git -C "$repo_path" config --local advice.detachedHead false
  fi

  local marker="$(git -C "$repo_path" rev-parse --absolute-git-dir)/iodsim-prepared"
  local is_dirty=false
  if [ -n "$(git -C "$repo_path" status --porcelain)" ]; then
    is_dirty=true
  fi

  # Gia' preparato con lo stesso ref e le stesse patch: non c'e' niente da fare.
  if [ -f "$marker" ] && [ "$(cat "$marker")" = "${expected}|$(git -C "$repo_path" rev-parse HEAD)" ]; then
    if [ "$is_dirty" = false ]; then
      echo ">> ${repo_path} already prepared (${checkout_ref}), nothing to do"
      return
    fi

    echo ">> ${repo_path} is prepared (${checkout_ref}) but has uncommitted changes"
    if [ "$FORCE" = false ]; then
      echo "   Leaving them untouched. Re-run with --force to discard and re-prepare."
      return
    fi
  fi

  local base_ref="$(resolve_ref "$repo_path" "$checkout_ref")"

  # Nessun marker, ma l'albero e' gia' nello stato giusto: lo si adotta. Solo in
  # assenza di marker: se ce n'e' uno che non torna, le patch sono cambiate sul
  # serio e vanno riapplicate anche a parita' di subject.
  if [ ! -f "$marker" ] && [ "$is_dirty" = false ] \
     && patches_already_applied "$repo_path" "$base_ref" "$patches_dir"; then
    echo ">> ${repo_path} already at ${checkout_ref} with its patches applied, adopting it"
    echo "${expected}|$(git -C "$repo_path" rev-parse HEAD)" > "$marker"
    return
  fi

  # Serve ripartire dal ref pinnato, quindi l'albero va azzerato.
  if [ "$is_dirty" = true ] && [ "$FORCE" = false ]; then
    echo "Error: ${repo_path} has uncommitted changes and needs to be re-prepared."
    echo "       Save them as a patch under tools/patches/, or re-run this script"
    echo "       with --force to discard them."
    exit 1
  fi

  echo ">> Preparing ${repo_path} @ ${checkout_ref}"

  # Un'esecuzione precedente interrotta puo' aver lasciato un git am a meta'.
  git -C "$repo_path" am --abort > /dev/null 2>&1 || true

  git -C "$repo_path" fetch --tags origin
  base_ref="$(resolve_ref "$repo_path" "$checkout_ref")"
  git -C "$repo_path" checkout --detach "$base_ref"
  git -C "$repo_path" reset --hard HEAD

  if [ -n "$patches_dir" ] && ls "$patches_dir"/*.patch > /dev/null 2>&1; then
    skip_ssh_id_and_signature "$repo_path"
    git -C "$repo_path" am "$patches_dir"/*.patch
  fi

  echo "${expected}|$(git -C "$repo_path" rev-parse HEAD)" > "$marker"
}

# ns-3 e i suoi moduli contrib. nr vive dentro ns3/contrib, quindi ns3 va per primo.
# rapidyyjson non compare qui: come sgp4 in leo/CMakeLists.txt, e' CMake stesso a
# clonarlo con FetchContent (si veda src/CMakeLists.txt).
prepare_repo "ns3" "https://gitlab.com/nsnam/ns-3-dev.git" "ns-3.48" "${ROOT_DIR}/tools/patches/ns3"
prepare_repo "ns3/contrib/nr" "https://gitlab.com/cttc-lena/nr.git" "5g-lena-v5.1.y" "${ROOT_DIR}/tools/patches/nr"

ln -fs ../../leo ./ns3/contrib/leo

chmod +x ./ns3/ns3
