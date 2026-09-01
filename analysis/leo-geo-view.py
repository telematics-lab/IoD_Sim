#!/usr/bin/env python3
"""Interactive 3D viewer for LEO / NR simulation results.

Renders, in ECEF coordinates, the satellite constellation, the ground vehicles
(UEs), the ISL routing tree, the UE-to-satellite service links and, when
present, the Radio Environment Map. Everything is animated over simulated time.

Usage:
    leo-geo-view.py [results_dir] [options]

With no results_dir the script lists the runs under ../results and asks which
one to open.
"""

import argparse
import os
import re
import sys
import webbrowser

import numpy as np
import pandas as pd
import plotly.graph_objects as go

# Mean Earth radius. Node positions in the traces are ECEF metres.
EARTH_RADIUS_M = 6.371e6

# Scenario::UpdateIslDelay (src/scenario-link.cc) writes this delay, in seconds,
# for a satellite with no route to any ground station.
ISL_NO_PATH_DELAY_S = 3600.0

TRACE_FILES = {
    'sat': 'leo-sat-trace.csv',
    'vehicle': 'vehicle-trace.csv',
    'isl': 'isl-delay-trace.csv',
    'rrc': 'ue-rrc-events.csv',
    'roles': 'node-roles.csv',
}

# Up to this many nodes each gets its own legend entry, so a single node can be
# isolated. Past it the layer collapses into one trace coloured by node, which
# keeps the legend and plotly's hit-testing usable.
MAX_LEGEND_NODES = 12

# Trails are per-frame data: every frame carries x, y, z and a colour index for
# each node's recent history, so the cost is nodes x trail x frames x 4 values.
# Roughly 3 MB of page at the ceiling below.
TRAIL_VALUE_BUDGET = 600000
TRAIL_ARRAYS = 4
MIN_USEFUL_TRAIL = 4

# Above this many path points the 'auto' view opens on the animation alone.
PATHS_WITH_ANIMATION = 12000

# Distinguishable at small marker sizes on a dark background.
NODE_PALETTE = [
    '#4c9aff', '#ff8b3d', '#3ddc84', '#ff5c8a', '#b388ff',
    '#ffd400', '#00d4c8', '#ff6b6b', '#8fd14f', '#c77dff',
    '#5ac8fa', '#ffa94d',
]


def discrete_colorscale(n):
    """Plotly colorscale that maps the integer 0..n-1 to NODE_PALETTE[i].

    scatter3d only accepts a numeric colour array for lines, so per-node line
    colouring has to go through a colorscale rather than colour names.
    """
    scale = []
    for i in range(n):
        colour = NODE_PALETTE[i % len(NODE_PALETTE)]
        scale.append([i / n, colour])
        scale.append([(i + 1) / n, colour])
    return scale


def layer_moves(df):
    """Whether any node in this layer changes position over the run."""
    if df.empty:
        return False
    return bool(df.groupby('Node')[['X', 'Y', 'Z']].nunique().to_numpy().max() > 1)


def node_colour_index(df):
    """Stable node -> palette index for one layer."""
    nodes = sorted(df['Node'].unique())
    return {node: i for i, node in enumerate(nodes)}


LAYER_STYLE = {
    'sat': dict(name='Satellites', label='Sat', color='#ff7f0e',
                symbol='circle', size=4),
    'vehicle': dict(name='Vehicles (UEs)', label='UE', color='#1f77b4',
                    symbol='square', size=5),
}


# --------------------------------------------------------------------------
# Run discovery and loading
# --------------------------------------------------------------------------

def find_rem_file(folder):
    """Path of the REM export in `folder`, if the run produced one."""
    try:
        for name in sorted(os.listdir(folder)):
            if name.startswith('nr-rem-') and name.endswith('-ecef.out'):
                return os.path.join(folder, name)
    except OSError:
        pass
    return None


def describe_run(folder):
    """(folder, description) if `folder` holds anything we can draw, else None."""
    present = []
    for key, filename in TRACE_FILES.items():
        if key in ('rrc', 'roles'):
            continue
        if os.path.exists(os.path.join(folder, filename)):
            present.append({'sat': 'satellites', 'vehicle': 'vehicles',
                            'isl': 'ISL delays'}[key])
    if find_rem_file(folder):
        present.append('REM')
    return ' + '.join(present) if present else None


def find_result_folders(results_root):
    runs = []
    if not os.path.isdir(results_root):
        return runs
    for item in sorted(os.listdir(results_root)):
        folder = os.path.join(results_root, item)
        if not os.path.isdir(folder):
            continue
        description = describe_run(folder)
        if description:
            runs.append((item, folder, description))
    return runs


