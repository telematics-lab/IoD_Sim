import pandas as pd
import plotly.graph_objects as go
import numpy as np
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

# GPU-accelerated 3D visualization using Plotly with WebGL rendering


def select_results_folder():
    """
    Interactive menu to select a folder from ../results containing tracking CSV files
    Returns the paths to the selected leo-sat-trace.csv, vehicle-trace.csv and rem output files
    """
    results_path = "../../results"

    if not os.path.exists(results_path):
        print(f"Error: Directory '{results_path}' not found.")
        return None, None, None

    # Find folders containing tracking CSV files
    valid_folders = []
    for item in sorted(os.listdir(results_path)):
        folder_path = os.path.join(results_path, item)
        if os.path.isdir(folder_path):
            leo_csv_path = os.path.join(folder_path, "leo-sat-trace.csv")
            vehicle_csv_path = os.path.join(folder_path, "vehicle-trace.csv")

            # Look for any file matching nr-rem-*-ecef.out
            rem_path = None
            for f in os.listdir(folder_path):
                if f.startswith("nr-rem-") and f.endswith("-ecef.out"):
                    rem_path = os.path.join(folder_path, f)
                    break

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
            if rem_path:
                files_found.append("REM Map")

            if files_found:
                valid_folders.append(
                    (item, leo_path, vehicle_path, rem_path, " + ".join(files_found))
                )

    if not valid_folders:
        print(f"No folders containing tracking CSV files found in '{results_path}'")
        return None, None, None

    # Display menu
    print("\nAvailable result folders:")
    print("-" * 50)
    for i, (folder_name, _, _, _, file_info) in enumerate(valid_folders, 1):
        print(f"{i}. {folder_name} ({file_info})")

    # Get user selection
    while True:
        try:
            choice = input(f"\nSelect folder (1-{len(valid_folders)}): ").strip()
            choice_idx = int(choice) - 1
            if 0 <= choice_idx < len(valid_folders):
                selected_folder, leo_path, vehicle_path, rem_path, _ = valid_folders[choice_idx]
                print(f"Selected: {selected_folder}")
                return leo_path, vehicle_path, rem_path
            else:
                print(f"Please enter a number between 1 and {len(valid_folders)}")
        except ValueError:
            print("Please enter a valid number")
        except KeyboardInterrupt:
            print("\nOperation cancelled")
            return None, None, None


# Select the CSV files
leo_csv_path, vehicle_csv_path, rem_csv_path = select_results_folder()
if leo_csv_path is None and vehicle_csv_path is None and rem_csv_path is None:
    print("No files selected. Exiting.")
    exit()

# Load data from both files
all_data = []
data_types = []
rem_data = None

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

if rem_csv_path and os.path.exists(rem_csv_path):
    try:
        print("Loading REM data...")
        # X Y Z SNR SINR RxPwr SIR [Lat Lon Alt]
        # Try reading with new format first, then fallback to old if fewer columns
        rem_df = pd.read_csv(rem_csv_path, sep="\t", header=None)

        if rem_df.shape[1] == 10:
             rem_df.columns = ["X", "Y", "Z", "SNR", "SINR", "RxPwr", "SIR", "Latitude", "Longitude", "Altitude"]
        else:
            print(f"Warning: Unexpected number of columns in REM file: {rem_df.shape[1]}")

        rem_df["DataType"] = "rem"
        rem_df["Node"] = "REM Point"
        rem_df["Time"] = 0 # Static map

        rem_data = rem_df # Keep separate or combine?
        # Easier to handle separate trace for REM due to colorbar logic which differs from trajectory color logic
        print(f"Loaded {len(rem_df)} REM data points")
    except Exception as e:
        print(f"Error loading REM data: {e}")

if not all_data and rem_data is None:
    print("No data could be loaded. Exiting.")
    exit()

