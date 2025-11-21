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

clone_checkout_ns3 ns-3.45
ln -s ../../leo ./ns3/contrib/

pushd ns3 > /dev/null
git am ../tools/*.patch
popd > /dev/null
