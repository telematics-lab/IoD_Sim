# %%
# NOTE: Wi-Fi SNR reported in PCAP files are SINR. Refs:
#   - https://www.nsnam.org/doxygen/interference-helper_8cc_source.html#l00362
#   - https://www.nsnam.org/tutorials/consortium14/ns-3-training-session-5.pdf

import subprocess
from pathlib import Path
from statistics import mean

import plotly.express as px
import plotly.graph_objects as go

# %%
results_dirpath = "../results"
scenario_name = "paper_3-wifi"
scenario_exec_datetime = "2022-03-03.18-53-11"
scenario_dirname = f"{scenario_name}-{scenario_exec_datetime}"
scenario_dirpath = f"{results_dirpath}/{scenario_dirname}"

n_relays = 4
# mac_stas = [
#   [
#     f'00:00:00:00:00:{i:02x}'
#     for i in range(11, 18)
#   ],
#   [
#     f'00:00:00:00:00:{i:02x}'
#     for i in range(18, 28)
#   ],
#   [
#     f'00:00:00:00:00:{i:02x}'
#     for i in range(28, 38)
#   ],
#   [
#     f'00:00:00:00:00:{i:02x}'
#     for i in range(38, 46)
#   ]
# ]
mac_stas = [f"00:00:00:00:00:{i:02x}" for i in range(10, 18)]
mac_stas += [f"00:00:00:00:00:{i:02x}" for i in range(18, 28)]
mac_stas += [f"00:00:00:00:00:{i:02x}" for i in range(28, 38)]
mac_stas += [f"00:00:00:00:00:{i:02x}" for i in range(38, 46)]

wlan_id_offsets = [4, 4, 4, 4]


# %%
def safe_float(x):
    try:
        return float(x)
    except ValueError:
        return None


# Filter out relays by looking at wifi STAs PCAPs only
def pcap_data(scenario_dirpath, phy_id, mac_stas, wlan_id_offset):
    for pcap_path in Path(scenario_dirpath).glob(f"./wifi-phy-{phy_id}-*-0.pcap"):
        wlan_host_id = int(pcap_path.name.split("-")[5]) - wlan_id_offset

        # print(mac_stas, pcap_path, wlan_host_id)
        # print(mac_stas[wlan_host_id])
        args = [
            "tshark",
            "-r",
            pcap_path,
            "-T",
            "fields",
            "-e",
            "wlan_radio.snr",
            f"wlan.addr == {mac_stas[wlan_host_id]} and wlan_radio.snr",
        ]

        try:
            resp = subprocess.run(args, check=True, capture_output=True)
        except subprocess.CalledProcessError as e:
            raise Exception(e.stderr)

        snr_samples = list(map(safe_float, resp.stdout.decode("utf-8").split("\n")))
        yield wlan_host_id, snr_samples


# %%
df = {
    hid: snrs
    for pi, wio in zip(range(n_relays), wlan_id_offsets)
    for hid, snrs in pcap_data(scenario_dirpath, pi, mac_stas, wio)
}

# %%
means = [mean(filter(None, v)) for v in df.values()]

# %% Plot
fig = go.Figure(data=[go.Box(y=v) for _, v in df.items()])
fig.show()

# %%
fig = px.bar(means)
fig.show()
