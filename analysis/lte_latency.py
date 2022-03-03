#%%
import re

import pandas as pd
import plotly.express as px

#%%
base_results_dir = '../results/'
scenario_name = 'paper_3-lte'
scenario_dir = f'{scenario_name}-2022-02-28.18-13-32'

tx_trace_filename = 'paper_3-test.log'

rex = re.compile(r'(?P<time>[0-9\.+]+)s (?P<nodeid>[0-9]+) .+ 200\.0\.0\.1 .+ ns3::SeqTsHeader \(\(seq=(?P<sn>[0-9]+).+')

tx_traces_raw = {}
pkts = []

with open(f'{base_results_dir}/{scenario_dir}/{tx_trace_filename}', 'r') as f:
  for l in f:
    r = rex.match(l)
    if r is not None:
      ## DEBUG
      # print(r[0])
      # print(r.groupdict())
      # break
      ##
      pkts.append({k: float(v) for k,v in r.groupdict().items()})

for p in pkts:
  if p['nodeid'] in tx_traces_raw:
    tx_traces_raw[p['nodeid']].append(p)
  else:
    tx_traces_raw[p['nodeid']] = [p]

tx_traces = []
for k,v in tx_traces_raw.items():
  tx_traces.append(pd.DataFrame(v))

#%%
internet_rx_trace_filename = 'internet-25-1.tr'

rx_traces = []

ip_addr_offset = 2
ip_addrs = [f'7.0.0.{i}' for i in range(ip_addr_offset, len(tx_traces_raw.keys()) + 1)]

for lte_ip in ip_addrs:
  rex = re.compile(fr'r (?P<time>[0-9\.+]+).+{lte_ip}.+ > 1337.+ns3::SeqTsHeader \(\(seq=(?P<sn>[0-9]+).+')
  pkts = []

  with open(f'{base_results_dir}/{scenario_dir}/{internet_rx_trace_filename}', 'r') as f:
    for l in f:
      r = rex.match(l)

      if r is not None:
        ## DEBUG
        # print(r[0])
        # print(r.groupdict())
        # break
        ##
        pkts.append({k: float(v) for k,v in r.groupdict().items()})
        pkts[-1]['ipaddr'] = lte_ip

  rx_traces.append(pd.DataFrame(pkts))

# %%
latency = {}

for i in range(len(tx_traces[2:])):
  tx = tx_traces[i][['time', 'sn']]
  rx = rx_traces[i][['time', 'sn']]
  lat = pd.merge(tx, rx, on=['sn', 'sn'])
  latency[rx_traces[i]['ipaddr'][0]] = (lat['time_y'] - lat['time_x']) * 1e-3

# %%
fig = px.box(latency, labels=dict(value='Latency [ms]', variable='GU IP Address'))
fig.show()