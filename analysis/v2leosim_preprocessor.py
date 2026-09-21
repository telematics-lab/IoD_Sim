#!/usr/bin/env python3
"""Turn a LEO-NR results folder into flat text logs for external consumers.

    python3 analysis/v2leosim_preprocessor.py <results-directory>

Run with --help for the list of output files and their columns.
"""

import argparse
import os
import re
import sys
import xml.etree.ElementTree as ET

import numpy as np
import pandas as pd

# 3600 s sentinel for "no route to a ground station" (Scenario::UpdateIslDelay)
ISL_NO_PATH_DELAY_MS = 3.5e6

NA = ''  # marker for unknown/missing values in output


# --------------------------------------------------------------------------- #
# loading
# --------------------------------------------------------------------------- #

def read_table(path, sep=','):
    """Read a trace file, or an empty frame if it's absent or empty."""
    if not os.path.exists(path) or os.path.getsize(path) == 0:
        return pd.DataFrame()
    try:
        df = pd.read_csv(path, sep=sep)
    except (pd.errors.EmptyDataError, pd.errors.ParserError):
        return pd.DataFrame()
    df.columns = df.columns.str.strip()
    return df


def load_run(results_dir):
    """Load every input file this exporter knows about."""
    j = lambda name: os.path.join(results_dir, name)
    run = {
        'app': read_table(j('app-statistics-periodic.txt'), sep='\t'),
        'veh': read_table(j('vehicle-trace.csv')),
        'sat': read_table(j('leo-sat-trace.csv')),
        'roles': read_table(j('node-roles.csv')),
        'rrc': read_table(j('ue-rrc-events.csv')),
        'isl': read_table(j('isl-delay-trace.csv')),
        'rx': read_table(j('RxPacketTrace.txt'), sep='\t'),
        'dl_data_sinr': read_table(j('DlDataSinr.txt'), sep='\t'),
        'dl_ctrl_sinr': read_table(j('DlCtrlSinr.txt'), sep='\t'),
    }
    # no SINR column -> fall back to the dedicated SINR traces
    if not run['rx'].empty and 'SINR(dB)' not in run['rx'].columns:
        run['rx'] = pd.DataFrame()

    run['final'] = parse_final_stats(j('app-statistics-final.txt'))
    run['summary'] = parse_summary_xml(j('summary.xml'))
    run['config'] = j('config.json') if os.path.exists(j('config.json')) else None
    return run


def parse_final_stats(path):
    """Parse app-statistics-final.txt into {flow_id: {...}} plus a 'mean' block."""
    if not os.path.exists(path):
        return {}

    flows = {}
    means = {}
    header = re.compile(r'^Flow (\d+) \(Node (\d+) \[([^\]]+)\] -> Node (\d+) \[([^\]]+)\]\)'
                        r'\s+proto (\S+)')
    current = None
    for line in open(path):
        m = header.match(line)
        if m:
            current = {
                'flow_id': int(m.group(1)),
                'src_node': int(m.group(2)), 'src': m.group(3),
                'dst_node': int(m.group(4)), 'dst': m.group(5),
                'proto': m.group(6),
            }
            flows[current['flow_id']] = current
            continue
        if ':' not in line:
            continue
        label, _, value = line.partition(':')
        label, value = label.strip(), value.strip()
        number = re.match(r'^(-?[\d.]+)', value)
        if not number:
            continue
        number = float(number.group(1))
        if label.startswith('Mean flow') or label.startswith('Mean packet'):
            means[label] = number
        elif current is not None:
            current[label] = number
    flows['mean'] = means
    return flows


def parse_summary_xml(path):
    """Pull scenario name, run timestamp and durations out of summary.xml."""
    if not os.path.exists(path) or os.path.getsize(path) == 0:
        return {}
    try:
        root = ET.parse(path).getroot()
    except ET.ParseError:
        return {}
    out = {'scenario': root.get('scenario'), 'executed_at': root.get('executedAt')}
    duration = root.find('duration')
    if duration is not None:
        for tag, key in (('real', 'wallclock_s'), ('virtual', 'sim_duration_s')):
            node = duration.find(tag)
            if node is not None and node.text:
                out[key] = float(node.text)
    return out


# --------------------------------------------------------------------------- #
# topology
# --------------------------------------------------------------------------- #

def parse_roles(df_roles):
    """Split node-roles.csv into vehicle list, satellite list, cell map."""
    users, sats, cell_to_node, node_ips = [], [], {}, {}
    if df_roles.empty:
        return users, sats, cell_to_node, node_ips

    key_re = re.compile(r'^([A-Za-z-]+)\[(\d+)\]$')
    for _, row in df_roles.iterrows():
        key = str(row.get('KeyIndex', '')).strip()
        node_id = int(row['NodeId'])

        ips = [ip for ip in str(row.get('IpAddresses', '') or '').split()
               if ip and ip != '127.0.0.1']
        node_ips[node_id] = ips

        cells = []
        for cid in str(row.get('CellIds', '') or '').split():
            try:
                cells.append(int(float(cid)))
            except ValueError:
                pass
        for beam, cid in enumerate(cells):
            cell_to_node[cid] = (node_id, beam)

        m = key_re.match(key)
        if not m:
            continue
        group, index = m.group(1), int(m.group(2))
        entry = {'index': index, 'node_id': node_id, 'key_index': key,
                 'ipv4': ips[0] if ips else NA, 'cells': cells}
        if group == 'vehicles':
            users.append(entry)
        elif group == 'leo-sats':
            sats.append(entry)

    users.sort(key=lambda e: e['index'])
    sats.sort(key=lambda e: e['index'])
    return users, sats, cell_to_node, node_ips


