#!/usr/bin/env python3
"""Relate the delay a user actually experiences to the ISL delay the model introduces.

Two figures, from one or more runs of an ISL scenario:

* `isl-delay-timeline-ue<N>.png` — for a single run, the end-to-end application
  delay of the UE's uplink and downlink flows next to the backhaul delay that
  the ISL routing model applied to its serving satellite, on a common
  millisecond axis, with the number of inter-satellite hops below it.

* `isl-delay-experienced-vs-introduced.png` — across several runs that differ
  only in `additionalDelay` (the per-route allowance the model introduces),
  the experienced end-to-end delay against that introduced delay.

Usage:
    plot-isl-delay-experience.py <results_dir> [<results_dir> ...] [--ue 20]

Give one directory for the timeline alone; give the directories of a sweep to
get the comparison figure as well.
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
from matplotlib.ticker import MaxNLocator

# Scenario::UpdateIslDelay (src/scenario-link.cc) writes TotalDelay = 3600 s for
# a satellite with no route to the ground. Real values are milliseconds.
ISL_NO_PATH_DELAY_S = 3600.0

# Categorical slots 1-3 of the reference palette (light mode), plus its ink and
# surface. Three series is the all-pairs-safe cap for that palette.
C_UL = '#2a78d6'
C_DL = '#eb6834'
C_ISL = '#1baf7a'
C_INK = '#0b0b0b'
C_INK2 = '#52514e'
C_GRID = '#e2e1d9'
SURFACE = '#fcfcfb'

HOP_RE = re.compile(r'\[([^;\]]+);([-0-9.eE+]+)m;([-0-9.eE+]+)s\]')


LEGEND_STYLE = dict(frameon=True, facecolor='white', edgecolor=C_GRID,
                    framealpha=1.0, borderpad=0.7, fontsize=9,
                    labelcolor=C_INK2)


def style_axes(ax):
    ax.set_facecolor(SURFACE)
    ax.grid(True, color=C_GRID, lw=0.7, zorder=0)
    ax.set_axisbelow(True)
    for side in ('top', 'right'):
        ax.spines[side].set_visible(False)
    for side in ('left', 'bottom'):
        ax.spines[side].set_color(C_GRID)
    ax.tick_params(colors=C_INK2, labelsize=9, length=0)
    ax.xaxis.label.set_color(C_INK2)
    ax.yaxis.label.set_color(C_INK2)


def read_csv(path, **kwargs):
    if not os.path.exists(path) or os.path.getsize(path) == 0:
        return pd.DataFrame()
    df = pd.read_csv(path, **kwargs)
    if df.columns.inferred_type == 'string':
        df.columns = df.columns.str.strip()
    return df


def additional_delay_ms(results_dir):
    """The `additionalDelay` the run was configured with, from its config copy.

    The config is copied verbatim, comments and all, so it is read with a regex
    rather than a JSON parser.
    """
    path = os.path.join(results_dir, 'config.json')
    if not os.path.exists(path):
        return None
    with open(path) as handle:
        text = handle.read()
    match = re.search(r'"additionalDelay"\s*:\s*"([0-9.]+)\s*(ms|s|us|ns)"', text)
    if not match:
        return None
    value, unit = float(match.group(1)), match.group(2)
    return value * {'ns': 1e-6, 'us': 1e-3, 'ms': 1.0, 's': 1e3}[unit]


def ue_node_ids(results_dir, vehicle_trace):
    roles = read_csv(os.path.join(results_dir, 'node-roles.csv'))
    if not roles.empty and 'KeyIndex' in roles.columns:
        vehicles = roles[roles['KeyIndex'].str.startswith('vehicles', na=False)]
        if not vehicles.empty:
            return sorted(vehicles['NodeId'].astype(int).unique())
    if not vehicle_trace.empty:
        return sorted(vehicle_trace['Node'].astype(int).unique())
    return []


def cell_to_node_map(roles):
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


def hop_count(path):
    """Inter-satellite hops before the downlink; 0 means a direct downlink."""
    if not isinstance(path, str):
        return np.nan
    return max(len(HOP_RE.findall(path)) - 1, 0)


def serving_isl_series(isl, intervals, ue):
    """Per ISL cycle: the delay and hop count of the UE's *serving* satellite.

    The trace holds every satellite's route; only the one the UE is attached to
    at that instant is on its path to the core.
    """
    rows = []
    isl = isl.drop_duplicates(subset=['Time', 'GNbNodeId'])
    by_time = {t: g.set_index('GNbNodeId') for t, g in isl.groupby('Time')}
    windows = [(sat, start, end) for u, sat, start, end in intervals if u == ue]
    for time, frame in sorted(by_time.items()):
        sat = next((s for s, start, end in windows if start <= time < end), None)
        if sat is None or sat not in frame.index:
            rows.append((time, np.nan, np.nan, np.nan))
            continue
        record = frame.loc[sat]
        if isinstance(record, pd.DataFrame):
            record = record.iloc[0]
        delay_s = float(record['TotalDelay'])
        delay_ms = np.nan if delay_s >= ISL_NO_PATH_DELAY_S else delay_s * 1000.0
        rows.append((time, sat, delay_ms, hop_count(record['NextHopPath'])))
    return pd.DataFrame(rows, columns=['Time', 'Sat', 'IslDelay_ms', 'Hops'])


def app_delay_series(app, ue):
    """Mean end-to-end delay per reporting interval, uplink and downlink.

    Intervals in which nothing arrived report a delay of 0.0; that is "no
    sample", not "instant", so it is dropped rather than averaged in.
    """
    if app.empty:
        return pd.DataFrame(), pd.DataFrame()

    def direction(mask):
        rows = app[mask].copy()
        if rows.empty:
            return pd.DataFrame(columns=['Time_s', 'Delay_ms'])
        rows['Delay_ms'] = rows['Delay_ms'].replace(0.0, np.nan)
        rows = rows.dropna(subset=['Delay_ms'])
        # Weight by the packets each flow actually delivered in the interval, so
        # a flow with two packets does not count as much as one with two hundred.
        rows['weight'] = rows['IntervalRxPkts'].clip(lower=1)
        grouped = rows.groupby('Time_s').apply(
            lambda g: np.average(g['Delay_ms'], weights=g['weight']),
            include_groups=False)
        return grouped.rename('Delay_ms').reset_index()

    return direction(app['SrcNodeID'] == ue), direction(app['DstNodeID'] == ue)


def load(results_dir, ue=None):
    run = {
        'app': read_csv(os.path.join(results_dir, 'app-statistics-periodic.txt'), sep='\t'),
        'isl': read_csv(os.path.join(results_dir, 'isl-delay-trace.csv')),
        'rrc': read_csv(os.path.join(results_dir, 'ue-rrc-events.csv')),
        'roles': read_csv(os.path.join(results_dir, 'node-roles.csv')),
        'vehicle': read_csv(os.path.join(results_dir, 'vehicle-trace.csv')),
    }
    if run['isl'].empty:
        sys.exit(f'error: no isl-delay-trace.csv in {results_dir}; '
                 'the scenario needs islDelayMode with "updateLog": true.')
    if run['app'].empty:
        sys.exit(f'error: no app-statistics-periodic.txt in {results_dir}; '
                 'set appStatisticsReportInterval in the scenario.')
    run['isl']['GNbNodeId'] = run['isl']['GNbNodeId'].astype(int)

    intervals = service_intervals(run['rrc'], cell_to_node_map(run['roles']))
    served = sorted({u for u, _, _, _ in intervals})
    if ue is None:
        candidates = [n for n in ue_node_ids(results_dir, run['vehicle']) if n in served]
        if not candidates:
            sys.exit(f'error: no UE is ever served by a satellite in {results_dir}.')
        ue = candidates[0]

    run['ue'] = ue
    run['serving'] = serving_isl_series(run['isl'], intervals, ue)
    run['ul'], run['dl'] = app_delay_series(run['app'], ue)
    run['handovers'] = sorted(
        run['rrc'][(run['rrc']['NodeId'] == ue)
                   & (run['rrc']['Event'] == 'HandoverEndOk')]['Time'].tolist()
    ) if not run['rrc'].empty else []
    run['additional_ms'] = additional_delay_ms(results_dir)
    run['name'] = os.path.basename(os.path.normpath(results_dir))
    return run


def label_last(ax, series, colour, text, dy=0):
    """Direct label at the end of a line, so identity is not colour-alone.

    `dy` staggers labels vertically: the backhaul delay and the downlink delay
    it dominates end the run only a few milliseconds apart, so their labels
    would otherwise sit on top of each other.
    """
    if series.empty:
        return
    ax.annotate(text, xy=(series.iloc[-1, 0], series.iloc[-1, 1]),
                xytext=(6, dy), textcoords='offset points',
                color=colour, fontsize=9, va='center', ha='left')


def plot_timeline(run, out_dir, dpi, suffix=''):
    ue = run['ue']
    serving = run['serving'].dropna(subset=['IslDelay_ms'])

    fig, (ax, ax_hops) = plt.subplots(
        2, 1, figsize=(10, 6.4), sharex=True, facecolor=SURFACE,
        gridspec_kw=dict(height_ratios=[3.2, 1.0], hspace=0.18))
    style_axes(ax)
    style_axes(ax_hops)

    if not run['ul'].empty:
        ax.plot(run['ul']['Time_s'], run['ul']['Delay_ms'], color=C_UL, lw=2,
                label='Experienced end-to-end delay, UE to core', zorder=4)
    if not run['dl'].empty:
        ax.plot(run['dl']['Time_s'], run['dl']['Delay_ms'], color=C_DL, lw=2,
                label='Experienced end-to-end delay, core to UE', zorder=4)
    ax.plot(serving['Time'], serving['IslDelay_ms'], color=C_ISL, lw=2,
            label='ISL backhaul delay introduced on the serving satellite', zorder=3)

    for time in run['handovers']:
        for axis in (ax, ax_hops):
            axis.axvline(time, color=C_INK2, lw=0.8, ls=':', zorder=1)
    if run['handovers']:
        ax.annotate('Handover', xy=(run['handovers'][0], 0.97),
                    xycoords=ax.get_xaxis_transform(), xytext=(3, 0),
                    textcoords='offset points', color=C_INK2, fontsize=8,
                    rotation=90, va='top', ha='left')

    label_last(ax, run['ul'], C_UL, 'Uplink')
    label_last(ax, run['dl'], C_DL, 'Downlink')
    # Only the backhaul label needs displacing: it ends a few milliseconds under
    # the downlink delay it dominates, which is closer than the label height.
    label_last(ax, serving[['Time', 'IslDelay_ms']], C_ISL, 'ISL backhaul', dy=-11)

    ax.set_ylabel('Delay [ms]')
    # Headroom for an opaque legend box: at the bottom it would cover the flat
    # backhaul trace, at the right it would cover the direct labels.
    peak = max((series.max() for series in
                (run['ul'].get('Delay_ms', pd.Series(dtype=float)),
                 run['dl'].get('Delay_ms', pd.Series(dtype=float)),
                 serving['IslDelay_ms'])
                if not series.empty), default=1.0)
    ax.set_ylim(0, peak * 1.38)
    ax.legend(loc='upper left', **LEGEND_STYLE)

    ax_hops.step(serving['Time'], serving['Hops'], where='post', color=C_ISL, lw=2)
    ax_hops.fill_between(serving['Time'], 0, serving['Hops'], step='post',
                         color=C_ISL, alpha=0.14, lw=0)
    ax_hops.set_ylabel('ISL hops')
    ax_hops.set_xlabel('Simulated time [s]')
    ax_hops.yaxis.set_major_locator(MaxNLocator(integer=True))
    ax_hops.set_ylim(bottom=0)

    sats = serving['Sat'].dropna().unique()
    subtitle = (f'Serving satellite{"s" if len(sats) > 1 else ""}: '
                + ', '.join(str(int(s)) for s in sats))
    if run['additional_ms'] is not None:
        subtitle += f'   ·   configured additionalDelay {run["additional_ms"]:g} [ms]'
    fig.suptitle('What the user experiences versus what ISL routing introduces',
                 color=C_INK, fontsize=13, x=0.06, ha='left', y=0.985)
    ax.set_title(subtitle, color=C_INK2, fontsize=9.5, loc='left', pad=8)

    path = os.path.join(out_dir, f'isl-delay-timeline-ue{ue}{suffix}.png')
    fig.savefig(path, dpi=dpi, facecolor=SURFACE, bbox_inches='tight')
    plt.close(fig)
    return path


def plot_sweep(runs, out_dir, dpi):
    """Experienced end-to-end delay against the introduced per-route allowance."""
    points = []
    for run in runs:
        if run['additional_ms'] is None:
            print(f'  warning: {run["name"]} has no additionalDelay in its config, skipping')
            continue
        points.append((
            run['additional_ms'],
            run['ul']['Delay_ms'].mean() if not run['ul'].empty else np.nan,
            run['dl']['Delay_ms'].mean() if not run['dl'].empty else np.nan,
            run['serving']['IslDelay_ms'].mean(),
        ))
    if len(points) < 2:
        print('  skipping the sweep figure: fewer than two runs with a known additionalDelay')
        return None

    data = pd.DataFrame(sorted(points),
                        columns=['Introduced_ms', 'UL_ms', 'DL_ms', 'Isl_ms'])

    fig, ax = plt.subplots(figsize=(8, 5.4), facecolor=SURFACE)
    style_axes(ax)

    ax.plot(data['Introduced_ms'], data['UL_ms'], color=C_UL, lw=2, marker='o',
            markersize=7, label='Experienced end-to-end delay, UE to core', zorder=4)
    ax.plot(data['Introduced_ms'], data['DL_ms'], color=C_DL, lw=2, marker='s',
            markersize=7, label='Experienced end-to-end delay, core to UE', zorder=4)
    ax.plot(data['Introduced_ms'], data['Isl_ms'], color=C_ISL, lw=2, marker='^',
            markersize=7, label='ISL backhaul delay applied to the serving satellite',
            zorder=3)

    # One-for-one reference, anchored at the smallest configured allowance: the
    # slope to compare against when reading how much of the introduced delay the
    # user actually pays for.
    base_x, base_y = data['Introduced_ms'].iloc[0], data['UL_ms'].iloc[0]
    ax.plot(data['Introduced_ms'], base_y + (data['Introduced_ms'] - base_x),
            color=C_INK2, lw=1, ls='--', zorder=2,
            label='One-for-one reference (slope 1)')

    for _, row in data.iterrows():
        ax.annotate(f'{row["UL_ms"]:.1f}', xy=(row['Introduced_ms'], row['UL_ms']),
                    xytext=(0, 9), textcoords='offset points', color=C_INK2,
                    fontsize=8.5, ha='center')

    ax.set_xlabel('Introduced ISL delay — configured additionalDelay [ms]')
    ax.set_ylabel('Mean delay over the run [ms]')
    ax.set_ylim(bottom=0)
    ax.legend(loc='lower right', **LEGEND_STYLE)
    fig.suptitle('Experienced delay against the ISL delay introduced per route',
                 color=C_INK, fontsize=13, x=0.02, ha='left', y=0.99)
    ax.set_title('One run per point; everything but additionalDelay held fixed',
                 color=C_INK2, fontsize=9.5, loc='left', pad=8)

    path = os.path.join(out_dir, 'isl-delay-experienced-vs-introduced.png')
    fig.savefig(path, dpi=dpi, facecolor=SURFACE, bbox_inches='tight')
    plt.close(fig)

    print(data.to_string(index=False,
                         float_format=lambda v: f'{v:.3f}'))
    return path


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('results_dir', nargs='+')
    parser.add_argument('--ue', type=int, help='UE node id (default: the first served vehicle)')
    parser.add_argument('--out', help='output directory (default: <first results_dir>/graphs)')
    parser.add_argument('--dpi', type=int, default=200)
    args = parser.parse_args(argv)

    out_dir = args.out or os.path.join(args.results_dir[0], 'graphs')
    os.makedirs(out_dir, exist_ok=True)

    runs = [load(d, args.ue) for d in args.results_dir]
    # Several runs share one output directory, and their UE is usually the same
    # node, so the run name has to be part of the filename or each timeline
    # overwrites the last.
    for run in runs:
        suffix = '' if len(runs) == 1 else f'-{run["name"]}'
        print(f'{run["name"]}: UE {run["ue"]}, '
              f'additionalDelay {run["additional_ms"]}, '
              f'{len(run["serving"])} ISL cycles')
        print('  ->', plot_timeline(run, out_dir, args.dpi, suffix))

    if len(runs) > 1:
        path = plot_sweep(runs, out_dir, args.dpi)
        if path:
            print('  ->', path)


if __name__ == '__main__':
    main()
