#!/usr/bin/env python
import csv
from argparse import ArgumentParser

P = ArgumentParser(
    description="Parse LTE MAC Ul stats per drone to evaluate its throughput"
)
P.add_argument("n_drones", type=int, help="Number of simulated drones")
P.add_argument("results_dir", type=str, help="Results directory of reference")
P.add_argument(
    "--use_Mbps",
    action="store_true",
    default=False,
    help="Report throughput in Mbps. Default: kbps",
)
args = P.parse_args()

INPUT_CSV_PREFIX = "lte-UlMacStats-drone"
OUTPUT_CSV_PREFIX = "lte-throughput-drone"

drones = []

for i_drone in range(args.n_drones):
    drones.append([])
    txbytes = 0
    timeref = 0.0

    with open(f"{args.results_dir}/{INPUT_CSV_PREFIX}_{i_drone}.csv") as csvf:
        csvr = csv.reader(csvf)
        next(csvr)  # skip heading

        for r in csvr:
            stime = float(r[0])
            scellid = int(r[1])
            ssizetb = int(r[2])
            # drones[i_drone].append([time, cellid, sizetb])

            if int(stime) > timeref:
                exp = 2 if args.use_Mbps else 1  # wether to use kbps or Mbps
                drones[i_drone].append(
                    [int(timeref), scellid, txbytes * 8.0 / (1024.0**exp)]
                )
                timeref = stime
                txbytes = 0.0

            txbytes += ssizetb


for i_drone in range(len(drones)):
    with open(f"{args.results_dir}/{OUTPUT_CSV_PREFIX}_{i_drone}.csv", "w") as csvf:
        csvw = csv.writer(csvf)

        hdr_scale = "Mbps" if args.use_Mbps else "kbps"
        csvw.writerow(["time", "cellid", hdr_scale])  # header
        csvw.writerows(drones[i_drone])
