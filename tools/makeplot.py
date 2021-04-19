import matplotlib.pyplot as plt
import matplotlib.cm as cm
import numpy as np
import sys

assert len(sys.argv) > 1, "Not enough arguments passed"

filepath = sys.argv[1]

x,y,z,values = np.loadtxt(filepath).T

print(x, y, z)

values = np.reshape(values, (len(set(x)), len(set(y)))).T
values = np.flipud(values)

print(values)

minimum = values.min()
maximum = values.max()

def normalize(x):
  return 100*(10*np.log10(x) - minimum)/(maximum-minimum)

values = normalize(values)

x_range = int(x.max()-x.min())
y_range = int(y.max()-y.min())
ratio_x = x_range / np.gcd(x_range, y_range)
ratio_y = y_range / np.gcd(x_range, y_range)
print(ratio_x, ratio_y)
#plt.rcParams["figure.figsize"] = [ratio_x,ratio_y]

plt.imshow(values, extent=(x.min(), x.max(), y.min(), y.max()),
           interpolation='nearest', cmap=cm.plasma)
#plt.axis("tight")

name = ".".join(filepath.split(".")[:-1])
plt.savefig(name + "-plot")
