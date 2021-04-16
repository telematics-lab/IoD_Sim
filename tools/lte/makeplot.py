import matplotlib.pyplot as plt
import numpy as np
import sys

assert len(sys.argv) > 1, "Not enough arguments passed"

filepath = sys.argv[1]

f = open(filepath, "r")

name = ".".join(filepath.split(".")[:-1])

x_ax = set()
y_ax = set()
sinr_val = []

for line in f:
  x, y, z, sinr = map(float, line.split("\t"))
  x_ax.add(x)
  y_ax.add(y)
  sinr_val.append(sinr)

sinr_val = np.array(sinr_val)
sinr_val = np.reshape(sinr_val, (len(x_ax), len(y_ax)))

# find a way to use real axis value instead of range
plt.pcolormesh(list(range(len(x_ax))), list(range(len(y_ax))), sinr_val, shading='auto')

plt.savefig(name + "-plot")
