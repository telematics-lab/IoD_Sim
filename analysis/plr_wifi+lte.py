
#%%
import json
import re
import socket

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
scenario_datetime = '2022-03-15.16-53-30'
scenario_dir = f'{scenario_name}-{scenario_datetime}'

wifi_tx_trace_filenames = [f'wifi-phy-0-drones-host-{i}-0.log'
                           for i in range(4,10)]
wifi_tx_trace_filenames += [f'wifi-phy-1-drones-host-{i}-0.log'
                            for i in range(10, 20)]
wifi_tx_trace_filenames += [f'wifi-phy-2-drones-host-{i}-0.log'
                            for i in range(20, 30)]
wifi_tx_trace_filenames += [f'wifi-phy-3-drones-host-{i}-0.log'
                            for i in range(30, 38)]

internet_rx_trace_filename = 'internet-39-1.tr'

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
  tx_traces[i] = tx_traces[i].groupby('sn', as_index=False).first()
  # print(tx_traces[i]['ipaddr'][0]) #DEBUG

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


##%
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
  # print(k) #DEBUG
  rx_traces.append(pd.DataFrame(pkts[k]))

# %%
def eval_plr(tx, rx):
  if tx.empty:
    return 0.0
  elif rx.empty:
    return 1.0
  else:
    return 1.0 - (rx.count()[0] / tx.count()[0])

plr = {
  tx_traces[i]['ipaddr'][0]: eval_plr(tx_traces[i], rx_traces[i])
  for i in range(len(tx_traces))
}

# %%
fig = px.bar(plr.values())
fig.update_yaxes(range = [0,1])
fig.show()

#%% export PLR to matlab
print(json.dumps(plr))
