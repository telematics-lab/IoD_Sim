#%%
import re
import socket

import pandas as pd
import plotly.graph_objects as go

#%% Parameters configuration
base_results_dir = '../results/'
scenario_name = 'paper_3-wifi'
# scenario_dir = f'{scenario_name}-2022-03-03.18-53-11'
scenario_dir = f'{scenario_name}-2022-03-15.16-53-30'

wifi_tx_trace_filenames = [f'wifi-phy-0-drones-host-{i}-0.log'
                           for i in range(4,10)]
wifi_tx_trace_filenames += [f'wifi-phy-1-drones-host-{i}-0.log'
                            for i in range(10, 20)]
wifi_tx_trace_filenames += [f'wifi-phy-2-drones-host-{i}-0.log'
                            for i in range(20, 30)]
wifi_tx_trace_filenames += [f'wifi-phy-3-drones-host-{i}-0.log'
                            for i in range(30, 38)]

internet_rx_trace_filename = 'internet-39-1.tr'

##% Helper routines

def custom_val_map(k, v):
  if k == 'ipaddr':
    return v
  else:
    return float(v)

##%
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
  print(tx_traces[i]['ipaddr'][0])

#%% Evaluate PNAT

pnat = {} # empty PNAT lookup table

rex = re.compile(r'([0-9\.]+):([0-9]+) -> ([0-9\.]+):([0-9]+)')

with open(f'{base_results_dir}/{scenario_dir}/{scenario_name}.log', 'r') as f:
  for l in f:
    r = rex.search(l)
    if r is None or r.lastindex != 4:
      continue

    int_ipaddr, int_port, ext_ipaddr, ext_port = r.groups()
    pnat[f'{ext_ipaddr}:{ext_port}'] = f'{int_ipaddr}'


#%%
rx_traces = []

rex = re.compile(r'r (?P<time>[0-9\.+]+).+7\.0\.0\.(?P<lte_host_id>[0-9]+).+'
                 r' (?P<lte_port>[0-9]+) > 1337.+ns3::SeqTsHeader'
                 r' \(\(seq=(?P<sn>[0-9]+).+')
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
      try:
        host_addr = pnat[f'7.0.0.{g["lte_host_id"]}:{g["lte_port"]}']
      except:
        print(r[0])
        print(r.groupdict())
        raise Exception()

      pkt = {
        'time': float(g['time']),
        'sn': float(g['sn'])
      }
      pkts[host_addr].append(pkt)

for k in sorted(pkts.keys(), key=lambda k: socket.inet_aton(k)):
  print(k)
  rx_traces.append(pd.DataFrame(pkts[k]))

# %%
latency = {}

for i in range(len(tx_traces)):
  lat = pd.merge(tx_traces[i], rx_traces[i], on='sn')
  latency[tx_traces[i]['ipaddr'][0]] = (lat['time_y'] - lat['time_x']) * 1e3 # to milliseconds

# %% Export data
lat2matlab = {
  'ClusterID': [],
  'IPAddr': [],
  'latency': []
}

hostid = 1
for k in latency.keys():
  for v in latency[k]:
    lat2matlab["ClusterID"].append(k.split('.')[1])
    lat2matlab["IPAddr"].append(hostid)
    lat2matlab["latency"].append(v)
  hostid += 1

pd.DataFrame(lat2matlab).to_csv('./s3-latency_wifi+lte.csv')

# %%
color_palette = ['rgb(0,0,0)'
                 for i in range(2, 9)]
color_palette += ['rgb(0.5,0.5,0.5)'
                  for i in range(2, 8)]
color_palette += ['rgb(0.7,0.7,0.7)'
                  for i in range(2, 9)]
color_palette += ['rgb(0.7,0.7,0.7)'
                  for i in range(2, 6)]

fig = go.Figure()
for k,v in latency.items():
  fig.add_trace(go.Box(y=v, name=k))
#fig = px.box(latency, labels=dict(value='Latency [ms]', variable='GU IP Address'))
fig.show()