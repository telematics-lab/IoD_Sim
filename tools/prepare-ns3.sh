#!/usr/bin/bash

git submodule update --init

pushd ns3
patch -s -p1 < ../tools/0001-Integrate-ns3-with-IoD-Sim.patch
patch -s -p1 < ../tools/0002-Add-ns3-quirks-for-IoD-Sim-interoperability.patch
patch -s -p1 < ../tools/0003-Add-LTE-temporary-quirks.patch
popd