def rrc_windows(df_rrc, node_id):
    """[(start, end, cell, rnti), ...] attachment windows for one UE."""
    if df_rrc.empty:
        return []

    mine = df_rrc[df_rrc['NodeId'] == node_id].sort_values('Time')
    windows = []
    cell = rnti = None
    since = 0.0

    def target(row):
        c, r = row['TargetCellId'], row['RNTI']
        if pd.isna(c) or pd.isna(r):
            return None, None
        return int(c), int(r)

    for _, row in mine.iterrows():
        event, now = row['Event'], float(row['Time'])
        if event in ('Attach', 'HandoverEndOk'):
            if cell is not None:
                windows.append((since, now, cell, rnti))
            cell, rnti = target(row)
            since = now
        elif event == 'HandoverStart':
            if cell is not None:
                windows.append((since, now, cell, rnti))
            cell = rnti = None

    if cell is not None:
        windows.append((since, float('inf'), cell, rnti))
    return windows


def serving_cell_series(windows, times):
    """Serving cell id at each time, NaN while detached/handing over."""
    cells = np.full(len(times), np.nan)
    for start, end, cell, _ in windows:
        cells[(times >= start) & (times < end)] = cell
    return cells


def select_rows(df, groups, windows, time_col='Time'):
    """Rows of a (cellId, rnti)-keyed trace that belong to one UE."""
    if df.empty or not windows:
        return df
    times = df[time_col].to_numpy()
    keep = []
    for start, end, cell, rnti in windows:
        idx = groups.get((cell, rnti))
        if idx is None:
            continue
        t = times[idx]
        keep.append(idx[(t >= start) & (t < end)])
    if not keep:
        return df.iloc[0:0]
    return df.take(np.unique(np.concatenate(keep)))


def group_by_cell_rnti(df, cell_col, rnti_col):
    if df.empty or cell_col not in df.columns or rnti_col not in df.columns:
        return {}
    return {(int(cell), int(rnti)): idx
            for (cell, rnti), idx in df.groupby([cell_col, rnti_col]).indices.items()}


# --------------------------------------------------------------------------- #
# resampling
# --------------------------------------------------------------------------- #

def detect_step(df_app, fallback=0.25):
    """Infer the sampling step from app-statistics report times."""
    if df_app.empty or 'Time_s' not in df_app.columns:
        return fallback
    times = np.unique(df_app['Time_s'].to_numpy(dtype=float))
    if len(times) < 2:
        return fallback
    return float(np.median(np.diff(times)))


def bin_mean(df, value_cols, step, grid, time_col='Time'):
    """Average `value_cols` over each [t, t+step) bin, reindexed onto `grid`."""
    out = pd.DataFrame(index=pd.Index(grid, name='time_s'))
    if df.empty:
        for col in value_cols:
            out[col] = np.nan
        return out
    bins = np.floor(df[time_col].to_numpy(dtype=float) / step + 1e-9) * step
    grouped = df.groupby(bins)[value_cols].mean()
    grouped.index = np.round(grouped.index, 9)
    return out.join(grouped.reindex(np.round(grid, 9)).set_axis(out.index))


def asof(grid, df, by_key, value_cols, key_series=None, time_col='Time',
         direction='backward'):
    """The trace row that applies at each grid point, optionally keyed per point."""
    empty = pd.DataFrame({c: np.full(len(grid), np.nan) for c in value_cols})
    if df.empty or by_key not in df.columns:
        return empty

    right = (df[[time_col, by_key] + value_cols]
             .dropna(subset=[time_col, by_key])
             .astype({by_key: float})
             .sort_values(time_col))
    if key_series is None:
        left = pd.DataFrame({time_col: grid})
        merged = pd.merge_asof(left, right.drop(columns=[by_key]),
                               on=time_col, direction=direction)
    else:
        # -1.0 sentinel so unkeyed points come back NaN, not another node's row
        left = pd.DataFrame({time_col: grid,
                             by_key: pd.Series(key_series, dtype=float).fillna(-1.0)})
        merged = pd.merge_asof(left, right, on=time_col, by=by_key, direction=direction)
    return merged[value_cols].reset_index(drop=True)


# --------------------------------------------------------------------------- #
# derived quantities
# --------------------------------------------------------------------------- #

def geometry(ue_xyz, sat_xyz):
    """Slant range (km) and elevation above the local horizon (deg)."""
    ux, uy, uz = ue_xyz
    sx, sy, sz = sat_xyz
    vx, vy, vz = sx - ux, sy - uy, sz - uz
    v = np.sqrt(vx * vx + vy * vy + vz * vz)
    u = np.sqrt(ux * ux + uy * uy + uz * uz)
    with np.errstate(invalid='ignore', divide='ignore'):
        cos_a = np.clip((vx * ux + vy * uy + vz * uz) / (v * u), -1.0, 1.0)
        elevation = 90.0 - np.degrees(np.arccos(cos_a))
    bad = (u <= 0) | (v <= 0) | np.isnan(sx)
    return np.where(bad, np.nan, v / 1000.0), np.where(bad, np.nan, elevation)


_route_cache = {}


def clean_route(raw):
    """"[Node_30;1.2m;0.1s] -> [Ground;...]" becomes "Node_30 > Ground"."""
    if raw not in _route_cache:
        hops = re.findall(r'\[([^;]+)', str(raw))
        _route_cache[raw] = ' > '.join(hops) if hops else NA
    return _route_cache[raw]


