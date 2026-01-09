#!/bin/bash

usage() {
    echo "Usage:"
    echo "sudo docker run --rm -v ./results:/IoD_Sim/results -v ./scenario:/IoD_Sim/scenario docker_iod_sim [--help] <simulation_file_name>"
    echo
    echo "Options:"
    echo "  --help      Shows help message."
    echo "  <simulation_file_name> Name of the .json file to simulate."
    exit 1
}

if [[ "$1" == "--help" ]]; then
    usage
elif [[ -z "$1" ]]; then
    usage
elif [[ ! -f "./scenario/$1.json" ]]; then
    echo "Error: File '$1.json' doesn't exist."
    usage
else
    source .venv/bin/activate && ./ns3/ns3 run "scenario --config=../scenario/$1.json"
    chmod -R 777 ./results/*
fi