
#%%
from pathlib import Path

import numpy as np
import pandas as pd

#%%
base_results_dir = '../results/'
# scenario_name = 'paper_3-lte'
# scenario_dir = f'{scenario_name}-2022-03-03.20-38-18'
scenario_name = 'paper_3-wifi'
scenario_dir = f'{scenario_name}-2022-03-03.18-53-11'
results_dir = f'{base_results_dir}/{scenario_dir}'

#%%
sinr_trace_filepath = list(Path(results_dir).glob('./*-PhySinrUlStats.txt'))[0]

df = pd.read_csv(sinr_trace_filepath, sep='\t')
df['sinr'] = 20 * np.log10(df['sinrLinear'])

df.boxplot(column=['sinr'], by=['IMSI'])

# %% Export to matlab
df[['IMSI', 'sinr']].to_csv('./sinr-lte-relay.csv')