def isl_for(grid, df_isl, sat_nodes):
    """ISL route and delay of whichever satellite serves the UE at each sample."""
    n = len(grid)
    if df_isl.empty:
        return np.full(n, np.nan), np.array([NA] * n, dtype=object)

    got = asof(grid, df_isl, 'GNbNodeId', ['TotalDelay', 'NextHopPath'],
               key_series=sat_nodes)
    delay_ms = pd.to_numeric(got['TotalDelay'], errors='coerce').to_numpy() * 1000.0
    paths = got['NextHopPath'].to_numpy(dtype=object)

    # blank the no-route sentinel instead of exporting it
    no_route = np.isnan(delay_ms) | (delay_ms >= ISL_NO_PATH_DELAY_MS) | pd.isna(paths)
    routes = np.array([NA if bad else f'Sat_{int(node)} > {clean_route(path)}'
                       for bad, node, path in zip(no_route, sat_nodes, paths)],
                      dtype=object)
    return np.where(no_route, np.nan, delay_ms), routes


# --------------------------------------------------------------------------- #
# writing
# --------------------------------------------------------------------------- #

def fmt(value, digits=3):
    if value is None:
        return NA
    if isinstance(value, float) and (np.isnan(value) or np.isinf(value)):
        return NA
    if isinstance(value, (float, np.floating)):
        return f'{value:.{digits}f}'
    if isinstance(value, (bool, np.bool_)):
        return '1' if value else '0'
    return str(value)


def write_tsv(path, columns, rows, digits=None):
    """Write a tab-separated table: one header line, then data."""
    digits = digits or {}
    with open(path, 'w') as out:
        out.write('\t'.join(columns) + '\n')
        for row in rows:
            out.write('\t'.join(fmt(v, digits.get(c, 3))
                                for c, v in zip(columns, row)) + '\n')
    return path


def write_kv(path, pairs):
    with open(path, 'w') as out:
        out.write('key\tvalue\n')
        for key, value in pairs:
            out.write(f'{key}\t{fmt(value)}\n')
    return path


USER_COLUMNS = [
    'time_s',
    'lat', 'lon', 'alt_m', 'x_m', 'y_m', 'z_m', 'speed_mps',
    'state', 'serving_cell', 'serving_sat', 'sat_node',
    'distance_km', 'elevation_deg',
    'dl_tx_kbps', 'dl_rx_kbps', 'ul_tx_kbps', 'ul_rx_kbps',
    'dl_delay_ms', 'ul_delay_ms', 'dl_jitter_ms', 'ul_jitter_ms',
    'dl_loss_pct', 'ul_loss_pct', 'dl_loss_cum_pct', 'ul_loss_cum_pct',
    'dl_sinr_db', 'ul_sinr_db', 'dl_mcs', 'dl_bler',
    'isl_delay_ms', 'isl_route',
]

AGG_COLUMNS = [
    'time_s', 'users_total', 'users_attached', 'users_in_handover',
    'dl_tx_kbps', 'dl_rx_kbps', 'ul_tx_kbps', 'ul_rx_kbps',
    'mean_dl_delay_ms', 'mean_ul_delay_ms', 'mean_dl_loss_pct', 'mean_ul_loss_pct',
    'mean_dl_sinr_db', 'min_dl_sinr_db',
    'mean_elevation_deg', 'mean_isl_delay_ms',
    'active_cells', 'handovers',
]

INT_COLUMNS = {'time_s': 3, 'dl_mcs': 1}
INT_COLUMNS.update({name: 0 for name in (
    'serving_cell', 'serving_sat', 'sat_node', 'first_cell', 'from_cell', 'to_cell',
    'users_total', 'users_attached', 'users_in_handover', 'active_cells', 'handovers',
    'node_id', 'user_id', 'sat_id', 'cell_id', 'beam', 'imsi', 'rnti', 'n_beams',
    'served_users', 'cells_visited', 'samples', 'n_users', 'n_satellites', 'n_cells',
    'dl_tx_pkts', 'dl_rx_pkts', 'dl_lost_pkts', 'ul_tx_pkts', 'ul_rx_pkts',
    'ul_lost_pkts')})


def flow_series(df_app, node_id, as_source, step, grid):
    """Per-bin application KPIs for one direction of one user's traffic."""
    blank = {k: np.full(len(grid), np.nan) for k in
             ('tx_kbps', 'rx_kbps', 'delay_ms', 'jitter_ms', 'loss_pct', 'loss_cum_pct')}
    if df_app.empty:
        return blank

    key = 'SrcNodeID' if as_source else 'DstNodeID'
    mine = df_app[df_app[key] == node_id]
    if mine.empty:
        return blank

    mine = mine.copy()
    # 0 ms means "nothing arrived this interval", not "arrived instantly"
    mine.loc[mine['IntervalRxPkts'] <= 0, ['Delay_ms', 'Jitter_ms']] = np.nan
    bins = np.round(np.floor(mine['Time_s'].to_numpy(dtype=float) / step + 1e-9) * step, 9)

    summed = mine.groupby(bins).agg(
        tx_kbps=('TxThroughput_kbps', 'sum'),
        rx_kbps=('RxThroughput_kbps', 'sum'),
        delay_ms=('Delay_ms', 'mean'),
        jitter_ms=('Jitter_ms', 'mean'),
        tx_pkts=('IntervalTxPkts', 'sum'),
        rx_pkts=('IntervalRxPkts', 'sum'),
        tot_tx=('TotalTxPkts', 'sum'),
        tot_rx=('TotalRxPkts', 'sum'),
    ).reindex(np.round(grid, 9))

    tx = summed['tx_pkts'].to_numpy(dtype=float)
    rx = summed['rx_pkts'].to_numpy(dtype=float)
    with np.errstate(invalid='ignore', divide='ignore'):
        # clip at 0 so a late-arriving bin never reports negative loss
        loss = np.where(tx > 0, np.clip((tx - rx) / tx, 0.0, 1.0) * 100.0, np.nan)
        tot_tx = summed['tot_tx'].to_numpy(dtype=float)
        tot_rx = summed['tot_rx'].to_numpy(dtype=float)
        loss_cum = np.where(tot_tx > 0, np.clip((tot_tx - tot_rx) / tot_tx, 0.0, 1.0) * 100.0,
                            np.nan)

    return {'tx_kbps': summed['tx_kbps'].to_numpy(dtype=float),
            'rx_kbps': summed['rx_kbps'].to_numpy(dtype=float),
            'delay_ms': summed['delay_ms'].to_numpy(dtype=float),
            'jitter_ms': summed['jitter_ms'].to_numpy(dtype=float),
            'loss_pct': loss, 'loss_cum_pct': loss_cum}


