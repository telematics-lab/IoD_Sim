#%%
import re
from pathlib import Path

import pandas as pd
import plotly.graph_objects as go

#%%
results_dirpath = '../results'
scenario_dirname = 'progresses'
scenario_dirpath = f'{results_dirpath}/{scenario_dirname}'

#%%
filenames = sorted(Path(scenario_dirpath).glob('./*.log'))
rex = re.compile(r'.*\( +(?P<speedup>[0-9\.]+)x real time\) (?P<events>[0-9]+) events processed')

samples = [
  [
    m.groupdict()
    for m in filter(None, map(rex.match, f.open(mode='r').readlines()))
  ]
  for f in filenames
]

performances = [pd.DataFrame(x) for x in samples]

#%%
#!BUG: asse Y non ordinato!
fig = go.Figure(data=[
  go.Box(y=p['speedup'])
  for p in performances
])
fig.show()

#%%
#!BUG: asse Y non ordinato!
fig = go.Figure(data=[
  go.Box(y=p['events'])
  for p in performances
])
fig.show()

#%% Export to matlab
p2mat = {'sid': [], 'speedup': [], 'events': []}

for i, p in enumerate(samples):
  for s in p:
    p2mat['sid'].append(i)
    p2mat['speedup'].append(s['speedup'])
    p2mat['events'].append(s['events'])

pd.DataFrame(p2mat).to_csv('./perf.csv')