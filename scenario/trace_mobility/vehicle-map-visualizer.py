import pandas as pd
import plotly
import plotly.graph_objects as go
import plotly.express as px
import numpy as np
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

# Visualizzazione 2D su mappa per tracce di veicoli urbani (con mappa di sfondo OSM)


def select_results_folder():
    """
    Menu interattivo per selezionare una cartella da ../results contenente file CSV di tracce
    Ritorna il percorso al file vehicle-trace.csv selezionato
    """
    results_path = "../../results"

    if not os.path.exists(results_path):
        print(f"Errore: Directory '{results_path}' non trovata.")
        return None

    # Trova cartelle contenenti file CSV di tracce
    valid_folders = []
    for item in sorted(os.listdir(results_path)):
        folder_path = os.path.join(results_path, item)
        if os.path.isdir(folder_path):
            vehicle_csv_path = os.path.join(folder_path, "vehicle-trace.csv")

            if os.path.exists(vehicle_csv_path):
                valid_folders.append((item, vehicle_csv_path))

    if not valid_folders:
        print(
            f"Nessuna cartella con file vehicle-trace.csv trovata in '{results_path}'"
        )
        return None

    # Mostra menu
    print("\nCartelle disponibili:")
    print("-" * 50)
    for i, (folder_name, _) in enumerate(valid_folders, 1):
        print(f"{i}. {folder_name}")

    # Ottieni selezione utente
    while True:
        try:
            choice = input(f"\nSeleziona cartella (1-{len(valid_folders)}): ").strip()
            choice_idx = int(choice) - 1
            if 0 <= choice_idx < len(valid_folders):
                selected_folder, vehicle_path = valid_folders[choice_idx]
                print(f"Selezionato: {selected_folder}")
                return vehicle_path
            else:
                print(f"Inserisci un numero tra 1 e {len(valid_folders)}")
        except ValueError:
            print("Inserisci un numero valido")
        except KeyboardInterrupt:
            print("\nOperazione annullata")
            return None


# Seleziona il file CSV
vehicle_csv_path = select_results_folder()
if vehicle_csv_path is None:
    print("Nessun file selezionato. Uscita.")
    exit()

# Carica dati
try:
    print("Caricamento dati veicoli...")
    df = pd.read_csv(vehicle_csv_path)
    print(f"Caricati {len(df)} punti di traccia")
    print(f"Colonne: {df.columns.tolist()}")
    print(f"Prime 5 righe:\n{df.head()}")
except Exception as e:
    print(f"Errore nel caricamento dati: {e}")
    exit()

# Estrai dati
nodes = df["Node"]
times = df["Time"]
latitudes = df["Latitude"]
longitudes = df["Longitude"]
altitudes = df["Altitude"]

# Statistiche
print("\nStatistiche Geografiche:")
print(f"Range latitudine: {latitudes.min():.6f}° - {latitudes.max():.6f}°")
print(f"Range longitudine: {longitudes.min():.6f}° - {longitudes.max():.6f}°")
print(f"Range altitudine: {altitudes.min():.1f}m - {altitudes.max():.1f}m")
print(f"Tempo: {times.min():.0f}s - {times.max():.0f}s")

# Calcola centro mappa
center_lat = (latitudes.min() + latitudes.max()) / 2
center_lon = (longitudes.min() + longitudes.max()) / 2

# Ottieni nodi unici
unique_nodes = df["Node"].unique()
print(f"\nNumero di veicoli: {len(unique_nodes)}")

# Palette di colori per i veicoli
colors = px.colors.qualitative.Plotly + px.colors.qualitative.Set1

# ---------------------------------------------------------------------------
# Scelta dell'API mappe in base alla versione di Plotly installata.
#   - Plotly >= 5.24: go.Scattermap  + layout.map     (MapLibre, consigliato)
#   - Plotly <  5.24: go.Scattermapbox + layout.mapbox (deprecato ma funzionante)
# Nessuno dei due richiede un token quando si usa lo stile "open-street-map".
# ---------------------------------------------------------------------------
_plotly_ver = tuple(int(p) for p in plotly.__version__.split(".")[:2])
_use_new_map = _plotly_ver >= (5, 24)

ScatterMap = go.Scattermap if _use_new_map else go.Scattermapbox
map_layout_key = "map" if _use_new_map else "mapbox"

# Stile della mappa di sfondo (gratuiti, senza token):
#   "open-street-map", "carto-positron", "carto-darkmatter", "carto-voyager"
map_style = "open-street-map"

# Crea figura
fig = go.Figure()

# Aggiungi una traccia (linea + marker) per ogni veicolo sopra la mappa
for i, node in enumerate(unique_nodes):
    node_df = df[df["Node"] == node].sort_values("Time")
    color = colors[i % len(colors)]

    fig.add_trace(
        ScatterMap(
            lat=node_df["Latitude"],
            lon=node_df["Longitude"],
            mode="lines+markers",
            marker=dict(size=8, color=color, opacity=0.85),
            line=dict(width=2, color=color),
            hovertemplate="<b>Veicolo %{text}</b><br>"
            + "Lat: %{lat:.6f}°<br>"
            + "Lon: %{lon:.6f}°<br>"
            + "Alt: %{customdata[0]:.1f}m<br>"
            + "Tempo: %{customdata[1]:.1f}s<br>"
            + "<extra></extra>",
            text=[node] * len(node_df),
            customdata=np.column_stack((node_df["Altitude"], node_df["Time"])),
            name=f"Veicolo {node}",
        )
    )

# Calcola uno zoom automatico in base all'estensione delle tracce
lat_range = latitudes.max() - latitudes.min()
lon_range = longitudes.max() - longitudes.min()
max_range = max(lat_range, lon_range)
if max_range > 0:
    zoom = float(np.clip(np.log2(360.0 / max_range), 1, 18))
else:
    zoom = 14  # tutte le tracce in un solo punto

# Impostazioni della mappa (chiave "map" o "mapbox" a seconda della versione)
map_settings = dict(
    style=map_style,
    center=dict(lat=center_lat, lon=center_lon),
    zoom=zoom,
)

fig.update_layout(
    title={
        "text": "Visualizzazione Tracce Veicoli - Mappa Urbana",
        "x": 0.5,
        "xanchor": "center",
        "font": {"size": 20},
    },
    autosize=True,
    height=900,
    margin=dict(l=0, r=0, t=60, b=0),  # Margini minimi
    showlegend=True,
    legend=dict(
        yanchor="top",
        y=0.99,
        xanchor="left",
        x=0.01,
        bgcolor="rgba(255, 255, 255, 0.9)",
        bordercolor="Black",
        borderwidth=1,
    ),
    **{map_layout_key: map_settings},
)

# Mostra visualizzazione
print("\nGenerazione visualizzazione mappa...")
print("Apertura browser per visualizzazione interattiva...")

fig.show(renderer="browser")

# Salva come HTML interattivo
output_file = "vehicle_map_visualization.html"
print(f"\nSalvataggio file HTML interattivo come '{output_file}'...")
fig.write_html(output_file)
print(f"✓ Visualizzazione completata! File salvato come '{output_file}'")

# Stampa istruzioni
print("\n" + "=" * 60)
print("ISTRUZIONI:")
print("- Usa il mouse per pan/zoom sulla mappa")
print("- Clicca sulle legende per mostrare/nascondere veicoli")
print("- Passa sopra i punti per vedere i dettagli")
print("- Per cambiare stile mappa modifica 'map_style' nel codice:")
print("  'open-street-map', 'carto-positron', 'carto-darkmatter', 'carto-voyager'")
print("=" * 60)