import pandas as pd
import plotly.graph_objects as go
import numpy as np
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

# GPU-accelerated 3D visualization using Plotly with WebGL rendering


def select_results_folder():
    """
    Interactive menu to select a folder from ../results containing tracking CSV files
    Returns the paths to the selected leo-sat-trace.csv and vehicle-trace.csv files
    """
    results_path = "../../results"

    if not os.path.exists(results_path):
        print(f"Error: Directory '{results_path}' not found.")
        return None, None

    # Find folders containing tracking CSV files
    valid_folders = []
    for item in sorted(os.listdir(results_path)):
        folder_path = os.path.join(results_path, item)
        if os.path.isdir(folder_path):
            leo_csv_path = os.path.join(folder_path, "leo-sat-trace.csv")
            vehicle_csv_path = os.path.join(folder_path, "vehicle-trace.csv")

            # Check which files exist
            files_found = []
            leo_path = leo_csv_path if os.path.exists(leo_csv_path) else None
            vehicle_path = (
                vehicle_csv_path if os.path.exists(vehicle_csv_path) else None
            )

            if leo_path:
                files_found.append("LEO satellites")
            if vehicle_path:
                files_found.append("vehicles")

            if files_found:
                valid_folders.append(
                    (item, leo_path, vehicle_path, " + ".join(files_found))
                )

    if not valid_folders:
        print(f"No folders containing tracking CSV files found in '{results_path}'")
        return None, None

    # Display menu
    print("\nAvailable result folders:")
    print("-" * 50)
    for i, (folder_name, _, _, file_info) in enumerate(valid_folders, 1):
        print(f"{i}. {folder_name} ({file_info})")

    # Get user selection
    while True:
        try:
            choice = input(f"\nSelect folder (1-{len(valid_folders)}): ").strip()
            choice_idx = int(choice) - 1
            if 0 <= choice_idx < len(valid_folders):
                selected_folder, leo_path, vehicle_path, _ = valid_folders[choice_idx]
                print(f"Selected: {selected_folder}")
                return leo_path, vehicle_path
            else:
                print(f"Please enter a number between 1 and {len(valid_folders)}")
        except ValueError:
            print("Please enter a valid number")
        except KeyboardInterrupt:
            print("\nOperation cancelled")
            return None, None


# Select the CSV files
leo_csv_path, vehicle_csv_path = select_results_folder()
if leo_csv_path is None and vehicle_csv_path is None:
    print("No files selected. Exiting.")
    exit()

# Load data from both files
all_data = []
data_types = []

if leo_csv_path and os.path.exists(leo_csv_path):
    try:
        print("Loading LEO satellite data...")
        leo_df = pd.read_csv(leo_csv_path)
        leo_df["DataType"] = "leo-sat"
        all_data.append(leo_df)
        print(f"Loaded {len(leo_df)} LEO satellite data points")
    except Exception as e:
        print(f"Error loading LEO satellite data: {e}")

if vehicle_csv_path and os.path.exists(vehicle_csv_path):
    try:
        print("Loading vehicle data...")
        vehicle_df = pd.read_csv(vehicle_csv_path)
        vehicle_df["DataType"] = "vehicle"
        all_data.append(vehicle_df)
        print(f"Loaded {len(vehicle_df)} vehicle data points")
    except Exception as e:
        print(f"Error loading vehicle data: {e}")

if not all_data:
    print("No data could be loaded. Exiting.")
    exit()

# Combine all data
df = pd.concat(all_data, ignore_index=True)
print(f"Total data points: {len(df)}")
print(f"Data types: {df['DataType'].unique().tolist()}")
print(f"Columns: {df.columns.tolist()}")
print(f"First 5 rows:\n{df.head()}")

# Extract data
whois = df["Node"]
x = df["X"]
y = df["Y"]
z = df["Z"]
latitudes = df["Latitude"]
longitudes = df["Longitude"]
altitudes = df["Altitude"]


# Function to format altitude with appropriate units
def format_altitude(altitude_m):
    """
    Format altitude with appropriate units based on magnitude
    """
    if abs(altitude_m) >= 1000:
        return f"{altitude_m/1000:.1f} km"
    else:
        return f"{altitude_m:.0f} m"


# Check for points below Earth surface
underground_mask = (
    altitudes < -0.01
)  # Threshold set to 0.01 m to avoid numerical errors
underground_count = np.sum(underground_mask)

if underground_count > 0:
    print(f"\n⚠️  WARNING: {underground_count} points are below Earth surface!")
    underground_points = df[underground_mask][
        ["Node", "Time", "X", "Y", "Z", "Latitude", "Longitude", "Altitude"]
    ]
    print("Underground points details:")
    print(underground_points.to_string(index=False))
    print(f"Most underground point: {format_altitude(altitudes.min())} below surface")

    # Group by node to see which nodes have underground points
    nodes_underground = underground_points["Node"].unique()
    print(f"Nodes with underground points: {nodes_underground}")
