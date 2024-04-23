# Retrieve PDR/PLR data by analyzing a PCAP file in IoD Sim

#%%
import json
import os
import re
import subprocess
from collections import Counter
from multiprocessing import Pool

import pandas as pd
import plotly.express as px

#%%
results_basepath = "../results"
# results_dir = 'paper_3-wifi-staticGU-UDP-2022-02-19.17-31-49'
# scenario_name = 'paper_3-wifi-staticGU-UDP'
results_dir = "paper_3-wifi-2022-03-03.18-53-11"
scenario_name = "paper_3-wifi"

net_prefixes = ["7.0.0"]
n_drones = [6, 9, 9, 8]

# assert(len(net_prefixes) == len(n_drones))
n_stacks = len(n_drones)

#%%
def pkt_per_drone_in_stack(args):
    base_path, n_drones, net_prefix, stack_id, key_field, pkt_type = args
    flt_str = " or ".join(
        [f"ip.src == {net_prefix}.{i}" for i in range(2, n_drones + 1)]
    )

    args = [
        "tshark",
        "-r",
        f"{base_path}/wifi-phy-{stack_id}-drones-host-{stack_id}-2.pcap",
        "-T",
        "fields",
        "-e",
        key_field,
        f"{pkt_type} and {flt_str}",
    ]
    ret = subprocess.run(args, capture_output=True)

    if ret.returncode != 0:
        print(ret)
        exit(1)

    out = ret.stdout.decode("UTF-8").strip().split(os.linesep)
    out.sort()
    cnt = Counter(out)

    return dict(cnt)


r = None
with Pool(n_stacks) as p:
    r = p.map(
        pkt_per_drone_in_stack,
        [
            (
                f"{results_basepath}/{results_dir}",
                n_drones[i],
                net_prefixes[i],
                i,
                "ip.src",
                "udp and not icmp",
            )
            for i in range(n_stacks)
        ],
    )

assert r is not None
udp_cnt = {k: v for d in r for k, v in d.items()}
print(udp_cnt)

#%% Retrieve IP-Port mapping to learn the PNAT table used in the scenario
n_ue = len(n_drones)  # UE drones control GU wifi networks
pnat = {}  # empty PNAT lookup table

rex = re.compile(r"([0-9\.]+):([0-9]+) -> ([0-9\.]+):([0-9]+)")

with open(f"{results_basepath}/{results_dir}/paper_3-wifi.log", "r") as f:
    for l in f:
        r = rex.search(l)
        if r is None or r.lastindex != 4:
            continue

        int_ipaddr, int_port, ext_ipaddr, ext_port = r.groups()
        pnat[f"{ext_ipaddr}:{ext_port}"] = f"{int_ipaddr}"

    print(json.dumps(pnat, indent=4, sort_keys=True))

#%% Now we should filter and get packets from internet. Retrieve IP-Port from NATApplication logs, then get udp packets from pcap and then evaluate PDR/PLR
def pkt_per_drone_internet(pcap_filename):
    args = [
        "tshark",
        "-r",
        pcap_filename,
        "-T",
        "fields",
        "-e",
        "ip.src",
        "-e",
        "udp.srcport",
        "udp",
    ]
    ret = subprocess.run(args, capture_output=True)

    if ret.returncode != 0:
        print(ret)
        exit(1)

    out = ret.stdout.decode("UTF-8").strip().replace("\t", ":").split(os.linesep)
    out.sort()
    cnt = Counter(out)

    return dict(cnt)


internet_stack_id = sum(n_drones) + len(n_drones) + 1
inet_cnt = pkt_per_drone_internet(
    f"{results_basepath}/{results_dir}/internet-{internet_stack_id}-1.pcap"
)

print(json.dumps(inet_cnt, indent=4, sort_keys=True))

#%%
inet_cnt = {pnat[k]: v for k, v in inet_cnt.items()}

print(json.dumps(inet_cnt, indent=4, sort_keys=True))

#%%
pdr = {}

for k, v in udp_cnt.items():
    if k == "":
        continue

    v1 = inet_cnt[k]
    pdr[k] = v1 * 100 / v

pdr = pd.DataFrame(pdr.items(), columns=["GU IP Addr", "PDR [%]"])
pdr

#%%
fig = px.histogram(pdr, x="GU IP Addr", y="PDR [%]")
fig.show()