def choose_folder_interactively(results_root):
    runs = find_result_folders(results_root)
    if not runs:
        print(f"No runs with drawable traces found under '{results_root}'.")
        return None

    print('\nAvailable result folders:')
    print('-' * 60)
    for i, (name, _, description) in enumerate(runs, 1):
        print(f'{i:3d}. {name}  ({description})')

    while True:
        try:
            choice = input(f'\nSelect folder (1-{len(runs)}): ').strip()
        except (KeyboardInterrupt, EOFError):
            print('\nCancelled.')
            return None
        try:
            index = int(choice) - 1
        except ValueError:
            print('Please enter a number.')
            continue
        if 0 <= index < len(runs):
            print(f'Selected: {runs[index][0]}')
            return runs[index][1]
        print(f'Please enter a number between 1 and {len(runs)}.')


def read_csv(path, **kwargs):
    if not path or not os.path.exists(path) or os.path.getsize(path) == 0:
        return pd.DataFrame()
    try:
        df = pd.read_csv(path, **kwargs)
    except (pd.errors.EmptyDataError, pd.errors.ParserError) as exc:
        print(f'  warning: could not read {os.path.basename(path)}: {exc}')
        return pd.DataFrame()
    df.columns = df.columns.str.strip()
    return df


def load_run(folder):
    """Read every trace of a run into a dict of DataFrames."""
    run = {}
    for key, filename in TRACE_FILES.items():
        run[key] = read_csv(os.path.join(folder, filename))

    rem_path = find_rem_file(folder)
    rem = read_csv(rem_path, sep='\t', header=None) if rem_path else pd.DataFrame()
    if not rem.empty:
        if rem.shape[1] == 10:
            rem.columns = ['X', 'Y', 'Z', 'SNR', 'SINR', 'RxPwr', 'SIR',
                           'Latitude', 'Longitude', 'Altitude']
        else:
            print(f'  warning: unexpected REM column count ({rem.shape[1]}), ignoring it')
            rem = pd.DataFrame()
    run['rem'] = rem

    # Node ids arrive as integers everywhere; keep them that way so that the
    # cell/node cross-references below actually match. Comparing an int64 column
    # against a str silently matched nothing in the previous version.
    for key in ('sat', 'vehicle'):
        if not run[key].empty and 'Node' in run[key].columns:
            run[key]['Node'] = run[key]['Node'].astype(int)
    if not run['isl'].empty and 'GNbNodeId' in run['isl'].columns:
        run['isl']['GNbNodeId'] = run['isl']['GNbNodeId'].astype(int)

    return run


def cell_to_node_map(roles):
    """Map NR cell ids to the node that owns them, from node-roles.csv."""
    mapping = {}
    if roles.empty or 'CellIds' not in roles.columns:
        return mapping
    for _, row in roles.dropna(subset=['CellIds']).iterrows():
        for cell in str(row['CellIds']).split():
            try:
                mapping[int(float(cell))] = int(row['NodeId'])
            except ValueError:
                pass
    return mapping


def service_intervals(rrc, cells):
    """(ue, satellite, start, end) windows during which a UE was attached.

    Rebuilt from the RRC event log: a UE is served from an Attach or a completed
    handover until the next handover starts.
    """
    intervals = []
    if rrc.empty:
        return intervals

    for ue, events in rrc.sort_values('Time').groupby('NodeId'):
        current, start = None, None
        for _, event in events.iterrows():
            kind = event['Event']
            if kind in ('Attach', 'HandoverEndOk'):
                if current is not None:
                    intervals.append((int(ue), current, start, event['Time']))
                cell = event['TargetCellId']
                current = cells.get(int(cell)) if pd.notna(cell) else None
                start = event['Time']
            elif kind == 'HandoverStart':
                if current is not None:
                    intervals.append((int(ue), current, start, event['Time']))
                current, start = None, None
        if current is not None:
            intervals.append((int(ue), current, start, np.inf))
    return intervals


# --------------------------------------------------------------------------
# Geometry helpers
# --------------------------------------------------------------------------

def ecef_from_latlon(lat_deg, lon_deg, alt_m=0.0):
    """Spherical Earth ECEF. Good enough to place ground stations on the globe."""
    lat, lon = np.radians(lat_deg), np.radians(lon_deg)
    r = EARTH_RADIUS_M + alt_m
    return (r * np.cos(lat) * np.cos(lon),
            r * np.cos(lat) * np.sin(lon),
            r * np.sin(lat))


def parse_ground_station(coords):
    """"lat,lon" as written by the ISL tracer -> ECEF, or None."""
    try:
        lat, lon = (float(v) for v in str(coords).split(','))
    except (ValueError, TypeError):
        return None
    return ecef_from_latlon(lat, lon)


def decimate(df, max_points, group_col='Node'):
    """Thin a trace to roughly `max_points` rows, keeping every node's shape.

    Sampling every k-th row per node preserves the trajectory outline while
    cutting the payload; drawing every sample of a 300-satellite constellation is
    what made the previous viewer unusable.
    """
    if df.empty or max_points <= 0 or len(df) <= max_points:
        return df, 1
    step = int(np.ceil(len(df) / max_points))
    if group_col and group_col in df.columns:
        ordered = df.sort_values([group_col, 'Time'])
        within_node = ordered.groupby(group_col).cumcount()
        thinned = ordered[within_node % step == 0]
    else:
        thinned = df.iloc[::step]
    return thinned, step


