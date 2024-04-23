#!/usr/bin/env python

# %%
import json
import re

import pandas as pd
import plotly.express as px

# %%
base_results_dir = "../results/"
scenario_name = "paper_3-lte"
scenario_datetime = "2022-03-15.16-53-33"
scenario_dir = f"{scenario_name}-{scenario_datetime}"
tx_trace_filename = "paper_3-lte.log"
internet_rx_trace_filename = "internet-35-1.tr"

lte_id2addr_offset = 2

rex = re.compile(
    r"(?P<time>[0-9\.+]+)s (?P<nodeid>[0-9]+) .+ 200\.0\.0\.1 .+ "
    r"ns3::SeqTsHeader \(\(seq=(?P<sn>[0-9]+).+"
)

tx_traces = {}
pkts = []

with open(f"{base_results_dir}/{scenario_dir}/{tx_trace_filename}", "r") as f:
    for l in f:
        r = rex.match(l)
        if r is not None:
            ## DEBUG
            # print(r[0])
            # print(r.groupdict())
            # break
            ##
            g = r.groupdict()
            pkts.append(
                {
                    "time": float(g["time"]),
                    "nodeid": int(g["nodeid"]),
                    "sn": int(g["sn"]),
                }
            )

for p in pkts:
    lte_host_addr = f'7.0.0.{p["nodeid"] + lte_id2addr_offset}'

    if lte_host_addr in tx_traces:
        tx_traces[lte_host_addr].append(p)
    else:
        tx_traces[lte_host_addr] = [p]

for k in tx_traces.keys():
    tx_traces[k] = pd.DataFrame(tx_traces[k])


# %%
ip_addrs = [f"7.0.0.{i}" for i in range(lte_id2addr_offset, len(tx_traces.keys()) + 1)]

rex = re.compile(
    r"r (?P<time>[0-9\.+]+).+7\.0\.0\.(?P<lte_host_id>[0-9]+).+"
    r" > 1337.+ns3::SeqTsHeader \(\(seq=(?P<sn>[0-9]+).+"
)
rx_traces = {k: [] for k in tx_traces.keys()}

with open(f"{base_results_dir}/{scenario_dir}/{internet_rx_trace_filename}", "r") as f:
    for l in f:
        r = rex.match(l)
        if r is not None:
            ## DEBUG
            # print(r[0])
            # print(r.groupdict())
            # break
            ##
            g = r.groupdict()
            host_addr = f'7.0.0.{g["lte_host_id"]}'

            rx_traces[host_addr].append({"time": float(g["time"]), "sn": int(g["sn"])})

rx_traces = {k: pd.DataFrame(v, columns=["time", "sn"]) for k, v in rx_traces.items()}


# %%
def eval_plr(tx, rx):
    if tx.empty:
        return 0.0
    elif rx.empty:
        return 1.0
    else:
        return 1.0 - (rx.count()[0] / tx.count()[0])


# ridimensiona tx_traces escludendo pacchetti trasmessi che sono ancora in
# circolo per la rete al termine della simulazione
def reduce(tx, rx):
    return tx[tx["sn"] <= rx["sn"].iloc[-1]]


plr = {k: eval_plr(tx_traces[k], rx_traces[k]) for k in tx_traces.keys()}

# %%
fig = px.bar(plr.values())
fig.update_yaxes(range=[0, 1])
fig.show()

# %% Export PDR to matlab
print(json.dumps(plr))