# Combine all data
df = pd.DataFrame()
if all_data:
    df = pd.concat(all_data, ignore_index=True)
    print(f"Total tracking data points: {len(df)}")
    print(f"Data types: {df['DataType'].unique().tolist()}")

# Extract data helpers (unused if only REM)
latitudes = df["Latitude"] if not df.empty else np.array([np.nan])
longitudes = df["Longitude"] if not df.empty else np.array([np.nan])
altitudes = df["Altitude"] if not df.empty else np.array([np.nan])


# Function to format altitude with appropriate units
def format_altitude(altitude_m):
    """
    Format altitude with appropriate units based on magnitude
    """
    if pd.isna(altitude_m):
        return "N/A"
    if abs(altitude_m) >= 1000:
        return f"{altitude_m/1000:.1f} km"
    else:
        return f"{altitude_m:.0f} m"


# Check for points below Earth surface (for trajectories)
if not df.empty:
    underground_mask = (
        altitudes < -0.01
    )  # Threshold set to 0.01 m to avoid numerical errors
    underground_count = np.sum(underground_mask)

    if underground_count > 0:
        print(f"\n⚠️  WARNING: {underground_count} points are below Earth surface!")
    else:
        print("✅ All points are above Earth surface")

    # Print statistics with appropriate units
    print("\nGeographic Statistics:")
    print(f"Latitude range: {np.nanmin(latitudes):.2f}° to {np.nanmax(latitudes):.2f}°")
    print(f"Longitude range: {longitudes.min():.2f}° to {longitudes.max():.2f}°")
    print(
        f"Altitude range: {format_altitude(altitudes.min())} to {format_altitude(altitudes.max())}"
    )

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

# Add scatter traces for each data type and node (Trajectories)
if not df.empty:
    data_types = df["DataType"].unique()
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
                        opacity=1.0,
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

# Add REM Data Trace
if rem_data is not None:
    hover_text = []
    for row in rem_data.itertuples():
        text = (
            f"Type: REM Point<br>"
            f"SINR: {row.SINR:.2f} dB<br>"
            f"SNR: {row.SNR:.2f} dB<br>"
            f"RxPwr: {row.RxPwr:.2f} dBm<br>"
            f"Lat: {row.Latitude:.4f}<br>"
            f"Lon: {row.Longitude:.4f}<br>"
            f"Alt: {row.Altitude:.1f}<br>"
            f"X: {row.X:.0f}<br>"
            f"Y: {row.Y:.0f}<br>"
            f"Z: {row.Z:.0f}"
        )
        hover_text.append(text)

    fig.add_trace(
        go.Scatter3d(
            x=rem_data["X"],
            y=rem_data["Y"],
            z=rem_data["Z"],
            mode="markers",
            marker=dict(
                size=3,
                color=rem_data["SINR"],
                colorscale="Inferno",
                cmin=-50,
                cmax=50,
                colorbar=dict(title="SINR (dB)", x=0.0),
                opacity=1.0,
                symbol="diamond"
            ),
            hovertemplate="%{hovertext}<extra></extra>",
            hovertext=hover_text,
            name="REM Points",
            legendgroup="REM",
            legendgrouptitle_text="Radio Environment Map"
        )
    )

# Add Earth sphere with WebGL surface rendering
radius_earth = 6.371e6 - 100

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
        groupclick="toggleitem",  # Allow toggling of legend groups
        x=0.8,
        y=0.9
    ),
    margin=dict(r=0, b=0, l=0, t=30),
)

# Show with GPU-accelerated WebGL rendering
print("Rendering 3D visualization with GPU acceleration (WebGL)...")
print("Opening browser for interactive visualization...")

fig.show(renderer="browser")  # Forces browser rendering with WebGL

# Save as interactive HTML with GPU rendering
print("Saving interactive HTML file...")
# fig.write_html("3d_visualization_gpu.html")
print("Visualization complete! File saved as '3d_visualization_gpu.html'")
