#!/usr/bin/env bash

# Cd on the root IoD_Sim directory
cd $(dirname "$0")/..

MODE="debug"

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --mode) MODE="$2"; shift ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

mkdir -p build
cmake -B build .

python -m venv .venv
source .venv/bin/activate
pip install -r analysis/requirements.txt \
            -r scenario/py_design_support/requirements.txt

pushd ns3
./ns3 configure --build-profile="$MODE" --enable-examples --enable-tests --disable-mpi \
                --disable-python --enable-modules=iodsim,nr,leo,point-to-point-layout
popd
