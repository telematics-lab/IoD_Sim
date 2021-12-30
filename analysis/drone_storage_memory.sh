#!/bin/bash

# Parse storage memory from debug log file and output a CSV file per drone

if [ -z "$2" ]; then
    echo "Usage $0 <num_drones> <debug_log_filepath>"
    exit 1
fi

NUM_DRONES=$1
DEBUG_LOG_FILEPATH=$2
DEBUG_LOG_FILENAME=$(basename $DEBUG_LOG_FILEPATH)
RESULTS_PATH=$(dirname $DEBUG_LOG_FILEPATH)
OUTPUT_FILE_PREFIX=mem-drone

pushd $RESULTS_PATH

for i in $(seq 0 $(( NUM_DRONES - 1 ))); do
    OUTPUT_FILE_NAME=${OUTPUT_FILE_PREFIX}_${i} # no extension

    grep "Stored memory on Drone \#${i}" $DEBUG_LOG_FILENAME \
        > /tmp/$OUTPUT_FILE_NAME.log

    # Convert to CSV
    # CSV Header
    echo "time,bits" > ${OUTPUT_FILE_NAME}.csv
    # CSV Body
    awk '{ print $1","$10; }' /tmp/${OUTPUT_FILE_NAME}.log \
    	| sed -e 's/+//g' -e 's/s//g' \
	>> ${OUTPUT_FILE_NAME}.csv

    rm /tmp/${OUTPUT_FILE_NAME}.log
done

popd

