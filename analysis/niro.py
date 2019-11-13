import xml.etree.ElementTree as ET
import matplotlib
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np

def parse_args():
    """
    Process arguments passed from the shell at the start of the program.
    """
    import argparse
    parser = argparse.ArgumentParser(description='Niro Analysis Tool')

    parser.add_argument('summary_file',
                        type=str,
                        help='XML Summary File of the Scenario')

    return parser.parse_args()

class Position:
    def __init__(self, xml_pos):
        self.x = float(xml_pos.find('x').text)
        self.y = float(xml_pos.find('y').text)
        self.z = float(xml_pos.find('z').text)

    def __repr__(self):
        return '<Entity pos ({},{},{})>'.format(self.x, self.y, self.z)


class PositionWithTime(Position):
    def __init__(self, xml_pos):
        self.t = int(xml_pos.find('t').text)
        super().__init__(xml_pos)

    def __repr__(self):
        return '<Entity '\
               'pos ({},{},{}) @ {}ns>'.format(self.x, self.y,
                                               self.z, self.t)


def get_entity_trajectory(xml_node):
    return [PositionWithTime(xml_pos)
            for xml_ent in xml_node
            for xml_pos in xml_ent.findall('./position')]


def parse_drone_trajectories(xml_drones):
    return [get_entity_trajectory(drone.findall('./trajectory'))
            for drone in xml_drones]


def generate_position_graph(root):
    # Parse ZSP positions
    zsps_position = get_entity_trajectory(root.findall('./zsps/zsp'))
    drone_trajectories = parse_drone_trajectories(root.findall('./drones/drone'))

    # Generate space graph
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.set_title('Drone Trajectories and ZSP positions')
    ax.set_xlabel('[m]')
    ax.set_ylabel('[m]')
    ax.set_zlabel('[m]')

    # Plot ZSPs as a serie of "+"
    zsps_x = [zsp.x for zsp in zsps_position]
    zsps_y = [zsp.y for zsp in zsps_position]
    zsps_z = [zsp.z for zsp in zsps_position]
    ax.scatter(zsps_x, zsps_y, zsps_z, marker='+', label='ZSP')

    # drone index
    cmap = matplotlib.cm.get_cmap('Set1')
    i = 1

    # Plot drones trajectories as 3D lines
    # a "o" indicates the start pos. of the drone
    # a "x" indicates the end pos. of the drone
    for dt in drone_trajectories:
        dx = [p.x for p in dt]
        dy = [p.y for p in dt]
        dz = [p.z for p in dt]

        color = cmap(i)

        ax.scatter(dx[0], dy[0], dz[0], marker='.', c=color, label='Drone start')
        ax.scatter(dx[-1], dy[-1], dz[-1], marker='x', c=color, label='Drone end')
        ax.plot(dx, dy, dz, c=color, label='Drone trajectory')

        i += 1

    ax.legend()


def generate_rssi_graph(root):
    drones_rssi = root.findall('./drones/drone/networkStacks/stack/phy/signal')

    # Generate graph
    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.set_title('RSSI over time')
    ax.set_xlabel('Time [s]')
    ax.set_ylabel('RSSI [dBm]')

    for drone_rssi in drones_rssi:
        entries = []
        senders = []

        for s in drone_rssi.findall('./rssi'):
            sender = s.attrib['from']
            if sender not in senders:
                senders.append(sender)

            entries.append({
                'value': float(s.attrib['value']),
                'time':  float(s.attrib['time']) * 0.000000001,
                'from':  s.attrib['from']
            })

        # discriminate information fluxes by sender address
        for sender in senders:
            flux = [e for e in entries if e['from'] == sender]
            time = [e['time'] for e in flux]
            rssi = [e['value'] for e in flux]

            ax.plot(time, rssi)


def welcome_the_user(root):
    datetime = root.attrib['executedAt'].split('.')
    date = datetime[0]
    time = datetime[1].split('-')

    print('Analysing Scenario {} started in {} at {}:{}:{}'
        .format(root.attrib['scenario'], date, time[0], time[1], time[2]))


def analyse(filepath):
    """
    Start the analysis process of a scenario output.
    """
    xml = ET.parse(filepath)
    root = xml.getroot()

    welcome_the_user(root)
    generate_position_graph(root)
    generate_rssi_graph(root)

    plt.show()


def main():
    args = parse_args()
    analyse(args.summary_file)


if __name__ == "__main__":
    main()