def phy_series(run, windows, groups, step, grid, attributable=True):
    """Per-bin PHY quality for one user, attributed through its RRC windows."""
    out = {k: np.full(len(grid), np.nan)
           for k in ('dl_sinr_db', 'ul_sinr_db', 'dl_mcs', 'dl_bler')}
    if not attributable:
        return out

    if not run['rx'].empty:
        mine = select_rows(run['rx'], groups['rx'], windows)
        for direction, prefix in (('DL', 'dl'), ('UL', 'ul')):
            rows = mine[mine['direction'] == direction]
            if rows.empty:
                continue
            binned = bin_mean(rows, ['SINR(dB)'], step, grid)
            out[f'{prefix}_sinr_db'] = binned['SINR(dB)'].to_numpy(dtype=float)
            if direction == 'DL':
                extra = ['mcs'] if 'mcs' in rows.columns else []
                extra += ['corrupt'] if 'corrupt' in rows.columns else []
                if extra:
                    binned = bin_mean(rows, extra, step, grid)
                    if 'mcs' in extra:
                        out['dl_mcs'] = binned['mcs'].to_numpy(dtype=float)
                    if 'corrupt' in extra:
                        out['dl_bler'] = binned['corrupt'].to_numpy(dtype=float)
        return out

    for frame, key in (('dl_data_sinr', 'data'), ('dl_ctrl_sinr', 'ctrl')):
        df = run[frame]
        if df.empty:
            continue
        mine = select_rows(df, groups[frame], windows)
        binned = bin_mean(mine, ['SINR(dB)'], step, grid)
        values = binned['SINR(dB)'].to_numpy(dtype=float)
        # data channel is preferred; control channel only fills its gaps
        out['dl_sinr_db'] = np.where(np.isnan(out['dl_sinr_db']), values, out['dl_sinr_db'])
    return out


# position trace has ~10 m quantisation; average over 2 s to smooth it out
SPEED_WINDOW_S = 2.0


def speed_series(x, y, z, grid, window_s=SPEED_WINDOW_S):
    """Ground speed (m/s) from a centred difference over `window_s`."""
    n = len(grid)
    speed = np.full(n, np.nan)
    if n < 2:
        return speed
    step = float(grid[1] - grid[0])
    half = max(1, int(round(window_s / step / 2)))
    for i in range(n):
        lo, hi = max(0, i - half), min(n - 1, i + half)
        if hi <= lo:
            continue
        dx, dy, dz = x[hi] - x[lo], y[hi] - y[lo], z[hi] - z[lo]
        dt = float(grid[hi] - grid[lo])
        if dt > 0:
            speed[i] = np.sqrt(dx * dx + dy * dy + dz * dz) / dt
    return speed


def travelled_km(x, y, z):
    """Path length along the sampled ECEF track."""
    valid = ~(np.isnan(x) | np.isnan(y) | np.isnan(z))
    if valid.sum() < 2:
        return np.nan
    dx, dy, dz = np.diff(x[valid]), np.diff(y[valid]), np.diff(z[valid])
    return float(np.sum(np.sqrt(dx * dx + dy * dy + dz * dz)) / 1000.0)


def handover_gaps(df_rrc, node_id):
    """[(start, end)] intervals during which this UE was between cells."""
    if df_rrc.empty:
        return []
    mine = df_rrc[df_rrc['NodeId'] == node_id].sort_values('Time')
    gaps, opened = [], None
    for _, row in mine.iterrows():
        if row['Event'] == 'HandoverStart':
            opened = float(row['Time'])
        elif row['Event'] == 'HandoverEndOk' and opened is not None:
            gaps.append((opened, float(row['Time'])))
            opened = None
    if opened is not None:
        gaps.append((opened, float('inf')))
    return gaps


