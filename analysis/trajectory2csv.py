#!/usr/bin/env python
import csv
from argparse import ArgumentParser
from xml.etree.ElementTree import ElementTree

P = ArgumentParser(description='Parse drone trajectories from summary file')
P.add_argument('summary_filepath', type=str, help='Summary file path')
P.add_argument('results_dir', type=str, help='Destination directory where to store CSV files')
args = P.parse_args()

OUTPUT_FILE_PREFIX = 'trajectory-drone'

def pick_float_from_el(parent, el_name):
  v = parent.find(el_name)
  assert(v is not None)
  v = v.text
  assert(v is not None)
  return float(v)

tree = ElementTree()
tree.parse(args.summary_filepath)

drones = tree.find("Drones")
assert(drones is not None)

i_drone = 1
for d in drones.iter('Drone'):
  with open(f'{args.results_dir}/{OUTPUT_FILE_PREFIX}_{i_drone}.csv', 'w') as f:
    csvw = csv.writer(f)

    csvw.writerow(['x','y','z','t']) # write headings

    trajectory = d.find('trajectory')
    assert(trajectory is not None)

    for p in trajectory.iter('position'):
      x = pick_float_from_el(p, 'x')
      y = pick_float_from_el(p, 'y')
      z = pick_float_from_el(p, 'z')
      t = pick_float_from_el(p, 't') / 1e9
      csvw.writerow([x,y,z,t])

  i_drone += 1
