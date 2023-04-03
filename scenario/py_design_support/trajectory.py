import numpy as np
import plotly.graph_objects as go
from scipy.special import comb

from . import Point

default_palette = ['lime', 'dodgerblue', 'dodgerblue', 'yellow', 'yellow', 'yellow',
                   'yellow', 'darkorange', 'blue', 'red', 'darkorange', 'darkorange',
                   'green', 'lime', 'mediumblue', 'lime', 'lime', 'lime', 'yellow']


def build_drone_trajectory(drone_positions, step=0.01):
    """
    Build drone trajectory using Bézier Curve model.

    Args:
        drone_positions: Positions of the drone assumed, as an array of Point(s).
        step: Optional step for detailed curve generation.

    Returns:
        Modelled trajectory as an array of Point(s).
    """
    # Interest = 0 => start/stop of a given trajectory, cut the Bézier Curve.
    trajectories = []
    trajectory = []

    # populate trajectories
    for pos in drone_positions:
        trajectory.append(pos)

        if pos.interest == 0 and len(trajectory) > 1:
            trajectories.append(trajectory)
            # clean up
            trajectory = [pos]

    # tolerance in case the user forgot to add the last point with interest = 0
    if len(trajectory) > 1:
        trajectories.append(trajectory)

    # construct trajectory
    B = []  # Bezier curve, our trajectory

    for tr in trajectories:
        for t in np.arange(0, 1, step):
            sum_x = 0.0
            sum_y = 0.0
            sum_z = 0.0
            n = len(tr) - 1

            for i in range(n + 1):
                a = comb(n, i, exact=True)
                b = (1 - t)**(n - i)
                c = t**i

                sum_x += a * b * c * tr[i].x
                sum_y += a * b * c * tr[i].y
                sum_z += a * b * c * tr[i].z

            B.append(Point([sum_x, sum_y, sum_z]))

    return B


def bezier(pois):
    proto = []

    # fill the proto
    for p in pois:
        l = p.interest if p.interest != 0 else 1
        for _ in range(l):
            proto.append(p)

    c = 0.001
    START = 0
    END = 1 + c
    STEP = 0.01

    B = []  # Bezier curve points

    for t in np.arange(START, END, STEP):
        sum_x = 0.0
        sum_y = 0.0
        sum_z = 0.0
        n = len(proto) - 1

        for i in range(n+1):
            a = comb(n, i, exact=True)
            b = (1 - t)**(n - i)
            c = t**i

            sum_x += a * b * c * proto[i].x
            sum_y += a * b * c * proto[i].y
            sum_z += a * b * c * proto[i].z

        B.append(Point([sum_x, sum_y, sum_z]))

    return B


def split_trajectory(points):
    trs = []

    j = 0

    for i in range(len(points)):
        if i == 0:
            trs.append([points[i]])
        else:
            trs[j].append(points[i])

            if points[i].interest == 0:
                j += 1
                trs.append([points[i]])

    return trs


def create_curve(points, color='darkblue'):
    x = [p.x for p in points]
    y = [p.y for p in points]
    z = [p.z for p in points]

    return go.Scatter3d(
        x=x,
        y=y,
        z=z,
        showlegend=False,
        marker=dict(
            size=0.1
        ),
        line=dict(
            color=color,
            width=5
        ),
        opacity=1
    )


def gen_trajectory(drones, palette=default_palette):
    tr_plots = []

    i = 0
    for d in drones:
        trajectories = split_trajectory(d)
        for tr in trajectories:
            curve = bezier(tr)
            plot = create_curve(curve, palette[i % len(palette)])
            tr_plots.append(plot)
        i += 1

    i = 0
    for d in drones:
        start = d[0]
        end = d[-1]
        marker = 'circle'
        tr_plots.append(go.Scatter3d(
            x=[start.x],
            y=[start.y],
            z=[start.z],
            showlegend=False,
            marker_symbol=marker,
            marker_color=palette[i % len(palette)],
            marker_size=4,
            opacity=1
        ))
        tr_plots.append(go.Scatter3d(
            x=[end.x],
            y=[end.y],
            z=[end.z],
            showlegend=False,
            marker_symbol='x',
            marker_color=palette[i % len(palette)],
            marker_size=4,
            opacity=1
        ))

        i += 1

    return tr_plots
