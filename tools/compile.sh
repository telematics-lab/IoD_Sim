#!/usr/bin/env bash

set -e

pushd ns3
./ns3 configure --enable-examples --disable-python --enable-modules=iodsim
./ns3 build
popd
