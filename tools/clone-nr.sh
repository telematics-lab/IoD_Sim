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

./tools/install-nr-dependencies.sh

contrib_path="./ns3/contrib"
nr_path="./ns3/contrib/nr"

if [ -d "$contrib_path" ]; then
  if [ -d "$nr_path" ]; then
    echo "Error: nr module already present."
  else
    git clone https://gitlab.com/cttc-lena/nr.git ./ns3/contrib/nr
    cd ./ns3/contrib/nr
    git checkout -b 5g-lena-v3.3.y origin/5g-lena-v3.3.y
  fi
else
  echo "Error: ns3 submodule not initialised"
fi
