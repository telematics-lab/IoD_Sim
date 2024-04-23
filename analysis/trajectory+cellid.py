#!/usr/bin/env python
import csv
from argparse import ArgumentParser

P = ArgumentParser(
    description="Combine LTE RLC data and trajectory in a single CSV per drone"
)
P.add_argument("n_drones", type=int, help="Number of simulated drones")
P.add_argument("results_dir", type=str, help="Results directory of reference")
args = P.parse_args()

INPUT_LTE_RLC_FILE_PREFIX = "lte-rlc-drone"
INPUT_TRAJECTORY_FILE_PREFIX = "trajectory-drone"
OUTPUT_FILE_PREFIX = "traj+cellid-drone"

for i_drone in range(args.n_drones):
    start = -1.0
    cellid = 0

    rlcf = open(f"{args.results_dir}/{INPUT_LTE_RLC_FILE_PREFIX}_{i_drone}.csv", "r")
    rlc_csvr = csv.reader(rlcf)
    next(rlc_csvr)  # discard header

    trf = open(f"{args.results_dir}/{INPUT_TRAJECTORY_FILE_PREFIX}_{i_drone}.csv", "r")
    trf_csvr = csv.reader(trf)
    next(trf_csvr)  # discard header

    destf = open(f"{args.results_dir}/{OUTPUT_FILE_PREFIX}_{i_drone}.csv", "w")
    dest_csvw = csv.writer(destf)
    dest_csvw.writerow(["x", "y", "z", "t", "cellId"])

    for r in trf_csvr:
        x = float(r[0])
        y = float(r[1])
        z = float(r[2])
        t = float(r[3])

        while t > start:
            rlc_row = rlc_csvr.__next__()
            start = float(rlc_row[0])
            cellid = int(rlc_row[2])

        dest_csvw.writerow([x, y, z, t, cellid])

    destf.close()
    trf.close()
    rlcf.close()
