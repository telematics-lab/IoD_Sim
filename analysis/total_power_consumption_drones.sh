#!/usr/bin/env bash

if [ -z "$1" ] || [ "$1" == '-h' ]; then
  echo "Usage: $0 <debug_log_file>"
  exit 1
fi

OUTPUT_FILE_PREFIX=total-power
DEBUG_LOG_FILE=$(basename $1)
RESULTS_DIR=$(dirname $1)
N_DRONES=$(grep -oP '(?<=Total Power Consumption for Drone #)([0-9]+)' $RESULTS_DIR/$DEBUG_LOG_FILE \
            | sort -nr \
            | head -n1)

for i in $(seq 0 $N_DRONES); do
  ipp=$(( $i + 1 ))
  echo 'time,power_W' > $RESULTS_DIR/$OUTPUT_FILE_PREFIX-drone_$ipp.csv # csv header

  grep "Total Power Consumption for Drone #${i}" $RESULTS_DIR/$DEBUG_LOG_FILE \
    | sed -r 's/^\+([0-9\.]+)s.* ([0-9\.]+) W/\1,\2/g' \
    >> $RESULTS_DIR/$OUTPUT_FILE_PREFIX-drone_$ipp.csv
done
