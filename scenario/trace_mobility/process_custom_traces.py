import sys
import tarfile
import gzip
import io

"""
This script parses a CSV file with the following format:
node_id;relative_time;lat;long;alt

The output is a "gz.tar" file that contains 2 files:
-- nodes.csv.gz
Format: id_devce;relative_first_time;first_latitude;first_longitude;first_altitude
Used to define how many devices exist and when/where they start.

-- traces.csv.gz
Format: node;relative_time;latitude;longitude;altitude
Used to define how the position of each device evolves over time, sorted by relative_time.

All CSVs have no header.
"""


def parse_line(line):
    """Parses a single line of the input file."""
    # Format: node_id;relative_time;lat;long;alt
    parts = line.strip().split(";")
    if len(parts) != 5:
        return None

    try:
        node_id = int(parts[0])
        relative_time = int(parts[1])
        lat = float(parts[2])
        lon = float(parts[3])
        alt = float(parts[4])
    except ValueError:
        return None

    return {
        "node_id": node_id,
        "relative_time": relative_time,
        "lat": lat,
        "lon": lon,
        "alt": alt,
    }


def main():
    if len(sys.argv) < 2:
        print("Usage: python process_custom_traces.py <input_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    data = []

    try:
        with open(input_file, "r") as f:
            for line in f:
                if not line.strip():
                    continue
                record = parse_line(line)
                if record:
                    data.append(record)
    except FileNotFoundError:
        print(f"Error: File {input_file} not found.")
        sys.exit(1)

    if not data:
        print("No valid data found.")
        sys.exit(1)

    # Sort by relative_time
    data.sort(key=lambda x: x["relative_time"])

    # Prepare nodes.csv content
    # Format: id_devce;relative_first_time;latitude;longitude;altitude
    # We need to find the first appearance of each node to get its initial details
    node_first_appearances = {}
    for d in data:
        nid = d["node_id"]
        if (
            nid not in node_first_appearances
            or d["relative_time"] < node_first_appearances[nid]["relative_time"]
        ):
            node_first_appearances[nid] = d

    nodes_csv_lines = []
    sorted_node_ids = sorted(node_first_appearances.keys())

    for nid in sorted_node_ids:
        first_rec = node_first_appearances[nid]
        nodes_csv_lines.append(
            f"{nid};{first_rec['relative_time']};{first_rec['lat']};{first_rec['lon']};{first_rec['alt']}"
        )

    nodes_content = "\n".join(nodes_csv_lines)

    # Prepare traces.csv content
    # Format: node;relative_time;latitude;longitude;altitude
    csv_lines = []
    for t in data:
        csv_lines.append(
            f"{t['node_id']};{t['relative_time']};{t['lat']};{t['lon']};{t['alt']}"
        )
    csv_content = "\n".join(csv_lines)

    # Create tar containing gzipped files
    output_filename = "output.trace"
    with tarfile.open(output_filename, "w") as tar:
        # Add nodes.csv.gz
        nodes_gz = gzip.compress(nodes_content.encode("utf-8"))
        nodes_info = tarfile.TarInfo(name="nodes.csv.gz")
        nodes_info.size = len(nodes_gz)
        tar.addfile(nodes_info, io.BytesIO(nodes_gz))

        # Add traces.csv.gz
        traces_gz = gzip.compress(csv_content.encode("utf-8"))
        traces_info = tarfile.TarInfo(name="traces.csv.gz")
        traces_info.size = len(traces_gz)
        tar.addfile(traces_info, io.BytesIO(traces_gz))

    print(f"Successfully created {output_filename}")


if __name__ == "__main__":
    main()
