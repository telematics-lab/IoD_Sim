"""
This scripts generates the plot of a Radio Environment Map using a .rem file.
Simply pass the path to the .rem file as an argument and a .png file is generated
in the same folder with same name and -plot as suffix.
"""

import sys

import matplotlib.cm as cm
import matplotlib.pyplot as plt
import numpy as np

assert len(sys.argv) > 1, "Not enough arguments passed"

isNrFile = "--nrMap" in sys.argv


filepath = sys.argv[1]

loaded_data = np.loadtxt(filepath).T
if isNrFile:
    x, y, z, snr, values, rx_power, sir = loaded_data
else:
    x, y, z, values = loaded_data


# print(x, y, z, values)

values = np.reshape(values, (len(set(x)), len(set(y)))).T
values = np.flipud(values)
minimum = values.min()
maximum = values.max()

x_range = int(x.max() - x.min())
y_range = int(y.max() - y.min())
ratio_x = x_range / np.gcd(x_range, y_range)
ratio_y = y_range / np.gcd(x_range, y_range)

# print(ratio_x, ratio_y)
# plt.rcParams["figure.figsize"] = [ratio_x,ratio_y]

im = plt.imshow(
    values,
    vmin=-50,
    vmax=50,
    extent=(x.min(), x.max(), y.min(), y.max()),
    interpolation="nearest",
    cmap=cm.inferno,
)

cbar = plt.colorbar(im, label="SINR (dB)" if isNrFile else "SNR (dB)")
# plt.axis("tight")


name = ".".join(filepath.split(".")[:-1])
plt.savefig(name + "-preview")