# --------------------------------------------------------------------------
# Static layers
# --------------------------------------------------------------------------

TRAJECTORY_HOVER = (
    '<b>%{customdata[0]} %{customdata[1]}</b><br>'
    't = %{customdata[2]:.3f} s<br>'
    'lat %{customdata[3]:.3f}°, lon %{customdata[4]:.3f}°<br>'
    'alt %{customdata[5]:.1f} km'
    '<extra></extra>'
)


def trajectory_customdata(df, label):
    """Per-point hover payload.

    Plotly formats these client-side from a numeric array. The previous version
    built one f-string per point in Python, which dominated both build time and
    the size of the resulting HTML.
    """
    return np.column_stack([
        np.full(len(df), label),
        df['Node'].to_numpy(),
        df['Time'].to_numpy(),
        df['Latitude'].to_numpy(),
        df['Longitude'].to_numpy(),
        df['Altitude'].to_numpy() / 1000.0,
    ])


def add_trajectories(fig, df, kind, colours, visible=True):
    """Draw the full path of every node in a layer.

    Few enough nodes and each gets its own trace, so the legend can isolate one.
    Beyond that the layer collapses into a single trace whose line colour varies
    per node: 300 separate traces make plotly's legend and hit-testing crawl,
    but a flat one-colour layer makes the nodes impossible to tell apart, which
    is what the per-node colouring restores.

    Returns the indices of the traces added, so the view-mode buttons can toggle
    them.
    """
    if df.empty:
        return []

    style = LAYER_STYLE[kind]
    label = style['label']
    df = df.sort_values(['Node', 'Time'])
    nodes = sorted(df['Node'].unique())
    indices = []

    if len(nodes) <= MAX_LEGEND_NODES:
        for node in nodes:
            node_df = df[df['Node'] == node]
            indices.append(len(fig.data))
            fig.add_trace(go.Scatter3d(
                x=node_df['X'].to_numpy(dtype=np.float32),
                y=node_df['Y'].to_numpy(dtype=np.float32),
                z=node_df['Z'].to_numpy(dtype=np.float32),
                mode='lines',
                line=dict(color=NODE_PALETTE[colours[node] % len(NODE_PALETTE)],
                          width=2),
                opacity=0.7,
                customdata=trajectory_customdata(node_df, label),
                hovertemplate=TRAJECTORY_HOVER,
                name=f'{label} {node} — path',
                legendgroup=kind,
                legendgrouptitle_text=style['name'],
                visible=True if visible else 'legendonly',
            ))
        return indices

    # A NaN vertex between nodes breaks the polyline so paths are not joined up.
    node_ids = df['Node'].to_numpy()
    gaps = np.flatnonzero(node_ids[1:] != node_ids[:-1]) + 1
    x = np.insert(df['X'].to_numpy(dtype=np.float32), gaps, np.nan)
    y = np.insert(df['Y'].to_numpy(dtype=np.float32), gaps, np.nan)
    z = np.insert(df['Z'].to_numpy(dtype=np.float32), gaps, np.nan)

    colour_idx = np.insert(
        np.array([colours[n] for n in node_ids], dtype=np.float32), gaps, np.nan)
    custom = trajectory_customdata(df, label)
    custom = np.insert(custom, gaps, custom[0], axis=0)

    indices.append(len(fig.data))
    fig.add_trace(go.Scatter3d(
        x=x, y=y, z=z,
        mode='lines',
        line=dict(color=colour_idx, colorscale=discrete_colorscale(len(nodes)),
                  cmin=0, cmax=len(nodes), width=2, showscale=False),
        opacity=0.6,
        customdata=custom,
        hovertemplate=TRAJECTORY_HOVER,
        name=f"{style['name']} — paths ({len(nodes)} nodes)",
        legendgroup=kind,
        legendgrouptitle_text=style['name'],
        visible=True if visible else 'legendonly',
    ))
    return indices


def add_rem(fig, rem, max_points):
    if rem.empty:
        return
    rem, step = decimate(rem, max_points, group_col=None)
    if step > 1:
        print(f'  REM decimated 1:{step} -> {len(rem)} points')

    custom = np.column_stack([rem['SINR'], rem['SNR'], rem['RxPwr'],
                              rem['Latitude'], rem['Longitude']])
    finite = rem['SINR'].replace([np.inf, -np.inf], np.nan).dropna()
    cmin, cmax = (float(finite.quantile(0.02)), float(finite.quantile(0.98))) \
        if not finite.empty else (-50.0, 50.0)

    fig.add_trace(go.Scatter3d(
        x=rem['X'], y=rem['Y'], z=rem['Z'],
        mode='markers',
        marker=dict(size=2.5, color=rem['SINR'], colorscale='Inferno',
                    cmin=cmin, cmax=cmax, symbol='diamond',
                    colorbar=dict(title='REM SINR (dB)', x=0.0, len=0.6)),
        customdata=custom,
        hovertemplate=('<b>REM</b><br>SINR %{customdata[0]:.2f} dB<br>'
                       'SNR %{customdata[1]:.2f} dB<br>'
                       'RxPwr %{customdata[2]:.2f} dBm<br>'
                       'lat %{customdata[3]:.4f}°, lon %{customdata[4]:.4f}°'
                       '<extra></extra>'),
        name='REM points',
        legendgroup='rem',
        legendgrouptitle_text='Radio Environment Map',
    ))


