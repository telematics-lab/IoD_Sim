#!/usr/bin/env bash

LAST_WIFI_RES_DIR=$(ls -d ./results/paper_3-wifi* | tail -n1)

for i in $LAST_WIFI_RES_DIR/wifi-*-2.pcap; do
  echo $i
  tshark -r $i -T fields -e ip.src udp and not icmp | sort -n | uniq
done

echo '===INTERNET==='

tshark -r $(ls $LAST_WIFI_RES_DIR/internet-*.pcap | head -n1 ) -T fields -e ip.src -e udp.srcport udp and not icmp | sort -n  | uniq