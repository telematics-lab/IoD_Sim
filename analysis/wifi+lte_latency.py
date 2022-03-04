#%%
import re

import pandas as pd
import plotly.express as px

#%%
def custom_val_map(k, v):
  if k == 'ipaddr':
    return v
  else:
    return float(v)

#%%
base_results_dir = '../results/'
scenario_name = 'paper_3-wifi'
scenario_dir = f'{scenario_name}-2022-02-28.17-07-31'

wifi_tx_trace_filenames = [f'wifi-phy-0-drones-host-{i}-0.log'
                           for i in range(4, 11)]
wifi_tx_trace_filenames += [f'wifi-phy-1-drones-host-{i}-0.log'
                            for i in range(11, 17)]
wifi_tx_trace_filenames += [f'wifi-phy-2-drones-host-{i}-0.log'
                            for i in range(17, 24)]
wifi_tx_trace_filenames += [f'wifi-phy-3-drones-host-{i}-0.log'
                            for i in range(24, 28)]

rex = re.compile(r't (?P<time>[0-9\.+]+).+HeMcs(?P<mcs>[0-9]+).+ (?P<ipaddr>[0-9\.]+) > 200\.0\.0\.1.+ns3::SeqTsHeader \(\(seq=(?P<sn>[0-9]+).+')

tx_traces = []

for fn in wifi_tx_trace_filenames:
  pkts = []

  with open(f'{base_results_dir}/{scenario_dir}/{fn}', 'r') as f:
    for l in f:
      r = rex.match(l)

      if r is not None:
        ## DEBUG
        # print(r[0])
        # print(r.groupdict())
        # break
        ##
        pkts.append({k: custom_val_map(k, v) for k,v in r.groupdict().items()})

  tx_traces.append(pd.DataFrame(pkts))


for i in range(len(tx_traces)):
  tx_traces[i] = tx_traces[i].groupby('sn').first()

#%%
pnat = {
    "7.0.0.2:1": "10.1.0.7",
    "7.0.0.2:2": "10.1.0.8",
    "7.0.0.2:3": "10.1.0.4",
    "7.0.0.2:4": "10.1.0.2",
    "7.0.0.2:5": "10.1.0.5",
    "7.0.0.2:6": "10.1.0.3",
    "7.0.0.2:7": "10.1.0.6",
    "7.0.0.3:1": "10.2.0.4",
    "7.0.0.3:2": "10.2.0.3",
    "7.0.0.3:3": "10.2.0.2",
    "7.0.0.3:4": "10.2.0.5",
    "7.0.0.3:5": "10.2.0.6",
    "7.0.0.3:6": "10.2.0.7",
    "7.0.0.4:1": "10.3.0.3",
    "7.0.0.4:2": "10.3.0.2",
    "7.0.0.4:3": "10.3.0.5",
    "7.0.0.4:4": "10.3.0.7",
    "7.0.0.4:5": "10.3.0.4",
    "7.0.0.4:6": "10.3.0.6",
    "7.0.0.4:7": "10.3.0.8",
    "7.0.0.5:1": "10.4.0.5",
    "7.0.0.5:2": "10.4.0.3",
    "7.0.0.5:3": "10.4.0.4",
    "7.0.0.5:4": "10.4.0.2"
}
internet_rx_trace_filename = 'internet-29-1.tr'

rx_traces = []

rex = re.compile(r'r (?P<time>[0-9\.+]+).+7\.0\.0\.(?P<lte_host_id>[0-9]+).+'
                 r'(?P<lte_port>[0-9]+) > 1337.+ns3::SeqTsHeader '
                 r'\(\(seq=(?P<sn>[0-9]+).+')
pkts = {k: [] for k in pnat.values()}

with open(f'{base_results_dir}/{scenario_dir}/{internet_rx_trace_filename}', 'r') as f:
  for l in f:
    r = rex.match(l)

    if r is not None:
      ## DEBUG
      # print(r[0])
      # print(r.groupdict())
      # break
      ##
      g = r.groupdict()
      host_addr = pnat[f'7.0.0.{g["lte_host_id"]}:{g["lte_port"]}']

      pkt = {
        'time': float(g['time']),
        'sn': float(g['sn'])
      }
      pkts[host_addr].append(pkt)

for k in sorted(pkts.keys()):
  rx_traces.append(pd.DataFrame(pkts[k]))

# %%
latency = {}

for i in range(len(tx_traces)):
  lat = pd.merge(tx_traces[i], rx_traces[i], on='sn')
  latency[tx_traces[i]['ipaddr'][0]] = (lat['time_y'] - lat['time_x']) * 1e3 # to milliseconds

# %%
fig = px.box(latency, labels=dict(value='Latency [ms]', variable='GU IP Address'))
fig.show()