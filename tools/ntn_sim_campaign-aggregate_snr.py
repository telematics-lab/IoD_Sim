#!/usr/bin/env python
import re
from pathlib import Path

import pandas as pd

# CONFIGURATION
RESULTS_PATH = './results-auto'
SCENARIO_NAME_PREFIX = 'auto-ntn_hap_static'
OUT_FILE = 'snr-ntn-trace.txt'
###

def main():
  results = Path(RESULTS_PATH).glob(f'{SCENARIO_NAME_PREFIX}*')

  freq_snr_df = pd.DataFrame(columns=['freq', 'snr'])

  for r in results:
    # Get frequency from directory name
    freq = float(re.search(r'(\d+\.\d+)', r.name).group(1))

    # Extract SNR from trace file
    snr_trace_file = r / 'ntn-snr-trace.txt'
    df = pd.read_csv(snr_trace_file, sep=" ", header=None)
    snr = df[1].max()

    freq_snr_df.loc[len(freq_snr_df)] = [freq, snr]

  freq_snr_df.sort_values(by=['freq'], inplace=True)
  freq_snr_df.to_csv(OUT_FILE, sep=' ', header=False, index=False)

if __name__ == '__main__':
  main()
