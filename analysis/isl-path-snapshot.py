#!/usr/bin/env python3
"""Static snapshots of the ISL route a UE's traffic takes to reach the core.

For one or more instants of simulated time this draws the constellation, the UE,
the radio link to its serving satellite and the hop-by-hop inter-satellite route
from that satellite down to the ground station that fronts the core network.

It complements `leo-geo-view.py`, which renders the same scene interactively and
animated. This one writes PNGs, so the figures can go straight into a document,
and it highlights one UE's route rather than the whole routing tree.

Usage:
    isl-path-snapshot.py <results_dir> [--times 5 12 20] [--ue 20]

With no --times the script picks the instants at which the served UE's route
changes shape, which are the ones worth illustrating.
"""

import argparse
import os
import re
import sys

import numpy as np
import pandas as pd

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D

EARTH_RADIUS_M = 6.371e6

# Scenario::UpdateIslDelay (src/scenario-link.cc) writes this delay, in seconds,
# for a satellite with no route to any ground station.
ISL_NO_PATH_DELAY_S = 3600.0

# Categorical slots 1-3 of the reference palette, plus recessive ink. Only three
# identities are ever on screen at once (route / service link / everything else),
# which is the all-pairs-safe cap for that palette.
C_ROUTE = '#2a78d6'      # the highlighted UE -> core path
C_UE = '#eb6834'         # the user terminal and its radio link
C_GS = '#1baf7a'         # ground stations
C_TREE = '#b9b8b0'       # the rest of the routing tree, recessive
C_SAT = '#8a8982'
C_EARTH = '#e6e5dd'
C_GRAT = '#c9c8bf'
C_INK = '#0b0b0b'
C_INK2 = '#52514e'
SURFACE = '#fcfcfb'

# [Node_7;3.6e+06m;0.0120886s] or [Ground;400000m;0.00133426s]
HOP_RE = re.compile(r'\[([^;\]]+);([-0-9.eE+]+)m;([-0-9.eE+]+)s\]')


def read_csv(path, **kwargs):
    if not os.path.exists(path) or os.path.getsize(path) == 0:
        return pd.DataFrame()
    df = pd.read_csv(path, **kwargs)
    if df.columns.inferred_type == 'string':
        df.columns = df.columns.str.strip()
    return df


def load_run(folder):
    run = {
        'sat': read_csv(os.path.join(folder, 'leo-sat-trace.csv')),
        'vehicle': read_csv(os.path.join(folder, 'vehicle-trace.csv')),
        'isl': read_csv(os.path.join(folder, 'isl-delay-trace.csv')),
        'rrc': read_csv(os.path.join(folder, 'ue-rrc-events.csv')),
        'roles': read_csv(os.path.join(folder, 'node-roles.csv')),
    }
    if run['isl'].empty:
        sys.exit(f'error: no isl-delay-trace.csv in {folder}. '
                 'The scenario needs islDelayMode with "updateLog": true.')
    run['isl']['GNbNodeId'] = run['isl']['GNbNodeId'].astype(int)
    # Every satellite logs a row per cycle; duplicates appear when a node owns
    # more than one gNB device, and they would otherwise draw the same hop twice.
    run['isl'] = run['isl'].drop_duplicates(subset=['Time', 'GNbNodeId'])
    for key in ('sat', 'vehicle'):
        if not run[key].empty:
            run[key]['Node'] = run[key]['Node'].astype(int)
    return run


def cell_to_node_map(roles):
    """NR cell id -> owning node id, from node-roles.csv."""
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

    Same reconstruction as leo-geo-view.py: a UE is served from an Attach or a
    completed handover until the next handover starts.
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


def serving_sat(intervals, ue, time):
    for u, sat, start, end in intervals:
        if u == ue and start <= time < end:
            return sat
    return None


def ecef_from_latlon(lat_deg, lon_deg, alt_m=0.0):
    lat, lon = np.radians(lat_deg), np.radians(lon_deg)
    r = EARTH_RADIUS_M + alt_m
    return np.array([r * np.cos(lat) * np.cos(lon),
                     r * np.cos(lat) * np.sin(lon),
                     r * np.sin(lat)])


