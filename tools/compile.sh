#!/bin/bash

# TODO: Shouldn't this be a proper Makefile?

pushd ns3
./waf configure --enable-examples --enable-mpi --disable-python --enable-modules=drone
./waf build -j$(grep -c Processor /proc/cpuinfo)
popd

