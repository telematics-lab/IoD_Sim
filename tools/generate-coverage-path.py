import numpy as np
import argparse
import sys

def main():
  parser = argparse.ArgumentParser(prog="Coverage Path Generator",
      description="This script generates a serpentine path for a drone to cover a given square area")
  parser.add_argument("-f", "--fileName",
      help="Name of the config file to create")
  parser.add_argument("-h", "--height", default=10.0,
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
      help="The duration of the simulation, drone speed will be calculated accordingly")
  parser.add_argument("-r", "--restTime", default=0.1,
      help="The duration of pause on each point, drone speed will be calculated accordingly")

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

  direction = 1 #1 = right/down, -1 = left/up

  points = []

  for _ in range(steps+1):
    points.append([round(point_x, 3), round(point_y, 3), round(point_z, 3)])
    for _ in range(steps):
      if (args.direction == "x"):
        point_x += stepSize*direction
      if (args.direction == "y"):
        point_y += stepSize*direction
      points.append([round(point_x, 3), round(point_y, 3), round(point_z, 3)])
    direction *= -1
    if (args.direction == "x"):
      point_y += stepSize
    if (args.direction == "y"):
      point_x += stepSize

  flightTime = duration - len(points)*restTime
  if flightTime <= 0:
    print("ERROR: Not enough time to complete the travel, please reduce --restTime or increase --duration")
    exit()

  # rounded up to 3 decimal places
  droneSpeed = np.ceil((len(points) * stepSize / flightTime) * 10**3) / 10**3

  if droneSpeed > 36:
    print(f"WARNING: The drone will be traveling too fast ({round(droneSpeed*3.6, 3)} km/h), results might be unrealistic")

  with open(args.fileName, "w") as config:
    indent = 0
    config.write("{\n")

    indent += 1 # level keys
    config.write("\t"*indent + f"\"duration\": {duration},\n")
    config.write("\t"*indent + "\"drones\": [\n")

    indent += 1 # level drones array object
    config.write("\t"*indent + "{\n")

    indent += 1 # level drone attributes key
    config.write("\t"*indent + "\"mobilityModel\": \"ns3::ParametricSpeedDroneMobilityModel\",\n")
    config.write("\t"*indent + f"\"speedCoefficients\": [0.0, {droneSpeed}],\n")

    config.write("\t"*indent + "\"trajectory\": [\n")
    indent += 1 # level trajectory array object
    for pt in points:
      config.write("\t"*indent + "{\n")
      indent += 1 # level trajectory point attributes key
      config.write("\t"*indent + f"\"position\": {pt},\n")
      config.write("\t"*indent + "\"interest\": 0,\n")
      config.write("\t"*indent + f"\"restTime\": {restTime}\n")
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



if __name__ == "__main__":
  main()