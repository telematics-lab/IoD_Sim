"""
Plot PDR/PLR data over distance for different simulations with
different datarate.
"""
import xml.etree.ElementTree as ET
import numpy as np
import plr
from collections import OrderedDict
import math
from matplotlib import pyplot as plt

def parse_args():
    """
    Parse program arguments, useful to behave like a full-fledged CLI program.

    Returns:
        A namespace with parsed arguments as its properties.
    """
    import argparse
    parser = argparse.ArgumentParser(description='Plot PDR/PLR data over '
                                     'distance for different simulations with '
                                     'different datarate.')

    parser.add_argument('report_files', type=str, nargs='+',
                        help='IoD Sim Scenario Reports.')

    return parser.parse_args()


def welcome_the_user(root):
    """
    Just welcome the user with the generics of this scenario.

    Args:
        root: Decoded XML DOM root
    """
    datetime = root.attrib['executedAt'].split('.')
    date = datetime[0]
    time = datetime[1].split('-')

    print('Analysing Scenario {} started in {} at {}:{}:{}'
        .format(root.attrib['scenario'], date, time[0], time[1], time[2]))


def get_entity_trajectory(xml_node, entity_identifier, entity_type):
    ret = {}

    for xml_pos in xml_node.findall('./position'):
        t = xml_pos.find('t').text
        ret[int(t)] = {
            'type': 'position',
            'entity': entity_identifier,
            'entity_type': entity_type,
            'x': float(xml_pos.find('x').text),
            'y': float(xml_pos.find('y').text),
            'z': float(xml_pos.find('z').text),
            't': int(t)
        }

    return ret

def parse_drone_trajectories(xml_drones):
    trajectories = {}

    for drone in xml_drones:
        trajectories.update(
            get_entity_trajectory(
                drone.find('./trajectory'),
                drone.find('./networkStacks/stack/ipv4/address').text,
                'drone'))

    return trajectories

def parse_zsp_positions(xml_zsps):
    positions = {}

    for zsp in xml_zsps:
        zsp_pos = get_entity_trajectory(zsp,
                                        zsp.find('./stack/ipv4/address').text,
                                        'zsp')

        # to resolve conflicts at 0ns, continuely add 1ns, which is not a critical offset
        for k, v in zsp_pos.items():
            while k in positions:
                k += 1

            positions[k] = v

    return positions

def get_entities_position(root):
    """
    Get the position of the entities simulated.

    Args:
        root: the XML DOM root
    """
    positions = {}

    # Update positions with the ZSP ones
    positions.update(parse_zsp_positions(root.findall('./zsps/zsp')))
    # then with drones
    positions.update(parse_drone_trajectories(root.findall('./drones/drone')))

    return positions


def organize_data(root, entity_path, data_container_name, is_rx=True, is_drone=False, multi_stack=False):
    """
    Generic data collection function useful for organize_rx_data and
    organize_tx_data.

    Args:
        root: Decoded XML DOM root
        entity_path: the XML path containing the entities to traverse.
        data_container_name: the XML path, relative to an entity, the contains
                             the packets to traverse.
        source_address: Indicate wether we should pick the SA or the RA as the
                        entity that is rx/tx packets.
        multi_stack: Indicate wether the simulated entity supports multiple
                     stacks or not.

    Returns:
        Multidimensional dictionary with all data traversed from each entity,
        organized by the IP address of the entity and each IP address that had
        communication with.
    """
    ms_path = 'networkStacks/' if multi_stack else ''
    entities = root.findall(entity_path)

    timeline = {}
    for e in entities:
        e_data = {}
        host = e.find(f'./{ms_path}stack/ipv4/address').text
        packets = e.findall(data_container_name)

        for pkt in packets:
            time = int(pkt.find('./time').text)

            if time in e_data:
                print("dup!")

            e_data[time] = {
                # pick only what is needed
                'type': 'packet',
                'entity': host,
                'entity_type': 'drone' if is_drone else 'zsp',
                'packet_type': 'rx' if is_rx else 'tx',
                'direction': pkt.find('./direction').text,
                'src_addr': pkt.find('./sourceAddress').text,
                'dest_addr': pkt.find('./destinationAddress').text
            }

        timeline.update(e_data)

    return timeline


