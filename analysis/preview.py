"""
A preview of a given simulation configuration in terms of drone trajectories and
ZSPs position.
"""

import jstyleson
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d import Axes3D
from scipy.special import binom


class Point:
    """
    A high-level construct to manipulate and debug 3D points easily.
    """

    def __init__(self, arr=[0, 0, 0], interest_level=0):
        """
        Default constructor.

        Args:
            arr: Array of 3D coordinates, x, y, and z respectively.
            interest_level: Optional interest level.
        """
        self.x = arr[0]
        self.y = arr[1]
        self.z = arr[2]
        self.interest = interest_level

    def __repr__(self):
        """
        Debug information contained in this structure.

        Returns:
            A construct that summarizes the properties in space of this
            Point instance.
        """
        return f"Point(x:{self.x}; y:{self.y}; z:{self.z}; int:{self.interest})"


def parse_args():
    """
    Parse program arguments, useful to behave like a full-fledged CLI program.

    Returns:
        A namespace with parsed arguments as its properties.
    """
    import argparse

    parser = argparse.ArgumentParser(
        description="Preview a given simulation "
        "in terms of drone trajectories and ZSPs "
        "position."
    )
    parser.add_argument(
        "--pdf",
        help="Plot the figure in a pdf file" "(default: display the plot on a GUI)",
        action="store_true",
    )

    parser.add_argument("input_file", type=str, help="IoD Sim Scenario Configuration.")

    return parser.parse_args()


def get_configuration(filepath):
    """
    Parse JSON configuration.

    Args:
        filepath: Path to the scenario configuration file.

    Returns:
        A dictionary containing decoded JSON data.
    """
    with open(filepath, "r") as f:
        jdoc = jstyleson.loads(f.read())

    return jdoc


def parse_positions(jd):
    """
    Parse ZSPs and Drones positions.
    Beware that a trajectory needs to be built and modelled using Bézier Curve.

    Args:
        jd: JSON Document that describes the scenario to be simulated.

    Returns:
        Three arrays containing ZSP positions, Drones positions and Drone trajectories curve step.
    """
    # ZSPs are the easiest
    zsps = jd["ZSPs"]
    zsp_positions = []

    for zsp in zsps:
        mobilityModel = zsp["mobilityModel"]["attributes"]
        if len(mobilityModel) > 0:
            for attributes in mobilityModel:
                if attributes["name"] == "Position":
                    zsp_positions.append(Point(attributes["value"]))
        else:
            # support for not defined ZSPs Position
            zsp_positions.append(Point([0.0, 0.0, 0.0]))

    # get drones positions
    drones = jd["drones"]

    drones_trajectories = []
    drones_curvestep = []
    flag = 0
    for drone in drones:
        mobilityModel = drone["mobilityModel"]["attributes"]
        trajectory = []
        for attributes in mobilityModel:
            if attributes["name"] == "FlightPlan":
                for point in attributes["value"]:
                    trajectory.append(Point(point["position"], point["interest"]))
                drones_trajectories.append(trajectory)
            if attributes["name"] == "CurveStep":
                flag = 1
                drones_curvestep.append(attributes["value"])
        # Support for not defined CurveStep
        if flag != 1:
            drones_curvestep.append(0.01)

    return zsp_positions, drones_trajectories, drones_curvestep


def build_zsp_space(ax, zsps):
    """
    Show ZSP coordinates in 3D plot.

    Args:
        ax: matplotlib initialised plot area.
        zsps: ZSP positions.
    """
    # Plot ZSPs as a serie of "+"
    zsps_x = [zsp.x for zsp in zsps]
    zsps_y = [zsp.y for zsp in zsps]
    zsps_z = [zsp.z for zsp in zsps]
    ax.scatter(zsps_x, zsps_y, zsps_z, marker="+", label="ZSP")


def build_drone_trajectory(drone_positions, step=0.01):
    """
    Build drone trajectory using Bézier Curve model.

    Args:
        drone_positions: Positions of the drone assumed, as an array of Point(s).
        step: Optional step for detailed curve generation.

    Returns:
        Modelled trajectory as an array of Point(s).
    """
    # Interest = 0 => start/stop of a given trajectory, cut the Bézier Curve.
    trajectories = []
    trajectory = []

    # populate trajectories
    for pos in drone_positions:
        trajectory.append(pos)

        if pos.interest == 0 and len(trajectory) > 1:
            trajectories.append(trajectory)
            # clean up
            trajectory = [pos]

    # tolerance in case the user forgot to add the last point with interest = 0
    if len(trajectory) > 1:
        trajectories.append(trajectory)

    # construct trajectory
    B = []  # Bezier curve, our trajectory

    for tr in trajectories:
        for t in np.arange(0, 1, step):
            sum_x = 0.0
            sum_y = 0.0
            sum_z = 0.0
            n = len(tr)

            for i in range(n):
                a = binom(n, i)
                b = (1 - t) ** (n - i)
                c = t**i

                sum_x += a * b * c * tr[i].x
                sum_y += a * b * c * tr[i].y
                sum_z += a * b * c * tr[i].z

            B.append(Point([sum_x, sum_y, sum_z]))

    return B


def build_drone_space(ax, drones):
    """
    Show Drone trajectories in 3D plot.

    Args:
        ax: matplotlib initialised plot area.
        drones: Drone positions.
    """
    # drone index
    cmap = matplotlib.cm.get_cmap("Set1")
    i = 1

    # Plot drones trajectories as 3D lines
    # a "o" indicates the start pos. of the drone
    # a "x" indicates the end pos. of the drone
    for dt in drones:
        dx = [p.x for p in dt]
        dy = [p.y for p in dt]
        dz = [p.z for p in dt]

        color = cmap(i)

        ax.scatter(dx[0], dy[0], dz[0], marker=".", c=color, label="Drone start")
        ax.scatter(dx[-1], dy[-1], dz[-1], marker="x", c=color, label="Drone end")
        ax.plot(dx, dy, dz, c=color, label="Drone trajectory")

        i += 1


def setup_3dplot():
    """
    Initialize 3D space plot.

    Returns:
        Matplotlib axes.
    """
    fig = plt.figure()
    ax = fig.add_subplot(111, projection="3d")
    ax.set_title("Drone Trajectories and ZSP positions")
    ax.set_xlabel("[m]")
    ax.set_ylabel("[m]")
    ax.set_zlabel("[m]")

    return ax


def main():
    """
    Program entry point if called directly.
    """
    args = parse_args()
    jdoc = get_configuration(args.input_file)

    zsps, drones, step = parse_positions(jdoc)

    ax = setup_3dplot()
    build_zsp_space(ax, zsps)

    trajectories = []
    for i, drone in enumerate(drones):
        trajectories.append(build_drone_trajectory(drone, step[i]))
    build_drone_space(ax, trajectories)

    ax.legend()
    if args.pdf:
        # For environment without graphic interface (Output is a .pdf file):
        plt.savefig(args.input_file.replace(".json", ".pdf"))
    else:
        # For environment with graphic interface:
        plt.show()


if __name__ == "__main__":
    main()
