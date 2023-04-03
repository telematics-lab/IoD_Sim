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

drone_palette = ['darkblue', 'red', 'darkcyan', 'darkviolet', 'darksalmon',
                 'darkolivegreen', 'darkorange', 'darkgreen']


def plot(rois, drones, ndrones):
    # Fix aspect ratio
    layout = go.Layout(
        width=700,
        height=700,
        margin=dict(autoexpand=False, b=0, l=0, r=0, t=0),
        scene=dict(
            aspectmode='manual',
            aspectratio=dict(
                x=1,
                y=1,
                z=0.6
            ),
            xaxis=dict(title='x [m]', range=[0, 1010]),
            yaxis=dict(title='y [m]', range=[0, 1010]),
            zaxis=dict(title='z [m]', range=[0, 10]),
            camera=dict(
                eye=dict(x=1.0, y=1.0, z=1.75),
                center=dict(x=0.1, y=0.05, z=0.0)
            )
        ),
        legend=dict(
            yanchor='top',
            xanchor='center',
            orientation='h',
            x=0.5,
            y=1,
            font=dict(
                size=20
            )
        )
    )

    drone_labels = []
    for i in range(1, ndrones + 1):
        drone_labels.append(go.Scatter3d(
            x=[-100, -101],
            y=[-100, -101],
            z=[-100, -101],
            line=dict(
                color=drone_palette[i-1],
                width=5
            ),
            marker=dict(
                size=0.1
            ),
            name=f'Drone {i}'
        ))

    fig = go.Figure(data=[*rois, *drones, *drone_labels], layout=layout)
    fig.show()

    return fig


# %%
rois = [
    # Buildings
    #   No buildings
    # RoIs
    # create_roi(170, 80, 13, 170, 90, 13, 'thistle')
]

drones = []

# Drone 1
d1ox = 30  # drone 1 origin over x axis
d1dx = 170  # drone 1 regular displacement over x axis
d1oy = 10  # drone 1 origin over y axis
d1dy = 30  # drone 1 reg. displacement over y axis
d1oz = 5  # drone 1 z axis

drones.append([
    Point([d1ox+0*d1dx, d1oy+0*d1dy, 0], interest_level=0),
    Point([d1ox+0*d1dx, d1oy+0*d1dy, d1oz], interest_level=1),
    Point([d1ox+1*d1dx, d1oy+0*d1dy, d1oz], interest_level=0),
    Point([d1ox+1*d1dx, d1oy+1*d1dy, d1oz], interest_level=0),
    Point([d1ox+2*d1dx, d1oy+1*d1dy, d1oz], interest_level=0),
    Point([d1ox+2*d1dx, d1oy+0*d1dy, d1oz], interest_level=0),
    Point([d1ox+3*d1dx, d1oy+0*d1dy, d1oz], interest_level=0),
    Point([d1ox+3*d1dx, d1oy+1*d1dy, d1oz], interest_level=0),
    Point([d1ox+4*d1dx, d1oy+1*d1dy, d1oz], interest_level=0),
    Point([d1ox+4*d1dx, d1oy+0*d1dy, d1oz], interest_level=0),
    Point([d1ox+5*d1dx, d1oy+0*d1dy, d1oz], interest_level=0),
    Point([d1ox+5*d1dx, d1oy+0*d1dy, d1oz], interest_level=0),
    Point([d1ox+5*d1dx, d1oy+0*d1dy, d1oz], interest_level=3),
    Point([d1ox+5*d1dx, d1oy+0*d1dy, 0], interest_level=0)
])

# Drone 2
d2ox = 900  # drone 2 origin over x axis
d2dx = -70  # drone 2 regular displacement over x axis
d2oy = 70  # drone 2 origin over y axis
d2dy = 40  # drone 2 reg. displacement over y axis
d2oz = 5  # drone 2 z axis

