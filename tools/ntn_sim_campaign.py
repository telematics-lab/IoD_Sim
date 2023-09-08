#!/usr/bin/env python
import subprocess
from multiprocessing import Pool, cpu_count
from pathlib import Path

import jstyleson as json
import numpy as np

# CONFIGURATION
CONFIG_TEMPLATE_FILE = 'scenario/ntn-hap-static.json'
SCENARIO_PREFIX = 'auto-ntn_hap_static'
IODSIM_EXECUTABLE = 'ns3/build/examples/ns3-dev-iodsim-debug'
###

def exec_scenario(config_file):
  print(f'Running {config_file}...')
  subprocess.run(f'{IODSIM_EXECUTABLE} --config={config_file}', shell=True, check=True)


def main():
  with open(CONFIG_TEMPLATE_FILE, 'r') as f:
    config = json.load(f)

  # Create tmp directory for storing lots of temporary JSON files
  # TODO: use linux tmpfs
  config_path = Path('./tmp')
  try:
    config_path.mkdir()
  except FileExistsError:
    for cf in config_path.glob('*.json'):
      if cf.is_file():
        cf.unlink()

  for freq in np.arange(20e9, 100e9, 400e6):
    TO_GHz = 1e-9

    # Set a remarkable scenario name
    config['name'] = f'{SCENARIO_PREFIX}-{freq * TO_GHz:.2f}'

    # Edit channel model frequency
    phy_layers = config['phyLayer']
    for pl in phy_layers:
      loss_model_attrs = pl['channel']['propagationLossModel']['attributes']
      for lma in loss_model_attrs:
        if lma['name'] == 'Frequency':
          lma['value'] = freq
          break

    # Edit antenna frequency
    nodes = config['nodes']
    for n in nodes:
      net_devices = n['netDevices']
      for nd in net_devices:
        upa_attrs = nd['antenna']['attributes']
        for ua in upa_attrs:
          if ua['name'] == 'AntennaElement':
            antenna_attrs = ua['value']['attributes']
            for aa in antenna_attrs:
              if aa['name'] == 'OperatingFrequency':
                aa['value'] = freq
                break

    # Write to file
    with(open(f'tmp/{SCENARIO_PREFIX}-{freq * TO_GHz:.2f}.json', 'w')) as f:
      json.dump(config, f, indent=2, sort_keys=True)

  # Run simulation in parallel, according to the number of CPU cores
  with Pool(cpu_count()) as p:
    p.map(exec_scenario, config_path.glob('*.json'))


if __name__ == '__main__':
  main()