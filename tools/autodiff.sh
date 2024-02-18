#!/usr/bin/env bash

if [[ -z "$1" || -z "$2" ]]
then
  echo "Usage: $0 /path/to/container_1 /path/to/container_2"
  exit 1
fi

CONTAINER_1=$1
CONTAINER_2=$2
SCRIPT_PATH=$(dirname $0)

for dir1 in $CONTAINER_1/*
do
  [[ $(basename "$dir1") =~ ^(.*)(-[0-9]+){3}.([0-9]+-?){3} ]]
  scenario_name=${BASH_REMATCH[1]}

  dir2=$(find $CONTAINER_2 -name "${scenario_name}*" | head -n 1)

  echo "=== TESTING $scenario_name ==="
  $SCRIPT_PATH/differ.sh "$dir1" "$dir2"
done