def add_earth(fig, resolution=24):
    """Opaque globe.

    Deliberately not translucent: Plotly sorts transparent surfaces per facet,
    which is both slow and prone to satellites bleeding through the far side.
    Hide it from the legend when you need to see through.
    """
    phi, theta = np.mgrid[0:np.pi:resolution * 1j, 0:2 * np.pi:resolution * 1j]
    r = EARTH_RADIUS_M - 100
    fig.add_trace(go.Surface(
        x=r * np.sin(phi) * np.cos(theta),
        y=r * np.sin(phi) * np.sin(theta),
        z=r * np.cos(phi),
        colorscale=[[0, '#123c26'], [1, '#123c26']],
        showscale=False, showlegend=True, name='Earth',
        hoverinfo='skip', opacity=1.0,
        lighting=dict(ambient=0.75, diffuse=0.5, specular=0.05),
    ))


def add_ground_stations(fig, isl):
    """Ground stations referenced by the ISL routing tree."""
    if isl.empty or 'GsCoords' not in isl.columns:
        return
    points = {}
    for coords in isl['GsCoords'].dropna().unique():
        xyz = parse_ground_station(coords)
        if xyz is not None:
            points[coords] = xyz
    if not points:
        return
    labels, xyz = zip(*points.items())
    xs, ys, zs = zip(*xyz)
    fig.add_trace(go.Scatter3d(
        x=xs, y=ys, z=zs, mode='markers',
        marker=dict(size=7, color='cyan', symbol='diamond'),
        customdata=np.array(labels).reshape(-1, 1),
        hovertemplate='<b>Ground station</b><br>%{customdata[0]}<extra></extra>',
        name='Ground stations', legendgroup='gs',
        legendgrouptitle_text='Ground stations',
    ))


# --------------------------------------------------------------------------
# Animation
# --------------------------------------------------------------------------

def positions_by_time(df):
    """{time -> frame rows}, for O(1) lookup while building frames."""
    if df.empty:
        return {}
    return {t: g for t, g in df.groupby('Time')}


def isl_segments(isl_rows, sat_positions):
    """Line segments for one ISL snapshot, plus their hover payload.

    Each satellite logs its own first hop, so the union of first hops over all
    satellites is the whole routing tree.
    """
    xs, ys, zs, custom = [], [], [], []
    if isl_rows is None or isl_rows.empty:
        return xs, ys, zs, custom

    attached = isl_rows[isl_rows['Attached'].astype(str).str.lower().isin(['true', 'yes'])]
    for row in attached.itertuples():
        if 'SatX' in attached.columns and pd.notna(row.SatX):
            src = (row.SatX, row.SatY, row.SatZ)
        else:
            src = sat_positions.get(int(row.GNbNodeId))
        if src is None:
            continue

        path = row.NextHopPath
        if not isinstance(path, str) or not path.strip():
            continue
        hop = re.match(r'\[([^;]+);', path)
        if not hop:
            continue
        target_name = hop.group(1)

        if target_name == 'Ground':
            dst = parse_ground_station(row.GsCoords)
        elif target_name.startswith('Node_'):
            try:
                dst = sat_positions.get(int(target_name.split('_')[1]))
            except (ValueError, IndexError):
                dst = None
        else:
            dst = None
        if dst is None or any(pd.isna(v) for v in dst):
            continue

        distance_km = float(np.linalg.norm(np.subtract(src, dst))) / 1000.0
        delay_ms = (np.nan if row.TotalDelay >= ISL_NO_PATH_DELAY_S
                    else row.TotalDelay * 1000.0)
        payload = [int(row.GNbNodeId), target_name, distance_km, delay_ms]

        xs.extend([src[0], dst[0], None])
        ys.extend([src[1], dst[1], None])
        zs.extend([src[2], dst[2], None])
        custom.extend([payload, payload, payload])

    return xs, ys, zs, custom


def service_segments(ue_rows, sat_positions, intervals, time):
    """UE -> serving satellite links at one instant."""
    xs, ys, zs, custom = [], [], [], []
    if ue_rows is None or ue_rows.empty or not intervals:
        return xs, ys, zs, custom

    ue_positions = {int(r.Node): (r.X, r.Y, r.Z) for r in ue_rows.itertuples()}
    for ue, sat, start, end in intervals:
        if not (start <= time < end):
            continue
        src, dst = ue_positions.get(ue), sat_positions.get(sat)
        if src is None or dst is None:
            continue
        distance_km = float(np.linalg.norm(np.subtract(src, dst))) / 1000.0
        payload = [ue, sat, distance_km]
        xs.extend([src[0], dst[0], None])
        ys.extend([src[1], dst[1], None])
        zs.extend([src[2], dst[2], None])
        custom.extend([payload, payload, payload])

    return xs, ys, zs, custom


