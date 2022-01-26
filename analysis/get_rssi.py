#!/usr/bin/env python
import csv
from argparse import ArgumentParser
from pathlib import PurePath
from xml.etree import ElementTree as ET

# TODO -- this script does not work, yet

P = ArgumentParser(description='Retrieve RSSI measurements from summary XML file and export them in separate CSVs, '
                               'one per drone.')
P.add_argument('summary_filepath', type=str, help='Input summary XML file of the scenario.')
args = P.parse_args()

base_path = PurePath(args.summary_filepath).parent
output_prefix = 'rssi-drone'

xmlroot = ET.parse(args.summary_filepath).getroot()
zsp_addr = xmlroot.find('ZSPs').find('ZSP')
print(zsp_addr)
zsp_addr = zsp_addr.find('NetDevices').find('NetDevice').find('phy').find('address').text

drones = xmlroot.find('Drones').findall('Drone')
i_drone = 0
for d in drones:
  for nd in d.find('NetDevices').findall('NetDevice'):
    dtype = nd.find('type').text
    if (dtype == 'ns3::WifiNetDevice'):
      with open(f'{base_path}/{output_prefix}_{i_drone}.csv', 'w') as outf:
        csvw = csv.writer(outf, lineterminator='\n')
        csvw.writerow(['time', 'rssi'])

        for s in nd.find('signal').findall('rssi'):
          attrs = s.attrib
          csvw.writerow([attrs['time'], attrs['value']])

  i_drone += 1
