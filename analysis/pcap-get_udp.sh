#!/usr/bin/env bash

# Given a pcap file, for each drone connected to the LTE network extrapolate tcp payload length and export everything in CSV
# This can be useful to evaluate throughput

if [ -z "$1" ] | [ -z "$2" ] | [[ "${2##*.}" -ne "pcap" ]]; then
    echo "Usage ${0} <num_drones> <pcap_file>"
    exit 1
fi

NUM_DRONES=$1
PCAP_FILEPATH=$2

NET_PREFIX=10.2.0
CSV_FILENAME_PREFIX=pcap_lte_tcp_len

for drone_i in $(seq 0 $(( $NUM_DRONES - 1 ))); do
    drone_i_hostip=$(( $drone_i + 2 ))
    CSV_FILENAME=${CSV_FILENAME_PREFIX}-drone_${drone_i}.csv
    CSV_FILEPATH=$(dirname $PCAP_FILEPATH)/$CSV_FILENAME

    # Headings
    echo time,len > $CSV_FILEPATH

    tshark -r $PCAP_FILEPATH      \
	   -T fields              \
	   -E separator=,         \
	   -e frame.time_relative \
	   -e udp.length          \
	   udp and not icmp and ip.src == ${NET_PREFIX}.${drone_i_hostip} \
    >> $CSV_FILEPATH
done
