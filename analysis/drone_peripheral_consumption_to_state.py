#!/usr/bin/env python
import csv
from argparse import ArgumentParser
from enum import Enum

P = ArgumentParser(description='Given power law of a peripheral, convert a given CSV with peripheral power data to deduce its state.')
P.add_argument('input_csv_filepath', type=str, help='Input CSV filepath containing peripheral power data.')
P.add_argument('output_csv_filepath', type=str, help='Output CSV filepath to report peripheral state.')
P.add_argument('off_state_power_W', type=float, help='Power consumption of the peripheral when in OFF state.')
P.add_argument('on_state_power_W', type=float, help='Power consumption of the peripheral when in ON state.')
P.add_argument('standby_state_power_W', type=float, help='Power consumption of the peripheral when in STANDBY state.')
args = P.parse_args()

peripheral_state = {
  args.off_state_power_W: 'OFF',
  args.on_state_power_W: 'ON',
  args.standby_state_power_W: 'STANDBY'
}

inf = open(args.input_csv_filepath, 'r')
inr = csv.reader(inf)
next(inr) # ignore header

outf = open(args.output_csv_filepath, 'w')
outw = csv.writer(outf, lineterminator='\n')
outw.writerow(['time','state'])

for r in inr:
  t = float(r[0])
  p = float(r[1])
  s = peripheral_state[p]
  outw.writerow([t,s])

inf.close()
outf.close()
