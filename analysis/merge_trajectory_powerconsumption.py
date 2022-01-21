#!/usr/bin/env python
import csv
from argparse import ArgumentParser
from math import isclose

P = ArgumentParser(description='Merge trajectory data with power consumption for a given drone.')
P.add_argument('trajectory_csv_filepath', type=str, help='Trajectory CSV file.')
P.add_argument('power_consumption_csv_filepath', type=str, help='Drone power consumption CSV file.')
P.add_argument('output_filepath', type=str, help='Output CSV filepath.')
args = P.parse_args()

trf = open(args.trajectory_csv_filepath, 'r')
tr_csvr = csv.reader(trf)
next(tr_csvr) # ignore header

pcf = open(args.power_consumption_csv_filepath, 'r')
pc_csvr = csv.reader(pcf)
next(pc_csvr) # ignore header

of = open(args.output_filepath, 'w')
of_csvw = csv.writer(of, lineterminator='\n')
of_csvw.writerow(['time', 'x', 'y', 'z', 'power', 'pstate'])

trr = next(tr_csvr)
pcr = next(pc_csvr)
trr_time = float(trr[3])
pcr_time = float(pcr[0])

try:
  while trr and pcr:
    if isclose(trr_time, pcr_time, abs_tol=0.05):
      of_csvw.writerow([trr_time, trr[0], trr[1], trr[2], pcr[1], pcr[2]])
      trr = next(tr_csvr)
      trr_time = float(trr[3])
    elif trr_time > pcr_time:
      pcr = next(pc_csvr)
      pcr_time = float(pcr[0])
    elif pcr_time > trr_time:
      trr = next(tr_csvr)
      trr_time = float(trr[3])
except:
  pass

of.close()
pcf.close()
trf.close()
