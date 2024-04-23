#!/usr/bin/env python
import csv
from argparse import ArgumentParser

P = ArgumentParser(
    description="Parse LTE MAC Ul Stats file and output one CSV per drone containing LTE Cell ID and TB size"
)
P.add_argument("results_dir", type=str, help="Results directory of reference")
args = P.parse_args()

COL_TIME = 0
COL_CELLID = 1
COL_IMSI = 2
COL_SIZETB = 7
INPUT_FILE_NAME = "lte-0-MacUlStats.txt"
OUT_FILE_PREFIX = "lte-UlMacStats-drone"

drones = []

with open(f"{args.results_dir}/{INPUT_FILE_NAME}", "r") as csvf:
    csvr = csv.reader(csvf, delimiter="\t")
    next(csvr)  # discard header

    for r in csvr:
        time = float(r[COL_TIME])
        cellid = int(r[COL_CELLID])
        droneid = int(r[COL_IMSI]) - 1
        sizetb = int(r[COL_SIZETB])

        while len(drones) <= droneid:
            drones.append([])

        drones[droneid].append([time, cellid, sizetb])

for i_drone in range(len(drones)):
    with open(f"{args.results_dir}/{OUT_FILE_PREFIX}_{i_drone}.csv", "w") as csvf:
        csvw = csv.writer(csvf)

        csvw.writerow(["time", "cellid", "sizetb"])  # heading
        csvw.writerows(drones[i_drone])
