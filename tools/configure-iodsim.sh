#!/usr/bin/env bash

# Cd on the root IoD_Sim directory
cd $(dirname "$0")/..

MODE="debug"
ENABLE_CLI=true

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --mode) MODE="$2"; shift ;;
        --no-cli) ENABLE_CLI=false ;;
        *) echo "Unknown parameter passed: $1"; exit 1 ;;
    esac
    shift
done

if [ "$ENABLE_CLI" = true ]; then
    ENABLE_CLI="-DENABLE_CLI_COMMANDS=ON"
else
    ENABLE_CLI=""
fi

mkdir -p build
cmake -B build .

python -m venv .venv
source .venv/bin/activate
pip install -r analysis/requirements.txt \
            -r scenario/py_design_support/requirements.txt

# ns-3 force-enables logging and asserts only for the "debug" profile: every other
# profile silently drops both unless they are requested explicitly. Asserts are what
# catches misconfigured scenarios before they produce quietly wrong results, and
# NS_LOG is the only way to inspect the schedulers, so keep them on in every profile.
# The point of a non-debug profile here is the optimisation level (-O2 instead of
# -O0), not the removal of the diagnostics.
EXTRA_FLAGS=""
if [ "$MODE" != "debug" ]; then
    EXTRA_FLAGS="--enable-asserts --enable-logs"
fi

pushd ns3
./ns3 configure --build-profile="$MODE" $EXTRA_FLAGS \
                --enable-examples --enable-tests --disable-mpi \
                --disable-python --enable-modules=iodsim,nr,leo,point-to-point-layout \
                -- -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $ENABLE_CLI
popd
