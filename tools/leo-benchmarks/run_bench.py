import sys
import yaml
import json
import itertools
import subprocess
import time
import os
import csv
import argparse
import shlex
import concurrent.futures
import threading
import uuid

CONFIG_FILE = "benchmark_config.yaml"

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
RESULTS_FILE = os.path.join(SCRIPT_DIR, "benchmark_results.csv")
SCENARIOS_DIR = os.path.join(SCRIPT_DIR, "scenarios")

csv_lock = threading.Lock()

def generate_scenario(num_sats, num_vehs, precision, enable_nr, duration, index):
    scenario = {
        "name": f"benchmark-scenario-{index}",
        "resultsPath": "./results/",
        "logOnFile": False,
        "duration": duration,
        "staticNs3Config": [
            {
                "name": "ns3::GeocentricMobilityModel::EarthSpheroidType",
                "value": "SPHERE"
            },
            {
                "name": "ns3::GeoLeoOrbitMobility::Precision",
                "value": precision
            },
            {
                "name": "ns3::GeoConstantVelocityMobility::Precision",
                "value": precision
            },
            {
                "name": "ns3::NrRlcUm::MaxTxBufferSize",
                "value": 999999999
            },
            {
                "name": "ns3::NrMacSchedulerNs3::EnableHarqReTx",
                "value": False
            },
            {
                "name": "ns3::NrSpectrumPhy::SameNodeInterference",
                "value": False
            }
        ],
        "world": {
            "size": {
                "X": "40000000",
                "Y": "40000000",
                "Z": "40000000"
            },
            "buildings": []
        },
        "phyLayer": [{"type": "none"}] if not enable_nr else [],
        "macLayer": [],
        "vehicles": [],
        "leo-sats": []
    }

    if enable_nr:
        scenario["networkLayer"] = [
            {
                "type": "ipv4",
                "address": "7.0.0.0",
                "mask": "255.0.0.0",
                "gateway": "7.0.0.1"
            }
        ]

    if enable_nr:
        scenario["phyLayer"].append({
            "type": "nr",
            "channels": [
                {
                    "scenario": "NTN-Rural",
                    "conditionModel": "Default",
                    "propagationModel": "ThreeGpp",
                    "pathlossAttributes": [
                        {"name": "ShadowingEnabled", "value": False}
                    ],
                    "channelConditionAttributes": [
                        {"name": "UpdatePeriod", "value": "100ms"}
                    ],
                    "bands": [
                        {
                            "type": "contiguous",
                            "carrier": {
                                "centralFrequency": 28e9,
                                "bandwidth": 100e6,
                                "numComponentCarriers": 1,
                                "numBandwidthParts": 1
                            }
                        }
                    ]
                }
            ],
            "ueAntenna": {
                "arrayProperties": [
                    {"name": "NumRows", "value": 1},
                    {"name": "NumColumns", "value": 1}
                ]
            },
            "gnbAntenna": {
                "type": "ns3::CircularApertureAntennaModel",
                "properties": [
                    {"name": "OperatingFrequency", "value": 28e9},
                    {"name": "AntennaMaxGainDb", "value": 50.0},
                    {"name": "AntennaCircularApertureRadius", "value": 0.01}
                ],
                "arrayProperties": [
                    {"name": "NumRows", "value": 1},
                    {"name": "NumColumns", "value": 1}
                ]
            }
        })
        scenario["macLayer"].append({"type": "nr"})

    # Setup vehicles
    veh_net_devices = []
    if enable_nr:
        veh_net_devices.append({
            "type": "nr",
            "networkLayer": 0,
            "role": "UE",
            "outputLinks": [{"sourceBwp": 0, "targetBwp": 0}],
            "bearers": [{"type": "NGBR_LOW_LAT_EMBB"}],
            "directivity": {"mode": "serving-gnb"}
        })

    for i in range(num_vehs):
        veh = {
            "netDevices": veh_net_devices,
            "mobilityModel": {
                "name": "ns3::GeoConstantVelocityMobility",
                "attributes": [
                    {"name": "InitialLatitude", "value": 45.0 + (i * 0.1)},
                    {"name": "InitialLongitude", "value": 9.0 + (i * 0.1)},
                    {"name": "Altitude", "value": 0.0},
                    {"name": "Speed", "value": 30.0},
                    {"name": "Azimuth", "value": 90.0}
                ]
            }
        }
        scenario["vehicles"].append(veh)

    # Setup sats
    sat_net_devices = []
    if enable_nr:
        sat_net_devices.append({
            "type": "nr",
            "networkLayer": 0,
            "role": "gNB",
            "outputLinks": [{"sourceBwp": 0, "targetBwp": 0}],
            "phy": [
                {"bwpId": 0, "name": "Numerology", "value": 0},
                {"bwpId": 0, "name": "Pattern", "value": "F|F|F|F|F|F|F|F|F|F|"},
                {"bwpId": 0, "name": "TxPower", "value": 4.0}
            ],
            "directivity": {"mode": "earth-centered"}
        })

    sat = {
        "!constellation": {
            "distribution": "uniform-orbits",
            "orbits": [
                {
                    "height": 400.0,
                    "inclination": 30.0,
                    "orbits-per-longitude": 1,
                    "sats-per-orbit": num_sats
                }
            ]
        },
        "netDevices": sat_net_devices
    }
    scenario["leo-sats"].append(sat)

    scenario_filename = os.path.join(SCENARIOS_DIR, f"scenario_{index}_sats{num_sats}_vehs{num_vehs}_prec{precision}_nr{enable_nr}_dur{duration}.json")
    with open(scenario_filename, "w") as f:
        json.dump(scenario, f, indent=2)
    return scenario_filename

