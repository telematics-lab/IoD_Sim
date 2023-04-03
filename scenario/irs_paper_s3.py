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

# %%
color_palette = ['lime', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue',
                 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue',
                 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'dodgerblue', 'red', 'yellow', 'yellow', 'yellow', 'yellow']

rois = [
    create_roi(50.0, 300.0, 0.0, 100.0, 50.0, 25.0, 'lightgrey'),
    create_roi(75.0, 75.0, 0.0, 50.0, 50.0, 20.0, 'lightgrey'),
    create_roi(200.0, 300.0, 0.0, 150.0, 50.0, 25.0, 'lightgrey'),
    create_roi(50.0, 175.0, 0.0, 100.0, 100.0, 25.0, 'lightgrey'),
    create_roi(275.0, 150.0, 0.0, 75.0, 125.0, 25.0, 'lightgrey'),
    create_roi(275.0, 50.0, 0.0, 75.0, 75.0, 25.0, 'lightgrey')
]

drones = [
    [Point(x=100, y=100, z=30, interest_level=0)],
    [Point(x=40, y=40, z=0, interest_level=0)],
    [Point(x=40, y=120, z=0, interest_level=0)],
    [Point(x=40, y=200, z=0, interest_level=0)],
    [Point(x=40, y=280, z=0, interest_level=0)],
    [Point(x=40, y=360, z=0, interest_level=0)],
    [Point(x=120, y=40, z=0, interest_level=0)],
    [Point(x=120, y=120, z=0, interest_level=0)],
    [Point(x=120, y=200, z=0, interest_level=0)],
    [Point(x=120, y=280, z=0, interest_level=0)],
    [Point(x=120, y=360, z=0, interest_level=0)],
    [Point(x=200, y=40, z=0, interest_level=0)],
    [Point(x=200, y=120, z=0, interest_level=0)],
    [Point(x=200, y=200, z=0, interest_level=0)],
    [Point(x=200, y=280, z=0, interest_level=0)],
    [Point(x=200, y=360, z=0, interest_level=0)],
    [Point(x=280, y=40, z=0, interest_level=0)],
    [Point(x=280, y=120, z=0, interest_level=0)],
    [Point(x=280, y=200, z=0, interest_level=0)],
    [Point(x=280, y=280, z=0, interest_level=0)],
    [Point(x=280, y=360, z=0, interest_level=0)],
    [Point(x=360, y=40, z=0, interest_level=0)],
    [Point(x=360, y=120, z=0, interest_level=0)],
    [Point(x=360, y=200, z=0, interest_level=0)],
    [Point(x=360, y=280, z=0, interest_level=0)],
    [Point(x=360, y=360, z=0, interest_level=0)],
    [Point(x=200, y=200, z=50, interest_level=0)],
    [Point(x=100, y=200, z=50, interest_level=0)],
    [Point(x=200, y=300, z=50, interest_level=0)],
    [Point(x=200, y=100, z=50, interest_level=0)],
    [Point(x=300, y=200, z=50, interest_level=0)]
]

# print(f'Number of drones: {len(drones)}')

trs = gen_trajectory(drones, palette=color_palette)
fig = go.Figure(data=[*rois, *trs])
# fig.update_layout(scene_aspectmode='manual', scene_aspectratio=dict(x=1, y=1, z=0.5))
fig.update_layout(scene_aspectmode='data')
fig.update_layout(showlegend=False)

fig.show()
