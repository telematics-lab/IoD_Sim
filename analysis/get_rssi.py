#!/usr/bin/env python
import csv
from argparse import ArgumentParser
from pathlib import PurePath
from xml.etree import ElementTree as ET

def parse_args():
    P = ArgumentParser(description='Retrieve Wi-Fi RSSI measuments from '
                                   'summary XML file for a given drone and '
                                   'export them in CSV.')
    P.add_argument('summary_filepath',
                    type=str,
                    help='Input summary XML file of the scenario.')
    P.add_argument('entity_mac',
                    type=str,
                    help='Drone MAC Address.')
    return P.parse_args()

def default_output_path(args):
    base_path = PurePath(args.summary_filepath).parent
    output_prefix = 'rssi-drone'
    entity_mac_encoded = args.entity_mac.replace(":","")
    return f'{base_path}/{output_prefix}-{entity_mac_encoded}.csv'

def retrieve_netdevice(entity_xml, model_name):
    netdevice_collection = entity_xml.find('NetDevices').findall('NetDevice')
    for nd in netdevice_collection:
        nd_type = nd.find('type').text
        if nd_type == model_name:
            yield nd

def retrieve_rssi(netdevice_xml, entity_mac):
    rssi_collection = netdevice_xml.find('phy').find('signal').findall('rssi')
    for r in rssi_collection:
        if r.attrib['from'] == entity_mac:
            yield float(r.attrib['time']), float(r.attrib['value'])

def save_csv(path, header_row, content):
    with open(path, 'w') as outf:
        csvw = csv.writer(outf, lineterminator='\n')
        csvw.writerow(header_row)
        csvw.writerows(content)

def main():
    args = parse_args()

    xmlroot = ET.parse(args.summary_filepath).getroot()
    zsp = xmlroot.find('ZSPs').find('ZSP')

    # Search for the first (and possibly unique) ns3::WifiNetDevice
    nd = list(retrieve_netdevice(zsp, 'ns3::WifiNetDevice'))[0]
    rssi = retrieve_rssi(nd, args.entity_mac)

    save_csv(default_output_path(args), ['time', 'rssi'], rssi)

if __name__ == '__main__':
    main()