def local_up(point):
    """Unit vector away from the Earth's centre at `point` — the local vertical.

    Label offsets have to follow this rather than the ECEF z axis, which only
    points 'up' for a node near a pole.
    """
    return np.asarray(point, dtype=float) / np.linalg.norm(point)


def local_east(point):
    """Unit vector pointing east at `point`, in its local tangent plane."""
    east = np.cross([0.0, 0.0, 1.0], local_up(point))
    norm = np.linalg.norm(east)
    return east / norm if norm > 0 else np.array([1.0, 0.0, 0.0])


def latlon_from_ecef(xyz):
    x, y, z = xyz
    r = np.sqrt(x * x + y * y + z * z)
    return np.degrees(np.arcsin(z / r)), np.degrees(np.arctan2(y, x))


def parse_ground_station(coords):
    try:
        lat, lon = (float(v) for v in str(coords).split(','))
    except (ValueError, TypeError):
        return None
    return ecef_from_latlon(lat, lon)


def parse_hops(path):
    """"[Node_7;..m;..s] -> [Ground;..m;..s]" -> [(name, metres, seconds), ...]."""
    if not isinstance(path, str):
        return []
    return [(name, float(dist), float(delay))
            for name, dist, delay in HOP_RE.findall(path)]


def route_points(row, sat_positions):
    """The full polyline of one satellite's route: itself -> hops -> ground.

    Returns (points, labels, hops). `points` is an (n, 3) ECEF array, `labels`
    names each vertex, `hops` is the parsed per-hop (name, distance, delay).
    """
    start = np.array([row.SatX, row.SatY, row.SatZ], dtype=float)
    points, labels = [start], [f'Sat {int(row.GNbNodeId)}']
    hops = parse_hops(row.NextHopPath)
    for name, _dist, _delay in hops:
        if name == 'Ground':
            xyz = parse_ground_station(row.GsCoords)
            label = 'Ground station'
        elif name.startswith('Node_'):
            xyz = sat_positions.get(int(name.split('_')[1]))
            label = f'Sat {name.split("_")[1]}'
        else:
            xyz = None
            label = name
        if xyz is None:
            break
        points.append(np.asarray(xyz, dtype=float))
        labels.append(label)
    return np.array(points), labels, hops


def tree_segments(frame, sat_positions):
    """First hop of every attached satellite: the routing tree as a whole."""
    segments = []
    attached = frame[frame['Attached'].astype(str).str.lower().isin(['true', 'yes'])]
    for row in attached.itertuples():
        hops = parse_hops(row.NextHopPath)
        if not hops:
            continue
        name = hops[0][0]
        if name == 'Ground':
            dst = parse_ground_station(row.GsCoords)
        elif name.startswith('Node_'):
            dst = sat_positions.get(int(name.split('_')[1]))
        else:
            dst = None
        if dst is None:
            continue
        segments.append((np.array([row.SatX, row.SatY, row.SatZ], dtype=float),
                         np.asarray(dst, dtype=float)))
    return segments


def hop_count(path):
    """Number of inter-satellite hops before the ground (0 = direct downlink)."""
    hops = parse_hops(path)
    return max(len(hops) - 1, 0)


def interesting_times(isl, intervals, ue, limit=3):
    """Instants worth drawing: the UE's first served sample, then each change in
    the shape of its route (different hop count or different serving satellite)."""
    times = np.sort(isl['Time'].unique())
    picked, previous = [], None
    for t in times:
        sat = serving_sat(intervals, ue, t)
        if sat is None:
            continue
        row = isl[(isl['Time'] == t) & (isl['GNbNodeId'] == sat)]
        if row.empty:
            continue
        signature = (sat, hop_count(row.iloc[0]['NextHopPath']))
        if signature != previous:
            picked.append(float(t))
            previous = signature
    if len(picked) > limit:
        # Keep the first and last, spread the rest evenly between them.
        idx = np.linspace(0, len(picked) - 1, limit).round().astype(int)
        picked = [picked[i] for i in dict.fromkeys(idx)]
    return picked


