#!/bin/bash

# TODO: Shouldn't this be a proper Makefile?

pushd ns3
./waf configure --enable-examples --enable-mpi --disable-python --enable-modules=drone --enable-module=irs
./waf build -j$(grep -c Processor /proc/cpuinfo)
popd
