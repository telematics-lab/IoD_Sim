import csv

base_results_dir = 'results/paper_2-2021-12-21.17-19-01'
n_drones = 5 # quanti droni?

start = -1.0
cellid = 0

for i_drone in range(n_drones):
  rlcf = open(f'{base_results_dir}/lte-rlc-drone-{i_drone}.csv', 'r')
  rlc_csvr = csv.reader(rlcf)
  next(rlc_csvr) # discard header

  trf = open(f'{base_results_dir}/{i_drone}.csv', 'r')
  trf_csvr = csv.reader(trf)
  next(trf_csvr) # discard header

  destf = open(f'{base_results_dir}/traj+cellid-drone-{i_drone}.csv', 'w')
  dest_csvw = csv.writer(destf)
  dest_csvw.writerow(['x', 'y', 'z', 't', 'cellId'])

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