def visible(points, camera):
    """Mask of points on the near side of the globe, seen from `camera`.

    matplotlib's 3D axes paint back-to-front per artist rather than per fragment,
    so anything behind the Earth surface is drawn over it. Culling the far
    hemisphere ourselves is what keeps the scene readable.
    """
    points = np.atleast_2d(points)
    return points @ camera > EARTH_RADIUS_M * 0.02


def camera_vector(elev, azim):
    e, a = np.radians(elev), np.radians(azim)
    return np.array([np.cos(e) * np.cos(a), np.cos(e) * np.sin(a), np.sin(e)])


def draw_globe(ax, camera):
    """Near-side hemisphere as a solid surface, with a graticule for orientation."""
    u, v = np.mgrid[0:2 * np.pi:90j, 0:np.pi:45j]
    r = EARTH_RADIUS_M * 0.999
    x, y, z = r * np.cos(u) * np.sin(v), r * np.sin(u) * np.sin(v), r * np.cos(v)
    # shade=False: the light source is fixed while the camera is not, so shading
    # would darken a different part of the globe in every snapshot.
    ax.plot_surface(x, y, z, color=C_EARTH, shade=False, linewidth=0,
                    antialiased=True, alpha=1.0, zorder=0)
    for lat in range(-60, 90, 30):
        lon = np.linspace(-180, 180, 361)
        pts = np.array([ecef_from_latlon(lat, l, 4e4) for l in lon])
        pts = pts[visible(pts, camera)]
        if len(pts) > 1:
            ax.plot(pts[:, 0], pts[:, 1], pts[:, 2], color=C_GRAT, lw=0.6, zorder=1)
    for lon in range(-180, 180, 30):
        lat = np.linspace(-85, 85, 171)
        pts = np.array([ecef_from_latlon(l, lon, 4e4) for l in lat])
        pts = pts[visible(pts, camera)]
        if len(pts) > 1:
            ax.plot(pts[:, 0], pts[:, 1], pts[:, 2], color=C_GRAT, lw=0.6, zorder=1)