def build_user(run, user, cell_to_node, sat_by_node, step, grid, n_users=1):
    """Join every trace into one per-bin record set for a single vehicle."""
    node_id = user['node_id']
    windows = rrc_windows(run['rrc'], node_id)

    # position
    veh = run['veh']
    mine = veh[veh['Node'] == node_id].sort_values('Time') if not veh.empty else pd.DataFrame()
    pos_cols = ['X', 'Y', 'Z', 'Latitude', 'Longitude', 'Altitude']
    pos = asof(grid, mine, 'Node', [c for c in pos_cols if c in mine.columns],
               direction='nearest')
    for col in pos_cols:
        if col not in pos.columns:
            pos[col] = np.nan
    x, y, z = (pos[c].to_numpy(dtype=float) for c in ('X', 'Y', 'Z'))

    speed = speed_series(x, y, z, grid)

    # serving cell / satellite
    cells = serving_cell_series(windows, grid)
    sat_nodes = np.array([cell_to_node.get(int(c), (np.nan, None))[0] if not np.isnan(c) else np.nan
                          for c in cells], dtype=float)
    sat_ids = np.array([sat_by_node.get(int(n), np.nan) if not np.isnan(n) else np.nan
                        for n in sat_nodes], dtype=float)

    sat_pos = asof(grid, run['sat'], 'Node', ['X', 'Y', 'Z'], key_series=sat_nodes,
                   direction='nearest')
    distance_km, elevation_deg = geometry(
        (x, y, z),
        (sat_pos['X'].to_numpy(dtype=float),
         sat_pos['Y'].to_numpy(dtype=float),
         sat_pos['Z'].to_numpy(dtype=float)))

    gaps = handover_gaps(run['rrc'], node_id)
    in_handover = np.zeros(len(grid), dtype=bool)
    for start, end in gaps:
        in_handover |= (grid >= start) & (grid < end)
    state = np.where(in_handover, 'handover',
                     np.where(np.isnan(cells), 'detached', 'attached'))

    isl_delay_ms, isl_route = isl_for(grid, run['isl'], sat_nodes)

    # radio / app KPIs
    groups = {
        'rx': group_by_cell_rnti(run['rx'], 'cellId', 'rnti'),
        'dl_data_sinr': group_by_cell_rnti(run['dl_data_sinr'], 'CellId', 'RNTI'),
        'dl_ctrl_sinr': group_by_cell_rnti(run['dl_ctrl_sinr'], 'CellId', 'RNTI'),
    }
    phy = phy_series(run, windows, groups, step, grid,
                     attributable=bool(windows) or n_users == 1)
    dl = flow_series(run['app'], node_id, as_source=False, step=step, grid=grid)
    ul = flow_series(run['app'], node_id, as_source=True, step=step, grid=grid)

    return {
        'user': user, 'windows': windows, 'grid': grid,
        'lat': pos['Latitude'].to_numpy(dtype=float),
        'lon': pos['Longitude'].to_numpy(dtype=float),
        'alt': pos['Altitude'].to_numpy(dtype=float),
        'x': x, 'y': y, 'z': z, 'speed': speed,
        'state': state, 'cells': cells, 'sat_ids': sat_ids, 'sat_nodes': sat_nodes,
        'distance_km': distance_km, 'elevation_deg': elevation_deg,
        'isl_delay_ms': isl_delay_ms, 'isl_route': isl_route,
        'dl': dl, 'ul': ul, 'phy': phy,
    }


def user_rows(rec):
    """Flatten one user's record set into USER_COLUMNS order."""
    dl, ul, phy = rec['dl'], rec['ul'], rec['phy']
    return list(zip(
        rec['grid'], rec['lat'], rec['lon'], rec['alt'],
        rec['x'], rec['y'], rec['z'], rec['speed'],
        rec['state'], rec['cells'], rec['sat_ids'], rec['sat_nodes'],
        rec['distance_km'], rec['elevation_deg'],
        dl['tx_kbps'], dl['rx_kbps'], ul['tx_kbps'], ul['rx_kbps'],
        dl['delay_ms'], ul['delay_ms'], dl['jitter_ms'], ul['jitter_ms'],
        dl['loss_pct'], ul['loss_pct'], dl['loss_cum_pct'], ul['loss_cum_pct'],
        phy['dl_sinr_db'], phy['ul_sinr_db'], phy['dl_mcs'], phy['dl_bler'],
        rec['isl_delay_ms'], rec['isl_route'],
    ))


def aggregate_rows(records, grid, handover_times, step):
    """Network-wide view: one row per bin, summed or averaged across users."""
    n = len(grid)
    if not records:
        return []

    def stack(getter):
        return np.vstack([getter(r) for r in records])

    with warnings_suppressed():
        attached = stack(lambda r: r['state'] == 'attached').sum(axis=0)
        handing = stack(lambda r: r['state'] == 'handover').sum(axis=0)
        dl_tx = np.nansum(stack(lambda r: r['dl']['tx_kbps']), axis=0)
        dl_rx = np.nansum(stack(lambda r: r['dl']['rx_kbps']), axis=0)
        ul_tx = np.nansum(stack(lambda r: r['ul']['tx_kbps']), axis=0)
        ul_rx = np.nansum(stack(lambda r: r['ul']['rx_kbps']), axis=0)
        dl_delay = np.nanmean(stack(lambda r: r['dl']['delay_ms']), axis=0)
        ul_delay = np.nanmean(stack(lambda r: r['ul']['delay_ms']), axis=0)
        dl_loss = np.nanmean(stack(lambda r: r['dl']['loss_pct']), axis=0)
        ul_loss = np.nanmean(stack(lambda r: r['ul']['loss_pct']), axis=0)
        sinr = stack(lambda r: r['phy']['dl_sinr_db'])
        dl_sinr_mean = np.nanmean(sinr, axis=0)
        dl_sinr_min = np.nanmin(sinr, axis=0)
        elevation = np.nanmean(stack(lambda r: r['elevation_deg']), axis=0)
        isl = np.nanmean(stack(lambda r: r['isl_delay_ms']), axis=0)

    cells = stack(lambda r: r['cells'])
    active_cells = np.array([len(np.unique(col[~np.isnan(col)])) for col in cells.T])

    handovers = np.zeros(n, dtype=int)
    for t in handover_times:
        idx = int(np.floor(t / step + 1e-9))
        if 0 <= idx < n:
            handovers[idx] += 1

    total = len(records)
    return list(zip(grid, np.full(n, total), attached, handing,
                    dl_tx, dl_rx, ul_tx, ul_rx,
                    dl_delay, ul_delay, dl_loss, ul_loss,
                    dl_sinr_mean, dl_sinr_min, elevation, isl,
                    active_cells, handovers))