def as_customdata(rows, width):
    """Plotly rejects a ragged/empty customdata; hand it a shaped array."""
    return np.array(rows, dtype=object) if rows else np.empty((0, width), dtype=object)


MARKER_HOVER = (
    '<b>%{customdata[0]} %{customdata[1]}</b><br>'
    'lat %{customdata[2]:.3f}°, lon %{customdata[3]:.3f}°<br>'
    'alt %{customdata[4]:.1f} km'
    '<extra></extra>'
)

ISL_HOVER = (
    '<b>ISL</b> sat %{customdata[0]} → %{customdata[1]}<br>'
    'distance %{customdata[2]:.1f} km<br>'
    'hop delay %{customdata[3]:.3f} ms'
    '<extra></extra>'
)

SERVICE_HOVER = (
    '<b>Service link</b><br>UE %{customdata[0]} → sat %{customdata[1]}<br>'
    'distance %{customdata[2]:.1f} km'
    '<extra></extra>'
)


def marker_frame(rows, kind, colours):
    """Current-position markers for one layer at one instant.

    Markers carry the same per-node colours as the trajectories, so a moving dot
    can be matched to the path it is travelling along.
    """
    style = LAYER_STYLE[kind]
    if rows is None or rows.empty:
        return dict(x=[], y=[], z=[], customdata=np.empty((0, 5), dtype=object),
                    marker_color=[])
    return dict(
        x=rows['X'].to_numpy(dtype=np.float32),
        y=rows['Y'].to_numpy(dtype=np.float32),
        z=rows['Z'].to_numpy(dtype=np.float32),
        customdata=np.column_stack([
            np.full(len(rows), style['label']),
            rows['Node'].to_numpy(),
            rows['Latitude'].to_numpy(),
            rows['Longitude'].to_numpy(),
            rows['Altitude'].to_numpy() / 1000.0,
        ]),
        marker_color=[NODE_PALETTE[colours[n] % len(NODE_PALETTE)]
                      for n in rows['Node']],
    )


def trail_frame(df, colours, time, trail, times_index):
    """The last `trail` samples of every node, up to `time`.

    A short comet tail behind each node: with only the instantaneous dot it is
    impossible to tell which way anything is going, and the full path is too much
    clutter to leave on while the animation runs.
    """
    empty = dict(x=[], y=[], z=[], line_color=[])
    if df.empty or trail <= 0:
        return empty

    upto = np.searchsorted(times_index, time, side='right')
    if upto == 0:
        return empty
    window = times_index[max(0, upto - trail):upto]
    recent = df[df['Time'].isin(window)].sort_values(['Node', 'Time'])
    if recent.empty:
        return empty

    node_ids = recent['Node'].to_numpy()
    gaps = np.flatnonzero(node_ids[1:] != node_ids[:-1]) + 1
    return dict(
        x=np.insert(recent['X'].to_numpy(dtype=np.float32), gaps, np.nan),
        y=np.insert(recent['Y'].to_numpy(dtype=np.float32), gaps, np.nan),
        z=np.insert(recent['Z'].to_numpy(dtype=np.float32), gaps, np.nan),
        line_color=np.insert(
            np.array([colours[n] for n in node_ids], dtype=np.float32),
            gaps, np.nan),
    )


