#!/usr/bin/bash

git submodule update --init

pushd ns3
patch -s -p1 < ../tools/0001-Integrate-ns-3-with-IoD-Sim.patch
popd