def organize_rx_data(root):
    """
    Organize Data received from each simulated entity as a multidimensional,
    easily manageable dictionary.

    Args:
        root: Decoded XML DOM root.

    Returns:
        Multidimensional dictionary with all data received from each entity,
        organized by Sender IP address.
    """
    rx_data = {}

    zsps = organize_data(root, './zsps/zsp', './dataRx/transfer', is_rx=True, is_drone=False, multi_stack=False)
    drones = organize_data(root, './drones/drone', './dataRx/transfer', is_rx=True, is_drone=True, multi_stack=True)

    rx_data.update(zsps)
    rx_data.update(drones)

    return rx_data


def organize_tx_data(root):
    """
    Organize Data transmitted from each simulated entity as a multidimensional,
    easily manageable dictionary.

    Args:
        root: Decoded XML DOM root.

    Returns:
        Multidimensional dictionary with all data transmitted from each entity,
        organized by Target IP Address.
    """
    tx_data = {}

    zsps = organize_data(root, './zsps/zsp', './dataTx/transfer', is_rx=False, is_drone=False, multi_stack=False)
    drones = organize_data(root, './drones/drone', './dataTx/transfer', is_rx=False, is_drone=True, multi_stack=True)

    tx_data.update(zsps)
    tx_data.update(drones)

    return tx_data


def gd(a, b):
    """
    Compute geometric distance between 3D points.
    """
    return math.sqrt((a['x'] + b['x']) ** 2
                     + (a['y'] + b['y']) ** 2
                     + (a['z'] + b['z']) ** 2)


def analyse(files):
    """
    Start the elaboration of PLR over distance from different scenario outputs.

    Args:
        files: file paths of the scenario reports the be opened and decoded.
    """
    xmls = [ET.parse(fp) for fp in files]
    roots = [x.getroot() for x in xmls]

    # gather data
    timelines = []
    for r in roots:
        timeline = {}

        welcome_the_user(r)

        timeline.update(get_entities_position(r))
        timeline.update(organize_rx_data(r))
        timeline.update(organize_tx_data(r))

        timelines.append(OrderedDict(sorted(timeline.items())))

    # elaborate them
    print("Events per scenario:", [len(t.keys()) for t in timelines])

    for t in timelines:
        zsp_pos = [p
                   for _,p in t.items()
                   if p['type'] == 'position' and p['entity_type'] == 'zsp']

        pkt_dist_stat = {}
        pkt_stat = {'rx': {}, 'tx': {}}

        for _, event in t.items():
            if event['type'] == 'position':
                # log distance with the closest ZSP
                if event['entity'] not in pkt_dist_stat:
                    pkt_dist_stat[event['entity']] = []

                # in case of key missing, we do not have enough data
                try:
                    pdrs = []
                    senders = pkt_stat['rx'][event['entity']]
                    for sender, pkts_rx in senders.items():
                        pkts_tx = pkt_stat['tx'][sender][event['entity']]
                        pdrs.append(float(pkts_rx) * 100.0 / float(pkts_tx))

                    pkt_dist_stat[event['entity']].append(
                        (min([gd(event, zp) for zp in zsp_pos]), pdrs))
                except KeyError:
                    continue
            elif event['type'] == 'packet':
                if event['entity'] not in pkt_stat[event['packet_type']]:
                    pkt_stat[event['packet_type']][event['entity']] = {}

                # could be either tx or rx
                addr_key = 'src_addr' if event['packet_type'] == 'rx' else 'dest_addr'
                if event[addr_key] not in pkt_stat[event['packet_type']][event['entity']]:
                    pkt_stat[event['packet_type']][event['entity']][event[addr_key]] = 0

                pkt_stat[event['packet_type']][event['entity']][event[addr_key]] += 1

        # we can now show them
        plt.figure()
        plt.title('PDR of Drones over their distance from ZSP')

        t_pkts = {}
        for host, data in pkt_dist_stat.items():
            x = []
            y = []

            for i in data:
                if len(x) > 0 and i[0] == x[-1]:
                    continue

                x.append(i[0])
                y.append(i[1][0])

            t_pkts[host] = {
                'x': x,
                'y': y
            }

            print(len(t_pkts[host]['x']), len(set(t_pkts[host]['x'])))

            plt.plot(t_pkts[host]['x'], t_pkts[host]['y'])
            plt.xlabel('Distance from ZSP [m]')
            plt.ylabel('PDR [%]')

        print(len(pkt_dist_stat.items()))

        plt.show()



def main():
    """
    Main control flow, called if this program is executed directly.
    """
    args = parse_args()
    analyse(args.report_files)


if __name__ == '__main__':
    main()
