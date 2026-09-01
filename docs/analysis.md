# Analysis Scripts

The `analysis` directory contains various scripts used to generate graphs and analyze data. They have been used in the various scientific publications involving IoD_Sim and can be a useful starting point for users utilizing the simulator.

## Python Scripts

- **drone_peripheral_consumption_to_state.py**: Analyzes drone peripheral power consumption and maps it to different operational states.
- **geo2kml-line.py**: Converts geographical coordinates into KML format as a line.
- **geo2kml.py**: Transforms geographical data into KML format.
- **geosnr-single.py**: Visualizes Signal-to-Noise Ratio (SNR) for a single simulation with a GEO satellite.
- **geosnr.py**: Computes and visualizes mean SNR over multiple simulations.
- **get_rssi.py**: Extracts Received Signal Strength Indicator (RSSI) from the XML summary file.
- **latency_lte.py**: Analyzes and plots latency data for scenarios in which LTE technology is employed.
- **latency_wifi+lte.py**: Analyzes and plots latency data for scenarios in which both Wi-Fi and LTE technologies are adopted.
- **leo-geo-view.py**: Interactive 3D viewer for LEO/NR runs. Draws the constellation, the UEs, the ISL routing tree, the UE-to-satellite service links and the REM in ECEF coordinates, animated over simulated time. Takes a results directory (or asks for one), writes `leo-geo-view.html` into it and opens it. Three views are available as buttons in the page: *Simulated* (nodes, links and trails move with the time slider), *Trajectories* (every path at once) and *Both*; `--view` picks the one to open on, and defaults to showing everything unless the run is large enough that the paths would slow the animation. Each node keeps one colour across its path, marker and trail, and gets its own legend entry when the layer has few enough nodes. Use `--max-points` / `--max-frames` / `--trail` to trade detail for smoothness on large runs, `--no-animation` for a static view and `--no-browser` to only write the file.
- **lte-split_MacUlStats_per_drone.py**: Parse LTE MAC Ul Stats file and output one CSV per drone containing LTE Cell ID and TB size.
- **lte-split_RlcUlStats_per_drone.py**: Parse LTE Rlc Ul Stats file and output one file per drone/IMSI containing the LTE Cell ID.
- **lte-throughput_per_drone.py**: Computes LTE throughput statistics per drone.
- **merge_trajectory_powerconsumption.py**: Merges drone trajectory data with power consumption statistics.
- **pcap-get_pdr.py**: Given a pcap file, for each drone connected to the LTE network extrapolate tcp payload length and export everything in CSV.
- **plot-leo-nr-ue-stats.py**: Per-UE experience plots for LEO/NR runs: delay and jitter, throughput, SINR (data and control) and the distance/elevation of the serving satellite, with RRC events, ISL path changes and scenario labels marked on the time axis. Also produces one aggregate figure per satellite (throughput, delay, connected UEs, SINR across served UEs). Takes a results directory and writes into its `graphs/` subfolder.
- **plr_datarate.py**: Plot PDR/PLR data over distance for different simulations with different datarate.
- **plr_\*.py**: Scripts to evaluate packet loss rate in different scenarios.
- **preview.py**: Provides a preview of a given simulation configuration in terms of drone trajectories and ZSPs position..
- **progress.py**: Tracks the progress of data processing and analysis tasks.
- **rem-2d-preview.py**: This script generates the plot of a 2D Radio Environment Map using a .rem file.
- **rem-3d-preview.py**: This script generates the plot of a 3D Radio Environment Map using a .rem file.
- **sinr_lte.py**: Computes Signal-to-Interference-plus-Noise Ratio (SINR) for LTE scenarios.
- **sinr_wifi.py**: Calculates SINR for Wi-Fi scenarios.
- **trajectory+cellid.py**: Combine LTE RLC data and trajectory in a single CSV per drone.
- **trajectory2csv.py**: Parse drone trajectories from summary file.
- **txt2ply.py**: Converts text-based data into PLY format for 3D visualization.

## Shell Scripts

- **drone_storage_memory.sh**: Parse storage memory from debug log file and output a CSV file per drone.
- **get_drones_peripheral_power_consumption.sh**: Gathers power consumption data for drone peripherals.
- **pcap-get_ip.sh**: Given a pcap file, for each drone connected to the LTE network extrapolate ip packet length and export everything in CSV.
- **pcap-get_lte_tcp_len.sh**: Given a pcap file, for each drone connected to the LTE network extrapolate tcp payload length and export everything in CSV.
- **pcap-get_udp.sh**: Given a pcap file, for each drone connected to the LTE network extrapolate udp payload length and export everything in CSV.
- **pcap-throughput_per_drone.py**: Translate TCP traffic from CSV in throughput.
- **total_power_consumption_drones.sh**: Aggregates and analyzes total power consumption data for drones.