class warnings_suppressed:
    """np.nanmean over an all-NaN bin is a legitimate 'no data here'."""

    def __enter__(self):
        import warnings
        self._ctx = warnings.catch_warnings()
        self._ctx.__enter__()
        warnings.filterwarnings('ignore', r'.*(Mean|Min|All-NaN).*')
        return self

    def __exit__(self, *exc):
        return self._ctx.__exit__(*exc)


EVENT_NAMES = {'Attach': 'attach', 'HandoverStart': 'handover_start',
               'HandoverEndOk': 'handover_end', 'HandoverEndError': 'handover_failed',
               'ConnectionTimeout': 'connection_timeout',
               'RadioLinkFailure': 'radio_link_failure'}

EVENT_COLUMNS = ['time_s', 'user_id', 'node_id', 'event', 'from_cell', 'to_cell',
                 'rnti', 'detail']


def event_rows(records, run, users_by_node):
    """RRC lifecycle events plus each user's ISL route changes."""
    rows = []

    if not run['rrc'].empty:
        for _, r in run['rrc'].iterrows():
            node = int(r['NodeId'])
            user = users_by_node.get(node)
            if user is None:
                continue
            rows.append((float(r['Time']), user['index'], node,
                         EVENT_NAMES.get(str(r['Event']), str(r['Event']).lower()),
                         r.get('SourceCellId'), r.get('TargetCellId'), r.get('RNTI'),
                         f"imsi={int(r['IMSI'])}" if not pd.isna(r.get('IMSI')) else NA))

    for rec in records:
        routes, grid = rec['isl_route'], rec['grid']
        cells, state = rec['cells'], rec['state']
        previous = None
        started = False
        for i, route in enumerate(routes):
            # skip the pre-attach baseline; it isn't a change
            if not started:
                if state[i] != 'attached':
                    continue
                started, previous = True, route
                continue
            if route == previous:
                continue
            if previous is not None:
                if route == NA:
                    event, detail = 'isl_link_lost', 'no route to a ground station'
                elif previous == NA:
                    event, detail = 'isl_link_restored', route
                else:
                    event, detail = 'isl_route_change', route
                cell = cells[i]
                rows.append((float(grid[i]), rec['user']['index'], rec['user']['node_id'],
                             event, NA, cell, NA, detail))
            previous = route

    rows.sort(key=lambda r: (r[0], r[1]))
    return rows


SUMMARY_USER_COLUMNS = [
    'user_id', 'node_id', 'ipv4', 'imsi',
    'dl_tx_pkts', 'dl_rx_pkts', 'dl_lost_pkts', 'dl_loss_pct',
    'dl_tx_kbps', 'dl_rx_kbps', 'dl_delay_ms', 'dl_jitter_ms',
    'ul_tx_pkts', 'ul_rx_pkts', 'ul_lost_pkts', 'ul_loss_pct',
    'ul_tx_kbps', 'ul_rx_kbps', 'ul_delay_ms', 'ul_jitter_ms',
    'handovers', 'cells_visited', 'attached_pct',
    'mean_dl_sinr_db', 'min_dl_sinr_db', 'mean_elevation_deg', 'min_elevation_deg',
    'mean_isl_delay_ms', 'max_isl_delay_ms', 'distance_travelled_km',
]

FINAL_KEYS = [('Tx Packets', 'tx_pkts'), ('Rx Packets', 'rx_pkts'),
              ('Lost Packets', 'lost_pkts'), ('Packet Loss Ratio', 'loss_pct'),
              ('Tx Throughput', 'tx_kbps'), ('Rx Throughput', 'rx_kbps'),
              ('Mean delay', 'delay_ms'), ('Mean jitter', 'jitter_ms')]


def final_for(flows, node_id, as_source):
    """Sum this user's end-of-run FlowMonitor totals for one direction."""
    key = 'src_node' if as_source else 'dst_node'
    mine = [f for k, f in flows.items() if k != 'mean' and f.get(key) == node_id]
    out = {name: np.nan for _, name in FINAL_KEYS}
    if not mine:
        return out

    for label, name in FINAL_KEYS:
        values = [f.get(label, np.nan) for f in mine]
        if name in ('delay_ms', 'jitter_ms'):
            # weight by delivered packets so small flows can't skew the mean
            weights = [f.get('Rx Packets', 0.0) or 0.0 for f in mine]
            pairs = [(v, w) for v, w in zip(values, weights)
                     if w > 0 and v is not None and not np.isnan(v)]
            out[name] = (sum(v * w for v, w in pairs) / sum(w for _, w in pairs)
                         if pairs else np.nan)
        elif name == 'loss_pct':
            tx = sum(f.get('Tx Packets', 0.0) or 0.0 for f in mine)
            rx = sum(f.get('Rx Packets', 0.0) or 0.0 for f in mine)
            out[name] = 100.0 * max(tx - rx, 0.0) / tx if tx > 0 else np.nan
        else:
            clean = [v for v in values if v is not None and not np.isnan(v)]
            out[name] = float(sum(clean)) if clean else np.nan
    return out


