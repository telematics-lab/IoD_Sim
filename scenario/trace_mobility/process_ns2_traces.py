import sys
import re
import argparse
import tarfile
import gzip
import io

from pyproj import Transformer

RE_SET = re.compile(r'^\$node_\((\d+)\) set ([XYZ])_ ([\d.eE+\-]+)')
RE_SETDEST = re.compile(
    r'^\$ns_ at ([\d.eE+\-]+) "\$node_\((\d+)\) setdest ([\d.eE+\-]+) ([\d.eE+\-]+) ([\d.eE+\-]+)"'
)

def parse_location_tag(text):
    """Extract the SUMO <location .../> attributes from a chunk of text.

    Returns netOffset (x, y), convBoundary, origBoundary
    (each a tuple of floats) and projParameter (str). Missing attributes are
    returned as None.
    """
    m = re.search(r'<location\b[^>]*>', text)
    if not m:
        raise ValueError("No <location ...> tag found.")
    tag = m.group(0)

    def attr(name):
        a = re.search(name + r'\s*=\s*"([^"]*)"', tag)
        return a.group(1) if a else None

    def floats(s):
        return tuple(float(v) for v in s.split(",")) if s else None

    return {
        "netOffset": floats(attr("netOffset")),
        "convBoundary": floats(attr("convBoundary")),
        "origBoundary": floats(attr("origBoundary")),
        "projParameter": attr("projParameter"),
    }

def load_location(arg):
    """Interpret --location: a raw tag string, or a path to a file with one."""
    text = arg
    # If it doesn't look like a tag, treat it as a file path.
    if "<location" not in arg:
        with open(arg, "r") as f:
            text = f.read()
    return parse_location_tag(text)

def make_unprojector(proj_param):
    """Return a function (proj_x, proj_y) -> (lat, lon) for the given projParameter."""
    transformer = Transformer.from_crs(proj_param, "EPSG:4326", always_xy=True)

    def unproject(px, py):
        lon, lat = transformer.transform(px, py)
        return lat, lon

    return unproject

def main():
    parser = argparse.ArgumentParser(
        description="Convert NS2 mobility .tcl file to IoD_Sim .trace format."
    )
    parser.add_argument("input_file", help="Input .tcl file")

    parser.add_argument(
        "--net-file",
        default=None,
        help="SUMO .net.xml file; its <location> tag is used for geo-referencing.",
    )
    parser.add_argument(
        "--location",
        default=None,
        help='SUMO <location .../> tag, given directly as a string or as a path to a '
             'file containing it. Alternative to --net-file.',
    )
    args = parser.parse_args()

    if bool(args.net_file) == bool(args.location):
        print("Error: provide exactly one of --net-file / --location.")
        sys.exit(1)

    try:
        if args.net_file:
            with open(args.net_file, "r") as f:
                location = parse_location_tag(f.read())
        else:
            location = load_location(args.location)
    except (OSError, ValueError) as e:
        print(f"Error reading location: {e}")
        sys.exit(1)

    if location["netOffset"] is None or location["projParameter"] is None:
        print("Error: <location> tag is missing netOffset or projParameter.")
        sys.exit(1)

    unproject = make_unprojector(location["projParameter"])
    off_x, off_y = location["netOffset"]

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

    def get_coords(w):
        proj_x = w["x"] - off_x
        proj_y = w["y"] - off_y
        return unproject(proj_x, proj_y)

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
    print(f"  netOffset: ({off_x}, {off_y})")
    print(f"  projParameter: {location['projParameter']}")


if __name__ == "__main__":
    main()