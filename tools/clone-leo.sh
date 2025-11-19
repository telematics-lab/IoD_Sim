#!/bin/bash

check_availability() {
  PRG_NAME=$1

  command -v $PRG_NAME &>/dev/null
  if [ "$?" -ne 0 ]; then
    echo "The program \"${PRG_NAME}\" is not available in your system. \
          Please install it, then re-run this script."
    exit 1
  fi
}

# check dependencies
check_availability git

#./tools/install-nr-dependencies.sh


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

#TODO: Evaluate if to retrieve leo from another repository or add it in the iodsim src code
#integrate_contrib_module "leo" "git@github.com:domysh/leo.git" "iodsim"
#mkdir ns3/contrib/leo
ln -s ../../leo ./ns3/contrib/