def draw_snapshot(ax, time, frame, sat_positions, ue_pos, ue_id, sat_id, route, labels):
    """One 3D view, oriented so that the highlighted route faces the viewer."""
    lat, lon = latlon_from_ecef(route.mean(axis=0) if len(route) else ue_pos)
    elev, azim = np.clip(lat, -60, 60), lon
    camera = camera_vector(elev, azim)
    ax.view_init(elev=elev, azim=azim)

    draw_globe(ax, camera)

    # The rest of the routing tree, recessive.
    for src, dst in tree_segments(frame, sat_positions):
        seg = np.array([src, dst])
        if not visible(seg, camera).all():
            continue
        ax.plot(seg[:, 0], seg[:, 1], seg[:, 2], color=C_TREE, lw=0.9,
                zorder=2, alpha=0.9)

    sats = np.array(list(sat_positions.values()))
    if len(sats):
        near = sats[visible(sats, camera)]
        if len(near):
            ax.scatter(near[:, 0], near[:, 1], near[:, 2], s=14, c=C_SAT,
                       depthshade=False, zorder=3)

    # Ground stations.
    gs = {c: parse_ground_station(c) for c in frame['GsCoords'].dropna().unique()}
    gs_pts = np.array([p for p in gs.values() if p is not None])
    if len(gs_pts):
        near = gs_pts[visible(gs_pts, camera)]
        if len(near):
            ax.scatter(near[:, 0], near[:, 1], near[:, 2], s=110, c=C_GS,
                       marker='D', depthshade=False, zorder=9,
                       edgecolors=SURFACE, linewidths=1.4)

    # The UE and its radio link to the serving satellite.
    ax.scatter(*ue_pos, s=130, c=C_UE, marker='^', depthshade=False, zorder=11,
               edgecolors=SURFACE, linewidths=1.2)
    if len(route):
        link = np.array([ue_pos, route[0]])
        ax.plot(link[:, 0], link[:, 1], link[:, 2], color=C_UE, lw=2.2,
                zorder=6, solid_capstyle='round')

    # The highlighted route to the core.
    if len(route) > 1:
        ax.plot(route[:, 0], route[:, 1], route[:, 2], color=C_ROUTE, lw=2.6,
                zorder=6, solid_capstyle='round')
        ax.scatter(route[:-1, 0], route[:-1, 1], route[:-1, 2], s=55, c=C_ROUTE,
                   depthshade=False, zorder=7, edgecolors=SURFACE, linewidths=0.8)
        # Alternate the offset so consecutive hop labels do not collide when the
        # route is drawn nearly edge-on.
        for index, (point, label) in enumerate(zip(route[:-1], labels[:-1])):
            # The last satellite is the one over the gateway; its label always
            # goes outward so it clears the ground station marker below it.
            last = index == len(route) - 2
            # The serving satellite and the one over the gateway both share
            # their patch of sky with another marker, so their labels go further
            # out than the intermediate hops need to.
            if last or index == 0:
                offset = 5.5e5
            else:
                offset = 3.4e5 if index % 2 == 0 else -4.4e5
            anchor = point + local_up(point) * offset
            ax.text(anchor[0], anchor[1], anchor[2], label, color=C_INK,
                    fontsize=8, ha='center', zorder=8,
                    bbox=dict(facecolor=SURFACE, edgecolor='none', pad=0.8, alpha=0.75))
        # The gateway sits almost under the last satellite, so its label is
        # offset sideways in the local tangent plane rather than up or down,
        # where it would land on that satellite's label.
        ground = route[-1]
        anchor = route[-1] + local_east(route[-1]) * -8.0e5
        ax.text(anchor[0], anchor[1], anchor[2], labels[-1], color=C_GS,
                fontsize=8, ha='center', zorder=10,
                bbox=dict(facecolor=SURFACE, edgecolor='none', pad=0.8, alpha=0.75))
    # The terminal sits directly under its serving satellite, so its label goes
    # sideways rather than up or down, where that satellite's label already is.
    ue_label = ue_pos + local_east(ue_pos) * -7.0e5
    ax.text(ue_label[0], ue_label[1], ue_label[2], 'UE', color=C_UE,
            fontsize=8, ha='center', zorder=12,
            bbox=dict(facecolor=SURFACE, edgecolor='none', pad=0.8, alpha=0.75))

    span = EARTH_RADIUS_M * 1.10
    centre = camera * EARTH_RADIUS_M * 0.30
    for setter, c in ((ax.set_xlim, centre[0]), (ax.set_ylim, centre[1]),
                      (ax.set_zlim, centre[2])):
        setter(c - span, c + span)
    ax.set_box_aspect((1, 1, 1), zoom=1.5)
    ax.set_axis_off()
    ax.set_facecolor(SURFACE)


