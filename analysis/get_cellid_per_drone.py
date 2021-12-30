#!/usr/bin/env python
import csv
import sys
from argparse import ArgumentParser

P = ArgumentParser(description="Parse LTE Rlc Ul Stats file and output one file per drone/IMSI containing the LTE Cell ID")

P.add_argument('rlc_filepath', type=str, help='LTE Rlc Ul Stats file path')
P.add_argument('output_dir', type=str, help='Output directory where new CSV files will be stored')

P.parse_args()

rlc_fp = P.rlc_filepath
out_csv_dir = P.output_dir

COL_START = 0
COL_END = 1
COL_CELLID = 2
COL_IMSI = 3

samples = []

# Read everything and agglomerate data in samples
with open(rlc_fp, 'r') as rlcf:
  csvr = csv.reader(rlcf, delimiter='\t')

  next(csvr) # Discard header
  for row in csvr:
    imsi = int(row[COL_IMSI])
    while imsi > len(samples):
      samples.append([])

    start = float(row[COL_START])
    end = float(row[COL_END])
    cellId = int(row[COL_CELLID])

    samples[imsi - 1].append([start, end, cellId])

# for each imsi in samples, write csv
i_drone = 0
for d in samples:
  with open(f'{out_csv_dir}/lte-rlc-drone-{i_drone}.csv', 'w') as f:
    csvw = csv.writer(f)
    csvw.writerow(['start', 'end', 'cellId']) # header
    csvw.writerows(d)

  i_drone += 1

