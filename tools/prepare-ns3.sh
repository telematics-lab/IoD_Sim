#!/bin/bash

function check_availability() {
  PRG_NAME=$1

  which $PRG_NAME &> /dev/null
  if [ "$?" -ne 0 ]; then
    echo "The program \"${PRG_NAME}\" is not available in your system. Please install it, then re-run this script."
    exit 1
  fi
}

# check dependencies
check_availability git
check_availability patch

git submodule update --init

pushd ns3
patch -s -p1 < ../tools/0001-Integrate-ns3-with-IoD-Sim.patch
patch -s -p1 < ../tools/0002-Add-ns3-quirks-for-IoD-Sim-interoperability.patch
patch -s -p1 < ../tools/0003-Add-LTE-temporary-quirks.patch
popd
