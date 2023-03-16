#!/bin/bash

# TODO: Shouldn't this be a proper Makefile?

pushd ns3
./ns3 configure --enable-examples --enable-mpi --disable-python --enable-modules=drone
./ns3 build -j$(grep -c Processor /proc/cpuinfo)
popd
