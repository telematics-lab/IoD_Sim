import pandas as pd
import matplotlib.pyplot as plt
import os
import re
import argparse
import numpy as np
import warnings

# Scenario::UpdateIslDelay (src/scenario-link.cc) writes TotalDelay = 3600 s when
# a satellite has no route to any ground station. Anything at or above this is
# that sentinel, not a real propagation delay (real ones are in the millisecond
# range).
ISL_NO_PATH_DELAY_MS = 3.5e6

# Path descriptions that mean "this UE currently has no usable ISL route".
DISCONNECTED_PATHS = frozenset({"No Path", "Unknown"})


def group_by_cell_rnti(df, cell_col, rnti_col):
    """Map (cellId, rnti) -> positional row indices for a whole trace.

    Computed once, then reused for every UE. Scanning an 800k-row frame with one
    boolean mask per UE is what dominated this script's runtime.
    """
    if df.empty or cell_col not in df.columns or rnti_col not in df.columns:
        return {}
    return {(int(cell), int(rnti)): idx
            for (cell, rnti), idx in df.groupby([cell_col, rnti_col]).indices.items()}


def select_ue_rows(df, groups, intervals, ue_id, what, time_col='Time'):
    """Return the rows of `df` that belong to one UE.

    `intervals` holds the (start, end, cellId, rnti) windows during which the UE
    was attached; a row is kept when it matches one of those (cell, rnti) pairs
    *and* falls inside the matching window.

    With no RRC history there is nothing to attribute rows by, so the caller gets
    the frame unchanged — correct for single-UE runs. When the history is known
    but nothing matches, the previous code fell back to the *entire* frame, which
    silently drew other UEs' samples on this UE's plot. Return an empty frame and
    say so instead.
    """
    if df.empty or not intervals:
        return df

    times = df[time_col].to_numpy()
    keep = []
    for start_t, end_t, cell, rnti in intervals:
        idx = groups.get((cell, rnti))
        if idx is None:
            continue
        t = times[idx]
        keep.append(idx[(t >= start_t) & (t < end_t)])

    if not keep:
        print(f"  warning: no {what} samples match UE {ue_id}'s attached cells; "
              f"leaving that series empty")
        return df.iloc[0:0]

    return df.take(np.unique(np.concatenate(keep)))


