#!/bin/bash

# Shouln't this be put in a proper Makefile?

pushd ns3
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$PWD/build/lib/
./waf build -j$(grep -c Processor /proc/cpuinfo) || exit
$PWD/build/src/drone/examples/ns3.35-iod-sim-debug --config=$PWD/src/drone/config/$1
popd

