#!/usr/bin/env python3
"""Render PNG plots from the text logs written by analysis/v2leosim_preprocessor.py.

    python3 analysis/v2leosim_viwer.py results/<run>/output
    python3 analysis/v2leosim_viwer.py results/<run>/output -o graphs/
"""

import argparse
import csv
import glob
import os
import re
import sys


# --------------------------------------------------------------------------- #
# reading the logs
# --------------------------------------------------------------------------- #

def read_tsv(path):
    """Every exported table is: one header line, then tab-separated rows."""
    if not os.path.exists(path):
        return []
    with open(path, newline='') as handle:
        return list(csv.DictReader(handle, delimiter='\t'))


def read_kv(path):
    return {row['key']: row['value'] for row in read_tsv(path)}


def num(value, default=None):
    """Exported files use an empty field for 'no data'; keep that distinct."""
    if value is None or value == '':
        return default
    try:
        return float(value)
    except ValueError:
        return default


def column(rows, name):
    return [num(r.get(name)) for r in rows]


def load(output_dir):
    j = lambda name: os.path.join(output_dir, name)
    series = {}
    for path in sorted(glob.glob(j('timeseries-user-*.txt'))):
        m = re.search(r'timeseries-user-(\d+)\.txt$', path)
        if m:
            series[m.group(1)] = read_tsv(path)
    return {
        'dir': output_dir,
        'run': read_kv(j('run.txt')),
        'events': read_tsv(j('events.txt')),
        'aggregate': read_tsv(j('timeseries-aggregate.txt')),
        'series': series,
    }


# --------------------------------------------------------------------------- #
# plots
# --------------------------------------------------------------------------- #

def plot(data, out_dir):
    """Write PNG versions of the network and per-user views."""
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt

    os.makedirs(out_dir, exist_ok=True)
    written = []

    def series_of(rows, name):
        t = column(rows, 'time_s')
        v = column(rows, name)
        pairs = [(a, b) for a, b in zip(t, v) if a is not None]
        return [a for a, _ in pairs], [b for _, b in pairs]

    agg = data['aggregate']
    if agg:
        fig, axes = plt.subplots(4, 1, figsize=(11, 11), sharex=True)
        for name, label in (('dl_rx_kbps', 'Downlink'), ('ul_rx_kbps', 'Uplink')):
            axes[0].plot(*series_of(agg, name), label=label)
        axes[0].set_ylabel('Throughput [kbps]')
        axes[0].legend()
        for name, label in (('mean_dl_delay_ms', 'Downlink'),
                            ('mean_ul_delay_ms', 'Uplink')):
            axes[1].plot(*series_of(agg, name), label=label)
        axes[1].set_ylabel('Mean delay [ms]')
        axes[1].legend()
        for name, label in (('mean_dl_sinr_db', 'Average'), ('min_dl_sinr_db', 'Worst user')):
            axes[2].plot(*series_of(agg, name), label=label)
        axes[2].set_ylabel('DL SINR [dB]')
        axes[2].legend()
        axes[3].plot(*series_of(agg, 'users_attached'), label='Attached users')
        axes[3].plot(*series_of(agg, 'active_cells'), label='Cells in use')
        axes[3].set_ylabel('Count [#]')
        axes[3].set_xlabel('Time [s]')
        axes[3].legend()
        for ax in axes:
            ax.grid(alpha=0.3)
        fig.suptitle(f"{data['run'].get('scenario', 'run')} - network")
        fig.tight_layout()
        path = os.path.join(out_dir, 'scenario-network.png')
        fig.savefig(path, dpi=120)
        plt.close(fig)
        written.append(path)

    for user_id in sorted(data['series'], key=int):
        rows = data['series'][user_id]
        fig, axes = plt.subplots(4, 1, figsize=(11, 11), sharex=True)
        axes[0].plot(*series_of(rows, 'dl_rx_kbps'), label='Downlink')
        axes[0].plot(*series_of(rows, 'ul_rx_kbps'), label='Uplink')
        axes[0].set_ylabel('Throughput [kbps]')
        axes[0].legend()
        axes[1].plot(*series_of(rows, 'dl_delay_ms'), label='Downlink')
        axes[1].plot(*series_of(rows, 'ul_delay_ms'), label='Uplink')
        axes[1].plot(*series_of(rows, 'isl_delay_ms'), label='ISL one-way')
        axes[1].set_ylabel('Delay [ms]')
        axes[1].legend()
        axes[2].plot(*series_of(rows, 'dl_sinr_db'), label='DL SINR')
        axes[2].set_ylabel('SINR [dB]')
        axes[2].legend()
        axes[3].plot(*series_of(rows, 'elevation_deg'))
        axes[3].set_ylabel('Elevation [deg]')
        axes[3].set_xlabel('Time [s]')


        for e in data['events']:
            if e['user_id'] != user_id or not e['event'].startswith('handover'):
                continue
            for ax in axes:
                ax.axvline(float(e['time_s']), color='red', alpha=0.4, lw=0.8)

        for ax in axes:
            ax.grid(alpha=0.3)
        fig.suptitle(f"{data['run'].get('scenario', 'run')} - user {user_id} "
                     f"(red = handover)")
        fig.tight_layout()
        path = os.path.join(out_dir, f'scenario-user-{user_id}.png')
        fig.savefig(path, dpi=120)
        plt.close(fig)
        written.append(path)

    return written


# --------------------------------------------------------------------------- #

def main():
    parser = argparse.ArgumentParser(
        description='Plot the exported LEO-NR web logs to PNG files.')
    parser.add_argument('output_dir',
                        help='an output/ folder written by v2leosim_preprocessor.py '
                             '(a results folder also works)')
    parser.add_argument('-o', '--out-dir', default=None,
                        help='where to write the PNGs (default: <output_dir>/graphs)')
    args = parser.parse_args()

    output_dir = os.path.abspath(args.output_dir)
    if not os.path.exists(os.path.join(output_dir, 'run.txt')):
        nested = os.path.join(output_dir, 'output')
        if os.path.exists(os.path.join(nested, 'run.txt')):
            output_dir = nested
        else:
            parser.error(f'no run.txt in {output_dir}; run v2leosim_preprocessor.py first')

    data = load(output_dir)
    out_dir = args.out_dir or os.path.join(output_dir, 'graphs')
    written = plot(data, out_dir)
    print(f'wrote {len(written)} plots to {out_dir}')
    return 0


if __name__ == '__main__':
    sys.exit(main())