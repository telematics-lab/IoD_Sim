

#!/bin/bash

# Script per eseguire la simulazione NS-3 e organizzare i file di output
# Uso: ./create_example.sh <nome_cartella>

# Verifica che sia stato fornito il nome della cartella
if [ $# -eq 0 ]; then
    echo "Errore: Specificare il nome della cartella di destinazione"
    echo "Uso: $0 <nome_cartella>"
    exit 1
fi

# Ottieni il percorso della directory dove si trova questo script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_FOLDER="$SCRIPT_DIR/$1"
NS_3_FOLDER="/Users/domysh/repos/ns-3-dev"

# Verifica che la cartella NS-3 esista
if [ ! -d "$NS_3_FOLDER" ]; then
    echo "Errore: La cartella NS-3 non esiste: $NS_3_FOLDER"
    exit 1
fi

echo "Esecuzione della simulazione NS-3..."
echo "Cartella di destinazione: $OUTPUT_FOLDER"

# Vai nella cartella NS-3
cd "$NS_3_FOLDER"

echo "Simulazione completata. Organizzo i file di output..."

# Crea la cartella di destinazione se non esiste
mkdir -p "$OUTPUT_FOLDER"

# Lista dei file di output da spostare (basata sui file che mi hai fornito)
FILES_TO_MOVE=(
    "DlCtrlSinr.txt"
    "DlDataSinr.txt"
    "DlPathlossTrace.txt"
    "NrDlMacStats.txt"
    "NrDlPdcpRxStats.txt"
    "NrDlPdcpStatsE2E.txt"
    "NrDlPdcpTxStats.txt"
    "NrDlRlcStatsE2E.txt"
    "NrDlRxRlcStats.txt"
    "NrDlTxRlcStats.txt"
    "NrUlMacStats.txt"
    "NrUlPdcpStatsE2E.txt"
    "NrUlRlcStatsE2E.txt"
    "RxedGnbMacCtrlMsgsTrace.txt"
    "RxedGnbPhyCtrlMsgsTrace.txt"
    "RxedUeMacCtrlMsgsTrace.txt"
    "RxedUePhyCtrlMsgsTrace.txt"
    "RxedUePhyDlDciTrace.txt"
    "RxPacketTrace.txt"
    "TxedGnbPhyCtrlMsgsTrace.txt"
    "TxedUeMacCtrlMsgsTrace.txt"
    "TxedUePhyCtrlMsgsTrace.txt"
    "UlPathlossTrace.txt"
)

# Sposta i file nella cartella di destinazione
for file in "${FILES_TO_MOVE[@]}"; do
    if [ -f "$file" ]; then
        echo "Spostando $file in $OUTPUT_FOLDER/"
        mv "$file" "$OUTPUT_FOLDER/"
    else
        echo "Avviso: File $file non trovato"
    fi
done

echo "Operazione completata! I file sono stati spostati in: $OUTPUT_FOLDER"
echo "Contenuto della cartella $OUTPUT_FOLDER:"
ls -la "$OUTPUT_FOLDER"
