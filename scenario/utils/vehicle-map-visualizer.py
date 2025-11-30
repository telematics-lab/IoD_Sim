import pandas as pd
import plotly.graph_objects as go
import plotly.express as px
import numpy as np
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

# Visualizzazione 2D su mappa per tracce di veicoli urbani


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
        print(f"Nessuna cartella con file vehicle-trace.csv trovata in '{results_path}'")
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

# Crea figura con mappa Mapbox
fig = go.Figure()

# Opzione 1: Usa Mapbox (richiede token, ma offre mappe dettagliate)
# Mapbox styles: "open-street-map", "carto-positron", "carto-darkmatter", "stamen-terrain", "stamen-toner", "stamen-watercolor"
use_mapbox = False  # Cambia a False per usare scatter geografico semplice

if use_mapbox:
    # Aggiungi tracce per ogni veicolo con linee di percorso
    for i, node in enumerate(unique_nodes):
        node_df = df[df["Node"] == node].sort_values("Time")
        color = colors[i % len(colors)]

        # Linea del percorso
        fig.add_trace(
            go.Scattermapbox(
                lat=node_df["Latitude"],
                lon=node_df["Longitude"],
                mode="lines+markers",
                marker=dict(
                    size=8,
                    color=color,
                    opacity=0.7,
                ),
                line=dict(
                    width=2,
                    color=color,
                ),
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

    # Layout per Mapbox - Fullscreen
    fig.update_layout(
        title="Visualizzazione Tracce Veicoli - Mappa Urbana",
        mapbox=dict(
            style="open-street-map",  # Mappa stradale gratuita
            center=dict(lat=center_lat, lon=center_lon),
            zoom=12,
        ),
        autosize=True,
        margin=dict(l=0, r=0, t=40, b=0),  # Margini minimi
        showlegend=True,
        legend=dict(
            yanchor="top",
            y=0.99,
            xanchor="left",
            x=0.01,
            bgcolor="rgba(255, 255, 255, 0.8)",
        ),
    )

else:
    # Opzione 2: Usa Scatter Geo semplice (non richiede token)
    for i, node in enumerate(unique_nodes):
        node_df = df[df["Node"] == node].sort_values("Time")
        color = colors[i % len(colors)]

        # Linea del percorso
        fig.add_trace(
            go.Scattergeo(
                lat=node_df["Latitude"],
                lon=node_df["Longitude"],
                mode="lines+markers",
                marker=dict(
                    size=6,
                    color=color,
                    opacity=0.8,
                    line=dict(width=0.5, color='white'),
                ),
                line=dict(
                    width=2,
                    color=color,
                ),
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

    # Calcola i bounds della mappa
    lat_range = latitudes.max() - latitudes.min()
    lon_range = longitudes.max() - longitudes.min()

    # Layout per Scatter Geo
    fig.update_layout(
        title={
            'text': "Visualizzazione Tracce Veicoli - Mappa Topografica",
            'x': 0.5,
            'xanchor': 'center',
            'font': {'size': 20}
        },
        geo=dict(
            scope='world',
            projection_type='mercator',
            center=dict(lat=center_lat, lon=center_lon),
            lonaxis=dict(
                range=[longitudes.min() - lon_range * 0.1,
                       longitudes.max() + lon_range * 0.1]
            ),
            lataxis=dict(
                range=[latitudes.min() - lat_range * 0.1,
                       latitudes.max() + lat_range * 0.1]
            ),
            showland=True,
            landcolor="rgb(243, 243, 243)",
            coastlinecolor="rgb(204, 204, 204)",
            showlakes=True,
            lakecolor="rgb(200, 220, 240)",
            showrivers=True,
            rivercolor="rgb(200, 220, 240)",
            showcountries=True,
            countrycolor="rgb(204, 204, 204)",
            resolution=50,  # Alta risoluzione
            domain=dict(x=[0, 1], y=[0, 1]),  # Occupa 100% dello spazio
        ),
        height=900,  # Altezza fissa grande per forzare fullscreen
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
    )

# Mostra visualizzazione
print("\nGenerazione visualizzazione mappa...")
print("Apertura browser per visualizzazione interattiva...")

fig.show(renderer="browser")

# Salva come HTML interattivo
output_file = "vehicle_map_visualization.html"
print(f"\nSalvataggio file HTML interattivo come '{output_file}'...")
#fig.write_html(output_file)
print(f"✓ Visualizzazione completata! File salvato come '{output_file}'")

# Stampa istruzioni
print("\n" + "=" * 60)
print("ISTRUZIONI:")
print("- Usa il mouse per pan/zoom sulla mappa")
print("- Clicca sulle legende per mostrare/nascondere veicoli")
print("- Passa sopra i punti per vedere i dettagli")
if use_mapbox:
    print("- Per cambiare stile mappa, modifica 'style' nel codice:")
    print("  'open-street-map', 'carto-positron', 'stamen-terrain', etc.")
print("=" * 60)
