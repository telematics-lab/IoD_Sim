# TODO: Replace with plt_lte.py when the following TODOs are finished.
#%%
import re
from pathlib import Path

import pandas as pd
import plotly.express as px

#%%
results_dirpath = '../results/'
scenario_name = 'paper_3-lte'
execution_datetime = '2022-03-03.20-38-18'
scenario_dirpath = f'{results_dirpath}/{scenario_name}-{execution_datetime}'

#%%
remote_trace_filepath = list(Path(scenario_dirpath).glob('./internet-*.tr'))[0]
rex = re.compile(r'^r.+ns3::Ipv4Header.+(?P<ipaddr>7\.0\.0\.[0-9]+) > '
                 r'200\.0\.0\.1.+ns3::SeqTsHeader \(\(seq=(?P<sn>[0-9]+) '
                 r'time=.+')

matches = None
with remote_trace_filepath.open() as f:
  matches = list(filter(None, map(lambda x: rex.match(x), f.readlines())))

assert(matches is not None)
rx_traces = pd.DataFrame([m.groupdict() for m in matches])

#%%
plr = {}
for k,g in rx_traces.groupby('ipaddr'):
  last_sn = int(g['sn'].iloc[-1])
  nrows = g.shape[0]
  loss = last_sn - nrows + 1
  plr[k] = loss / last_sn

#! BUG: Mancano gli host con PLR al 100%
# TODO: Il dizionario deve essere ordinato
# TODO: Se tutto è ok e i risultati risultano in linea con plt_lte.py,
#   individua il più efficiente e sostituisci

#%%
fig = px.bar(x=plr.keys(), y=plr.values(), log_y=True)
fig.update_xaxes(type='category')
fig.show()