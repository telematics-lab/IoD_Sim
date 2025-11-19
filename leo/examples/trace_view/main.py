import pandas as pd
import plotly.graph_objects as go
import numpy as np
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

# GPU-accelerated 3D visualization using Plotly with WebGL rendering

try:
    print("Loading CSV data...")
    df = pd.read_csv('punti_3d.csv')
    print(f"Loaded {len(df)} data points")
    print(f"Columns: {df.columns.tolist()}")
    print(f"First 5 rows:\n{df.head()}")
except FileNotFoundError:
    print("Errore: il file 'punti_3d.csv' non è stato trovato.")
    print("Assicurati che il file si trovi nella stessa directory dello script o specifica il percorso completo.")
    exit()

# Extract data
whois = df["Node"]
x = df['X']
y = df['Y']
z = df['Z']
latitudes = df['Latitude']
longitudes = df['Longitude']
altitudes = df['Altitude']

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
underground_mask = altitudes < -0.01 # Threshold set to 0.01 m to avoid numerical errors
underground_count = np.sum(underground_mask)

if underground_count > 0:
    print(f"\n⚠️  WARNING: {underground_count} points are below Earth surface!")
    underground_points = df[underground_mask][['Node', 'Time', 'X', 'Y', 'Z', 'Latitude', 'Longitude', 'Altitude']]
    print("Underground points details:")
    print(underground_points.to_string(index=False))
    print(f"Most underground point: {format_altitude(altitudes.min())} below surface")
    
    # Group by node to see which nodes have underground points
    nodes_underground = underground_points['Node'].unique()
    print(f"Nodes with underground points: {nodes_underground}")
else:
    print("✅ All points are above Earth surface")

# Print statistics with appropriate units
print("\nGeographic Statistics:")
print(f"Latitude range: {np.nanmin(latitudes):.2f}° to {np.nanmax(latitudes):.2f}°")
print(f"Longitude range: {longitudes.min():.2f}° to {longitudes.max():.2f}°")
print(f"Altitude range: {format_altitude(altitudes.min())} to {format_altitude(altitudes.max())}")
print(f"Average altitude: {format_altitude(np.nanmean(altitudes))}")

# Get unique nodes and colors
nodi_unici = whois.unique()
colori = ['red', 'blue', 'green', 'orange', 'purple', 'brown', 'pink', 'gray', 'olive', 'cyan']

# Create figure with GPU-accelerated WebGL rendering
fig = go.Figure()

# Add scatter traces for each node with WebGL for GPU acceleration
for i, nodo in enumerate(nodi_unici):
    mask = whois == nodo
    colore = colori[i % len(colori)]
    
    # Create hover text with geographic coordinates
    hover_text = []
    for idx in df[mask].index:
        text = (f"Node: {nodo}<br>"
                f"Lat: {df.loc[idx, 'Latitude']:.3f}°<br>"
                f"Lon: {df.loc[idx, 'Longitude']:.3f}°<br>"
                f"Alt: {format_altitude(df.loc[idx, 'Altitude'])}<br>"
                f"X: {df.loc[idx, 'X']:.0f}<br>"
                f"Y: {df.loc[idx, 'Y']:.0f}<br>"
                f"Z: {df.loc[idx, 'Z']:.0f}")
        hover_text.append(text)
    
    fig.add_trace(go.Scatter3d(
        x=x[mask],
        y=y[mask], 
        z=z[mask],
        mode='markers',
        marker=dict(
            color=colore,
            size=3,
            opacity=1,
            line=dict(width=0.3, color=colore)
        ),
        hovertemplate='%{hovertext}<extra></extra>',
        hovertext=hover_text,
        name=f'Nodo {nodo}'
    ))

# Add Earth sphere with WebGL surface rendering
radius_earth = 6.371e6 - 10

# Create sphere data
phi, theta = np.mgrid[0:np.pi:50j, 0:2*np.pi:50j]
x_sphere = radius_earth * np.sin(phi) * np.cos(theta)
y_sphere = radius_earth * np.sin(phi) * np.sin(theta) 
z_sphere = radius_earth * np.cos(phi)

fig.add_trace(go.Surface(
    x=x_sphere,
    y=y_sphere,
    z=z_sphere,
    colorscale=[[0, 'darkgreen'], [1, 'darkgreen']],
    showscale=False,
    showlegend=False,
    hoverinfo='skip',
    opacity=0.3,
    name='Terra'
))


# Configure layout for optimal GPU rendering
fig.update_layout(
    title='Visualizzazione 3D dei Punti',
    scene=dict(
        xaxis_title='Asse X',
        yaxis_title='Asse Y', 
        zaxis_title='Asse Z',
        aspectmode='data',  # Equal aspect ratio
        xaxis=dict(showspikes=False),
        yaxis=dict(showspikes=False),
        zaxis=dict(showspikes=False)
    ),
)

# Show with GPU-accelerated WebGL rendering
print("Rendering 3D visualization with GPU acceleration (WebGL)...")
print("Opening browser for interactive visualization...")

fig.show(renderer='browser')  # Forces browser rendering with WebGL

# Save as interactive HTML with GPU rendering
print("Saving interactive HTML file...")
fig.write_html("grafico_3d_gpu.html")
print("Visualization complete! File saved as 'grafico_3d_gpu.html'")
