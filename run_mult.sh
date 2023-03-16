#!/bin/bash


for (( i=1 ; i<=$2 ; i++ ));
do
    echo "Running $1 for the $i° time"
    ./ns3/ns3 run "iodsim --config=../scenario/$1.json"
done
