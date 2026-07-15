import pandas as pd
import matplotlib.pyplot as plt
import os
import argparse
import numpy as np
import warnings
def plot_user_isl_experience(results_dir):
    app_stats_file = os.path.join(results_dir, 'app-statistics-periodic.txt')
    vehicle_trace_file = os.path.join(results_dir, 'vehicle-trace.csv')
    delay_trace_file = os.path.join(results_dir, 'isl-delay-trace.csv')
    data_sinr_file = os.path.join(results_dir, 'DlDataSinr.txt')
    ctrl_sinr_file = os.path.join(results_dir, 'DlCtrlSinr.txt')
    rx_packet_trace_file = os.path.join(results_dir, 'RxPacketTrace.txt')
    rrc_events_file = os.path.join(results_dir, 'ue-rrc-events.csv')
    leo_sat_file = os.path.join(results_dir, 'leo-sat-trace.csv')

    required_files = [app_stats_file, vehicle_trace_file, delay_trace_file]
    for rf in required_files:
        if not os.path.exists(rf):
            print(f"Error: {rf} not found in {results_dir}")
            return

    # Read data
    df_app = pd.read_csv(app_stats_file, sep='\t')
    df_app.columns = df_app.columns.str.strip()
    df_veh = pd.read_csv(vehicle_trace_file)
    df_delay = pd.read_csv(delay_trace_file)
    df_delay.columns = df_delay.columns.str.strip()

    df_rx_packet = pd.DataFrame()
    if os.path.exists(rx_packet_trace_file) and os.path.getsize(rx_packet_trace_file) > 0:
        try:
            df_rx_packet = pd.read_csv(rx_packet_trace_file, sep='\t')
            df_rx_packet.columns = df_rx_packet.columns.str.strip()
            # If RxPacketTrace is missing SINR(dB) for some reason, we fall back to DlDataSinr.txt
            if 'SINR(dB)' not in df_rx_packet.columns:
                df_rx_packet = pd.DataFrame()
        except pd.errors.EmptyDataError:
            df_rx_packet = pd.DataFrame()

    df_data_sinr = pd.DataFrame()
    if df_rx_packet.empty and os.path.exists(data_sinr_file) and os.path.getsize(data_sinr_file) > 0:
        try:
            df_data_sinr = pd.read_csv(data_sinr_file, sep='\t')
            df_data_sinr.columns = df_data_sinr.columns.str.strip()
        except pd.errors.EmptyDataError:
            df_data_sinr = pd.DataFrame()

    df_ctrl_sinr = pd.DataFrame()
    if os.path.exists(ctrl_sinr_file) and os.path.getsize(ctrl_sinr_file) > 0:
        try:
            df_ctrl_sinr = pd.read_csv(ctrl_sinr_file, sep='\t')
            df_ctrl_sinr.columns = df_ctrl_sinr.columns.str.strip()
        except pd.errors.EmptyDataError:
            df_ctrl_sinr = pd.DataFrame()

    df_rrc_events = pd.DataFrame()
    if os.path.exists(rrc_events_file) and os.path.getsize(rrc_events_file) > 0:
        try:
            df_rrc_events = pd.read_csv(rrc_events_file)
        except pd.errors.EmptyDataError:
            df_rrc_events = pd.DataFrame()

    df_sat = pd.DataFrame()
    if os.path.exists(leo_sat_file) and os.path.getsize(leo_sat_file) > 0:
        try:
            df_sat = pd.read_csv(leo_sat_file)
        except pd.errors.EmptyDataError:
            df_sat = pd.DataFrame()

    out_dir = os.path.join(results_dir, 'graphs')
    os.makedirs(out_dir, exist_ok=True)

    # Load custom labels if available
    custom_labels_file = os.path.join(results_dir, 'custom-time-labels.csv')
    df_custom_labels = pd.DataFrame()
    if os.path.exists(custom_labels_file) and os.path.getsize(custom_labels_file) > 0:
        try:
            df_custom_labels = pd.read_csv(custom_labels_file)
        except pd.errors.EmptyDataError:
            pass

    global_sat_stats = []
    global_sat_sinr_stats = []


    # Check if we have an explicit node-roles.csv file to identify UEs
    node_roles_file = os.path.join(results_dir, 'node-roles.csv')
    cell_to_node = {}
    if os.path.exists(node_roles_file):
        df_node_roles = pd.read_csv(node_roles_file)
        # Find nodes where KeyIndex starts with "vehicles"
        ue_nodes = df_node_roles[df_node_roles['KeyIndex'].str.startswith('vehicles', na=False)]['NodeId'].unique()

        # Build cell_to_node mapping
        if 'CellIds' in df_node_roles.columns:
            for _, row in df_node_roles.dropna(subset=['CellIds']).iterrows():
                # CellIds is a space-separated string
                cids_str = str(row['CellIds']).strip()
                if cids_str:
                    for cid in cids_str.split():
                        try:
                            cell_to_node[int(float(cid))] = row['NodeId']
                        except ValueError:
                            pass
    else:
        # Fallback to finding UEs from vehicle_trace
        ue_nodes = df_veh['Node'].unique()

    for ue_id in ue_nodes:
        # 1. Filter vehicle trace for this UE
        df_ue_veh = df_veh[df_veh['Node'] == ue_id].copy()
        df_ue_veh.sort_values('Time', inplace=True)

        # 2. Get App Stats for this UE
        # UL Stats (UE is source)
        df_ul_app = df_app[df_app['SrcNodeID'] == ue_id].copy()
        if not df_ul_app.empty:
            df_ul_app['Delay_ms'] = df_ul_app['Delay_ms'].replace(0.0, np.nan)
            df_ul_grouped = df_ul_app.groupby('Time_s').agg({
                'RxThroughput_kbps': 'sum',
                'Delay_ms': 'mean',
                'Jitter_ms': 'mean'
            }).reset_index()
        else:
            df_ul_grouped = pd.DataFrame(columns=['Time_s', 'RxThroughput_kbps', 'Delay_ms', 'Jitter_ms'])

        # DL Stats (UE is destination)
        df_dl_app = df_app[df_app['DstNodeID'] == ue_id].copy()
        if not df_dl_app.empty:
            df_dl_app['Delay_ms'] = df_dl_app['Delay_ms'].replace(0.0, np.nan)
            df_dl_grouped = df_dl_app.groupby('Time_s').agg({
                'RxThroughput_kbps': 'sum',
                'Delay_ms': 'mean',
                'Jitter_ms': 'mean'
            }).reset_index()
        else:
            df_dl_grouped = pd.DataFrame(columns=['Time_s', 'RxThroughput_kbps', 'Delay_ms', 'Jitter_ms'])

        # 3. Get SINR for this UE (using RRC events to filter if available)
        df_ue_rrc = pd.DataFrame()
        active_cells_rntis = []

        if not df_rrc_events.empty:
            df_ue_rrc = df_rrc_events[df_rrc_events['NodeId'] == ue_id].copy()
            df_ue_rrc.sort_values('Time', inplace=True)
            # Track Active RNTI and CellId over time
            current_cell = None
            current_rnti = None
            last_time = 0.0

            for _, row in df_ue_rrc.iterrows():
                if row['Event'] == 'Attach':
                    if current_cell is not None and current_rnti is not None:
                        active_cells_rntis.append((last_time, row['Time'], current_cell, current_rnti))
                    current_cell = row['TargetCellId']
                    current_rnti = row['RNTI']
                    last_time = row['Time']
                elif row['Event'] == 'HandoverStart':
                    if current_cell is not None and current_rnti is not None:
                        active_cells_rntis.append((last_time, row['Time'], current_cell, current_rnti))
                    current_cell = None
                    current_rnti = None
                elif row['Event'] == 'HandoverEndOk':
                    current_cell = row['TargetCellId']
                    current_rnti = row['RNTI']
                    last_time = row['Time']

            if current_cell is not None and current_rnti is not None:
                active_cells_rntis.append((last_time, float('inf'), current_cell, current_rnti))

        # Filter SINR
        df_ul_data_sinr_grouped = pd.DataFrame()
        df_dl_data_sinr_grouped = pd.DataFrame()

        if not df_rx_packet.empty:
            if active_cells_rntis:
                mask = pd.Series(False, index=df_rx_packet.index)
                for start_t, end_t, cid, rnti in active_cells_rntis:
                    mask = mask | ((df_rx_packet['Time'] >= start_t) & (df_rx_packet['Time'] < end_t) & (df_rx_packet['cellId'] == cid) & (df_rx_packet['rnti'] == rnti))
                filtered_rx = df_rx_packet[mask]
                if filtered_rx.empty:
                    filtered_rx = df_rx_packet
            else:
                filtered_rx = df_rx_packet

            ul_rx = filtered_rx[filtered_rx['direction'] == 'UL']
            dl_rx = filtered_rx[filtered_rx['direction'] == 'DL']

            if not ul_rx.empty:
                df_ul_data_sinr_grouped = ul_rx.groupby('Time').agg({'SINR(dB)': 'mean'}).reset_index()
            if not dl_rx.empty:
                df_dl_data_sinr_grouped = dl_rx.groupby('Time').agg({'SINR(dB)': 'mean'}).reset_index()
                
        if df_dl_data_sinr_grouped.empty and not df_data_sinr.empty:
            if active_cells_rntis:
                # Build a boolean mask for precise filtering
                mask = pd.Series(False, index=df_data_sinr.index)
                for start_t, end_t, cid, rnti in active_cells_rntis:
                    mask = mask | ((df_data_sinr['Time'] >= start_t) & (df_data_sinr['Time'] < end_t) & (df_data_sinr['CellId'] == cid) & (df_data_sinr['RNTI'] == rnti))

                filtered_data_sinr = df_data_sinr[mask]
                if filtered_data_sinr.empty:
                    # Fallback if strict filtering fails
                    filtered_data_sinr = df_data_sinr
            else:
                filtered_data_sinr = df_data_sinr

            df_dl_data_sinr_grouped = filtered_data_sinr.groupby('Time').agg({'SINR(dB)': 'mean'}).reset_index()

        if not df_ctrl_sinr.empty:
            if active_cells_rntis:
                mask = pd.Series(False, index=df_ctrl_sinr.index)
                for start_t, end_t, cid, rnti in active_cells_rntis:
                    mask = mask | ((df_ctrl_sinr['Time'] >= start_t) & (df_ctrl_sinr['Time'] < end_t) & (df_ctrl_sinr['CellId'] == cid) & (df_ctrl_sinr['RNTI'] == rnti))

                filtered_ctrl_sinr = df_ctrl_sinr[mask]
                if filtered_ctrl_sinr.empty:
                    filtered_ctrl_sinr = df_ctrl_sinr
            else:
                filtered_ctrl_sinr = df_ctrl_sinr

            df_ctrl_sinr_grouped = filtered_ctrl_sinr.groupby('Time').agg({'SINR(dB)': 'mean'}).reset_index()
        else:
            df_ctrl_sinr_grouped = pd.DataFrame()

        # 4. Determine Topology Changes
        # Topology changes when either the Attached Sat changes, or the Sat's ISL path changes
        # Merge vehicle trace with delay trace based on Time and NearestSatId == GNbNodeId

        # We need to approximate the join because times might not match exactly.
        # Let's use merge_asof
        df_ue_veh['Time_ms'] = (df_ue_veh['Time'] * 1000).astype(int)
        df_delay['Time_ms'] = (df_delay['Time'] * 1000).astype(int)

        df_ue_veh.sort_values('Time_ms', inplace=True)
        df_delay.sort_values('Time_ms', inplace=True)

        # For each vehicle time point, we want the ISL path of its NearestSatId
        # We can iterate or do a custom merge
        paths = []
        last_path = None
        last_clean_path = None
        topology_change_times = []
        isl_break_times = []
        isl_restore_times = []

        distances = []
        elevations = []
        isl_delays = []

        import re

        for idx, row in df_ue_veh.iterrows():
            t = row['Time']

            # Determine actual connected satellite (if known from RRC), else fallback to NearestSatId ONLY for old logs
            sat_id = None
            if not df_ue_rrc.empty:
                for start_t, end_t, cid, rnti in active_cells_rntis:
                    if start_t <= t < end_t:
                        sat_id = cell_to_node.get(cid)
                        break
            else:
                sat_id = row['NearestSatId']

            # Find the closest delay trace for this satellite before or at this time
            sat_delays = df_delay[(df_delay['GNbNodeId'] == sat_id) & (df_delay['Time'] <= t)]
            if not sat_delays.empty:
                current_path = sat_delays.iloc[-1]['NextHopPath']
                curr_isl_delay = sat_delays.iloc[-1]['TotalDelay'] * 1000.0 # Convert to ms

                # Strip out weights to keep only the node sequence
                # Example: "[Node_30;1.2m;0.1s] -> [Ground;...]" -> "Node_30 -> Ground"
                if pd.isna(current_path) or curr_isl_delay >= 3500000.0:
                    clean_path = "No Path"
                    curr_isl_delay = np.nan
                else:
                    nodes_in_path = re.findall(r'\[([^;]+)', str(current_path))
                    clean_path = " -> ".join(nodes_in_path)
            else:
                clean_path = "Unknown"
                curr_isl_delay = np.nan

            isl_delays.append(curr_isl_delay)

            # The full user path is logically: User -> Sat(sat_id) -> ISL(clean_path)
            if sat_id is not None:
                full_path = f"Sat_{int(sat_id)} -> {clean_path}"
            else:
                full_path = "Unknown"
                
            paths.append(full_path)

            if last_path is not None and full_path != last_path:
                topology_change_times.append(t)


            if last_clean_path is not None:
                if last_clean_path != "Unknown" and clean_path == "Unknown":
                    isl_break_times.append(t)
                elif last_clean_path == "Unknown" and clean_path != "Unknown":
                    isl_restore_times.append(t)

            last_path = full_path
            last_clean_path = clean_path

            # Compute distance and elevation
            if sat_id is not None and not df_sat.empty:
                sat_row = df_sat[(df_sat['Node'] == sat_id) & (df_sat['Time'] <= t)]
                if not sat_row.empty:
                    sat_row = sat_row.iloc[-1]
                    sx, sy, sz = sat_row['X'], sat_row['Y'], sat_row['Z']
                    ux, uy, uz = row['X'], row['Y'], row['Z']

                    dist = np.sqrt((sx - ux)**2 + (sy - uy)**2 + (sz - uz)**2)
                    distances.append(dist / 1000.0)

                    vx, vy, vz = sx - ux, sy - uy, sz - uz
                    v_norm = np.sqrt(vx**2 + vy**2 + vz**2)
                    u_norm = np.sqrt(ux**2 + uy**2 + uz**2)

                    if u_norm > 0 and v_norm > 0:
                        dot_prod = (vx*ux + vy*uy + vz*uz) / (v_norm * u_norm)
                        dot_prod = max(-1.0, min(1.0, dot_prod))
                        elev = 90.0 - np.degrees(np.arccos(dot_prod))
                        elevations.append(elev)
                    else:
                        elevations.append(np.nan)
                else:
                    distances.append(np.nan)
                    elevations.append(np.nan)
            else:
                distances.append(np.nan)
                elevations.append(np.nan)

        df_ue_veh['Distance_km'] = distances
        df_ue_veh['Elevation_deg'] = elevations
        df_ue_veh['ISL_Delay_ms'] = isl_delays

        # Create the subplots
        # We want to plot: Delay, Throughput, SINR, Distance/Elevation
        fig, axes = plt.subplots(4, 1, figsize=(14, 16), sharex=True)

        # Plot 1: Delay and Jitter
        if not df_dl_grouped.empty:
            axes[0].plot(df_dl_grouped['Time_s'], df_dl_grouped['Delay_ms'], color='tab:red', label='DL Delay (ms)', linewidth=2)
            lower_bound_dl = (df_dl_grouped['Delay_ms'] - df_dl_grouped['Jitter_ms']).clip(lower=0)
            upper_bound_dl = df_dl_grouped['Delay_ms'] + df_dl_grouped['Jitter_ms']
            axes[0].fill_between(df_dl_grouped['Time_s'], lower_bound_dl, upper_bound_dl, color='tab:red', alpha=0.2, label='DL Jitter')

        if not df_ul_grouped.empty:
            axes[0].plot(df_ul_grouped['Time_s'], df_ul_grouped['Delay_ms'], color='purple', linestyle='-', label='UL Delay (ms)', linewidth=2)
            lower_bound_ul = (df_ul_grouped['Delay_ms'] - df_ul_grouped['Jitter_ms']).clip(lower=0)
            upper_bound_ul = df_ul_grouped['Delay_ms'] + df_ul_grouped['Jitter_ms']
            axes[0].fill_between(df_ul_grouped['Time_s'], lower_bound_ul, upper_bound_ul, color='purple', alpha=0.1, label='UL Jitter')

        # Plot theoretical ISL Delay
        axes[0].plot(df_ue_veh['Time'], df_ue_veh['ISL_Delay_ms'], color='tab:gray', linestyle='-.', label='ISL Path Delay (ms)', linewidth=2)

        axes[0].set_ylabel('Delay & Jitter (ms)')
        axes[0].set_title(f'User Experience & Topology Changes (UE Node {ue_id})')
        axes[0].grid(True, linestyle='--', alpha=0.6)
        leg1 = axes[0].legend(loc='upper left', ncol=2)
        axes[0].add_artist(leg1)

        # Plot 2: Throughput
        if not df_ul_grouped.empty:
            axes[1].plot(df_ul_grouped['Time_s'], df_ul_grouped['RxThroughput_kbps'], color='tab:orange', label='UL Throughput (Received)', linewidth=2)
        if not df_dl_grouped.empty:
            axes[1].plot(df_dl_grouped['Time_s'], df_dl_grouped['RxThroughput_kbps'], color='tab:blue', label='DL Throughput (Received)', linewidth=2)
        axes[1].set_ylabel('Throughput (kbps)')
        if axes[1].get_legend_handles_labels()[0]:
            axes[1].legend(loc='upper left')
        axes[1].grid(True, linestyle='--', alpha=0.6)

        # Plot 3: SINR
        has_sinr = False
        if not df_ctrl_sinr_grouped.empty:
            axes[2].plot(df_ctrl_sinr_grouped['Time'], df_ctrl_sinr_grouped['SINR(dB)'], color='tab:gray', linestyle=':', label='DL Ctrl SINR (dB)', linewidth=1.5)
            has_sinr = True

        if not df_dl_data_sinr_grouped.empty:
            # Break the line if the time gap is more than 0.5 seconds
            df_dl_data_sinr_grouped['Time_diff'] = df_dl_data_sinr_grouped['Time'].diff()
            df_dl_data_sinr_grouped.loc[df_dl_data_sinr_grouped['Time_diff'] > 0.5, 'SINR(dB)'] = np.nan
            axes[2].plot(df_dl_data_sinr_grouped['Time'], df_dl_data_sinr_grouped['SINR(dB)'], color='tab:green', marker='.', markersize=4, label='DL Data SINR (dB)', linewidth=2)
            has_sinr = True

        if not df_ul_data_sinr_grouped.empty:
            df_ul_data_sinr_grouped['Time_diff'] = df_ul_data_sinr_grouped['Time'].diff()
            df_ul_data_sinr_grouped.loc[df_ul_data_sinr_grouped['Time_diff'] > 0.5, 'SINR(dB)'] = np.nan
            axes[2].plot(df_ul_data_sinr_grouped['Time'], df_ul_data_sinr_grouped['SINR(dB)'], color='tab:orange', marker='.', markersize=4, label='UL Data SINR (dB)', linewidth=2)
            has_sinr = True

        if has_sinr:
            axes[2].legend(loc='lower left')
        else:
            axes[2].text(0.5, 0.5, 'SINR data not available', horizontalalignment='center', verticalalignment='center', transform=axes[2].transAxes)

        axes[2].set_ylabel('SINR (dB)', color='tab:green')
        axes[2].tick_params(axis='y', labelcolor='tab:green')
        axes[2].grid(True, linestyle='--', alpha=0.6)

        # Plot 4: Distance and Elevation
        ax4 = axes[3]
        ax4_2 = ax4.twinx()

        l1 = ax4.plot(df_ue_veh['Time'], df_ue_veh['Distance_km'], color='tab:brown', label='Distance (km)', linewidth=2)
        ax4.set_ylabel('Distance (km)', color='tab:brown')
        ax4.tick_params(axis='y', labelcolor='tab:brown')

        l2 = ax4_2.plot(df_ue_veh['Time'], df_ue_veh['Elevation_deg'], color='tab:cyan', label='Elevation (°)', linewidth=2)
        ax4_2.set_ylabel('Elevation Angle (°)', color='tab:cyan')
        ax4_2.tick_params(axis='y', labelcolor='tab:cyan')

        lns = l1 + l2
        labs = [l.get_label() for l in lns]
        ax4.legend(lns, labs, loc='upper right')
        ax4.grid(True, linestyle='--', alpha=0.6)

        axes[3].set_xlabel('Time (s)')

        # Add vertical lines for RRC events and ISL topology changes
        legend_elements = []
        from matplotlib.lines import Line2D

        # Plot ISL Topology Changes
        for t in topology_change_times:
            if t > 0:
                for ax in axes:
                    ax.axvline(x=t, color='purple', linestyle='--', alpha=0.5)
        if topology_change_times:
            legend_elements.append(Line2D([0], [0], color='purple', linestyle='--', lw=2, label='ISL Path Change'))

        # Plot ISL Break and Restore
        for t in isl_break_times:
            if t > 0:
                for ax in axes:
                    ax.axvline(x=t, color='black', linestyle='--', linewidth=2, alpha=0.8)
        if isl_break_times:
            legend_elements.append(Line2D([0], [0], color='black', linestyle='--', lw=2, label='ISL Break'))

        for t in isl_restore_times:
            if t > 0:
                for ax in axes:
                    ax.axvline(x=t, color='blue', linestyle='-', linewidth=2, alpha=0.8)
        if isl_restore_times:
            legend_elements.append(Line2D([0], [0], color='blue', linestyle='--', lw=2, label='ISL Restore'))

        # Plot RRC Events
        if not df_ue_rrc.empty:
            events_list = df_ue_rrc.to_dict('records')
            events_to_plot = []

            i = 0
            while i < len(events_list):
                row = events_list[i]
                if row['Event'] == 'HandoverStart':
                    # Check if the next event is HandoverEndOk
                    if i + 1 < len(events_list) and events_list[i+1]['Event'] == 'HandoverEndOk':
                        # Handover succeeded. Do not plot HandoverStart.
                        pass
                    else:
                        events_to_plot.append(row)
                else:
                    events_to_plot.append(row)
                i += 1

            for row in events_to_plot:
                t = row['Time']
                evt = row['Event']
                target_cell = row['TargetCellId']
                node_id_str = f"Node {cell_to_node[target_cell]}" if target_cell in cell_to_node else f"Cell {target_cell}"

                if evt == 'Attach':
                    color = 'green'
                    label_suffix = f" to {node_id_str}"
                elif evt == 'HandoverStart':
                    color = 'red'
                    label_suffix = f" to {node_id_str}"
                elif evt == 'HandoverEndOk':
                    color = 'orange'
                    label_suffix = f" on {node_id_str}"
                else:
                    color = 'black'
                    label_suffix = ""

                for ax in axes:
                    ax.axvline(x=t, color=color, linestyle='--', alpha=0.8)

                # Add text label slightly to the right
                axes[0].text(t + 0.05, axes[0].get_ylim()[1] * 0.9, evt + label_suffix, color=color, rotation=90, verticalalignment='top', fontsize=8)

            # Only add legend elements if we actually plotted them
            plotted_events = set(row['Event'] for row in events_to_plot)
            if 'Attach' in plotted_events:
                legend_elements.append(Line2D([0], [0], color='green', linestyle='--', lw=2, label='Attach'))
            if 'HandoverStart' in plotted_events:
                legend_elements.append(Line2D([0], [0], color='red', linestyle='--', lw=2, label='Handover Failed/Pending'))
            if 'HandoverEndOk' in plotted_events:
                legend_elements.append(Line2D([0], [0], color='orange', linestyle='--', lw=2, label='Handover Success'))


            if not df_custom_labels.empty:
                for _, cl_row in df_custom_labels.iterrows():
                    ct = cl_row['Time']
                    clbl = cl_row['Label']
                    for ax in axes:
                        ax.axvline(x=ct, color='purple', linestyle=':', alpha=0.8)
                    axes[0].text(ct + 0.05, axes[0].get_ylim()[1] * 0.9, clbl, color='purple', rotation=90, verticalalignment='top', fontsize=8)

        if legend_elements:
            axes[0].legend(handles=legend_elements, loc='upper right')



        if not df_ul_data_sinr_grouped.empty:
            for _, r in df_ul_data_sinr_grouped.iterrows():
                t = r['Time']
                sat_id = None
                if not df_ue_rrc.empty:
                    for start_t, end_t, cid, rnti in active_cells_rntis:
                        if start_t <= t < end_t:
                            sat_id = cell_to_node.get(cid)
                            break
                if sat_id is not None:
                    global_sat_sinr_stats.append({'Time': t, 'SatId': sat_id, 'UL_SINR': r['SINR(dB)'], 'DL_SINR': np.nan})
                    
        if not df_dl_data_sinr_grouped.empty:
            for _, r in df_dl_data_sinr_grouped.iterrows():
                t = r['Time']
                sat_id = None
                if not df_ue_rrc.empty:
                    for start_t, end_t, cid, rnti in active_cells_rntis:
                        if start_t <= t < end_t:
                            sat_id = cell_to_node.get(cid)
                            break
                if sat_id is not None:
                    global_sat_sinr_stats.append({'Time': t, 'SatId': sat_id, 'UL_SINR': np.nan, 'DL_SINR': r['SINR(dB)']})

        # --- Collect global satellite stats ---
        if not df_ul_grouped.empty or not df_dl_grouped.empty:
            df_merged = pd.merge(
                df_ul_grouped if not df_ul_grouped.empty else pd.DataFrame(columns=['Time_s', 'RxThroughput_kbps', 'Delay_ms']),
                df_dl_grouped if not df_dl_grouped.empty else pd.DataFrame(columns=['Time_s', 'RxThroughput_kbps', 'Delay_ms']),
                on='Time_s', how='outer', suffixes=('_UL', '_DL')
            )
            for _, r in df_merged.iterrows():
                t = r['Time_s']
                sat_id = None
                if not df_ue_rrc.empty:
                    for start_t, end_t, cid, rnti in active_cells_rntis:
                        if start_t <= t < end_t:
                            sat_id = cell_to_node.get(cid)
                            break
                else:
                    if not df_ue_veh.empty:
                        # find nearest in time
                        sat_id = df_ue_veh.iloc[(df_ue_veh['Time'] - t).abs().argsort()[:1]]['NearestSatId'].values[0]
                
                if sat_id is not None:
                    global_sat_stats.append({
                        'Time': t,
                        'SatId': sat_id,
                        'UEId': ue_id,
                        'UL_Throughput': r.get('RxThroughput_kbps_UL', np.nan),
                        'DL_Throughput': r.get('RxThroughput_kbps_DL', np.nan),
                        'UL_Delay': r.get('Delay_ms_UL', np.nan),
                        'DL_Delay': r.get('Delay_ms_DL', np.nan)
                    })

        with warnings.catch_warnings():
            warnings.simplefilter("ignore", UserWarning)
            plt.tight_layout()
            
        plt.savefig(os.path.join(out_dir, f'user_experience_node_{ue_id}.png'))
        plt.close()


    # --- Generate per-satellite aggregate graphs ---
    df_global = pd.DataFrame(global_sat_stats)
    if not df_global.empty:
        for sat_id, df_sat_grp in df_global.groupby('SatId'):
            fig, axes = plt.subplots(3, 1, figsize=(12, 18))
            
            # Aggregate per Time
            df_sat_time = df_sat_grp.groupby('Time').agg({
                'UL_Throughput': ['mean', 'var'],
                'DL_Throughput': ['mean', 'var'],
                'UL_Delay': ['mean', 'var'],
                'DL_Delay': ['mean', 'var'],
                'UEId': 'nunique'
            }).reset_index()
            
            # flatten columns
            df_sat_time.columns = ['_'.join(col).strip('_') if col[1] else col[0] for col in df_sat_time.columns.values]
            
            t = df_sat_time['Time']
            
            # Throughput
            axes[0].plot(t, df_sat_time['UL_Throughput_mean'] / 1000.0, label='UL Mean (Mbps)', color='blue')
            axes[0].fill_between(t, (df_sat_time['UL_Throughput_mean'] - df_sat_time['UL_Throughput_var'].fillna(0)**0.5)/1000.0,
                                    (df_sat_time['UL_Throughput_mean'] + df_sat_time['UL_Throughput_var'].fillna(0)**0.5)/1000.0, color='blue', alpha=0.2)
            
            axes[0].plot(t, df_sat_time['DL_Throughput_mean'] / 1000.0, label='DL Mean (Mbps)', color='orange')
            axes[0].fill_between(t, (df_sat_time['DL_Throughput_mean'] - df_sat_time['DL_Throughput_var'].fillna(0)**0.5)/1000.0,
                                    (df_sat_time['DL_Throughput_mean'] + df_sat_time['DL_Throughput_var'].fillna(0)**0.5)/1000.0, color='orange', alpha=0.2)
            
            axes[0].set_ylabel('Throughput (Mbps)')
            axes[0].set_title(f'Satellite {int(sat_id)} Average Throughput')
            axes[0].legend()
            axes[0].grid(True)
            
            # Delay
            axes[1].plot(t, df_sat_time['UL_Delay_mean'], label='UL Delay (ms)', color='blue')
            axes[1].fill_between(t, df_sat_time['UL_Delay_mean'] - df_sat_time['UL_Delay_var'].fillna(0)**0.5,
                                    df_sat_time['UL_Delay_mean'] + df_sat_time['UL_Delay_var'].fillna(0)**0.5, color='blue', alpha=0.2)
            
            axes[1].plot(t, df_sat_time['DL_Delay_mean'], label='DL Delay (ms)', color='orange')
            axes[1].fill_between(t, df_sat_time['DL_Delay_mean'] - df_sat_time['DL_Delay_var'].fillna(0)**0.5,
                                    df_sat_time['DL_Delay_mean'] + df_sat_time['DL_Delay_var'].fillna(0)**0.5, color='orange', alpha=0.2)
                                    
            axes[1].set_ylabel('Delay (ms)')
            axes[1].set_title(f'Satellite {int(sat_id)} Average Delay')
            axes[1].legend()
            axes[1].grid(True)
            
            # Users Count
            axes[2].plot(t, df_sat_time['UEId_nunique'], label='Connected UEs', color='purple', drawstyle='steps-post')
            axes[2].set_ylabel('Number of UEs')
            axes[2].set_xlabel('Time (s)')
            axes[2].set_title(f'Satellite {int(sat_id)} Connected UEs')
            axes[2].legend()
            axes[2].grid(True)

            # Custom labels
            if not df_custom_labels.empty:
                for _, cl_row in df_custom_labels.iterrows():
                    ct = cl_row['Time']
                    clbl = cl_row['Label']
                    for ax in axes:
                        ax.axvline(x=ct, color='purple', linestyle=':', alpha=0.8)
                    axes[0].text(ct + 0.05, axes[0].get_ylim()[1] * 0.9, clbl, color='purple', rotation=90, verticalalignment='top', fontsize=8)
            
            with warnings.catch_warnings():
                warnings.simplefilter("ignore", UserWarning)
                plt.tight_layout()
            plt.savefig(os.path.join(out_dir, f'satellite_experience_node_{int(sat_id)}.png'))
            plt.close()


    print(f"User experience graphs generated successfully in {out_dir}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Plot User ISL Experience')
    parser.add_argument('results_dir', type=str, help='Path to the results directory')
    args = parser.parse_args()
    plot_user_isl_experience(args.results_dir)
