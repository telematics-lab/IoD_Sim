#%%
from pathlib import Path

import pandas as pd
import plotly.graph_objects as go
import plotly.express as px

#%% Helper functions
def get_seed_name(x):
  # param x: Path object
  return int(x.name.split('-')[1].split('_')[1])

#%%
results_path = Path('../results')
results = [x for x in results_path.iterdir() if x.is_dir() and x.name.startswith('ntn_hap-seed')]

# Expected format: ntn_hap-seed_<seed>-<date>.<time>
results.sort(key=get_seed_name)

#%%
traces = [x / 'ntn-snr-trace.txt' for x in results]

# Read traces and store in a dataframe
df = pd.DataFrame()

for trace in traces:
  temp_df = pd.read_csv(trace, sep=' ', header=None, names=['time', 'snr', 'propagationLoss'])
  df = pd.concat((df, temp_df), axis=1)

mean_snr = df[['snr']].mean(axis=1)
mean_propagation_loss = df[['propagationLoss']].mean(axis=1)

#%%
# plot SNR
fig = go.Figure()
fig.add_trace(go.Scatter(
  y=mean,
  mode='lines',
  name='SNR'
))
fig.update_layout(
  title='SNR',
  xaxis_title='Time (s)',
  yaxis_title='SNR (dB)'
)
fig.show()

#%%
# plot propagation loss
fig = go.Figure()
fig.add_trace(go.Scatter(
  y=mean_propagation_loss,
  mode='lines',
  name='PropagationLoss (dB)'
))
fig.update_layout(
  title='Propagation Loss',
  xaxis_title='Time (s)',
  yaxis_title='PropagationLoss (dB)'
)
fig.show()
#%%
from pathlib import Path

import pandas as pd
import plotly.graph_objects as go
import plotly.express as px

#%% Helper functions
def get_seed_name(x):
  # param x: Path object
  return int(x.name.split('-')[1].split('_')[1])

#%%
results_path = Path('../results')
results = [x for x in results_path.iterdir() if x.is_dir() and x.name.startswith('ntn_hap-seed')]

# Expected format: ntn_hap-seed_<seed>-<date>.<time>
results.sort(key=get_seed_name)

#%%
traces = [x / 'ntn-snr-trace.txt' for x in results]

# Read traces and store in a dataframe
df = pd.DataFrame()

for trace in traces:
  temp_df = pd.read_csv(trace, sep=' ', header=None, names=['time', 'snr', 'propagationLoss'])
  df = pd.concat((df, temp_df), axis=1)

mean_snr = df[['snr']].mean(axis=1)
mean_propagation_loss = df[['propagationLoss']].mean(axis=1)

#%%
# plot SNR
fig = go.Figure()
fig.add_trace(go.Scatter(
  y=mean,
  mode='lines',
  name='SNR'
))
fig.update_layout(
  title='SNR',
  xaxis_title='Time (s)',
  yaxis_title='SNR (dB)'
)
fig.show()

#%%
# plot propagation loss
fig = go.Figure()
fig.add_trace(go.Scatter(
  y=mean_propagation_loss,
  mode='lines',
  name='PropagationLoss (dB)'
))
fig.update_layout(
  title='Propagation Loss',
  xaxis_title='Time (s)',
  yaxis_title='PropagationLoss (dB)'
)
fig.show()