def summary_user_rows(records, run):
    rows = []
    for rec in records:
        user = rec['user']
        node_id = user['node_id']
        dl = final_for(run['final'], node_id, as_source=False)
        ul = final_for(run['final'], node_id, as_source=True)

        handovers = 0
        if not run['rrc'].empty:
            mine = run['rrc'][run['rrc']['NodeId'] == node_id]
            handovers = int((mine['Event'] == 'HandoverEndOk').sum())

        cells = rec['cells']
        cells_visited = len(np.unique(cells[~np.isnan(cells)]))
        attached_pct = 100.0 * float(np.mean(rec['state'] == 'attached'))

        with warnings_suppressed():
            sinr = rec['phy']['dl_sinr_db']
            elevation = rec['elevation_deg']
            isl = rec['isl_delay_ms']
            stats = [np.nanmean(sinr), np.nanmin(sinr),
                     np.nanmean(elevation), np.nanmin(elevation),
                     np.nanmean(isl), np.nanmax(isl)]

        imsi = NA
        if not run['rrc'].empty:
            mine = run['rrc'][run['rrc']['NodeId'] == node_id]
            if not mine.empty and not pd.isna(mine.iloc[0].get('IMSI')):
                imsi = int(mine.iloc[0]['IMSI'])

        rows.append((user['index'], node_id, user['ipv4'], imsi,
                     dl['tx_pkts'], dl['rx_pkts'], dl['lost_pkts'], dl['loss_pct'],
                     dl['tx_kbps'], dl['rx_kbps'], dl['delay_ms'], dl['jitter_ms'],
                     ul['tx_pkts'], ul['rx_pkts'], ul['lost_pkts'], ul['loss_pct'],
                     ul['tx_kbps'], ul['rx_kbps'], ul['delay_ms'], ul['jitter_ms'],
                     handovers, cells_visited, attached_pct,
                     *stats, travelled_km(rec['x'], rec['y'], rec['z'])))
    return rows


SAT_COLUMNS = ['time_s', 'sat_id', 'node_id', 'lat', 'lon', 'alt_m',
               'x_m', 'y_m', 'z_m', 'cells', 'served_users',
               'isl_delay_ms', 'isl_route']


def satellite_rows(run, sats, records, grid, step):
    """Satellite tracks resampled onto the shared grid, with load and ISL state."""
    rows = []
    if run['sat'].empty:
        return rows

    load = {s['node_id']: np.zeros(len(grid), dtype=int) for s in sats}
    for rec in records:
        for i, node in enumerate(rec['sat_nodes']):
            if not np.isnan(node) and int(node) in load:
                load[int(node)][i] += 1

    for sat in sats:
        node_id = sat['node_id']
        mine = run['sat'][run['sat']['Node'] == node_id].sort_values('Time')
        if mine.empty:
            continue
        cols = [c for c in ('X', 'Y', 'Z', 'Latitude', 'Longitude', 'Altitude')
                if c in mine.columns]
        pos = asof(grid, mine, 'Node', cols, direction='nearest')
        keys = np.full(len(grid), float(node_id))
        isl_delay, isl_route = isl_for(grid, run['isl'], keys)
        cells = ' '.join(str(c) for c in sat['cells'])
        rows.extend(zip(grid,
                        np.full(len(grid), sat['index']),
                        np.full(len(grid), node_id),
                        pos.get('Latitude', pd.Series(np.nan, index=range(len(grid)))),
                        pos.get('Longitude', pd.Series(np.nan, index=range(len(grid)))),
                        pos.get('Altitude', pd.Series(np.nan, index=range(len(grid)))),
                        pos.get('X', pd.Series(np.nan, index=range(len(grid)))),
                        pos.get('Y', pd.Series(np.nan, index=range(len(grid)))),
                        pos.get('Z', pd.Series(np.nan, index=range(len(grid)))),
                        [cells] * len(grid),
                        load[node_id],
                        isl_delay, isl_route))
    rows.sort(key=lambda r: (r[0], r[1]))
    return rows


OUTPUT_FILES_HELP = """output files (all tab-separated, header row, no comments):
  run.txt                        run metadata (scenario, duration, counts)
  users.txt                      vehicle list: user_id, node_id, ip, imsi, first cell
  satellites.txt                 satellite list: sat_id, node_id, cells, beam count
  cells.txt                      cell_id -> satellite/beam map
  timeseries-user-<user_id>.txt  per-vehicle time series (position, radio, throughput)
  timeseries-aggregate.txt       same grid, summed/averaged across all users
  events.txt                     RRC attach/handover events + ISL route changes
  summary-users.txt              end-of-run per-user KPIs (FlowMonitor totals)
  summary-network.txt            end-of-run network roll-up

All time series share one regular grid (see --step). user_id is the stable
per-vehicle identifier; node_id is the underlying ns-3 node id. Throughput is
kbit/s, delay/jitter ms, distances km unless a column says metres.
"""