else:
    print("✅ All points are above Earth surface")

# Print statistics with appropriate units
print("\nGeographic Statistics:")
print(f"Latitude range: {np.nanmin(latitudes):.2f}° to {np.nanmax(latitudes):.2f}°")
print(f"Longitude range: {longitudes.min():.2f}° to {longitudes.max():.2f}°")
print(
    f"Altitude range: {format_altitude(altitudes.min())} to {format_altitude(altitudes.max())}"
)
print(f"Average altitude: {format_altitude(np.nanmean(altitudes))}")

# Get unique nodes and data types
nodi_unici = df["Node"].unique()
data_types = df["DataType"].unique()

# Define colors for different data types
type_colors = {
    "leo-sat": ["red", "orange", "yellow", "pink", "purple"],
    "vehicle": ["blue", "green", "cyan", "brown", "gray"],
}

type_names = {
    "leo-sat": "LEO Satellite",
    "vehicle": "Vehicle",
}

# Define marker symbols for different data types
type_symbols = {"leo-sat": "circle", "vehicle": "square"}

# Create figure with GPU-accelerated WebGL rendering
fig = go.Figure()

# Add scatter traces for each data type and node
for data_type in data_types:
    type_df = df[df["DataType"] == data_type]
    type_nodes = type_df["Node"].unique()
    colors = type_colors.get(data_type, ["gray"])
    symbol = type_symbols.get(data_type, "circle")

    for i, nodo in enumerate(type_nodes):
        # Filter and sort by time to ensure correct line drawing
        mask = (df["Node"] == nodo) & (df["DataType"] == data_type)
        node_df = df[mask].sort_values("Time")

        colore = colors[i % len(colors)]

        # Create hover text with geographic coordinates
        hover_text = []
        for idx in node_df.index:
            text = (
                f"Type: {type_names.get(data_type, data_type)}<br>"
                f"Node: {nodo}<br>"
                f"Lat: {node_df.loc[idx, 'Latitude']:.3f}°<br>"
                f"Lon: {node_df.loc[idx, 'Longitude']:.3f}°<br>"
                f"Alt: {format_altitude(node_df.loc[idx, 'Altitude'])}<br>"
                f"X: {node_df.loc[idx, 'X']:.0f}<br>"
                f"Y: {node_df.loc[idx, 'Y']:.0f}<br>"
                f"Z: {node_df.loc[idx, 'Z']:.0f}"
            )
            hover_text.append(text)

        fig.add_trace(
            go.Scatter3d(
                x=node_df["X"],
                y=node_df["Y"],
                z=node_df["Z"],
                mode="lines+markers",
                marker=dict(
                    color=colore,
                    size=2 if data_type == "leo-sat" else 3,
                    symbol=symbol,
                    opacity=0.8,
                    line=dict(width=0.3, color=colore),
                ),
                line=dict(
                    color=colore,
                    width=4,
                ),
                hovertemplate="%{hovertext}<extra></extra>",
                hovertext=hover_text,
                name=f"{type_names.get(data_type, data_type)} - Node {nodo}",
                legendgroup=data_type,
                legendgrouptitle_text=type_names.get(data_type, data_type),
            )
        )

# Add Earth sphere with WebGL surface rendering
radius_earth = 6.371e6 - 10

# Create sphere data
phi, theta = np.mgrid[0 : np.pi : 50j, 0 : 2 * np.pi : 50j]
x_sphere = radius_earth * np.sin(phi) * np.cos(theta)
y_sphere = radius_earth * np.sin(phi) * np.sin(theta)
z_sphere = radius_earth * np.cos(phi)

fig.add_trace(
    go.Surface(
        x=x_sphere,
        y=y_sphere,
        z=z_sphere,
        colorscale=[[0, "darkgreen"], [1, "darkgreen"]],
        showscale=False,
        showlegend=False,
        hoverinfo="skip",
        opacity=0.3,
        name="Earth",
    )
)


# Configure layout for optimal GPU rendering
fig.update_layout(
    title="3D Visualization - LEO Satellites and Vehicles",
    scene=dict(
        xaxis_title="X Axis (meters)",
        yaxis_title="Y Axis (meters)",
        zaxis_title="Z Axis (meters)",
        aspectmode="data",  # Equal aspect ratio
        xaxis=dict(showspikes=False),
        yaxis=dict(showspikes=False),
        zaxis=dict(showspikes=False),
    ),
    legend=dict(
        groupclick="toggleitem"  # Allow toggling of legend groups
    ),
)

# Show with GPU-accelerated WebGL rendering
print("Rendering 3D visualization with GPU acceleration (WebGL)...")
print("Opening browser for interactive visualization...")

fig.show(renderer="browser")  # Forces browser rendering with WebGL

# Save as interactive HTML with GPU rendering
print("Saving interactive HTML file...")
# fig.write_html("3d_visualization_gpu.html")
print("Visualization complete! File saved as '3d_visualization_gpu.html'")
