#!/bin/bash

set -e

if [[ -z "$1" ]]
then
  echo "Please specify results scenario path."
  exit 1
fi

SCENARIO_PATH=$1
CHECKSUM_FILENAME=check.sha256sum

pushd $SCENARIO_PATH > /dev/null
truncate --size 0 $CHECKSUM_FILENAME

# Clean up summary.xml
sed -e 's/executedAt=".*"/executedAt="REDACTED"/g'      \
    -e 's/\(<real>\).*\(<\/real>\)/\1REDACTED\2/'       \
    -e 's/\(<virtual>\).*\(<\/virtual>\)/\1REDACTED\2/' \
    ./summary.xml > summary.redacted.xml
sha256sum --binary summary.redacted.xml >> $CHECKSUM_FILENAME

# Clean up scenario log file
# Replace all references to a memory address with "REDACTED"
sed 's/0x[0-9a-f]\+/REDACTED/g' scenario.log > scenario.redacted.log
sha256sum --binary scenario.redacted.log >> $CHECKSUM_FILENAME

sha256sum --binary *.{txt,tr,pcap} >> $CHECKSUM_FILENAME

popd > /dev/null
