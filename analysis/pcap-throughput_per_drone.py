#!/usr/bin/env python
import csv
from argparse import ArgumentParser

P = ArgumentParser(description='Translate TCP traffic from CSV in throughput')
P.add_argument('n_drones', type=int, help='Number of simulated drones')
P.add_argument('results_path', type=str, help='Results directory of reference')
P.add_argument('--use_Mbps', action='store_true', default=False, help='Report throughput in Mbps. Default: kbps')
args = P.parse_args()

input_file_prefix = 'pcap_lte_tcp_len'
output_file_prefix = 'pcap-throughput'
drones = []

for i_drone in range(args.n_drones):
  drones.append([])
  txbytes = 0
  timeref = 0.0

  with open(f'{args.results_path}/{input_file_prefix}-drone_{i_drone}.csv') as csvf:
    csvr = csv.reader(csvf)
    next(csvr) # skip heading

    for r in csvr:
      stime = float(r[0])
      spayload = int(r[1])

      if int(stime) > timeref:
        exp = 2 if args.use_Mbps else 1
        drones[i_drone].append([int(timeref), txbytes * 8.0 / (1024.0 ** exp)])
        timeref = stime
        txbytes = 0.0

      txbytes += spayload


for i_drone in range(len(drones)):
    with open(f'{args.results_path}/{output_file_prefix}-drone_{i_drone}.csv', 'w') as csvf:
        csvw = csv.writer(csvf)

        hdr_scale = 'Mbps' if args.use_Mbps else 'kbps'
        csvw.writerow(['time', hdr_scale]) # header
        csvw.writerows(drones[i_drone])

