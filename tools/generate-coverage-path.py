'''
Coverage Path Generator - Michele Abruzzese 2021
This script generates or plots the result of a serpentine path for a drone to cover a given square area.
The available options and parameters can be listed passing the -h or --help flag.

Beware that for the moment plotting works only if a single drone is present in the simulation.
If more than one drone is present the plot (if successful) will not be meaningful.
'''

import matplotlib.pyplot as plt
import matplotlib.cm as cm
from matplotlib.collections import LineCollection
from matplotlib.colors import ListedColormap, BoundaryNorm
import numpy as np
import argparse
import sys

def main():
  parser = argparse.ArgumentParser(prog="Coverage Path Generator",
      description="This script generates or plots the result of a serpentine path for a drone to cover a given square area")
  parser.add_argument("-p", "--plot", action='store_true', default=False,
      help="If present, the script generates the plot of a result instead of the config file (works only with waypoints and a single drone)")
  parser.add_argument("-f", "--fileName",
      help="Name of the config file to create or the file to use as source (when using --plot)")
  parser.add_argument("-z", "--height", default=10.0,
      help="The constant Z coordinate of the path")
  parser.add_argument("-o", "--origin", nargs=2, default=[0, 0],
      help="The top-left/central point of the square")
  parser.add_argument("--originType", choices=["topleft", "center"], default="center",
      help="If origin refers to the top-left corner of the square or its center")
  parser.add_argument("-w", "--size", default=200,
      help="The size of the square along each direction")
  parser.add_argument("-s", "--steps", default=10, type=int,
      help="The number of subdivisions along each direction")
  parser.add_argument("--direction", choices=["x", "y"], default="x",
      help="The direction of the straight pieces of the path")
  parser.add_argument("-d", "--duration", default=100.0,
      help="The duration of the simulation, drone speed or time step will be calculated accordingly")
  parser.add_argument("-r", "--restTime", default=0.1,
      help="The duration of pause on each point, drone speed or time step will be calculated accordingly")
  parser.add_argument("--trajectoryType", choices=["waypoints", "flightplan"], default="waypoints",
      help="The type of points to generate, whether to be used with WaypointMobilityModel (waypoints) or ParametricSpeedDroneMobilityModel (flightplan)")
  parser.add_argument("--color", default="rainbow",
      help="The colormap of the plot, check matplotlib.cm for available options")

  args = parser.parse_args(sys.argv[1:])

  point_x, point_y = map(float, args.origin)
  point_z = float(args.height)
  size = float(args.size)
  steps = int(args.steps)
  duration = float(args.duration)
  restTime = float(args.restTime)
  if args.originType == "center":
    point_x -= size / 2
    point_y -= size / 2

  stepSize = size / steps

  pointsN = steps * (steps + 2) + 1
  flightTime = duration - pointsN * restTime - 0.2

  if flightTime <= 0:
    print("ERROR: Not enough time to complete the travel, please reduce --restTime or increase --duration")
    exit()

  timeStep = flightTime / (pointsN - 1)
  droneSpeed = np.ceil((stepSize / timeStep) * 10**3) / 10**3

  if droneSpeed > 36:
    print(f"WARNING: The drone is traveling too fast ({round(droneSpeed*3.6, 3)} km/h), results might be unrealistic")


  timeMap = {}
  points = []
  direction = 1 #1 = right/down, -1 = left/up
  time = 0.2
  for _ in range(steps+1):
    points.append([round(point_x, 3), round(point_y, 3), round(point_z, 3)])
    timeMap[round(time, 3)] = points[-1]
    time += restTime
    timeMap[round(time, 3)] = points[-1]
    time += timeStep
    for _ in range(steps):
      if (args.direction == "x"):
        point_x += stepSize*direction
      if (args.direction == "y"):
        point_y += stepSize*direction
      points.append([round(point_x, 3), round(point_y, 3), round(point_z, 3)])
      timeMap[round(time, 3)] = points[-1]
      time += restTime
      timeMap[round(time, 3)] = points[-1]
      time += timeStep
    direction *= -1
    if (args.direction == "x"):
      point_y += stepSize
    if (args.direction == "y"):
      point_x += stepSize


  if not args.plot:

    with open(args.fileName, "w") as config:
      indent = 0
      config.write("{\n")

      indent += 1 # level keys
      config.write("\t"*indent + f"\"duration\": {duration},\n")
      config.write("\t"*indent + "\"dronesMobilityModel\": \"mixed\",\n")
      config.write("\t"*indent + "\"drones\": [\n")

      indent += 1 # level drones array object
      config.write("\t"*indent + "{\n")

      indent += 1 # level drone attributes key
      config.write("\t"*indent + "\"name\": \"drone_coverage\",\n")
      config.write("\t"*indent + "\"interfaces\": [0],\n")
      if (args.trajectoryType == "waypoints"):
        config.write("\t"*indent + "\"mobilityModel\": \"ns3::WaypointMobilityModel\",\n")
      elif (args.trajectoryType == "flightplan"):
        config.write("\t"*indent + "\"mobilityModel\": \"ns3::ParametricSpeedDroneMobilityModel\",\n")
        config.write("\t"*indent + f"\"speedCoefficients\": [{droneSpeed}],\n")

      config.write("\t"*indent + "\"trajectory\": [\n")
      indent += 1 # level trajectory array object

      time = 0.2
      for pt in points:
        config.write("\t"*indent + "{\n")
        indent += 1 # level trajectory point attributes key
        config.write("\t"*indent + f"\"position\": {pt},\n")
        if (args.trajectoryType == "flightplan"):
          config.write("\t"*indent + "\"interest\": 0,\n")
          config.write("\t"*indent + f"\"restTime\": {restTime}\n")
        elif (args.trajectoryType == "waypoints"):
          config.write("\t"*indent + f"\"time\": {round(time, 3)}\n")
          if (restTime > 0):
            indent -= 1 # level trajectory array object
            config.write("\t"*indent + "},\n")
            # duplicate the position at time + restTime so it waits there
            time += restTime
            config.write("\t"*indent + "{\n")
            indent += 1 # level trajectory point attributes key
            config.write("\t"*indent + f"\"position\": {pt},\n")
            config.write("\t"*indent + f"\"time\": {round(time, 3)}\n")
          time += timeStep
        indent -= 1 # level trajectory array object
        if pt is points[-1]: # last point, no comma required
          config.write("\t"*indent + "}\n")
        else:
          config.write("\t"*indent + "},\n")

      indent -= 1 # level drone attributes key
      config.write("\t"*indent + "]\n")

      indent -= 1 # level drones array object
      config.write("\t"*indent + "}\n")

      indent -= 1 # level keys
      config.write("\t"*indent + "]\n")

      indent = 0
      config.write("}\n")

  else:

    times, _, _, _, rsrp, sinr, _ = np.loadtxt(args.fileName).T

    # convert time to position
    x = []
    y = []
    time = 0.2
    resting = True

    for t in times:
      while t >= time:
        if resting:
          time += restTime
          resting = False
        else:
          time += timeStep
          resting = True
      if resting:
        # the time is before resting, so the drone is traveling
        # lets interpolate this point with the previous to get the position at an intermediate time
        time_p = time - timeStep
        diff = t - time_p
        perc = diff / timeStep
        ax, ay, _ = timeMap[round(time_p, 3)]
        bx, by, _ = timeMap[round(time, 3)]
        pos_x = ax * (1-perc) + bx * perc
        pos_y = ay * (1-perc) + by * perc
        x.append(round(pos_x, 3))
        y.append(round(pos_y, 3))
      else:
        # the time is during resting, so the drone is still
        # so the position is the same as the one at the previous time point
        time_p = time - restTime
        pos_x, pos_y, _ = timeMap[round(time_p, 3)]
        x.append(round(pos_x, 3))
        y.append(round(pos_y, 3))

    # Create a set of line segments so that we can color them individually
    # This creates the points as a N x 1 x 2 array so that we can stack points
    # together easily to get the segments. The segments array for line collection
    # needs to be (numlines) x (points per line) x 2 (for x and y)
    points = np.array([x, y]).T.reshape(-1, 1, 2)
    segments = np.concatenate([points[:-1], points[1:]], axis=1)

    watt2dbm = lambda x: 10*np.log10(1000*x)
    rsrpDBm = watt2dbm(rsrp)

    fig, axs = plt.subplots(1, 1, sharex=True, sharey=True)
    # Create a continuous norm to map from data points to colors
    norm = plt.Normalize(rsrpDBm.min(), rsrpDBm.max())
    lc = LineCollection(segments, cmap='rainbow', norm=norm)
    # Set the values used for colormapping
    lc.set_array(rsrpDBm)
    lc.set_linewidth(2)
    line = axs.add_collection(lc)
    fig.colorbar(line, ax=axs)

    axs.set_xlim(min(x)-10, max(x)+10)
    axs.set_ylim(min(y)-10, max(y)+10)
    #plt.show()

    name = ".".join(args.fileName.split(".")[:-1])
    plt.savefig(name + "-RSRPplot")

    lin2db = lambda x: 10*np.log10(x)
    sinrDB = lin2db(sinr)

    plt.clf()
    fig, axs = plt.subplots(1, 1, sharex=True, sharey=True)
    # Create a continuous norm to map from data points to colors
    norm = plt.Normalize(sinrDB.min(), sinrDB.max())
    lc = LineCollection(segments, cmap='rainbow', norm=norm)
    # Set the values used for colormapping
    lc.set_array(sinrDB)
    lc.set_linewidth(2)
    line = axs.add_collection(lc)
    fig.colorbar(line, ax=axs)

    axs.set_xlim(min(x)-10, max(x)+10)
    axs.set_ylim(min(y)-10, max(y)+10)
    #plt.show()

    name = ".".join(args.fileName.split(".")[:-1])
    plt.savefig(name + "-SINRplot")


if __name__ == "__main__":
  main()