# NR PHY Layer Configuration Examples

Questi file JSON mostrano come configurare il layer PHY 5G NR in IoD_Sim utilizzando la nuova implementazione `NrPhyLayerConfiguration`.

## File disponibili

### 1. `nr-simple-example.json`
Configurazione base per test iniziali:
- 2 droni con UE NR
- 1 stazione base (ZSP) con gNB NR
- 1 remote host
- Configurazione minimale per debug

### 2. `nr-phy-complete-example.json`
Configurazione completa che mostra tutte le funzionalità:
- 4 droni con mobilità
- 2 stazioni base con configurazioni avanzate
- Beamforming con array di antenne
- Múltipli bande di frequenza
- Attributi PHY dettagliati
- Modelli di canale 3GPP

## Struttura della configurazione NR

### Sezione PHY Layer
```json
"phyLayer": {
  "type": "nr",
  "attributes": [...],           // Attributi generali PHY
  "beamforming": {...},         // Configurazione beamforming
  "scheduler": {...},           // Configurazione scheduler MAC
  "ueAntenna": {...},          // Antenna UE (per droni)
  "gnbAntenna": {...},         // Antenna gNB (per stazioni base)
  "channel": {...},            // Configurazione canale
  "uePhyAttributes": [...],    // Attributi specifici PHY UE
  "gnbPhyAttributes": [...]    // Attributi specifici PHY gNB
}
```

### Configurazione Beamforming
```json
"beamforming": {
  "method": "ns3::IdealBeamformingAlgorithm",
  "attributes": [
    {
      "name": "UpdatePeriod",
      "value": {"type": "ns3::TimeValue", "value": "100ms"}
    }
  ]
}
```

### Configurazione Antenna
```json
"ueAntenna": {
  "type": "ns3::IsotropicAntennaModel",
  "properties": [              // Proprietà elemento antenna
    {"name": "Gain", "value": {"type": "ns3::DoubleValue", "value": 0.0}}
  ],
  "arrayProperties": [         // Proprietà array antenna
    {"name": "NumRows", "value": {"type": "ns3::UintegerValue", "value": 2}},
    {"name": "NumColumns", "value": {"type": "ns3::UintegerValue", "value": 4}}
  ]
}
```

### Configurazione Canale
```json
"channel": {
  "scenario": "UMi-StreetCanyon",        // Scenario 3GPP
  "condition": "Default",                // Condizione canale
  "model": "ThreeGpp",                  // Modello propagazione
  "propagationLossModel": {...},        // Modello perdita propagazione
  "spectrumModel": {...},              // Modello spettro
  "updatePeriodMs": 100.0,             // Periodo aggiornamento
  "enableBeamforming": true,           // Abilita beamforming
  "bands": [                           // Bande di frequenza
    {
      "centralFrequency": 28000000000,  // 28 GHz
      "bandwidth": 100000000,           // 100 MHz
      "numCcs": 1,                     // Component Carriers
      "numerology": 1,                 // Numerologia 5G NR
      "direction": "DL_UL",           // Direzione traffico
      "contiguous": true              // CC contigui
    }
  ]
}
```

## Parametri principali

### Frequenze supportate
- **Sub-6 GHz**: 3.5 GHz, 5 GHz
- **mmWave**: 28 GHz, 39 GHz, 60 GHz

### Numerologie 5G NR
- **0**: 15 kHz subcarrier spacing
- **1**: 30 kHz subcarrier spacing (default mmWave)
- **2**: 60 kHz subcarrier spacing
- **3**: 120 kHz subcarrier spacing

### Scenari 3GPP
- **UMi-StreetCanyon**: Urban Micro
- **UMa**: Urban Macro
- **RMa**: Rural Macro
- **InH-OfficeMixed**: Indoor Hotspot

### Algoritmi Beamforming
- **ns3::IdealBeamformingAlgorithm**: Beamforming ideale
- **ns3::CellScanBeamforming**: Beam search
- **ns3::RealisticBeamformingAlgorithm**: Beamforming realistico

### Scheduler supportati
- **ns3::NrMacSchedulerTdmaRr**: Round Robin TDMA
- **ns3::NrMacSchedulerOfdmaRr**: Round Robin OFDMA
- **ns3::NrMacSchedulerOfdmaPf**: Proportional Fair OFDMA

## Come eseguire gli esempi

```bash
# Esempio semplice
cd ns3/
./ns3 run "scenario --config=../scenario/nr-simple-example.json"

# Esempio completo
./ns3 run "scenario --config=../scenario/nr-phy-complete-example.json"

# Con logging dettagliato
NS_LOG="NrHelper:NrGnbPhy:NrUePhy" ./ns3 run "scenario --config=../scenario/nr-simple-example.json"
```

## Note implementative

1. **Beamforming**: Richiede array di antenne con almeno 2x2 elementi
2. **mmWave**: Frequenze > 6 GHz richiedono numerologia ≥ 1
3. **Component Carriers**: Múltipli CC richiedono bandwidth totale compatibile
4. **Mobilità**: Beamforming si adatta automaticamente al movimento dei droni

## Troubleshooting

- **Errore frequenza**: Verificare compatibilità numerologia/frequenza
- **Beamforming fallito**: Controllare dimensioni array antenna
- **Connessione fallita**: Verificare posizioni iniziali nodi
- **Performance basse**: Aumentare potenza TX o ridurre distanze