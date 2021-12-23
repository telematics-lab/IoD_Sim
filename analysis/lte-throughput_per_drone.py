import csv

base_results_dir = 'results/paper_2-2021-12-21.17-19-01'
n_drones = 5 # quanti droni?

drones = []

for i_drone in range(n_drones):
  drones.append([])
  txbytes = 0
  timeref = 0.0

  with open(f'{base_results_dir}/lte-DlMacStats-drone_{i_drone}.csv') as csvf:
    csvr = csv.reader(csvf)
    next(csvr) # skip heading

    for r in csvr:
      stime = float(r[0])
      scellid = int(r[1])
      ssizetb = int(r[2])
      #drones[i_drone].append([time, cellid, sizetb])

      if int(stime) > timeref:
        drones[i_drone].append([int(timeref), scellid, txbytes * 8.0 / 1024.0])
        timeref = stime
        txbytes = 0.0

      txbytes += ssizetb


for i_drone in range(len(drones)):
  with open(f'{base_results_dir}/lte-throughput-drone_{i_drone}.csv', 'w') as csvf:
    csvw = csv.writer(csvf)

    csvw.writerow(['time', 'cellid', 'Mbps']) # header
    csvw.writerows(drones[i_drone])