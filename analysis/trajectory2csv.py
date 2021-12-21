import csv
from xml.etree.ElementTree import ElementTree

def pick_float_from_el(parent, el_name):
  v = parent.find(el_name)
  assert(v is not None)
  v = v.text
  assert(v is not None)
  return float(v)

# TODO: Argument parser to get summary_fp and destination_csv_dir from stdin
# TODO: main func
summary_fp = 'results/paper_2-2021-12-21.17-19-01/-summary.xml'
destination_csv_dir = 'results/paper_2-2021-12-21.17-19-01/'

tree = ElementTree()
tree.parse(summary_fp)

drones = tree.find("Drones")
assert(drones is not None)

i_drone = 0
for d in drones.iter('Drone'):
  with open(f'{destination_csv_dir}/{i_drone}.csv', 'w') as f:
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
