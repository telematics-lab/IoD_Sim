import sys
import re
import math
import argparse
import tarfile
import gzip
import io

"""
This script parses the NS2 mobility format (.tcl / .ns2mobility.tcl):

  $node_(N) set X_ value
  $node_(N) set Y_ value
  $node_(N) set Z_ value
  $ns_ at TIME "$node_(N) setdest X Y SPEED"

Notes:
  - TIME is in seconds (float); converted to milliseconds in the output.
  - The third argument of setdest is SPEED (m/s), not altitude.
  - Altitude is taken from the initial "set Z_" declaration and kept
    constant for all waypoints of that node (NS2 mobility is 2D).
  - X and Y are simulation-space coordinates in meters. If --lat/--lon are
    provided, they are converted to geographic coordinates using a flat-Earth
    approximation: the given lat/lon is treated as the geographic position of
    the center of the Cartesian bounding box.

The output is an "output.trace" tar file containing:

-- nodes.csv.gz
   Format: id_device;relative_first_time_ms;latitude;longitude;altitude
   One entry per node, describing the first waypoint.

-- traces.csv.gz
   Format: node;relative_time_ms;latitude;longitude;altitude
   All waypoints sorted by time.

Both CSVs have no header.
"""

RE_SET = re.compile(r'^\$node_\((\d+)\) set ([XYZ])_ ([\d.eE+\-]+)')
RE_SETDEST = re.compile(
    r'^\$ns_ at ([\d.eE+\-]+) "\$node_\((\d+)\) setdest ([\d.eE+\-]+) ([\d.eE+\-]+) ([\d.eE+\-]+)"'
)

METERS_PER_DEGREE_LAT = 111320.0


def cartesian_to_geo(x, y, center_x, center_y, ref_lat, ref_lon):
    """Convert Cartesian (meters) to geographic coordinates.

    The center of the Cartesian bounding box maps to (ref_lat, ref_lon).
    X grows east, Y grows north.
    """
    delta_x = x - center_x
    delta_y = y - center_y
    lat = ref_lat + delta_y / METERS_PER_DEGREE_LAT
    lon = ref_lon + delta_x / (METERS_PER_DEGREE_LAT * math.cos(math.radians(ref_lat)))
    return lat, lon


def main():
    parser = argparse.ArgumentParser(
        description="Convert NS2 mobility .tcl file to IoD_Sim .trace format."
    )
    parser.add_argument("input_file", help="Input .tcl file")
    parser.add_argument(
        "--lat",
        type=float,
        default=None,
        help="Latitude of the center of the Cartesian coordinate space",
    )
    parser.add_argument(
        "--lon",
        type=float,
        default=None,
        help="Longitude of the center of the Cartesian coordinate space",
    )
    args = parser.parse_args()

    if (args.lat is None) != (args.lon is None):
        print("Error: --lat and --lon must be provided together.")
        sys.exit(1)

    convert = args.lat is not None

    node_init = {}  # node_id -> {'X': float, 'Y': float, 'Z': float}
    waypoints = []  # list of dicts: node, time_ms, x, y

    try:
        with open(args.input_file, "r") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue

                m = RE_SET.match(line)
                if m:
                    nid = int(m.group(1))
                    coord = m.group(2)
                    val = float(m.group(3))
                    if nid not in node_init:
                        node_init[nid] = {}
                    node_init[nid][coord] = val
                    continue

                m = RE_SETDEST.match(line)
                if m:
                    time_ms = int(float(m.group(1)) * 1000)
                    nid = int(m.group(2))
                    x = float(m.group(3))
                    y = float(m.group(4))
                    # m.group(5) is speed — not used in the trace output
                    waypoints.append({"node": nid, "time_ms": time_ms, "x": x, "y": y})

    except FileNotFoundError:
        print(f"Error: File '{args.input_file}' not found.")
        sys.exit(1)

    if not waypoints:
        print("No valid waypoints found.")
        sys.exit(1)

    waypoints.sort(key=lambda w: w["time_ms"])

    if convert:
        all_x = [w["x"] for w in waypoints]
        all_y = [w["y"] for w in waypoints]
        center_x = (min(all_x) + max(all_x)) / 2
        center_y = (min(all_y) + max(all_y)) / 2

    def get_coords(w):
        if convert:
            lat, lon = cartesian_to_geo(w["x"], w["y"], center_x, center_y, args.lat, args.lon)
            return lat, lon
        return w["x"], w["y"]

    # nodes.csv: one entry per node at its first appearance
    node_first = {}
    for w in waypoints:
        nid = w["node"]
        if nid not in node_first:
            node_first[nid] = w

    nodes_lines = []
    for nid in sorted(node_first.keys()):
        w = node_first[nid]
        z = node_init.get(nid, {}).get("Z", 0.0)
        lat, lon = get_coords(w)
        nodes_lines.append(f"{nid};{w['time_ms']};{lat};{lon};{z}")

    # traces.csv: all waypoints
    traces_lines = []
    for w in waypoints:
        z = node_init.get(w["node"], {}).get("Z", 0.0)
        lat, lon = get_coords(w)
        traces_lines.append(f"{w['node']};{w['time_ms']};{lat};{lon};{z}")

    nodes_content = "\n".join(nodes_lines)
    traces_content = "\n".join(traces_lines)

    output_filename = "output.trace"
    with tarfile.open(output_filename, "w") as tar:
        nodes_gz = gzip.compress(nodes_content.encode("utf-8"))
        nodes_info = tarfile.TarInfo(name="nodes.csv.gz")
        nodes_info.size = len(nodes_gz)
        tar.addfile(nodes_info, io.BytesIO(nodes_gz))

        traces_gz = gzip.compress(traces_content.encode("utf-8"))
        traces_info = tarfile.TarInfo(name="traces.csv.gz")
        traces_info.size = len(traces_gz)
        tar.addfile(traces_info, io.BytesIO(traces_gz))

    print(f"Successfully created '{output_filename}'")
    print(f"  Nodes: {len(node_first)}, Waypoints: {len(waypoints)}")
    if convert:
        print(f"  Cartesian center: ({center_x:.2f}, {center_y:.2f}) m")
        print(f"  Mapped to: ({args.lat}, {args.lon})")


if __name__ == "__main__":
    main()
