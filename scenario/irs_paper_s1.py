# This script can be used as a notebook to programatically design a scenario.
#
# This program is free software you can redistribute it and / or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program if not , write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

# %%
import plotly.graph_objects as go
from py_design_support import create_roi, gen_trajectory, Point

# %% Create a world according to the specifications of the IRS paper, Scenario #1
rois = [create_roi(190, 190, 0, 20, 20, 25, 'lightgrey')]

drones = [
    [Point(x=300, y=200, z=0, interest_level=0)],
    [Point(x=100, y=200, z=30, interest_level=0)],
    [Point(x=200, y=200, z=50, interest_level=0)]
]

# print(f'Number of drones: {len(drones)}')
trs = gen_trajectory(drones)
fig = go.Figure(data=[*rois, *trs])
# fig.update_layout(scene_aspectmode='manual', scene_aspectratio=dict(x=1, y=1, z=0.5))
fig.update_layout(scene_aspectmode='data')
fig.update_layout(showlegend=False)
fig.show()
