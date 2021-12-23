import csv

base_results_dir = 'results/paper_2-2021-12-21.17-19-01'
n_drones = 5 # quanti droni?

COL_TIME = 0
COL_CELLID = 1
COL_IMSI = 2
COL_SIZETB1 = 7

drones = []

with open(f'{base_results_dir}/lte-0-MacDlStats.txt', 'r') as csvf:
  csvr = csv.reader(csvf, delimiter='\t')
  next(csvr) # discard header

  for r in csvr:
    time = float(r[COL_TIME])
    cellid = int(r[COL_CELLID])
    droneid = int(r[COL_IMSI]) - 1
    sizetb = int(r[COL_SIZETB1])

    while len(drones) <= droneid:
      drones.append([])

    drones[droneid].append([time,cellid,sizetb])

for i_drone in range(len(drones)):
  with open(f'{base_results_dir}/lte-DlMacStats-drone_{i_drone}.csv', 'w') as csvf:
    csvw = csv.writer(csvf)

    csvw.writerow(['time', 'cellid', 'sizetb']) # heading
    csvw.writerows(drones[i_drone])
