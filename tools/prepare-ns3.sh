#!/usr/bin/bash

git submodule update --init

pushd ns3
patch -s -p1 < ../tools/*.patch
popd
