#!/usr/bin/env bash

set -e

# Cd on the root IoD_Sim directory
cd $(dirname "$0")/..

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

# Funzione per integrare un modulo in contrib
clone_checkout_ns3() {
  local module_name="ns3"
  local repo_url="https://gitlab.com/nsnam/ns-3-dev.git"
  local checkout_ref="$1"

    if [ -d "$module_name" ]; then
        cd "$module_name"
        git checkout "$checkout_ref"
	cd - > /dev/null
    else
      git clone "$repo_url" "$module_name"
      if [ -n "$checkout_ref" ]; then
        cd "$module_name"
        git checkout "$checkout_ref"
        cd - > /dev/null
      fi
    fi
}

# Funzione per integrare un modulo in contrib
integrate_contrib_module() {
  local module_name="$1"
  local repo_url="$2"
  local checkout_ref="$3" # opzionale

  local contrib_path="./ns3/contrib"
  local module_path="${contrib_path}/${module_name}"

  if [ -d "$contrib_path" ]; then
    if [ -d "$module_path" ]; then
      echo "Error: ${module_name} module already present."
    else
      git clone "$repo_url" "$module_path"
      if [ -n "$checkout_ref" ]; then
        cd "$module_path"
        git checkout "$checkout_ref"
        cd - > /dev/null
      fi
    fi
  else
    echo "Error: ns3 submodule not initialised"
  fi
}

clone_checkout_ns3 ns-3.45
ln -fs ../../leo ./ns3/contrib/leo

# If this step fails, continue anyway
./tools/install-nr-dependencies.sh || {
  echo "Warning: ./tools/install-nr-dependencies.sh failed — continuing."
}
integrate_contrib_module "nr" "https://gitlab.com/cttc-lena/nr.git" "5g-lena-v4.1.y"

pushd ns3 > /dev/null
# Deactivate signature of commit if any setted globally on current global git config
git config --local commit.gpgsign false
git am ../tools/*.patch
popd > /dev/null
