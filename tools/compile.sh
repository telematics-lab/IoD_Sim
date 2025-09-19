#!/usr/bin/env bash

set -e

pushd ns3
./ns3 configure --build-profile=debug --enable-examples --enable-tests --disable-mpi \
  --disable-python --enable-modules=iodsim,nr,leo,point-to-point-layout
./ns3 build
popd
