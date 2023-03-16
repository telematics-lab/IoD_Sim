from plyfile import PlyData, PlyElement
import matplotlib.pyplot as plt
import numpy as np
import sys

assert len(sys.argv) > 1, "Not enough arguments passed"
ply_file_path=sys.argv[1]


# Load the PLY file
plydata = PlyData.read(ply_file_path)

# Extract vertex data
vertices = np.array(plydata['vertex'].data.tolist())

# Extract point positions
positions = vertices[:, 0:3]

# Extract point intensities
intensities = vertices[:, 3]

# Create a 3D plot
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# Plot the point cloud with intensities
ax.scatter(positions[:, 0], positions[:, 1], positions[:, 2], c=intensities/max(intensities), s=1, marker='o')

# Set the axis labels
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')

# Save the figure
name = ".".join(ply_file_path.split(".")[:-1])
plt.savefig(name + "-preview", dpi=300, bbox_inches='tight')