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
import json
import plotly.graph_objects as go
from py_design_support import gen_trajectory, Point

# %%
ground_nodes = [
    [Point(x=50, y=200, z=0, interest_level=0)],
    [Point(x=260, y=303.923, z=1, interest_level=0)],
    [Point(x=290, y=355.885, z=1, interest_level=0)],
    [Point(x=282.765, y=99.074, z=0, interest_level=0)],
    [Point(x=303.978, y=62.331, z=0, interest_level=0)],
    [Point(x=267.235, y=41.118, z=0, interest_level=0)],
    [Point(x=246.022, y=77.860, z=0, interest_level=0)],
    [Point(x=200, y=200, z=30, interest_level=0)]
]
a = 1
b = -400

drone = [
    [Point(x=float(x),
           y=float((-1*b-pow((pow(b, 2)-4*a*(pow(x, 2)-400*x+57500)), 0.5))/(2*a)),
           z=50.0)
     for x in range(125, 50, -1)] +
    [Point(x=float(x),
           y=float((-1*b+pow((pow(b, 2)-4*a*(pow(x, 2)-400*x+57500)), 0.5))/(2*a)),
           z=50.0)
     for x in range(50, 351, 1)] +
    [Point(x=float(x),
           y=float((-1*b-pow((pow(b, 2)-4*a*(pow(x, 2)-400*x+57500)), 0.5))/(2*a)),
           z=50.0)
     for x in range(349, 125, -1)]
]

# print(f'Number of drones: {len(drone)}')

trs = gen_trajectory(ground_nodes + drone)
fig = go.Figure(data=[*trs])
# fig.update_layout(scene_aspectmode='manual', scene_aspectratio=dict(x=1, y=1, z=0.5))
fig.update_layout(scene_aspectmode='data')
fig.update_layout(showlegend=False)
fig.show()

# %% Generate JSON configuration for the drone

print(json.dumps([{
    "mobilityModel": {
      "name": "ns3::ParametricSpeedDroneMobilityModel",
      "attributes": [
          {
              "name": "SpeedCoefficients",
              "value": [10.0]
          },
          {
              "name": "FlightPlan",
              "value": [
                  {
                      "position": [float(p.x), float(p.y), float(p.z)],
                      "interest": int(p.interest),
                      "restTime": 0.0
                  }
                  for p in drone[0]]
          }
      ]}
}]))