drones.append([
    Point([d2ox+0*d2dx, d2oy+0*d2dy, 0], interest_level=0),
    Point([d2ox+0*d2dx, d2oy+0*d2dy, d2oz], interest_level=1),
    Point([d2ox+1*d2dx, d2oy+0*d2dy, d2oz], interest_level=0),
    Point([d2ox+1*d2dx, d2oy+1*d2dy, d2oz], interest_level=0),
    Point([d2ox+2*d2dx, d2oy+1*d2dy, d2oz], interest_level=0),
    Point([d2ox+2*d2dx, d2oy+0*d2dy, d2oz], interest_level=0),
    Point([d2ox+3*d2dx, d2oy+0*d2dy, d2oz], interest_level=0),
    Point([d2ox+3*d2dx, d2oy+1*d2dy, d2oz], interest_level=0),
    Point([d2ox+4*d2dx, d2oy+1*d2dy, d2oz], interest_level=0),
    Point([d2ox+4*d2dx, d2oy+0*d2dy, d2oz], interest_level=0),
    Point([d2ox+5*d2dx, d2oy+0*d2dy, d2oz], interest_level=0),
    Point([d2ox+5*d2dx, d2oy+1*d2dy, d2oz], interest_level=0),
    Point([d2ox+6*d2dx, d2oy+1*d2dy, d2oz], interest_level=0),
    Point([d2ox+6*d2dx, d2oy+0*d2dy, d2oz], interest_level=0),
    Point([d2ox+7*d2dx, d2oy+0*d2dy, d2oz], interest_level=0),
    Point([d2ox+7*d2dx, d2oy+1*d2dy, d2oz], interest_level=0),
    Point([d2ox+8*d2dx, d2oy+1*d2dy, d2oz], interest_level=0),
    Point([d2ox+8*d2dx, d2oy+0*d2dy, d2oz], interest_level=0),
    Point([d2ox+9*d2dx, d2oy+0*d2dy, d2oz], interest_level=0),
    Point([d2ox+9*d2dx, d2oy+1*d2dy, d2oz], interest_level=0),
    Point([d2ox+10*d2dx, d2oy+1*d2dy, d2oz], interest_level=0),
    Point([d2ox+10*d2dx, d2oy+1*d2dy, d2oz], interest_level=3),
    Point([d2ox+10*d2dx, d2oy+1*d2dy, 0], interest_level=0)
])

# Drone 3
d3ox = 150  # drone 3 origin over x axis
d3dx = 250  # drone 3 regular displacement over x axis
d3oy = 300  # drone 3 origin over y axis
d3dy = 70  # drone 3 reg. displacement over y axis
d3oz = 7  # drone 3 z axis

drones.append([
    Point([d3ox+0*d3dx, d3oy+0*d3dy, 0], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+0*d3dy, d3oz], interest_level=1),
    Point([d3ox+0*d3dx, d3oy+1*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+1*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+2*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+2*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+3*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+3*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+4*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+4*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+5*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+5*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+6*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+6*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+7*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+7*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+8*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+8*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+9*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+9*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+10*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+10*d3dy, d3oz], interest_level=3),
    Point([d3ox+1*d3dx, d3oy+10*d3dy, 0], interest_level=0)
])

# Drone 4
d3ox = 500  # drone 3 origin over x axis
d3dx = 200  # drone 3 regular displacement over x axis
d3oy = 300  # drone 3 origin over y axis
d3dy = 70  # drone 3 reg. displacement over y axis
d3oz = 7  # drone 3 z axis

drones.append([
    Point([d3ox+0*d3dx, d3oy+0*d3dy, 0], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+0*d3dy, d3oz], interest_level=1),
    Point([d3ox+0*d3dx, d3oy+1*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+1*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+2*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+2*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+3*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+3*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+4*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+4*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+5*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+5*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+6*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+6*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+7*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+7*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+8*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+8*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+9*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+9*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+10*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+10*d3dy, d3oz], interest_level=3),
    Point([d3ox+1*d3dx, d3oy+10*d3dy, 0], interest_level=0)
])

# Drone 5
d3ox = 800  # drone 3 origin over x axis
d3dx = 150  # drone 3 regular displacement over x axis
d3oy = 300  # drone 3 origin over y axis
d3dy = 70  # drone 3 reg. displacement over y axis
d3oz = 7  # drone 3 z axis

drones.append([
    Point([d3ox+0*d3dx, d3oy+0*d3dy, 0], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+0*d3dy, d3oz], interest_level=1),
    Point([d3ox+0*d3dx, d3oy+1*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+1*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+2*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+2*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+3*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+3*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+4*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+4*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+5*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+5*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+6*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+6*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+7*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+7*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+8*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+8*d3dy, d3oz], interest_level=0),
    Point([d3ox+0*d3dx, d3oy+9*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+9*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+10*d3dy, d3oz], interest_level=0),
    Point([d3ox+1*d3dx, d3oy+10*d3dy, d3oz], interest_level=3),
    Point([d3ox+1*d3dx, d3oy+10*d3dy, 0], interest_level=0)
])

trs = gen_trajectory(drones, palette=drone_palette)
fig = plot(rois, trs, len(drones))

# %% Export proto-trajectories as JSON snippets
i = 0
j = [
    {
        "name": "FlightPlan",
        "value": [
            {
                "position": [float(p.x), float(p.y), float(p.z)],
                "interest": p.interest,
                "restTime": 1.0
            }
            for p in d
        ]
    }
    for d in drones
]

t = json.dumps(j)
with open('paper_s2-drones.json', 'w') as f:
  f.write(t)

# %% Export scenario design to PDF
fig.write_image('paper_s2.pdf')
