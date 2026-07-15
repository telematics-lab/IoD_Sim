import json
import argparse

def generate_congestion_scenario(num_users=200, batches=10, max_time=100.0):
    base_file = '../scenario/leo-nr-multibeam-handover.json'

    import re
    with open(base_file, 'r') as f:
        content = f.read()
    # Remove C-style single-line comments
    content = re.sub(r'//.*', '', content)
    data = json.loads(content)

    # Rename scenario
    data['name'] = f'leo-nr-congestion-{num_users}users'
    data['duration'] = max_time + 2  # Ensure simulation runs past the last batch
    data['appStatisticsReportInterval'] = 0.05
    data['internetBackbone'] = {
        "dataRate": "100Gbps"
    }

    # We will clear existing vehicles and remotes
    data['vehicles'] = []

    # We will keep one remote that handles all connections, just clearing its applications
    # Or we can just create 1 remote with many applications.
    if len(data['remotes']) == 0:
        data['remotes'] = [{"networkLayer": 0, "applications": []}]

    data['remotes'][0]['applications'] = []

    # Center around Longitude 102.0, Latitude 30.0 (where satellite 1 is)
    center_lon = 102.0
    center_lat = 30.0

    # Calculate custom time labels
    custom_labels = []
    batch_counts = [0] * batches
    for i in range(num_users):
        b_idx = (i * batches) // num_users
        batch_counts[b_idx] += 1

    cumulative_users = 0
    for b in range(batches):
        st = 1.0 + (b * (max_time / batches))
        cumulative_users += batch_counts[b]
        custom_labels.append({
            "time": st,
            "label": f"{batch_counts[b]} users starts (total {cumulative_users} users)"
        })
    data['custom-time-labels'] = custom_labels

    # Port counter
    base_port = 1000

    for i in range(num_users):
        batch_idx = (i * batches) // num_users
        start_time = 1.0 + (batch_idx * (max_time / batches))
        stop_time = data['duration'] - 1.0

        # Spread vehicles slightly around the center
        offset_lon = (i % 10) * 0.01 - 0.05
        offset_lat = ((i // 10) % 10) * 0.01 - 0.05

        veh_lon = center_lon + offset_lon
        veh_lat = center_lat + offset_lat

        port1 = base_port + (i * 2)
        port2 = base_port + (i * 2) + 1

        vehicle = {
            "netDevices": [
                {
                    "type": "nr",
                    "networkLayer": 0,
                    "role": "UE",
                    "qosFlows": [{"type": "NGBR_VIDEO_TCP_DEFAULT"}]
                }
            ],
            "mobilityModel": {
                "name": "ns3::GeoConstantVelocityMobility",
                "attributes": [
                    {"name": "InitialLongitude", "value": veh_lon},
                    {"name": "InitialLatitude", "value": veh_lat},
                    {"name": "Altitude", "value": 0.0},
                    {"name": "Speed", "value": 0.0}
                ]
            },
            "applications": [
                {
                    "name": "ns3::PacketSink",
                    "attributes": [
                        {"name": "Protocol", "value": "ns3::UdpSocketFactory"},
                        {"name": "StartTime", "value": start_time},
                        {"name": "StopTime", "value": stop_time},
                        {"name": "Local", "value": ["0.0.0.0", port1]}
                    ]
                },
                {
                    "name": "ns3::OnOffApplication",
                    "attributes": [
                        {"name": "Protocol", "value": "ns3::UdpSocketFactory"},
                        {"name": "DataRate", "value": "15Mbps"},
                        {"name": "PacketSize", "value": 1000},
                        {"name": "OnTime", "value": "ns3::ConstantRandomVariable[Constant=1]"},
                        {"name": "OffTime", "value": "ns3::ConstantRandomVariable[Constant=0]"},
                        {"name": "StartTime", "value": start_time},
                        {"name": "StopTime", "value": stop_time},
                        {"name": "Remote", "value": {
                            "@ip": {
                                "key": "remotes",
                                "index": 0,
                                "device": 0,
                                "port": port2
                            }
                        }}
                    ]
                }
            ]
        }
        data['vehicles'].append(vehicle)

        # Add corresponding applications to the remote
        data['remotes'][0]['applications'].append({
            "name": "ns3::OnOffApplication",
            "attributes": [
                {"name": "Protocol", "value": "ns3::UdpSocketFactory"},
                {"name": "DataRate", "value": "15Mbps"},
                {"name": "PacketSize", "value": 1000},
                {"name": "OnTime", "value": "ns3::ConstantRandomVariable[Constant=1]"},
                {"name": "OffTime", "value": "ns3::ConstantRandomVariable[Constant=0]"},
                {"name": "StartTime", "value": start_time},
                {"name": "StopTime", "value": stop_time},
                {"name": "Remote", "value": {
                    "@ip": {
                        "key": "vehicles",
                        "index": i,
                        "device": 0,
                        "port": port1
                    }
                }}
            ]
        })
        data['remotes'][0]['applications'].append({
            "name": "ns3::PacketSink",
            "attributes": [
                {"name": "Protocol", "value": "ns3::UdpSocketFactory"},
                {"name": "StartTime", "value": start_time},
                {"name": "StopTime", "value": stop_time},
                {"name": "Local", "value": ["0.0.0.0", port2]}
            ]
        })

    out_file = f'../scenario/leo-nr-congestion-{num_users}users.json'
    with open(out_file, 'w') as f:
        json.dump(data, f, indent=2)

    print(f"Generated scenario {out_file} with {num_users} users starting in {batches} batches.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate Congestion Scenario')
    parser.add_argument('--users', type=int, default=200, help='Number of total users')
    parser.add_argument('--batches', type=int, default=10, help='Number of batches to start applications')
    parser.add_argument('--maxtime', type=float, default=100.0, help='Max time for the last batch to start')

    args = parser.parse_args()
    generate_congestion_scenario(args.users, args.batches, args.maxtime)