def main():
    parser = argparse.ArgumentParser(
        description='Export an IoD_Sim LEO-NR results folder as flat text logs.',
        epilog=OUTPUT_FILES_HELP,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('results_dir', help='a folder under results/, e.g. '
                                            'results/<scenario_name>-<date>.<time>')
    parser.add_argument('-o', '--out-dir', default=None,
                        help='output folder (default: <results_dir>/output)')
    parser.add_argument('--step', type=float, default=None,
                        help='time grid step in seconds '
                             '(default: the run\'s appStatisticsReportInterval)')
    parser.add_argument('--quiet', action='store_true')
    args = parser.parse_args()

    results_dir = os.path.abspath(args.results_dir)
    if not os.path.isdir(results_dir):
        parser.error(f'{results_dir} is not a folder')

    out_dir = args.out_dir or os.path.join(results_dir, 'output')
    os.makedirs(out_dir, exist_ok=True)

    say = (lambda *a: None) if args.quiet else print
    say(f'reading  {results_dir}')
    run = load_run(results_dir)
    if run['veh'].empty and run['roles'].empty:
        parser.error('no vehicle-trace.csv or node-roles.csv here -- is this a '
                     'LEO-NR results folder?')

    users, sats, cell_to_node, _ = parse_roles(run['roles'])
    if not users:
        parser.error('node-roles.csv lists no vehicles[] entries; nothing to export')
    sat_by_node = {s['node_id']: s['index'] for s in sats}
    users_by_node = {u['node_id']: u for u in users}

    step = args.step or detect_step(run['app'])
    duration = run['summary'].get('sim_duration_s')
    if duration is None:
        candidates = [df['Time'].max() for df in (run['veh'], run['sat'], run['isl'])
                      if not df.empty and 'Time' in df.columns]
        if not run['app'].empty:
            candidates.append(run['app']['Time_s'].max())
        duration = max(candidates) if candidates else 0.0
    grid = np.round(np.arange(0.0, float(duration) + step / 2, step), 9)
    say(f'grid     {len(grid)} samples of {step:g} s over {duration:g} s')

    say(f'joining  {len(users)} vehicles, {len(sats)} satellites, '
        f'{len(cell_to_node)} cells')
    records = [build_user(run, u, cell_to_node, sat_by_node, step, grid, len(users))
               for u in users]

    written = []

    def emit(name, cols, rows):
        path = write_tsv(os.path.join(out_dir, name), cols, rows, digits=INT_COLUMNS)
        written.append(path)
        say(f'  wrote  {name} ({len(rows)} rows)')
        return path

    # lists
    user_cols = ['user_id', 'node_id', 'key_index', 'ipv4', 'imsi',
                 'first_cell', 'init_lat', 'init_lon', 'init_alt_m']
    user_list = []
    for rec in records:
        u = rec['user']
        first_cell = next((c for c in rec['cells'] if not np.isnan(c)), np.nan)
        imsi = NA
        if not run['rrc'].empty:
            mine = run['rrc'][run['rrc']['NodeId'] == u['node_id']]
            if not mine.empty and not pd.isna(mine.iloc[0].get('IMSI')):
                imsi = int(mine.iloc[0]['IMSI'])
        first = next((i for i, v in enumerate(rec['lat']) if not np.isnan(v)), None)
        user_list.append((u['index'], u['node_id'], u['key_index'], u['ipv4'], imsi,
                            first_cell,
                            rec['lat'][first] if first is not None else np.nan,
                            rec['lon'][first] if first is not None else np.nan,
                            rec['alt'][first] if first is not None else np.nan))
    emit('users.txt', user_cols, user_list)

    emit('satellites.txt', ['sat_id', 'node_id', 'key_index', 'cells', 'n_beams'],
         [(s['index'], s['node_id'], s['key_index'],
           ' '.join(str(c) for c in s['cells']), len(s['cells'])) for s in sats])

    emit('cells.txt', ['cell_id', 'sat_id', 'sat_node', 'beam'],
         sorted((cid, sat_by_node.get(node, np.nan), node, beam)
                for cid, (node, beam) in cell_to_node.items()))

    # per-user and aggregate series
    for rec in records:
        emit(f"timeseries-user-{rec['user']['index']}.txt", USER_COLUMNS, user_rows(rec))

    handover_times = []
    if not run['rrc'].empty:
        done = run['rrc'][run['rrc']['Event'] == 'HandoverEndOk']
        handover_times = [float(t) for t in done['Time']]
    totals = aggregate_rows(records, grid, handover_times, step)
    emit('timeseries-aggregate.txt', AGG_COLUMNS, totals)

    emit('events.txt', EVENT_COLUMNS, event_rows(records, run, users_by_node))
    emit('summary-users.txt', SUMMARY_USER_COLUMNS, summary_user_rows(records, run))
    emit('satellite-tracks.txt', SAT_COLUMNS, satellite_rows(run, sats, records, grid, step))

    # run metadata and network roll-up
    run_meta = {
        'scenario': run['summary'].get('scenario', os.path.basename(results_dir)),
        'executed_at': run['summary'].get('executed_at', NA),
        'sim_duration_s': float(duration),
        'wallclock_s': run['summary'].get('wallclock_s'),
        'grid_step_s': step,
        'samples': len(grid),
        'n_users': len(users),
        'n_satellites': len(sats),
        'n_cells': len(cell_to_node),
        'source': os.path.basename(results_dir),
    }
    written.append(write_kv(os.path.join(out_dir, 'run.txt'), list(run_meta.items())))
    say('  wrote  run.txt')

    means = run['final'].get('mean', {}) if run['final'] else {}
    at = AGG_COLUMNS.index  # read the aggregate back by name, not by position

    def over_time(column):
        if not totals:
            return np.nan
        return float(np.nanmean([row[at(column)] for row in totals]))

    with warnings_suppressed():
        net = [
            ('handovers_total', len(handover_times)),
            ('mean_dl_throughput_kbps', over_time('dl_rx_kbps')),
            ('mean_ul_throughput_kbps', over_time('ul_rx_kbps')),
            ('mean_dl_delay_ms', over_time('mean_dl_delay_ms')),
            ('mean_ul_delay_ms', over_time('mean_ul_delay_ms')),
            ('mean_dl_sinr_db', over_time('mean_dl_sinr_db')),
            ('mean_elevation_deg', over_time('mean_elevation_deg')),
            ('mean_isl_delay_ms', over_time('mean_isl_delay_ms')),
        ]
    for label, value in means.items():
        net.append((label.lower().replace(' ', '_'), value))
    written.append(write_kv(os.path.join(out_dir, 'summary-network.txt'), net))
    say('  wrote  summary-network.txt')

    say(f'\n{len(written)} files in {out_dir}')
    return 0


if __name__ == '__main__':
    sys.exit(main())