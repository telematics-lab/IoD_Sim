import sys
import datetime
import re
import tarfile
import gzip
import io

"""
This script parse this kind of format file:
node_id;timestamp_iso;POINT(lat lon)
"""

def parse_line(line):
    """Parses a single line of the input file."""
    # Format: 156;2014-02-01 00:00:00.739166+01;POINT(41.8836718276551 12.4877775603346)
    parts = line.strip().split(';')
    if len(parts) != 3:
        return None

    node_id = parts[0]
    timestamp_str = parts[1]
    point_str = parts[2]

    try:
        dt = datetime.datetime.fromisoformat(timestamp_str)
    except ValueError:
        return None

    # Parse POINT
    match = re.search(r'POINT\(([\d\.]+) ([\d\.]+)\)', point_str)
    if match:
        lat = float(match.group(1))
        lon = float(match.group(2))
    else:
        return None

    return {
        'node_id': int(node_id),
        'timestamp': dt,
        'lat': lat,
        'lon': lon
    }

def main():
    if len(sys.argv) < 2:
        print("Usage: python process_traces.py <input_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    data = []

    try:
        with open(input_file, 'r') as f:
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

    # Find min time
    min_time = min(d['timestamp'] for d in data)

    # Process data
    processed_traces = []
    nodes = set()

    for d in data:
        nodes.add(d['node_id'])
        # Relative time in milliseconds
        diff = d['timestamp'] - min_time
        rel_time_ms = int(diff.total_seconds() * 1000)

        processed_traces.append({
            'node': d['node_id'],
            'relative_time': rel_time_ms,
            'lat': d['lat'],
            'long': d['lon']
        })

    # Sort by relative_time
    processed_traces.sort(key=lambda x: x['relative_time'])

    # Prepare nodes.csv content
    # Format: id_devce;relative_first_time;absolute_first_time;lat;long
    # We need to find the first appearance of each node to get its initial details
    node_first_appearances = {}
    for d in data:
        nid = d['node_id']
        if nid not in node_first_appearances or d['timestamp'] < node_first_appearances[nid]['timestamp']:
            node_first_appearances[nid] = d

    nodes_csv_lines = [] # No header: without "id_devce;relative_first_time;absolute_first_time;latitude;longitude;altitude"
    sorted_node_ids = sorted(node_first_appearances.keys())

    for nid in sorted_node_ids:
        first_rec = node_first_appearances[nid]
        # Calculate relative first time
        diff = first_rec['timestamp'] - min_time
        rel_first_time = int(diff.total_seconds() * 1000)
        # abs_time = first_rec['timestamp'].isoformat() # Removed

        nodes_csv_lines.append(f"{nid};{rel_first_time};{first_rec['lat']};{first_rec['lon']};1.0")

    nodes_content = "\n".join(nodes_csv_lines)

    # Prepare traces.csv content
    csv_lines = [] # No header: without "node;relative_time;latitude;longitude;altitude"
    for t in processed_traces:
        csv_lines.append(f"{t['node']};{t['relative_time']};{t['lat']};{t['long']};1.0")
    csv_content = "\n".join(csv_lines)

    # Create tar containing gzipped files
    output_filename = "output.gz.tar"
    with tarfile.open(output_filename, "w") as tar:
        # Add nodes.csv.gz
        nodes_gz = gzip.compress(nodes_content.encode('utf-8'))
        nodes_info = tarfile.TarInfo(name="nodes.csv.gz")
        nodes_info.size = len(nodes_gz)
        tar.addfile(nodes_info, io.BytesIO(nodes_gz))

        # Add traces.csv.gz
        traces_gz = gzip.compress(csv_content.encode('utf-8'))
        traces_info = tarfile.TarInfo(name="traces.csv.gz")
        traces_info.size = len(traces_gz)
        tar.addfile(traces_info, io.BytesIO(traces_gz))

    print(f"Successfully created {output_filename}")

if __name__ == "__main__":
    main()
