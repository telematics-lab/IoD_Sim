"""
Plot PLR over distance of a drone from its ZSP.
"""

import math
import xml.etree.ElementTree as ET
from collections import OrderedDict

import numpy as np
import plr
from matplotlib import pyplot as plt


def parse_args():
    import argparse

    parser = argparse.ArgumentParser(
        description="Plot PDR/PLR data over "
        "distance for different simulations with "
        "different datarate."
    )

    parser.add_argument(
        "report_files", type=str, nargs="+", help="IoD Sim Scenario Reports."
    )

    return parser.parse_args()


def main():
    args = parse_args()

    folder = args.report_files[0]

    xml = ET.parse(folder)
    root = xml.getroot()

    # what is the address of the ZSP?
    zsp_addr = root.find("./zsps/zsp/stack/ipv4/address").text

    # what is the position of the ZSP?
    xml_zsp_pos = root.find("./zsps/zsp/position")
    zsp_pos = {
        "x": float(xml_zsp_pos.find("x").text),
        "y": float(xml_zsp_pos.find("y").text),
        "z": float(xml_zsp_pos.find("z").text),
    }

    xml_drones = root.findall("./drones/drone")
    drones = {}
    for xml_drone in xml_drones:
        # what drone is it?
        drone_host = xml_drone.find("./networkStacks/stack/ipv4/address").text

        # what are the position assumed by the Drone (unique pos.)? At what time?
        xml_pos = xml_drone.findall("./trajectory/position")
        pos = {}
        for xp in xml_pos:
            x = float(xp.find("x").text)
            y = float(xp.find("y").text)
            z = float(xp.find("z").text)
            t = int(xp.find("t").text)

            pos[t] = {"x": x, "y": y, "z": z}

        # what are the packets tx by the drone to the ZSP? At what time?
        xml_pkts = xml_drone.findall("./dataTx/transfer")
        tx = {}
        for xp in xml_pkts:
            t = int(xp.find("./time").text)
            src_addr = xp.find("./sourceAddress").text
            dst_addr = xp.find("./destinationAddress").text

            if dst_addr != zsp_addr:
                continue

            tx[t] = {"src": src_addr, "dst": dst_addr}

        # what are the packets rx by the ZSP from the drone? At what time?
        xml_pkts = root.findall("./zsps/zsp/dataRx/transfer")
        rx = {}
        for xp in xml_pkts:
            t = int(xp.find("./time").text)
            src_addr = xp.find("./sourceAddress").text
            dst_addr = xp.find("./destinationAddress").text

            if src_addr != drone_host:
                continue

            rx[t] = {"src": src_addr, "dst": dst_addr}

        # add new data to dictionary
        drones[drone_host] = {"pos": pos, "tx": tx, "rx": rx}

    # data acquired.

    for host, drone in drones.items():
        # plt.figure()
        # plt.title(f'Drone {host}')

        x = []
        y = []

        tx = 1.0
        rx = 1.0

        for post, posp in drone["pos"].items():
            dist = gd(posp, zsp_pos)

            for pktt, _ in drone["tx"].items():
                if pktt > post:
                    break

                tx += 1.0

            for pktt, _ in drone["rx"].items():
                if pktt > post:
                    break

                rx += 1.0

            x.append(dist)
            # y.append(rx * 100.0 / tx)
            y.append(tx - rx)

        # NEED TO ORDER AND FILTER OUT DUPLICATES OF X AND Y, ELSE THEY CANNOT BE PLOTTED!

        plt.figure()

        x_uniq = np.unique(x)
        y_filt = []
        for xi in x_uniq:
            v = [y[i] for i in range(len(x)) if xi == x[i]]
            y_filt.append(np.mean(v))

        plt.plot(x_uniq, y_filt)
        # plt.hist(y, log=True)
        # plt.scatter(x, y)
        plt.show()


def gd(a, b):
    """
    Compute geometric distance between 3D points.
    """
    return math.sqrt(
        (a["x"] + b["x"]) ** 2 + (a["y"] + b["y"]) ** 2 + (a["z"] + b["z"]) ** 2
    )


if __name__ == "__main__":
    main()