def annotate_event(ax, x, text, color):
    """Label a vertical event line.

    The y position is given in axes coordinates, so the label stays just below
    the top of the plot no matter how the y axis is autoscaled afterwards. The
    previous code read get_ylim() before the data was final, which pushed labels
    off-screen on plots with large outliers.
    """
    ax.annotate(text,
                xy=(x, 0.98), xycoords=ax.get_xaxis_transform(),
                xytext=(3, 0), textcoords='offset points',
                color=color, rotation=90, va='top', ha='left', fontsize=8)
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

    # Indexed once here; select_ue_rows() then slices per UE instead of masking
    # the whole frame 50 times over.
    rx_groups = group_by_cell_rnti(df_rx_packet, 'cellId', 'rnti')
    data_sinr_groups = group_by_cell_rnti(df_data_sinr, 'CellId', 'RNTI')
    ctrl_sinr_groups = group_by_cell_rnti(df_ctrl_sinr, 'CellId', 'RNTI')

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

            def cell_rnti(row):
                # Kept as ints so they can key the (cellId, rnti) index built above;
                # the CSV columns come through as floats whenever a row leaves
                # SourceCellId empty.
                cell, rnti = row['TargetCellId'], row['RNTI']
                if pd.isna(cell) or pd.isna(rnti):
                    return None, None
                return int(cell), int(rnti)

            for _, row in df_ue_rrc.iterrows():
                if row['Event'] == 'Attach':
                    if current_cell is not None and current_rnti is not None:
                        active_cells_rntis.append((last_time, row['Time'], current_cell, current_rnti))
                    current_cell, current_rnti = cell_rnti(row)
                    last_time = row['Time']
                elif row['Event'] == 'HandoverStart':
                    if current_cell is not None and current_rnti is not None:
                        active_cells_rntis.append((last_time, row['Time'], current_cell, current_rnti))
                    current_cell = None
                    current_rnti = None
                elif row['Event'] == 'HandoverEndOk':
                    current_cell, current_rnti = cell_rnti(row)
                    last_time = row['Time']

            if current_cell is not None and current_rnti is not None:
                active_cells_rntis.append((last_time, float('inf'), current_cell, current_rnti))

        # Filter SINR
        df_ul_data_sinr_grouped = pd.DataFrame()
        df_dl_data_sinr_grouped = pd.DataFrame()

        if not df_rx_packet.empty:
            filtered_rx = select_ue_rows(df_rx_packet, rx_groups, active_cells_rntis,
                                         ue_id, 'RxPacketTrace')

            ul_rx = filtered_rx[filtered_rx['direction'] == 'UL']
            dl_rx = filtered_rx[filtered_rx['direction'] == 'DL']

            if not ul_rx.empty:
                df_ul_data_sinr_grouped = ul_rx.groupby('Time').agg({'SINR(dB)': 'mean'}).reset_index()
            if not dl_rx.empty:
                df_dl_data_sinr_grouped = dl_rx.groupby('Time').agg({'SINR(dB)': 'mean'}).reset_index()
                
        if df_dl_data_sinr_grouped.empty and not df_data_sinr.empty:
            filtered_data_sinr = select_ue_rows(df_data_sinr, data_sinr_groups,
                                                active_cells_rntis, ue_id, 'DlDataSinr')

            df_dl_data_sinr_grouped = filtered_data_sinr.groupby('Time').agg({'SINR(dB)': 'mean'}).reset_index()

        if not df_ctrl_sinr.empty:
            filtered_ctrl_sinr = select_ue_rows(df_ctrl_sinr, ctrl_sinr_groups,
                                                active_cells_rntis, ue_id, 'DlCtrlSinr')

            df_ctrl_sinr_grouped = filtered_ctrl_sinr.groupby('Time').agg({'SINR(dB)': 'mean'}).reset_index()
        else:
            df_ctrl_sinr_grouped = pd.DataFrame()

        # 4. Determine topology changes
        #
        # For every vehicle-trace sample we need the ISL route of the satellite
        # the UE is attached to, plus the geometry to that satellite. Both are
        # "the most recent row at or before this time, for this satellite", which
        # is exactly merge_asof; the previous per-row scans of the delay and
        # satellite traces made this the slowest part of the script.
        df_ue_veh = df_ue_veh.sort_values('Time').reset_index(drop=True)

        # Which satellite serves the UE at each sample.
        sat_ids = np.full(len(df_ue_veh), np.nan)
        veh_times = df_ue_veh['Time'].to_numpy()
        if not df_ue_rrc.empty:
            for start_t, end_t, cell, rnti in active_cells_rntis:
                node = cell_to_node.get(cell)
                if node is None:
                    continue
                sat_ids[(veh_times >= start_t) & (veh_times < end_t)] = node
        elif 'NearestSatId' in df_ue_veh.columns:
            sat_ids = df_ue_veh['NearestSatId'].to_numpy(dtype=float)
        df_ue_veh['SatId'] = sat_ids

        # merge_asof cannot group on NaN, so park unattached samples on a
        # sentinel that no satellite id can take.
        NO_SAT = -1.0
        df_ue_veh['_sat_key'] = df_ue_veh['SatId'].fillna(NO_SAT)

        # ISL route and delay of the serving satellite.
        df_ue_veh['NextHopPath'] = np.nan
        df_ue_veh['TotalDelay'] = np.nan
        if not df_delay.empty:
            delay_right = (df_delay[['Time', 'GNbNodeId', 'NextHopPath', 'TotalDelay']]
                           .dropna(subset=['Time', 'GNbNodeId'])
                           .astype({'GNbNodeId': float})
                           .sort_values('Time'))
            merged = pd.merge_asof(
                df_ue_veh[['Time', '_sat_key']],
                delay_right.rename(columns={'GNbNodeId': '_sat_key'}),
                on='Time', by='_sat_key', direction='backward')
            df_ue_veh['NextHopPath'] = merged['NextHopPath'].to_numpy()
            df_ue_veh['TotalDelay'] = merged['TotalDelay'].to_numpy()

        isl_delay_ms = df_ue_veh['TotalDelay'].to_numpy(dtype=float) * 1000.0
        has_delay_row = ~np.isnan(isl_delay_ms)
        no_route = np.isnan(isl_delay_ms) | (isl_delay_ms >= ISL_NO_PATH_DELAY_MS) \
            | df_ue_veh['NextHopPath'].isna().to_numpy()
        # The sentinel is not a delay: blank it out rather than plotting 3600 s.
        isl_delay_ms = np.where(no_route, np.nan, isl_delay_ms)
        df_ue_veh['ISL_Delay_ms'] = isl_delay_ms

        # "[Node_30;1.2m;0.1s] -> [Ground;...]" becomes "Node_30 -> Ground".
        # Cached over unique strings: a run has a handful of distinct routes.
        path_cache = {}

        def clean_route(raw):
            if raw not in path_cache:
                path_cache[raw] = " -> ".join(re.findall(r'\[([^;]+)', str(raw)))
            return path_cache[raw]

        clean_paths = np.where(
            no_route,
            np.where(has_delay_row, "No Path", "Unknown"),
            [clean_route(v) for v in df_ue_veh['NextHopPath']])

        full_paths = np.where(
            df_ue_veh['SatId'].isna().to_numpy(),
            "Unknown",
            ["Sat_%d -> %s" % (s, c) if not np.isnan(s) else "Unknown"
             for s, c in zip(df_ue_veh['SatId'], clean_paths)])

        # Transitions, skipping the first sample (nothing to compare it against).
        changed = full_paths[1:] != full_paths[:-1]
        topology_change_times = list(veh_times[1:][changed])

        was_conn = ~np.isin(clean_paths[:-1], list(DISCONNECTED_PATHS))
        is_conn = ~np.isin(clean_paths[1:], list(DISCONNECTED_PATHS))
        isl_break_times = list(veh_times[1:][was_conn & ~is_conn])
        isl_restore_times = list(veh_times[1:][~was_conn & is_conn])

        # Distance to, and elevation of, the serving satellite.
        distances = np.full(len(df_ue_veh), np.nan)
        elevations = np.full(len(df_ue_veh), np.nan)
        if not df_sat.empty:
            sat_right = (df_sat[['Time', 'Node', 'X', 'Y', 'Z']]
                         .dropna(subset=['Time', 'Node'])
                         .astype({'Node': float})
                         .sort_values('Time')
                         .rename(columns={'Node': '_sat_key',
                                          'X': 'SatX', 'Y': 'SatY', 'Z': 'SatZ'}))
            geo = pd.merge_asof(df_ue_veh[['Time', '_sat_key']], sat_right,
                                on='Time', by='_sat_key', direction='backward')

            sx = geo['SatX'].to_numpy(dtype=float)
            sy = geo['SatY'].to_numpy(dtype=float)
            sz = geo['SatZ'].to_numpy(dtype=float)
            ux = df_ue_veh['X'].to_numpy(dtype=float)
            uy = df_ue_veh['Y'].to_numpy(dtype=float)
            uz = df_ue_veh['Z'].to_numpy(dtype=float)

            vx, vy, vz = sx - ux, sy - uy, sz - uz
            v_norm = np.sqrt(vx * vx + vy * vy + vz * vz)
            u_norm = np.sqrt(ux * ux + uy * uy + uz * uz)

            distances = v_norm / 1000.0

            # Elevation above the local horizon: 90 deg minus the angle between
            # the UE's zenith (its ECEF position vector) and the satellite.
            with np.errstate(invalid='ignore', divide='ignore'):
                cos_a = (vx * ux + vy * uy + vz * uz) / (v_norm * u_norm)
                cos_a = np.clip(cos_a, -1.0, 1.0)
                elevations = 90.0 - np.degrees(np.arccos(cos_a))
            bad = (u_norm <= 0) | (v_norm <= 0) | np.isnan(sx)
            distances = np.where(bad, np.nan, distances)
            elevations = np.where(bad, np.nan, elevations)

        df_ue_veh['Distance_km'] = distances
        df_ue_veh['Elevation_deg'] = elevations

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
        def break_gaps(df, max_gap=0.5):
            """Insert NaNs where samples are more than max_gap apart, so matplotlib
            leaves a hole instead of drawing a straight line across the silence."""
            out = df.copy()
            out.loc[out['Time'].diff() > max_gap, 'SINR(dB)'] = np.nan
            return out

        has_sinr = False
        if not df_ctrl_sinr_grouped.empty:
            ctrl_plot = break_gaps(df_ctrl_sinr_grouped)
            axes[2].plot(ctrl_plot['Time'], ctrl_plot['SINR(dB)'], color='tab:gray', linestyle=':', label='DL Ctrl SINR (dB)', linewidth=1.0, alpha=0.6)
            has_sinr = True

        if not df_dl_data_sinr_grouped.empty:
            dl_plot = break_gaps(df_dl_data_sinr_grouped)
            axes[2].plot(dl_plot['Time'], dl_plot['SINR(dB)'], color='tab:green', marker='.', markersize=4, label='DL Data SINR (dB)', linewidth=2)
            has_sinr = True

        if not df_ul_data_sinr_grouped.empty:
            ul_plot = break_gaps(df_ul_data_sinr_grouped)
            axes[2].plot(ul_plot['Time'], ul_plot['SINR(dB)'], color='tab:orange', marker='.', markersize=4, label='UL Data SINR (dB)', linewidth=2)
            has_sinr = True

        if has_sinr:
            axes[2].legend(loc='lower left')
        else:
            axes[2].text(0.5, 0.5, 'SINR data not available', horizontalalignment='center', verticalalignment='center', transform=axes[2].transAxes)

        axes[2].set_ylabel('SINR (dB)')
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
            legend_elements.append(Line2D([0], [0], color='blue', linestyle='-', lw=2, label='ISL Restore'))

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

                annotate_event(axes[0], t, evt + label_suffix, color)

            # Only add legend elements if we actually plotted them
            plotted_events = set(row['Event'] for row in events_to_plot)
            if 'Attach' in plotted_events:
                legend_elements.append(Line2D([0], [0], color='green', linestyle='--', lw=2, label='Attach'))
            if 'HandoverStart' in plotted_events:
                legend_elements.append(Line2D([0], [0], color='red', linestyle='--', lw=2, label='Handover Failed/Pending'))
            if 'HandoverEndOk' in plotted_events:
                legend_elements.append(Line2D([0], [0], color='orange', linestyle='--', lw=2, label='Handover Success'))


        # Custom time labels are independent of the RRC events: plot them whether
        # or not this UE produced any.
        if not df_custom_labels.empty:
            for _, cl_row in df_custom_labels.iterrows():
                ct = cl_row['Time']
                clbl = cl_row['Label']
                for ax in axes:
                    ax.axvline(x=ct, color='purple', linestyle=':', alpha=0.8)
                annotate_event(axes[0], ct, clbl, 'purple')
            legend_elements.append(Line2D([0], [0], color='purple', linestyle=':', lw=2, label='Scenario event'))

        if legend_elements:
            axes[0].legend(handles=legend_elements, loc='upper right')



        def serving_sat(times):
            """Serving satellite id for each timestamp, NaN when unattached."""
            out = np.full(len(times), np.nan)
            if df_ue_rrc.empty:
                return out
            times = np.asarray(times, dtype=float)
            for start_t, end_t, cell, rnti in active_cells_rntis:
                node = cell_to_node.get(cell)
                if node is not None:
                    out[(times >= start_t) & (times < end_t)] = node
            return out

        for grouped, col in ((df_ul_data_sinr_grouped, 'UL_SINR'),
                             (df_dl_data_sinr_grouped, 'DL_SINR')):
            if grouped.empty:
                continue
            sats = serving_sat(grouped['Time'].to_numpy())
            keep = ~np.isnan(sats)
            if not keep.any():
                continue
            block = pd.DataFrame({'Time': grouped['Time'].to_numpy()[keep],
                                  'SatId': sats[keep],
                                  'UL_SINR': np.nan,
                                  'DL_SINR': np.nan})
            block[col] = grouped['SINR(dB)'].to_numpy()[keep]
            global_sat_sinr_stats.append(block)

        # --- Collect global satellite stats ---
        if not df_ul_grouped.empty or not df_dl_grouped.empty:
            df_merged = pd.merge(
                df_ul_grouped if not df_ul_grouped.empty else pd.DataFrame(columns=['Time_s', 'RxThroughput_kbps', 'Delay_ms']),
                df_dl_grouped if not df_dl_grouped.empty else pd.DataFrame(columns=['Time_s', 'RxThroughput_kbps', 'Delay_ms']),
                on='Time_s', how='outer', suffixes=('_UL', '_DL')
            )
            times = df_merged['Time_s'].to_numpy(dtype=float)
            if not df_ue_rrc.empty:
                sats = serving_sat(times)
            elif not df_ue_veh.empty and 'NearestSatId' in df_ue_veh.columns:
                # No RRC history: fall back to the nearest satellite recorded in
                # the vehicle trace at the closest sample in time.
                veh_t = df_ue_veh['Time'].to_numpy(dtype=float)
                nearest = np.searchsorted(veh_t, times).clip(0, len(veh_t) - 1)
                sats = df_ue_veh['NearestSatId'].to_numpy(dtype=float)[nearest]
            else:
                sats = np.full(len(times), np.nan)

            keep = ~np.isnan(sats)
            if keep.any():
                global_sat_stats.append(pd.DataFrame({
                    'Time': times[keep],
                    'SatId': sats[keep],
                    'UEId': ue_id,
                    'UL_Throughput': df_merged.get('RxThroughput_kbps_UL', np.nan).to_numpy()[keep]
                        if 'RxThroughput_kbps_UL' in df_merged else np.nan,
                    'DL_Throughput': df_merged.get('RxThroughput_kbps_DL', np.nan).to_numpy()[keep]
                        if 'RxThroughput_kbps_DL' in df_merged else np.nan,
                    'UL_Delay': df_merged['Delay_ms_UL'].to_numpy()[keep]
                        if 'Delay_ms_UL' in df_merged else np.nan,
                    'DL_Delay': df_merged['Delay_ms_DL'].to_numpy()[keep]
                        if 'Delay_ms_DL' in df_merged else np.nan,
                }))

        with warnings.catch_warnings():
            warnings.simplefilter("ignore", UserWarning)
            plt.tight_layout()
            
        plt.savefig(os.path.join(out_dir, f'user_experience_node_{ue_id}.png'))
        plt.close()


    # --- Generate per-satellite aggregate graphs ---
    df_global = pd.concat(global_sat_stats, ignore_index=True) if global_sat_stats else pd.DataFrame()
    df_global_sinr = (pd.concat(global_sat_sinr_stats, ignore_index=True)
                      if global_sat_sinr_stats else pd.DataFrame())
    if not df_global.empty:
        for sat_id, df_sat_grp in df_global.groupby('SatId'):
            fig, axes = plt.subplots(4, 1, figsize=(12, 22), sharex=True)
            
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
            
            # Throughput. The band is one standard deviation across the served
            # UEs; clipped at zero, since neither throughput nor delay can be
            # negative and an unclipped band suggests they can.
            def band(mean_col, var_col, scale=1.0):
                mean = df_sat_time[mean_col] / scale
                std = df_sat_time[var_col].fillna(0) ** 0.5 / scale
                return (mean - std).clip(lower=0), mean + std

            axes[0].plot(t, df_sat_time['UL_Throughput_mean'] / 1000.0, label='UL Mean (Mbps)', color='blue')
            axes[0].fill_between(t, *band('UL_Throughput_mean', 'UL_Throughput_var', 1000.0), color='blue', alpha=0.2)

            axes[0].plot(t, df_sat_time['DL_Throughput_mean'] / 1000.0, label='DL Mean (Mbps)', color='orange')
            axes[0].fill_between(t, *band('DL_Throughput_mean', 'DL_Throughput_var', 1000.0), color='orange', alpha=0.2)

            axes[0].set_ylabel('Throughput (Mbps)')
            axes[0].set_title(f'Satellite {int(sat_id)} Average Throughput')
            axes[0].legend()
            axes[0].grid(True)
            
            # Delay
            axes[1].plot(t, df_sat_time['UL_Delay_mean'], label='UL Delay (ms)', color='blue')
            axes[1].fill_between(t, *band('UL_Delay_mean', 'UL_Delay_var'), color='blue', alpha=0.2)

            axes[1].plot(t, df_sat_time['DL_Delay_mean'], label='DL Delay (ms)', color='orange')
            axes[1].fill_between(t, *band('DL_Delay_mean', 'DL_Delay_var'), color='orange', alpha=0.2)

            axes[1].set_ylabel('Delay (ms)')
            axes[1].set_title(f'Satellite {int(sat_id)} Average Delay')
            axes[1].legend()
            axes[1].grid(True)
            
            # Users Count
            axes[2].plot(t, df_sat_time['UEId_nunique'], label='Connected UEs', color='purple', drawstyle='steps-post')
            axes[2].set_ylabel('Number of UEs')
            axes[2].set_title(f'Satellite {int(sat_id)} Connected UEs')
            axes[2].legend()
            axes[2].grid(True)

            # SINR across every UE this satellite serves. These samples were
            # already being collected but never plotted.
            axes[3].set_ylabel('SINR (dB)')
            axes[3].set_xlabel('Time (s)')
            axes[3].set_title(f'Satellite {int(sat_id)} SINR across served UEs')
            axes[3].grid(True)

            sat_sinr = (df_global_sinr[df_global_sinr['SatId'] == sat_id]
                        if not df_global_sinr.empty else pd.DataFrame())
            if not sat_sinr.empty:
                agg = sat_sinr.groupby('Time').agg(
                    UL_mean=('UL_SINR', 'mean'), UL_min=('UL_SINR', 'min'), UL_max=('UL_SINR', 'max'),
                    DL_mean=('DL_SINR', 'mean'), DL_min=('DL_SINR', 'min'), DL_max=('DL_SINR', 'max'),
                ).reset_index()
                st = agg['Time']
                for mean, lo, hi, color, label in (
                        ('UL_mean', 'UL_min', 'UL_max', 'tab:orange', 'UL SINR (mean)'),
                        ('DL_mean', 'DL_min', 'DL_max', 'tab:green', 'DL SINR (mean)')):
                    if agg[mean].notna().any():
                        axes[3].plot(st, agg[mean], color=color, label=label, linewidth=1.5)
                        axes[3].fill_between(st, agg[lo], agg[hi], color=color, alpha=0.15,
                                             label=f'{label.split(" ")[0]} min-max')
                axes[3].legend(loc='lower left')
            else:
                axes[3].text(0.5, 0.5, 'No SINR samples attributed to this satellite',
                             ha='center', va='center', transform=axes[3].transAxes)

            # Custom labels
            if not df_custom_labels.empty:
                for _, cl_row in df_custom_labels.iterrows():
                    ct = cl_row['Time']
                    clbl = cl_row['Label']
                    for ax in axes:
                        ax.axvline(x=ct, color='purple', linestyle=':', alpha=0.8)
                    annotate_event(axes[0], ct, clbl, 'purple')
            
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
