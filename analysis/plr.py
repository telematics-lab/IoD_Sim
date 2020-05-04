"""
Calculate and plot the PLR of given simulation results.
"""
import xml.etree.ElementTree as ET
import matplotlib.pyplot as plt
import numpy as np


def parse_args():
    """
    Parse program arguments, useful to behave like a full-fledged CLI program.

    Returns:
        A namespace with parsed arguments as its properties.
    """
    import argparse
    parser = argparse.ArgumentParser(description='Calculate and plot the PLR '
                                     'of given simulation results.')

    parser.add_argument('summary_file', type=str,
                        help='IoD Sim Scenario Report.')

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


def organize_data(root, entity_path, data_container_name, source_address, multi_stack=False):
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
    addr = 'sourceAddress' if source_address else 'destinationAddress'
    entities = root.findall(entity_path)

    entities_data = {}
    for e in entities:
        e_data = {}
        ipv4 = e.find(f'./{ms_path}stack/ipv4/address').text
        packets = e.findall(data_container_name)

        for pkt in packets:
            sara = pkt.find(f'./{addr}').text

            if sara not in e_data:
                e_data[sara] = 0

            # just count them
            e_data[sara] += 1

        entities_data[ipv4] = e_data

    return entities_data


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
    zsps = organize_data(root, './zsps/zsp', './dataRx/transfer', True)
    drones = organize_data(root, './drones/drone', './dataRx/transfer', True, True)

    zsps.update(drones)  # merge
    return zsps


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
    zsps = organize_data(root, './zsps/zsp', './dataTx/transfer', False)
    drones = organize_data(root, './drones/drone', './dataTx/transfer', False, True)

    zsps.update(drones)  # merge
    return zsps


def analyse(filepath):
    """
    Start the elaboration of PLR from a scenario output.

    Args:
        filepath: file path of the scenario report the be opened and decoded.
    """
    xml = ET.parse(filepath)
    root = xml.getroot()

    welcome_the_user(root)
    rx = organize_rx_data(root)
    tx = organize_tx_data(root)

    stats = []
    for entity, senders in rx.items():
        for sender, pkts_rx in senders.items():
            pkts_tx = tx[sender][entity]
            pdr = float(pkts_rx) * 100.0 / float(pkts_tx)

            stats.append({
                'from': entity,
                'to': sender,
                'pdr': pdr,
                'plr': 100.0 - pdr
            })

    # plot stats
    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.set_title('PDR and PLR for each communication flux.')
    ax.set_xlabel('Host')
    ax.set_ylabel('[%]')

    width = 0.27  # width of the bars
    ind = np.arange(len(stats))  # x location of the groups

    y = [x['pdr'] for x in stats]
    ax.bar(ind, y, width, color='g')

    y = [x['plr'] for x in stats]
    ax.bar(ind + width, y, width, color='r')

    ax.set_xticks(ind + width, minor=False)
    ax.set_xticklabels([f"{i['from']} -> {i['to']}" for i in stats], fontdict=None, minor=False)

    plt.show()


def main():
    """
    Main control flow, called if this program is executed directly.
    """
    args = parse_args()
    analyse(args.summary_file)


if __name__ == '__main__':
    main()