def route_caption(row, hops):
    """Plain-language summary of what the route costs."""
    isl_hops = max(len(hops) - 1, 0)
    total_ms = float(row.TotalDelay) * 1000.0
    if float(row.TotalDelay) >= ISL_NO_PATH_DELAY_S:
        return 'No route to any ground station'
    prop_ms = sum(h[2] for h in hops) * 1000.0
    extra_ms = total_ms - prop_ms
    span = ' + '.join(f'{h[1] / 1000.0:.0f}' for h in hops)
    plural = '' if isl_hops == 1 else 's'
    line = (f'Route: {isl_hops} inter-satellite hop{plural}, then the downlink\n'
            f'Hop lengths: {span} [km]\n'
            f'Propagation {prop_ms:.2f} [ms]')
    if abs(extra_ms) > 1e-3:
        line += f' + {extra_ms:.2f} [ms] allowance'
    return line + f'\nBackhaul delay applied: {total_ms:.2f} [ms]'


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('results_dir')
    parser.add_argument('--ue', type=int,
                        help='UE node id to follow (default: the first vehicle)')
    parser.add_argument('--times', type=float, nargs='+',
                        help='simulated seconds to draw (default: when the route changes)')
    parser.add_argument('--max-auto', type=int, default=4,
                        help='how many instants to pick automatically (default 4)')
    parser.add_argument('--out', help='output directory (default: <results_dir>/graphs)')
    parser.add_argument('--dpi', type=int, default=200)
    args = parser.parse_args(argv)

    run = load_run(args.results_dir)
    out_dir = args.out or os.path.join(args.results_dir, 'graphs')
    os.makedirs(out_dir, exist_ok=True)

    intervals = service_intervals(run['rrc'], cell_to_node_map(run['roles']))
    if not intervals:
        sys.exit('error: no RRC attach events, so no UE is served by any satellite.')

    ue = args.ue
    if ue is None:
        served = {u for u, _, _, _ in intervals}
        ue = min(served)
    print(f'following UE node {ue}')

    times = args.times or interesting_times(run['isl'], intervals, ue, args.max_auto)
    if not times:
        sys.exit(f'error: UE {ue} is never attached to a satellite in this run.')

    isl_times = np.sort(run['isl']['Time'].unique())
    written = []
    for requested in times:
        time = float(isl_times[np.argmin(np.abs(isl_times - requested))])
        frame = run['isl'][run['isl']['Time'] == time]
        sat_positions = {int(r.GNbNodeId): np.array([r.SatX, r.SatY, r.SatZ], dtype=float)
                         for r in frame.itertuples()}

        sat = serving_sat(intervals, ue, time)
        if sat is None:
            print(f'  t = {time:.2f} s: UE {ue} is not attached, skipping')
            continue
        sat_row = frame[frame['GNbNodeId'] == sat]
        if sat_row.empty:
            print(f'  t = {time:.2f} s: no ISL record for satellite {sat}, skipping')
            continue
        sat_row = sat_row.iloc[0]

        veh = run['vehicle'][run['vehicle']['Node'] == ue]
        veh = veh.iloc[(veh['Time'] - time).abs().argsort()[:1]]
        if veh.empty:
            print(f'  t = {time:.2f} s: no position for UE {ue}, skipping')
            continue
        ue_pos = veh[['X', 'Y', 'Z']].to_numpy(dtype=float)[0]

        route, labels, hops = route_points(sat_row, sat_positions)

        fig = plt.figure(figsize=(7.4, 7.9), facecolor=SURFACE)
        ax = fig.add_axes([0.0, 0.225, 1.0, 0.72], projection='3d',
                          computed_zorder=False)
        draw_snapshot(ax, time, frame, sat_positions, ue_pos, ue, sat, route, labels)

        fig.text(0.5, 0.975, 'UE reaches the core through the constellation'
                             f'  —  t = {time:.2f} [s]',
                 color=C_INK, fontsize=13, ha='center', va='top')
        fig.text(0.5, 0.215, route_caption(sat_row, hops), color=C_INK2,
                 fontsize=9.5, ha='center', va='top', linespacing=1.5)

        handles = [
            Line2D([], [], color=C_UE, marker='^', lw=2.2, markersize=8,
                   label='UE and its radio link'),
            Line2D([], [], color=C_ROUTE, marker='o', lw=2.6, markersize=6,
                   label=f'Route from satellite {sat} to the ground'),
            Line2D([], [], color=C_GS, marker='D', lw=0, markersize=7,
                   label='Ground station (core network)'),
            Line2D([], [], color=C_TREE, lw=0.9,
                   label='Routing tree of the other satellites'),
            Line2D([], [], color=C_SAT, marker='o', lw=0, markersize=5,
                   label='Satellite'),
        ]
        fig.legend(handles=handles, loc='lower center', ncol=2, frameon=True,
                   facecolor='white', edgecolor=C_GRAT, framealpha=1.0,
                   borderpad=0.8, fontsize=8.5, labelcolor=C_INK2,
                   bbox_to_anchor=(0.5, 0.005))

        path = os.path.join(out_dir, f'isl-path-ue{ue}-t{time:07.3f}.png')
        fig.savefig(path, dpi=args.dpi, facecolor=SURFACE)
        plt.close(fig)
        written.append(path)
        print(f'  t = {time:6.2f} s  sat {sat}  '
              f'{max(len(hops) - 1, 0)} ISL hops  '
              f'{float(sat_row.TotalDelay) * 1000:.2f} ms  ->  {path}')

    if not written:
        sys.exit('error: nothing could be drawn.')


if __name__ == '__main__':
    main()
