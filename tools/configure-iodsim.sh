#!/usr/bin/env bash

mkdir build
cmake -B build .

python -m venv .venv
source .venv/bin/activate
pip install -r analysis/requirements.txt \
            -r scenario/py_design_support/requirements.txt

pushd ns3
./ns3 configure --build-profile=debug --enable-examples --enable-tests --disable-mpi \
                --disable-python --enable-modules=iodsim,nr,leo,point-to-point-layout
popd