def build_animation(fig, run, intervals, colours, max_frames, trail):
    """Add the animated layers and the frames that drive them.

    Only what actually moves is animated: the position markers, the two kinds of
    link and the short trails. The full trajectories stay static underneath, so
    frame payloads stay small even for a 300-satellite constellation.

    Returns (frames, trace indices by role) so the view-mode buttons can toggle
    the right traces.
    """
    sat, vehicle, isl = run['sat'], run['vehicle'], run['isl']

    times = pd.unique(pd.concat([
        sat['Time'] if not sat.empty else pd.Series(dtype=float),
        vehicle['Time'] if not vehicle.empty else pd.Series(dtype=float),
    ], ignore_index=True))
    if len(times) == 0:
        return [], {'marker': [], 'link': [], 'trail': []}
    times = np.sort(times)
    if max_frames > 0 and len(times) > max_frames:
        times = times[np.linspace(0, len(times) - 1, max_frames).astype(int)]
        print(f'  animation limited to {len(times)} frames '
              f'(use --max-frames to change)')

    sat_by_time = positions_by_time(sat)
    veh_by_time = positions_by_time(vehicle)

    # The ISL trace ticks on its own schedule: for each frame use the most recent
    # snapshot at or before it.
    isl_times = np.sort(isl['Time'].unique()) if not isl.empty else np.array([])
    isl_by_time = positions_by_time(isl)

    def snapshot(by_time, keys, t):
        if len(keys) == 0:
            return None
        idx = np.searchsorted(keys, t, side='right') - 1
        return by_time.get(keys[idx]) if idx >= 0 else None

    sat_times = np.sort(np.array(list(sat_by_time.keys()))) if sat_by_time else np.array([])
    veh_times = np.sort(np.array(list(veh_by_time.keys()))) if veh_by_time else np.array([])

    # Trails cost nodes x trail x frames. Shorten them to fit the budget rather
    # than dropping them, and skip layers that never move: a trail behind a fixed
    # ground node is payload with nothing to show.
    trails = {}
    trail_len = {}
    for kind, df in (('sat', sat), ('vehicle', vehicle)):
        if df.empty or trail <= 0:
            continue
        if not layer_moves(df):
            print(f'  {LAYER_STYLE[kind]["name"]} do not move, trail skipped')
            continue

        n_nodes = df['Node'].nunique()
        per_sample = max(1, n_nodes * len(times) * TRAIL_ARRAYS)
        length = min(trail, TRAIL_VALUE_BUDGET // per_sample)
        if length < MIN_USEFUL_TRAIL:
            print(f'  {LAYER_STYLE[kind]["name"]} trail skipped: {n_nodes} nodes '
                  f'over {len(times)} frames does not leave room for a useful '
                  f'trail (lower --max-frames, or --trail 0 to silence this)')
            continue
        if length < trail:
            print(f'  {LAYER_STYLE[kind]["name"]} trail shortened to {length} '
                  f'samples to keep the page small')

        trails[kind] = np.sort(df['Time'].unique())
        trail_len[kind] = length

    frames = []
    for t in times:
        sat_rows = snapshot(sat_by_time, sat_times, t)
        veh_rows = snapshot(veh_by_time, veh_times, t)
        sat_positions = ({int(r.Node): (r.X, r.Y, r.Z) for r in sat_rows.itertuples()}
                         if sat_rows is not None else {})

        ix, iy, iz, icustom = isl_segments(snapshot(isl_by_time, isl_times, t), sat_positions)
        sx, sy, sz, scustom = service_segments(veh_rows, sat_positions, intervals, t)

        data = [
            go.Scatter3d(**marker_frame(sat_rows, 'sat', colours['sat'])),
            go.Scatter3d(**marker_frame(veh_rows, 'vehicle', colours['vehicle'])),
            go.Scatter3d(x=ix, y=iy, z=iz, customdata=as_customdata(icustom, 4)),
            go.Scatter3d(x=sx, y=sy, z=sz, customdata=as_customdata(scustom, 3)),
        ]
        for kind in ('sat', 'vehicle'):
            if kind in trails:
                data.append(go.Scatter3d(**trail_frame(
                    sat if kind == 'sat' else vehicle,
                    colours[kind], t, trail_len[kind], trails[kind])))

        frames.append(go.Frame(name=f'{t:.3f}', data=data,
                               traces=list(range(len(data)))))

    # The animated traces themselves, seeded with the first frame.
    first = frames[0].data
    indices = {'marker': [], 'link': [], 'trail': []}

    for slot, kind in ((0, 'sat'), (1, 'vehicle')):
        style = LAYER_STYLE[kind]
        indices['marker'].append(len(fig.data))
        fig.add_trace(go.Scatter3d(
            x=first[slot].x, y=first[slot].y, z=first[slot].z,
            customdata=first[slot].customdata,
            mode='markers',
            marker=dict(color=first[slot].marker.color, size=style['size'],
                        symbol=style['symbol'],
                        line=dict(width=0.5, color='rgba(0,0,0,0.6)')),
            hovertemplate=MARKER_HOVER,
            name=f"{style['name']} — now",
            legendgroup=kind, legendgrouptitle_text=style['name'],
        ))

    indices['link'].append(len(fig.data))
    fig.add_trace(go.Scatter3d(
        x=first[2].x, y=first[2].y, z=first[2].z, customdata=first[2].customdata,
        mode='lines', line=dict(color='#ffd400', width=4),
        hovertemplate=ISL_HOVER,
        name='ISL links', legendgroup='isl', legendgrouptitle_text='Links',
    ))
    indices['link'].append(len(fig.data))
    fig.add_trace(go.Scatter3d(
        x=first[3].x, y=first[3].y, z=first[3].z, customdata=first[3].customdata,
        mode='lines', line=dict(color='#39ff88', width=2),
        opacity=0.8, hovertemplate=SERVICE_HOVER,
        name='UE service links', legendgroup='isl',
    ))

    for offset, kind in enumerate(k for k in ('sat', 'vehicle') if k in trails):
        slot = 4 + offset
        style = LAYER_STYLE[kind]
        n_nodes = (sat if kind == 'sat' else vehicle)['Node'].nunique()
        indices['trail'].append(len(fig.data))
        fig.add_trace(go.Scatter3d(
            x=first[slot].x, y=first[slot].y, z=first[slot].z,
            mode='lines',
            line=dict(color=first[slot].line.color,
                      colorscale=discrete_colorscale(n_nodes),
                      cmin=0, cmax=n_nodes, width=4, showscale=False),
            opacity=0.9, hoverinfo='skip',
            name=f"{style['name']} — trail",
            legendgroup=kind, legendgrouptitle_text=style['name'],
        ))

    return frames, indices


def view_mode_controls(n_traces, indices, has_frames):
    """Buttons switching between the animated view and the trajectory view.

    "Simulated" plays the run: nodes, links and trails move, the full paths stay
    out of the way. "Trajectories" shows every path at once for a static overview.
    "Both" overlays them. Everything not owned by a mode (Earth, REM, ground
    stations) is left visible throughout.
    """
    animated = set(indices['marker']) | set(indices['link']) | set(indices['trail'])
    paths = set(indices['path'])
    if not animated and not paths:
        return []

    def visibility(show_animated, show_paths):
        state = []
        for i in range(n_traces):
            if i in animated:
                state.append(show_animated)
            elif i in paths:
                state.append(show_paths)
            else:
                state.append(True)
        return state

    modes = [('Simulated', True, False), ('Trajectories', False, True),
             ('Both', True, True)]
    if not has_frames:
        modes = [('Trajectories', False, True), ('Both', True, True)]

    return [dict(
        type='buttons', direction='right', showactive=True,
        x=0.0, y=1.02, xanchor='left', yanchor='top',
        pad=dict(r=6, t=2),
        # Plotly highlights the active button by lightening bgcolor towards
        # white, so a light background with dark text is the only pairing that
        # stays readable in both states.
        bgcolor='#d5d5d5', bordercolor='#8a8a8a',
        font=dict(size=11, color='#1a1a1a'),
        buttons=[dict(label=label, method='restyle',
                      args=[{'visible': visibility(anim, path)}])
                 for label, anim, path in modes],
    )]


def animation_controls(frames):
    """Play/pause buttons and a time slider."""
    if not frames:
        return {}
    step_ms = 80
    return dict(
        updatemenus=[dict(
            type='buttons', direction='left', showactive=False,
            x=0.01, y=-0.02, xanchor='left', yanchor='top',
            pad=dict(r=8, t=8),
            buttons=[
                dict(label='▶ Play', method='animate',
                     args=[None, dict(frame=dict(duration=step_ms, redraw=True),
                                      fromcurrent=True,
                                      transition=dict(duration=0))]),
                dict(label='⏸ Pause', method='animate',
                     args=[[None], dict(frame=dict(duration=0, redraw=False),
                                        mode='immediate',
                                        transition=dict(duration=0))]),
            ],
        )],
        sliders=[dict(
            active=0, x=0.13, y=-0.02, len=0.84,
            xanchor='left', yanchor='top',
            currentvalue=dict(prefix='t = ', suffix=' s', font=dict(size=14)),
            pad=dict(b=10, t=8),
            steps=[dict(label=f.name, method='animate',
                        args=[[f.name], dict(mode='immediate',
                                             frame=dict(duration=0, redraw=True),
                                             transition=dict(duration=0))])
                   for f in frames],
        )],
    )


# --------------------------------------------------------------------------
# Entry point
# --------------------------------------------------------------------------

def summarise(run):
    for key, label in (('sat', 'satellites'), ('vehicle', 'vehicles')):
        df = run[key]
        if df.empty:
            continue
        print(f'  {len(df):>7d} {label} samples '
              f'({df["Node"].nunique()} nodes, {df["Time"].nunique()} time steps)')
        below = (df['Altitude'] < -0.01).sum()
        if below:
            print(f'    warning: {below} {label} samples are below the Earth surface')
    if not run['isl'].empty:
        print(f'  {len(run["isl"]):>7d} ISL trace rows')
    if not run['rem'].empty:
        print(f'  {len(run["rem"]):>7d} REM points')


def build_figure(run, args):
    cells = cell_to_node_map(run['roles'])
    intervals = service_intervals(run['rrc'], cells)
    if run['rrc'].empty:
        print('  note: no RRC events recorded, UE service links unavailable')
    elif not cells:
        print('  note: no CellIds in node-roles.csv, UE service links unavailable')

    # One colour per node, shared by that node's path, marker and trail.
    colours = {kind: (node_colour_index(run[kind]) if not run[kind].empty else {})
               for kind in ('sat', 'vehicle')}

    fig = go.Figure()

    if args.no_animation:
        frames, indices = [], {'marker': [], 'link': [], 'trail': []}
    else:
        frames, indices = build_animation(fig, run, intervals, colours,
                                          args.max_frames, args.trail)
    indices['path'] = []

    layers = {}
    for kind in ('sat', 'vehicle'):
        df = run[kind]
        if df.empty:
            continue
        thinned, step = decimate(df, args.max_points)
        if step > 1:
            print(f'  {LAYER_STYLE[kind]["name"]} paths decimated 1:{step} '
                  f'-> {len(thinned)} points')
        layers[kind] = thinned

    # 'auto' shows everything on runs light enough to redraw comfortably, and
    # falls back to the animation alone on heavy ones: plotly rebuilds every
    # trace on each frame, so leaving a huge path layer on makes the animation
    # crawl. Either way all three buttons stay available.
    view = args.view
    if view == 'auto':
        total = sum(len(df) for df in layers.values())
        view = 'simulated' if (not args.no_animation
                               and total > PATHS_WITH_ANIMATION) else 'both'
        print(f'  opening in "{view}" view ({total} path points); '
              f'switch with the buttons or --view')

    show_paths = args.no_animation or view != 'simulated'
    for kind, thinned in layers.items():
        indices['path'] += add_trajectories(fig, thinned, kind, colours[kind],
                                            visible=show_paths)

    add_ground_stations(fig, run['isl'])
    add_rem(fig, run['rem'], args.max_points)
    add_earth(fig)

    if frames:
        fig.frames = frames

    layout = dict(
        title=dict(text=f'IoD_Sim — {os.path.basename(os.path.normpath(args.results_dir))}',
                   x=0.5, xanchor='center', y=0.98, yanchor='top'),
        template='plotly_dark',
        scene=dict(
            xaxis_title='X (m)', yaxis_title='Y (m)', zaxis_title='Z (m)',
            aspectmode='data',
            xaxis=dict(showspikes=False), yaxis=dict(showspikes=False),
            zaxis=dict(showspikes=False),
        ),
        # Compact so that runs with many per-node entries still fit; the list
        # scrolls when it does not.
        legend=dict(groupclick='toggleitem', x=1.0, y=0.98,
                    xanchor='right', yanchor='top',
                    font=dict(size=10), itemsizing='constant',
                    bgcolor='rgba(0,0,0,0.45)'),
        # Room at the top for the title and the view buttons, at the bottom for
        # the transport controls, which would otherwise sit on the axis labels.
        margin=dict(r=0, b=90, l=0, t=80),
        hovermode='closest',
        showlegend=True,
    )
    layout.update(animation_controls(frames))

    menus = view_mode_controls(len(fig.data), indices, bool(frames))
    if menus:
        layout.setdefault('updatemenus', [])
        layout['updatemenus'] = list(layout['updatemenus']) + menus
    fig.update_layout(**layout)
    return fig


def main(argv=None):
    here = os.path.dirname(os.path.abspath(__file__))
    parser = argparse.ArgumentParser(
        description='Interactive 3D viewer for LEO / NR simulation results.')
    parser.add_argument('results_dir', nargs='?',
                        help='run directory; omit to pick one interactively')
    parser.add_argument('--output', help='HTML file to write '
                                         '(default: <results_dir>/leo-geo-view.html)')
    parser.add_argument('--max-points', type=int, default=20000,
                        help='point budget per static layer, 0 to disable '
                             'decimation (default: %(default)s)')
    parser.add_argument('--max-frames', type=int, default=150,
                        help='animation frame budget, 0 for every time step '
                             '(default: %(default)s)')
    parser.add_argument('--view',
                        choices=('auto', 'simulated', 'trajectories', 'both'),
                        default='auto',
                        help='which view to open on; all three stay available '
                             'as buttons (default: %(default)s)')
    parser.add_argument('--trail', type=int, default=15,
                        help='samples of recent history drawn behind each moving '
                             'node, 0 to disable (default: %(default)s)')
    parser.add_argument('--no-animation', action='store_true',
                        help='draw trajectories only, no time slider')
    parser.add_argument('--no-browser', action='store_true',
                        help='write the HTML without opening it')
    args = parser.parse_args(argv)

    if args.results_dir is None:
        args.results_dir = choose_folder_interactively(
            os.path.join(here, os.pardir, 'results'))
        if args.results_dir is None:
            return 1
    if not os.path.isdir(args.results_dir):
        parser.error(f'not a directory: {args.results_dir}')
    args.results_dir = os.path.normpath(args.results_dir)

    print(f'Loading {args.results_dir} ...')
    run = load_run(args.results_dir)
    if all(df.empty for df in run.values()):
        print('Nothing to draw: no readable traces in that directory.')
        return 1
    summarise(run)

    print('Building figure ...')
    fig = build_figure(run, args)

    output = args.output or os.path.join(args.results_dir, 'leo-geo-view.html')
    # Fill the browser window rather than plotly's default 450 px box.
    fig.write_html(output, include_plotlyjs='cdn', auto_open=False,
                   default_width='100%', default_height='98vh',
                   config=dict(displaylogo=False, scrollZoom=True,
                               # Without this plotly keeps the size it measured
                               # on first paint and leaves the window half empty.
                               responsive=True,
                               toImageButtonOptions=dict(format='png', scale=2)))
    size_mb = os.path.getsize(output) / 1e6
    print(f'Wrote {output} ({size_mb:.1f} MB)')

    if not args.no_browser:
        webbrowser.open('file://' + os.path.abspath(output))
    return 0


if __name__ == '__main__':
    sys.exit(main())