def find_binary():
    import sys
    script_dir = os.path.dirname(os.path.abspath(__file__))
    compile_bin_dir = os.path.abspath(os.path.join(script_dir, "..", "compile", "bin"))

    if not os.path.exists(compile_bin_dir):
        print(f"Directory {compile_bin_dir} not found. Please compile the simulator first.")
        sys.exit(1)

    binaries = [f for f in os.listdir(compile_bin_dir) if os.path.isfile(os.path.join(compile_bin_dir, f)) and not f.startswith('.')]

    if not binaries:
        print("No binary found in tools/compile/bin. Please compile the simulator first.")
        sys.exit(1)

    # Ordine alfabetico più basso (es: ns3.48 viene prima di ns3.49)
    binaries.sort()
    selected_binary = binaries[0]

    print(f"Selected binary: {selected_binary} from {binaries}")
    return os.path.join(compile_bin_dir, selected_binary)

def run_simulation(binary_path, scenario_file):
    # Esegui il binario statico puntando al suo percorso assoluto
    # La CWD rimane la cartella in cui si trova questo script python
    script_dir = os.path.dirname(os.path.abspath(__file__))
    cmd = [binary_path, f"--config={scenario_file}"]

    start_time = time.time()
    result = subprocess.run(cmd, cwd=script_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    end_time = time.time()

    execution_time = end_time - start_time
    status = "SUCCESS" if result.returncode == 0 else "FAILED"

    if result.returncode != 0:
        print(f"Simulation failed with return code {result.returncode}")
        print(f"Stderr: {result.stderr.decode('utf-8')[:500]}")

    return execution_time, status

def worker(task):
    i, num_sats, num_vehs, precision, enable_nr, duration, binary_path, total = task
    print(f"Starting benchmark {i}/{total}: sats={num_sats}, vehs={num_vehs}, precision={precision}, nr={enable_nr}, duration={duration}")
    scenario_file = generate_scenario(num_sats, num_vehs, precision, enable_nr, duration, i)
    exec_time, status = run_simulation(binary_path, scenario_file)
    print(f"Finished benchmark {i}/{total} in {exec_time:.2f}s with status {status}")

    with csv_lock:
        with open(RESULTS_FILE, "a", newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow([num_sats, num_vehs, precision, enable_nr, duration, f"{exec_time:.4f}", status])

def main():
    parser = argparse.ArgumentParser(description="Run benchmarks")
    parser.add_argument("-j", "--jobs", type=int, default=1, help="Number of concurrent processes")
    args, unknown = parser.parse_known_args()

    if sys.platform != "linux":
        print("Not on Linux. Auto-executing inside a Docker container...")
        repo_root = os.path.abspath(os.path.join(SCRIPT_DIR, "../.."))
        args_str = " ".join(shlex.quote(a) for a in sys.argv[1:])
        container_name = f"leo-bench-{uuid.uuid4().hex[:8]}"
        cmd = [
            "docker", "run", "--rm", "--name", container_name,
            "-v", f"{repo_root}:/IoD_Sim",
            "-w", "/IoD_Sim/tools/leo-benchmarks",
            "ubuntu:26.04",
            "bash", "-c",
            f"apt-get update && apt-get install -y python3 python3-yaml && python3 -u run_bench.py {args_str}"
        ]
        try:
            sys.exit(subprocess.run(cmd).returncode)
        except KeyboardInterrupt:
            print(f"\nInterrupted by user. Stopping Docker container {container_name}...")
            subprocess.run(["docker", "stop", "-t", "0", container_name], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            sys.exit(130)

    os.makedirs(SCENARIOS_DIR, exist_ok=True)
    binary_path = find_binary()

    with open(CONFIG_FILE, "r") as f:
        config = yaml.safe_load(f)

    num_sats_list = config.get("num_satellites", [10])
    num_vehs_list = config.get("num_vehicles", [10])
    precisions = config.get("mobility_precisions", ["1s"])
    enable_nr_list = config.get("enable_nr", [False])
    durations = config.get("durations", [1])

    combinations = list(itertools.product(num_sats_list, num_vehs_list, precisions, enable_nr_list, durations))
    total = len(combinations)

    write_header = not os.path.exists(RESULTS_FILE) or os.path.getsize(RESULTS_FILE) == 0
    with open(RESULTS_FILE, "a", newline='') as csvfile:
        writer = csv.writer(csvfile)
        if write_header:
            writer.writerow(["num_satellites", "num_vehicles", "mobility_precision", "enable_nr", "duration", "execution_time_s", "status"])

    tasks = [(i+1, num_sats, num_vehs, precision, enable_nr, duration, binary_path, total) for i, (num_sats, num_vehs, precision, enable_nr, duration) in enumerate(combinations)]

    print(f"Starting {total} benchmarks using {args.jobs} concurrent process(es)...")
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as executor:
        executor.map(worker, tasks)

if __name__ == "__main__":
    main()
