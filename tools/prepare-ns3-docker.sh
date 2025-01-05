#!/usr/bin/env bash

set -e

check_availability() {
  PRG_NAME=$1

  command -v $PRG_NAME &> /dev/null
  if [ "$?" -ne 0 ]; then
    echo "The program \"${PRG_NAME}\" is not available in your system. \
          Please install it, then re-run this script."
    exit 1
  fi
}

# check dependencies
check_availability git
check_availability patch

git submodule update --init

pushd ns3 > /dev/null
for patchfile in ../tools/*.patch
do
  patch -s -p1 < $patchfile
done
popd > /dev/null
