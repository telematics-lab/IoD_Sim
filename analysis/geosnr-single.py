# %%
from pathlib import Path

import pandas as pd
import plotly.express as px
import plotly.graph_objects as go


# %% Helper functions
def get_seed_name(x):
    # param x: Path object
    return int(x.name.split("-")[1].split("_")[1])


# %%
results_path = Path("../results/ntn_hap-big-2023-07-13.10-16-53")
trace = results_path / "ntn-snr-trace.txt"

# Read traces and store in a dataframe
df = pd.read_csv(
    trace,
    sep=" ",
    header=None,
    names=["time", "snr", "propagationLoss", "geocentricDistance", "projectedDistance"],
)

# %%
# plot SNR
fig = go.Figure()
fig.add_trace(go.Scatter(y=df["snr"], mode="lines", name="SNR"))
fig.update_layout(title="SNR", xaxis_title="Time (s)", yaxis_title="SNR (dB)")
fig.show()

# %%
# plot propagation loss
# fig = go.Figure()
# fig.add_trace(go.Scatter(
#   y=df['propagationLoss'],
#   mode='lines',
#   name='PropagationLoss (dB)'
# ))
# fig.update_layout(
#   title='Propagation Loss',
#   xaxis_title='Time (s)',
#   yaxis_title='PropagationLoss (dB)'
# )
# fig.show()
