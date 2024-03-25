#!/usr/bin/env bash

set -e

if [[ -z "$1" || -z "$2" ]]
then
  echo "Usage: $0 /path/to/scenario_1 /path/to/scenario_2"
  exit 1
fi

SCENARIO_1=$1
SCENARIO_2=$2
SCRIPT_PATH=$(dirname $0)
CHECKSUM_FILENAME=check.sha256sum
TAB='    '

find_different_files() {
  diff --side-by-side --suppress-common-lines -W 999 \
    $SCENARIO_1/$CHECKSUM_FILENAME $SCENARIO_2/$CHECKSUM_FILENAME \
    | awk '{print $2}' \
    | tr -d '*'
}

SCENARIOS=( $SCENARIO_1 $SCENARIO_2 )
for s in ${SCENARIOS[*]}
do
  $SCRIPT_PATH/cleaner.sh $s
done

DIFFS=$(find_different_files)

echo "The following files differ:"
for f in $DIFFS
do
  echo -e "- $f"
done
