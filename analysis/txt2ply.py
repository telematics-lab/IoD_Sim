import sys

assert len(sys.argv) > 1, "Not enough arguments passed"
txt_file_path = sys.argv[1]
name = ".".join(txt_file_path.split(".")[:-1])
ply_file_path = name + ".ply"
# Open the .txt file and read its contents
with open(txt_file_path, "r") as txt_file:
    txt_contents = txt_file.read().splitlines()

# Extract the vertices and faces from the .txt file
vertices = []
faces = []
for line in txt_contents:
    vertex = [float(x) for x in line.split("\t")]
    vertices.append(vertex)


# Write the vertices and faces to a new .ply file
with open(ply_file_path, "w") as ply_file:
    ply_file.write("ply\n")
    ply_file.write("format ascii 1.0\n")
    ply_file.write("element vertex {}\n".format(len(vertices)))
    ply_file.write("property float x\n")
    ply_file.write("property float y\n")
    ply_file.write("property float z\n")
    ply_file.write("property float intensity\n")
    ply_file.write("end_header\n")
    for vertex in vertices:
        ply_file.write(
            "{} {} {} {}\n".format(vertex[0], vertex[1], vertex[2], vertex[3])
        )
