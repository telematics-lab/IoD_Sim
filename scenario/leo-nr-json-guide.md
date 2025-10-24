# Guida LEO-NR Communication Scenario

Questo documento descrive tutte le opzioni configurabili nelle simulazioni che usano i moduli leo e i moduli nr in IoD-Sim.

## Indice
1. [Configurazione Globale](#configurazione-globale)
2. [Configurazione del Mondo](#configurazione-del-mondo)
3. [Layer Fisico (PHY Layer)](#layer-fisico-phy-layer)
4. [Veicoli sulla Terra](#veicoli-sulla-terra)
5. [Satelliti LEO](#satelliti-leo)
6. [Comunicazioni Remote](#comunicazioni-remote)
7. [Registrazione Log](#registrazione-log)

---

## Configurazione Globale

### `staticNs3Config`
**Tipo:** `array[object]`
**Descrizione:** Array di configurazioni statiche ns-3 da applicare prima della simulazione. Ogni oggetto ha `name` (nome dell'attributo) e `value`.

Conviene impostare alcuni parametri globali utili alla configurazione di alcune impostazioni di nr e le configuazioni sul modello della terra usato in LEO e nei modelli di propagazione NTN.

Per Nr, negli esempi di lena si "annulla" il valore di default del buffer RLC UM per evitare perdite di pacchetti in scenari ad alta latenza come quelli satellitari.
```json
{
  "name": "ns3::NrRlcUm::MaxTxBufferSize",
  "value": 999999999
}
```

Mentre per le configurazioni LEO, è importante specificare il tipo di sferoid terrestre usato nel modello di mobilità geocentrica e nei modelli di propagazione NTN. Usualmente è comodo usare una sfera perfetta (`SPHERE`) per il posizionamento dei satelliti e dei veicoli terresti.
```json
{
  "name": "ns3::GeocentricMobilityModel::EarthSpheroidType",
  "value": "SPHERE"
}
```

Inoltre possiamo specificare la precisione dei modelli di mobilità, impostando per i vari modelli la frequenza di aggiornamento della posizione (schedulando gli aggiornamenti in cui verrà chiamato CourseChange).

```json
{
  "name": "ns3::GeoLeoOrbitMobility::Precision",
  "value": "1s"
},
{
  "name": "ns3::GeoConstantVelocityMobility::Precision",
  "value": "1s"
}
```

In questo esempio impostiamo a 1 seocondo la frequenza nei modelli di mobilità orbitale LEO e nei modelli di mobilità a velocità costante usati dai veicoli terrestri.

## Configurazione del Mondo

### `world.size`
**Descrizione:** Dimensioni dello spazio simulato in metri.

| Parametro | Tipo | Descrizione |
|-----------|------|-------------|
| `X` | string | Dimensione lungo l'asse X |
| `Y` | string | Dimensione lungo l'asse Y |
| `Z` | string | Dimensione lungo l'asse Z (altitudine) |

**Esempio:**
```json
"world": {
  "size": {
    "X": "40000000",
    "Y": "40000000",
    "Z": "40000000"
  }
}
```

In questi modelli è necessario impostare una dimensione del mondo molto ampia poichè al centro delle coordinate avremo la terra e attorno i satelliti in orbite LEO quindi dobbiamo considerare una dimensione che possa comprendere tutto come quella fornita in questo esempio.

---

## Layer Fisico (PHY Layer)

Nelle impostazioni `phyLayer` possiamo configurare un layer fisico di tipo `nr` che verrà supportato da 5g-lena e ne permette la configurazioni con le seguenti opzioni:


### `type`
**Tipo:** `string`
**Opzioni:** `nr`, `lte`, `wifi`, `none`
**Descrizione:** Tipo di layer fisico. Nel nostro caso è `nr` per comunicazioni 5G NR (New Radio).

### `bands`
**Descrizione:** Array di bande di frequenza disponibili per la comunicazione.

## Configurazioni delle bande NR

Abbiamo la possibilità di configurare varie bande NR con differenti scenari, modelli di propagazione e frequenze.

In particolare per la singola banda abbiamo i seguenti parametri configurabili:

### `bands[i].scenario`
**Tipo**: `string`
**Opzioni:** `InF`, `InH`, `UMa`, `UMi`, `RMa`, `InH-OfficeMixed`, `InH-OfficeOpen`, `V2V-Highway`, `V2V-Urban`, `NTN-DenseUrban`, `NTN-Urban`, `NTN-Suburban`, `NTN-Rural`
**Descrizione:** Scenario di propagazione per la banda NR.

Negli scenari di nostro interesse per comunicazioni LEO-satellite, conviene usare gli scenari `NTN-*` come `NTN-Suburban`, `NTN-Urban`, `NTN-DenseUrban` o `NTN-Rural` a seconda del tipo di ambiente terrestre in cui si trovano i veicoli.

### `bands[i].propagationModel`
**Tipo:** `string`
**Opzioni:** `NYU`, `TwoRay`, `ThreeGpp`
**Descrizione:** Modello di propagazione per la banda NR.

La scelta dello scenario, delle condizioni del modello e del tipo di modello di propagazione, influenzerà il calcolo della path loss e della qualità del canale tra i nodi, andando a definire secondo una tabella il ChannelModel di ns3.

In particolare con questo parametro andiamo a definire lo "Spectrum Propagation Loss Model" usato per calcolare la path loss tra i nodi.
| ChannelModel | SpectrumPropagationLossModel |
|---------------|------------------------------|
| ChannelModel::ThreeGpp | ThreeGppSpectrumPropagationLossModel |
| ChannelModel::TwoRay | TwoRaySpectrumPropagationLossModel |
| ChannelModel::NYU | NYUSpectrumPropagationLossModel |

### `bands[i].conditionModel`
**Tipo:** `string`
**Opzioni:** `NLOS`, `LOS`, `Buildings`, `Default`
**Descrizione:** Modello della condizione del canale per la banda NR.

Con questa opzione possiamo specificare un Channel Condition Model specifico. Se selezioniamo 'Default', questo verrà scelto automaticamente in base allo scenario selezionato e al modello di propagazione.

#### Tabella di Lookup: Modelli di Propagazione Disponibili

La seguente tabella mostra le combinazioni valide di **Propagation Model** e **Scenario**:

##### ThreeGpp e TwoRay Models

In caso impostiamo TwoRay come modello di propagazione, verrà usato lo stesso schema di ThreeGpp per gli scenari disponibili.

Il Channel Condition Model verrà usato se conditionModel è "Default".

| Scenario | Propagation Loss Model | Channel Condition Model (Default conditions) | Descrizione |
|----------|------------------------|------------------------|-------------|
| `RMa` | ThreeGppRmaPropagationLossModel | ThreeGppRmaChannelConditionModel | Rural Macrocell |
| `UMa` | ThreeGppUmaPropagationLossModel | ThreeGppUmaChannelConditionModel | Urban Macrocell |
| `UMi` | ThreeGppUmiStreetCanyonPropagationLossModel | ThreeGppUmiStreetCanyonChannelConditionModel | Urban Microcell (Street Canyon) |
| `InH-OfficeOpen` | ThreeGppIndoorOfficePropagationLossModel | ThreeGppIndoorOpenOfficeChannelConditionModel | Indoor Office (Open) |
| `InH-OfficeMixed` | ThreeGppIndoorOfficePropagationLossModel | ThreeGppIndoorMixedOfficeChannelConditionModel | Indoor Office (Mixed) |
| `V2V-Highway` | ThreeGppV2vHighwayPropagationLossModel | ThreeGppV2vHighwayChannelConditionModel | Vehicle-to-Vehicle Highway |
| `V2V-Urban` | ThreeGppV2vUrbanPropagationLossModel | ThreeGppV2vUrbanChannelConditionModel | Vehicle-to-Vehicle Urban |
| `NTN-DenseUrban` | ThreeGppNTNDenseUrbanPropagationLossModel | ThreeGppNTNDenseUrbanChannelConditionModel | Non-Terrestrial Network (Dense Urban) |
| `NTN-Urban` | ThreeGppNTNUrbanPropagationLossModel | ThreeGppNTNUrbanChannelConditionModel | Non-Terrestrial Network (Urban) |
| `NTN-Suburban` | ThreeGppNTNSuburbanPropagationLossModel | ThreeGppNTNSuburbanChannelConditionModel | Non-Terrestrial Network (Suburban) |
| `NTN-Rural` | ThreeGppNTNRuralPropagationLossModel | ThreeGppNTNRuralChannelConditionModel | Non-Terrestrial Network (Rural) |

##### NYU Models

Il Channel Condition Model verrà usato se conditionModel è "Default".

| Scenario | Propagation Loss Model | Channel Condition Model (Default conditions) | Descrizione |
|----------|------------------------|------------------------|-------------|
| `RMa` | NYURmaPropagationLossModel | NYURmaChannelConditionModel | Rural Macrocell |
| `UMa` | NYUUmaPropagationLossModel | NYUUmaChannelConditionModel | Urban Macrocell |
| `UMi` | NYUUmiPropagationLossModel | NYUUmiChannelConditionModel | Urban Microcell |
| `InH` | NYUInHPropagationLossModel | NYUInHChannelConditionModel | Indoor |
| `InF` | NYUInFPropagationLossModel | NYUInFChannelConditionModel | In-Factory |


#### Channel Condition Model su conditionModel != "Default"

Se conditionModel è impostato su un valore diverso da "Default", il Channel Condition Model verrà scelto in base alla seguente tabella:

| Condizione | Channel Condition Model |
|------------|-------------------------|
| NLOS       | NeverLosChannelConditionModel |
| LOS        | AlwaysLosChannelConditionModel |
| Buildings  | BuildingsChannelConditionModel |

### `bands[i].contiguousCc`
**Tipo:** `boolean`
**Descrizione:** Se `true`, le component carrier (CC) sono allocate in modo contiguo nello spettro; se `false`, possono essere allocate in modo non contiguo.
**Nota:** Questo parametro influisce sulla gestione delle risorse radio e sulla pianificazione delle bande di frequenza: se a true sarà possibile impostare 1 solo intervallo di frequenza per le CC, mentre a false si potranno avere CC in intervalli separati.

### `bands[i].pathlossAttributes`
**Tipo:** `array`
**Descrizione:** Elenco degli attributi di perdita di percorso per la banda specificata. Ogni attributo di perdita di percorso è definito da un oggetto che specifica il nome e il valore dell'attributo.
**Note:** Questi attributi vengono passati al modello di perdita di percorso selezionato di PropagationLoss, pertanto i parametri disponibili dipendono dal modello di propagazione scelto.

In generale per i modelli ThreeGpp (o TwoRay) abbiamo questi attributi presi dal modello generico ThreeGppPropagationLossModel:

| Nome | Tipo | Valore Iniziale | Descrizione |
|------|------|----------------|-------------|
| `BuildingPenetrationLossesEnabled` | boolean | true | Abilita/disabilita le perdite di penetrazione degli edifici |
| `EnforceParameterRanges` | boolean | false | Se applicare rigorosamente i range di applicabilità TR38.901 |
| `ShadowingEnabled` | boolean | true | Abilita/disabilita l'effetto shadowing |

Mentre per i modelli NYU abbiamo questi attributi presi dal modello generico NYUPropagationLossModel:

| Nome | Tipo | Valore Iniziale | Descrizione |
|------|------|----------------|-------------|
| `FoliageLoss` | double | 0.4 | La perdita di foliage in dB |
| `ShadowingEnabled` | boolean | true | Abilita/disabilita l'effetto shadowing |
| `O2ILosstype` | string | "Low Loss" | Tipo di perdita di penetrazione Outdoor-to-Indoor (O2I) - "Low Loss" o "High Loss" |
| `FoliageLossEnabled` | boolean | false | Abilita/disabilita la perdita di foliage |
| `AtmosphericLossEnabled` | boolean | false | Abilita/disabilita la perdita atmosferica |
| `Pressure` | double | 1013.25 | La pressione barometrica in mbar |
| `Humidity` | double | 50 | L'umidità in percentuale |
| `Temperature` | double | 20 | La temperatura in gradi celsius |
| `RainRate` | double | 0 | Il tasso di pioggia in mm/hr |

Per modelli specifici potrebbero esserci ulteriori attributi disponibili più specifici per cui si lascia il riferimento alla documentazione di ns-3 e di 5g-lena.

### `bands[i].channelConditionAttributes`
**Descrizione:** Attributi della condizione del canale per la banda specificata. Ogni attributo della condizione del canale è definito da un oggetto che specifica il nome e il valore dell'attributo.

Anche in questo caso, gli attributi disponibili dipendono dal modello di condizione del canale selezionato.

Ad esempio per channelCondition "Default" con modelli ThreeGpp abbiamo questi attributi comuni (da ):

| Nome | Tipo | Valore Iniziale | Descrizione |
|------|------|----------------|-------------|
| `LinkO2iConditionToAntennaHeight` | boolean | false | Specifica se la condizione O2I sarà determinata in base all'altezza dell'antenna UE (se l'altezza UE è 1.5m allora è O2O, altrimenti è O2I) |
| `O2iLowLossThreshold` | double (0:1) | 1 | Specifica il rapporto delle perdite di penetrazione O2I basse vs. alte. Valore di default 1.0 significa che tutte le perdite saranno basse |
| `O2iThreshold` | double (0:1) | 0 | Specifica il rapporto delle condizioni del canale O2I. Valore di default 0 corrisponde a 0 perdite O2I |
| `UpdatePeriod` | Time | 0ns | Specifica il periodo di tempo dopo il quale la condizione del canale viene ricalcolata. Se impostato a 0, la condizione del canale non viene mai aggiornata |

Nel caso di modelli NYU abbiamo questi attributi comuni (da NYUPropagationLossModel):

| Nome | Tipo | Valore Iniziale | Descrizione |
|------|------|----------------|-------------|
| `UpdatePeriod` | Time | 0ms | Specifica il periodo di tempo dopo il quale la condizione del canale viene ricalcolata. Se impostato a 0, la condizione del canale non viene mai aggiornata |

Mentre per i modelli NLOS, LOS e Buildings non abbiamo attributi specificabili.


### `bands[i].phasedSpectrumAttributes`
**Descrizione:** Attributi dello spettro a fasi per la banda specificata. Ogni attributo dello spettro a fasi è definito da un oggetto che specifica il nome e il valore dell'attributo.

Qui possiamo speificare attributi per gli Specrum PropagationLossModel. In particolare tuttavia non ci sono attributi utili associabili a questi modelli.
È comunque lasciata la possibilità di utilizzare questa funzionalità derivata dall'nr helper di 5g-lena.

### `bands[i].frequencyBands`
**Tipo:** `array`
**Descrizione:** Elenco delle bande di frequenza per la banda NR specificata. Ogni banda di frequenza è definita da un oggetto che specifica la frequenza centrale, la larghezza di banda, il numero di component carrier e il numero di bandwidth part.

Esempio di un oggetto frequencyBands:
```json
{
  "centralFrequency": 28e9,
  "bandwidth": 10e6,
  "numComponentCarriers": 1,
  "numBandwidthParts": 1
}
```


### `phyLayer[0].epc`
**Descrizione:** Configurazione dell'EPC (Evolved Packet Core).

In generale come helper per l'epc abbiamo come classi disponibili: `ns3::NrPointToPointEpcHelper`, `ns3::NrPointToPointEpcHelperBase`, `ns3::NrNoBackhaulEpcHelper`.

Si noti come `ns3::NrPointToPointEpcHelper` e `ns3::NrPointToPointEpcHelperBase` derivino da `ns3::NrNoBackhaulEpcHelper` e quindi ereditino tutti i suoi attributi.

Esempio di configurazione EPC:

```json
"epc": {
  "helper": "ns3::NrPointToPointEpcHelper",
  "attributes": [
    {
      "name": "S1uLinkDelay",
      "value": "0ms"
    }
  ]
}
```

Attibuti disponibili per `ns3::NrPointToPointEpcHelper` e `ns3::NrPointToPointEpcHelperBase` (oltre a quelli ereditati da `ns3::NrNoBackhaulEpcHelper`):

| Nome | Tipo | Valore Iniziale | Descrizione |
|------|------|----------------|-------------|
| `S1uLinkDataRate` | DataRate | 10000000000bps | La velocità di trasmissione da utilizzare per il prossimo link S1-U da creare |
| `S1uLinkDelay` | Time | +0ns | Il ritardo da utilizzare per il prossimo link S1-U da creare |
| `S1uLinkMtu` | uint16_t (0:65535) | 2000 | La MTU del prossimo link S1-U da creare. Nota: a causa dell'overhead aggiuntivo del tunneling GTP/UDP/IP, è necessaria una MTU maggiore della MTU end-to-end che si vuole supportare |
| `S1uLinkPcapPrefix` | string | "s1u" | Prefisso per i file Pcap generati dal link S1-U |
| `S1uLinkEnablePcap` | boolean | false | Abilita la generazione di file Pcap per il link S1-U |

Qui gli attributi ereditati da `ns3::NrNoBackhaulEpcHelper`:

| Nome | Tipo | Valore Iniziale | Descrizione |
|------|------|----------------|-------------|
| `S5LinkDataRate` | DataRate | 10000000000bps | La velocità di trasmissione da utilizzare per il prossimo link S5 da creare |
| `S5LinkDelay` | Time | +0ns | Il ritardo da utilizzare per il prossimo link S5 da creare |
| `S5LinkMtu` | uint16_t (0:65535) | 2000 | La MTU del prossimo link S5 da creare |
| `S11LinkDataRate` | DataRate | 10000000000bps | La velocità di trasmissione da utilizzare per il prossimo link S11 da creare |
| `S11LinkDelay` | Time | +0ns | Il ritardo da utilizzare per il prossimo link S11 da creare |
| `S11LinkMtu` | uint16_t (0:65535) | 2000 | La MTU del prossimo link S11 da creare |
| `X2LinkDataRate` | DataRate | 10000000000bps | La velocità di trasmissione da utilizzare per il prossimo link X2 da creare |
| `X2LinkDelay` | Time | +0ns | Il ritardo da utilizzare per il prossimo link X2 da creare |
| `X2LinkMtu` | uint16_t (0:65535) | 3000 | La MTU del prossimo link X2 da creare. Nota: a causa di alcuni messaggi X2 grandi, è necessaria una MTU grande |
| `X2LinkPcapPrefix` | string | "x2" | Prefisso per i file Pcap generati dal link X2 |
| `X2LinkEnablePcap` | boolean | false | Abilita la generazione di file Pcap per il link X2 |

Si noti come alcuni Prefix vengono già sovrascritti di default in IoD-Sim per evitare conflitti tra i vari link generati: sostiuirli nuovamente potrebbe compromettere la generazione dei file di reportistica.

### `phyLayer[0].beamforming`
**Descrizione:** Configurazione del beamforming.

In generale come helper per il beamforming abbiamo come classi disponibili: `ns3::IdealBeamformingHelper`, `ns3::RealisticBeamformingHelper`.
Per questi helper non abbiam particolari attributi specifici, ma possiamo scegliere il metodo di beamforming (o algoritmo) tra `ns3::IdealBeamformingAlgorithm`, `ns3::DirectPathBeamforming`, `ns3::DirectPathQuasiOmniBeamforming` e `ns3::QuasiOmniDirectPathBeamforming`.
Anche per gli algoritmi di beamforming non abbiamo particolari attributi specifici.

Esempio di configurazione del beamforming:

```json
"beamforming": {
  "helper": "ns3::IdealBeamformingHelper",
  "method": "ns3::DirectPathBeamforming",
  "algorithmAttributes": [],
  "attributes": []
}
```

### `phyLayer[0].ueAntenna` e `gnbAntenna`
**Descrizione:** Configurazione dell'antenna per UE e gNB.

Esempio di configurazione dell'antenna UE:

```json
"ueAntenna": {
  "type": "ns3::IsotropicAntennaModel",
  "properties": [],
  "arrayProperties": [
    {"name": "NumRows", "value": 2},
    {"name": "NumColumns", "value": 4}
  ]
}
```

In Type Possiamo inserire una qualunque antenna presente su ns-3, che sarà inserita in un array di antenne (come definito da 5g-lena), i cui parametri di configurazione possono essere specificati in arrayProperties.

Le proprietà specifiche dell'antenna (diverse da NumRows e NumColumns) possono essere specificate nell'array `properties` e dipendono dal tipo di antenna selezionato.

In particolare l'Array di antenne è costituito da un `UniformPlanarArray` di antenne del tipo specificato in `type` e può avere questi attributi (in arrayProperties):

| Nome | Tipo | Valore Iniziale | Range | Descrizione |
|------|------|----------------|--------|-------------|
| `AntennaHorizontalSpacing` | double | 0.5 | 0:1.79769e+308 | Spaziatura orizzontale tra elementi antenna, in multipli della lunghezza d'onda |
| `AntennaVerticalSpacing` | double | 0.5 | 0:1.79769e+308 | Spaziatura verticale tra elementi antenna, in multipli della lunghezza d'onda |
| `BearingAngle` | double | 0 | -π:π | Angolo di orientamento in radianti |
| `DowntiltAngle` | double | 0 | -π:π | Angolo di inclinazione verso il basso in radianti |
| `IsDualPolarized` | boolean | false | - | Se true, antenna a doppia polarizzazione |
| `NumColumns` | uint32_t | 4 | 1:4294967295 | Dimensione orizzontale dell'array |
| `NumHorizontalPorts` | uint32_t | 1 | 0:4294967295 | Numero di porte orizzontali |
| `NumRows` | uint32_t | 4 | 1:4294967295 | Dimensione verticale dell'array |
| `NumVerticalPorts` | uint32_t | 1 | 0:4294967295 | Numero di porte verticali |
| `PolSlantAngle` | double | 0 | -π:π | Angolo di inclinazione della polarizzazione in radianti |

mentre di seguito si riportano i tipi di antenne disponibili con i relativi parametri:

#### ns3::IsotropicAntennaModel

Antenna isotropa che irradia uniformemente in tutte le direzioni. È la più semplice da configurare.

| Nome | Tipo | Valore Iniziale | Range | Descrizione |
|------|------|----------------|--------|-------------|
| `Gain` | double | 0 | -∞:+∞ | Guadagno dell'antenna in dB |

#### ns3::CosineAntennaModel

Antenna con pattern di radiazione coseno, utile per modellare antenne direttive con lobi principali definiti.

| Nome | Tipo | Valore Iniziale | Range | Descrizione |
|------|------|----------------|--------|-------------|
| `HorizontalBeamwidth` | double | 120 | 0:360 | Larghezza del fascio orizzontale a 3 dB (gradi). 360° = guadagno costante |
| `VerticalBeamwidth` | double | 360 | 0:360 | Larghezza del fascio verticale a 3 dB (gradi). 360° = guadagno costante |
| `MaxGain` | double | 0 | -∞:+∞ | Guadagno massimo (dB) nella direzione di puntamento |
| `Orientation` | double | 0 | -360:360 | Orientamento dell'antenna sul piano x-y rispetto all'asse x (gradi) |


#### ns3::ParabolicAntennaModel

Antenna parabolica con pattern di radiazione direttivo, ideale per comunicazioni punto-punto ad alta direttività.

| Nome | Tipo | Valore Iniziale | Range | Descrizione |
|------|------|----------------|--------|-------------|
| `Beamwidth` | double | 60 | 0:180 | Larghezza del fascio a 3 dB (gradi) |
| `MaxAttenuation` | double | 20 | -∞:+∞ | Attenuazione massima (dB) del pattern di radiazione |
| `Orientation` | double | 0 | -360:360 | Orientamento dell'antenna sul piano x-y rispetto all'asse x (gradi) |


#### ns3::CircularApertureAntennaModel

Antenna ad apertura circolare che simula il comportamento di antenne riflettore o array con apertura fisica definita.

| Nome | Tipo | Valore Iniziale | Range | Descrizione |
|------|------|----------------|--------|-------------|
| `AntennaCircularApertureRadius` | double | 0.5 | ≥0 | Raggio dell'apertura dell'antenna (metri) |
| `OperatingFrequency` | double | 2e9 | >0 | Frequenza operativa dell'antenna (Hz) |
| `AntennaMinGainDb` | double | -100.0 | -∞:+∞ | Guadagno minimo dell'antenna (dB) |
| `AntennaMaxGainDb` | double | 1.0 | ≥0 | Guadagno massimo dell'antenna (dB) |
| `ForceGainBounds` | boolean | true | - | Forza GetGainDb nell'intervallo [AntennaMinGainDb, AntennaMaxGainDb] |

#### ns3::ThreeGppAntennaModel

Antenna basata su un'approssimazione parabolica del pattern di radiazione del lobo principale, conforme agli standard 3GPP.

| Nome | Tipo | Valore Iniziale | Descrizione |
|------|------|----------------|-------------|
| `RadiationPattern` | string | "Outdoor" | Pattern di radiazione dell'antenna 3GPP |

**Valori possibili per RadiationPattern:**
- `Outdoor` - Pattern per ambienti esterni
- `Indoor` - Pattern per ambienti interni

### `gnbBwpManager` e `ueBwpManager`
**Descrizione:** Configurazione del Bandwidth Part (BWP) Manager per gNB e UE.

È possibile specificare il tipo del BWP Manager Algorithm con il campo "type", anche se attualmente disponibile solo `ns3::BwpManagerAlgorithmStatic`, e i relativi attributi quali:

**Attributi di `ns3::BwpManagerAlgorithmStatic`:**

| Nome Attributo | Tipo | Range | Valore Iniziale | Descrizione |
|----------------|------|-------|-----------------|-------------|
| `GBR_CONV_VOICE` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo GBR_CONV_VOICE (voce conversazionale) |
| `GBR_CONV_VIDEO` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo GBR_CONV_VIDEO (video conversazionale) |
| `GBR_GAMING` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo GBR_GAMING (gaming) |
| `GBR_NON_CONV_VIDEO` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo GBR_NON_CONV_VIDEO (video non conversazionale) |
| `GBR_MC_PUSH_TO_TALK` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo GBR_MC_PUSH_TO_TALK (push-to-talk mission critical) |
| `GBR_NMC_PUSH_TO_TALK` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo GBR_NMC_PUSH_TO_TALK (push-to-talk non mission critical) |
| `GBR_MC_VIDEO` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo GBR_MC_VIDEO (video mission critical) |
| `GBR_V2X` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo GBR_V2X (Vehicle-to-Everything) |
| `NGBR_IMS` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo NGBR_IMS (IP Multimedia Subsystem) |
| `NGBR_VIDEO_TCP_OPERATOR` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo NGBR_VIDEO_TCP_OPERATOR (video TCP operatore) |
| `NGBR_VOICE_VIDEO_GAMING` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo NGBR_VOICE_VIDEO_GAMING (voce/video/gaming) |
| `NGBR_VIDEO_TCP_PREMIUM` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo NGBR_VIDEO_TCP_PREMIUM (video TCP premium) |
| `NGBR_VIDEO_TCP_DEFAULT` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo NGBR_VIDEO_TCP_DEFAULT (video TCP default) |
| `NGBR_MC_DELAY_SIGNAL` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo NGBR_MC_DELAY_SIGNAL (segnalazione delay-sensitive MC) |
| `NGBR_MC_DATA` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo NGBR_MC_DATA (dati mission critical) |
| `NGBR_V2X` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo NGBR_V2X (Vehicle-to-Everything non-GBR) |
| `NGBR_LOW_LAT_EMBB` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo NGBR_LOW_LAT_EMBB (eMBB bassa latenza) |
| `DGBR_DISCRETE_AUT_SMALL` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo DGBR_DISCRETE_AUT_SMALL (automazione discreta small) |
| `DGBR_DISCRETE_AUT_LARGE` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo DGBR_DISCRETE_AUT_LARGE (automazione discreta large) |
| `DGBR_ITS` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo DGBR_ITS (Intelligent Transport Systems) |
| `DGBR_ELECTRICITY` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo DGBR_ELECTRICITY (distribuzione elettrica) |
| `GBR_LIVE_UL_71` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo GBR_LIVE_UL_71 (live uplink 71) |
| `GBR_LIVE_UL_72` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo GBR_LIVE_UL_72 (live uplink 72) |
| `GBR_LIVE_UL_73` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo GBR_LIVE_UL_73 (live uplink 73) |
| `GBR_LIVE_UL_74` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo GBR_LIVE_UL_74 (live uplink 74) |
| `GBR_LIVE_UL_76` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo GBR_LIVE_UL_76 (live uplink 76) |
| `DGBR_INTER_SERV_87` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo DGBR_INTER_SERV_87 (servizi interattivi 87) |
| `DGBR_INTER_SERV_88` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo DGBR_INTER_SERV_88 (servizi interattivi 88) |
| `DGBR_VISUAL_CONTENT_89` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo DGBR_VISUAL_CONTENT_89 (contenuto visuale 89) |
| `DGBR_VISUAL_CONTENT_90` | uint8_t | 0:5 | 0 | Indice BWP per flussi di tipo DGBR_VISUAL_CONTENT_90 (contenuto visuale 90) |

**Nota:** Ogni attributo specifica l'indice della Bandwidth Part (BWP) a cui devono essere inoltrati i flussi del rispettivo tipo QCI (Quality of Service Class Identifier). Il valore deve essere compreso tra 0 e 5, corrispondente all'indice della BWP configurata.

Esempio di configurazione del BWP Manager:

```json
"gnbBwpManager": {
  "type": "ns3::BwpManagerAlgorithmStatic",
  "attributes": [
    {
      "name": "NGBR_LOW_LAT_EMBB",
      "value": 0
    }
  ]
}
```

### `phyLayer[0].uePhyAttributes` e `gnbPhyAttributes`
**Descrizione:** Attributi fisici specifici per UE e gNB.

Attributi disponibili per `uePhyAttributes`:

| Nome | Tipo | Range | Valore Iniziale | Descrizione |
|------|------|-------|-----------------|-------------|
| `TxPower` | double | -1.79769e+308:1.79769e+308 | 2 | Potenza di trasmissione in dBm |
| `NoiseFigure` | double | -1.79769e+308:1.79769e+308 | 5 | Perdita (dB) nel rapporto segnale-rumore dovuta a non-idealità nel ricevitore |
| `PowerAllocationType` | enum | UniformPowerAllocBw\|UniformPowerAllocUsed | UniformPowerAllocUsed | Tipo di allocazione della potenza sui RB |
| `LBTThresholdForCtrl` | Time | -9.22337e+18ns:+9.22337e+18ns | +25000ns | Soglia per considerare il canale libero per trasmissione controllo |
| `TbDecodeLatency` | Time | -9.22337e+18ns:+9.22337e+18ns | +100000ns | Latenza di decodifica del transport block |
| `EnableUplinkPowerControl` | boolean | - | false | Abilita il controllo della potenza in uplink |
| `WbPmiUpdateInterval` | Time | -9.22337e+18ns:+9.22337e+18ns | +1e+07ns | Intervallo di aggiornamento PMI wideband |
| `SbPmiUpdateInterval` | Time | -9.22337e+18ns:+9.22337e+18ns | +2e+06ns | Intervallo di aggiornamento PMI subband |
| `AlphaCovMat` | double | 0:1 | 1 | Parametro alpha per calcolo matrice covarianza interferenza |
| `CsiImDuration` | uint8_t | 1:12 | 1 | Durata CSI-IM in numero di simboli OFDM |
| `UeMeasurementsFilterPeriod` | Time | -9.22337e+18ns:+9.22337e+18ns | +2e+08ns | Periodo di filtraggio per misure UE |
| `EnableRlfDetection` | boolean | - | true | Abilita la rilevazione RLF (Radio Link Failure) |

Attributi disponibili per `gnbPhyAttributes`:

| Nome | Tipo | Range | Valore Iniziale | Descrizione |
|------|------|-------|-----------------|-------------|
| `RbOverhead` | double | 0:0.5 | 0.04 | Overhead nel calcolo del numero di RB utilizzabili |
| `TxPower` | double | -1.79769e+308:1.79769e+308 | 4 | Potenza di trasmissione in dBm |
| `NoiseFigure` | double | -1.79769e+308:1.79769e+308 | 5 | Perdita (dB) nel rapporto segnale-rumore dovuta a non-idealità nel ricevitore |
| `PowerAllocationType` | enum | UniformPowerAllocBw\|UniformPowerAllocUsed | UniformPowerAllocUsed | Tipo di allocazione della potenza: uniforme su tutta la banda o sui RB utilizzati |
| `N0Delay` | uint32_t | 0:1 | 0 | Ritardo minimo di elaborazione per decodificare DL DCI e dati DL |
| `N1Delay` | uint32_t | 0:4 | 2 | Ritardo minimo (lato UE) dalla fine della ricezione dati DL all'inizio della trasmissione ACK/NACK |
| `N2Delay` | uint32_t | 0:4 | 2 | Ritardo minimo di elaborazione per decodificare UL DCI e preparare dati UL |
| `TbDecodeLatency` | Time | -9.22337e+18ns:+9.22337e+18ns | +100000ns | Latenza di decodifica del transport block |
| `Numerology` | uint16_t | 0:65535 | 0 | La numerologia 3GPP da utilizzare |
| `SymbolsPerSlot` | uint16_t | 0:65535 | 14 | Numero di simboli in uno slot |
| `Pattern` | string | - | F\|F\|F\|F\|F\|F\|F\|F\|F\|F\| | Pattern degli slot |
| `CsiRsModel` | enum | CsiRsPerUe\|CsiRsPerBeam | CsiRsPerUe | Tipo di modello CSI-RS: per UE o per beam |
| `CsiRsPeriodicity` | uint16_t | 0:65535 | 10 | Periodicità CSI predefinita in numero di slot |

**Nota:** Dall'attributo `Pattern` possiamo definire il pattern degli slot in termini di Downlink (D), Uplink (U) e Flexible (F). Ad esempio, un pattern di "DDFU" indica che i primi due slot sono Downlink, il terzo è Flexible e il quarto è Uplink.

**Esempio:**
```json
"uePhyAttributes": [
  {"name": "TxPower", "value": 23.0},
  {"name": "EnableUplinkPowerControl", "value": false}
]
```

### Esempio Completo di Configurazione NR PHY Layer
Ecco un esempio completo di configurazione del livello fisico NR in formato JSON:
```json
"phyLayer": [
   {
      "type": "nr",
      "bands": [
        {
          "scenario": "NTN-Suburban",
          "conditionModel": "Default",
          "propagationModel": "ThreeGpp",
          "contiguousCc": true,
          "pathlossAttributes": [
            {
              "name": "ShadowingEnabled",
              "value": false
            }
          ],
          "channelConditionAttributes": [
            {
              "name": "UpdatePeriod",
              "value": "100ms"
            }
          ],
          "frequencyBands": [
            {
              "centralFrequency": 28e9,
              "bandwidth": 10e6,
              "numComponentCarriers": 1
            }
          ]
        }
      ],
      "epc": {
        "helper": "ns3::NrPointToPointEpcHelper",
        "attributes": [
          {
            "name": "S1uLinkDelay",
            "value": "0ms"
          }
        ]
      },
      "beamforming": {
        "helper": "ns3::IdealBeamformingHelper",
        "method": "ns3::DirectPathBeamforming",
        "algorithmAttributes": [],
        "attributes": []
      },
      "ueAntenna": {
        "type": "ns3::IsotropicAntennaModel",
        "arrayProperties": [
          {
            "name": "NumRows",
            "value": 2
          },
          {
            "name": "NumColumns",
            "value": 4
          }
        ]
      },
      "gnbAntenna": {
        "type": "ns3::IsotropicAntennaModel",
        "arrayProperties": [
          {
            "name": "NumRows",
            "value": 4
          },
          {
            "name": "NumColumns",
            "value": 8
          }
        ]
      },
      "gnbBwpManager": {
        "attributes": [
          {
            "name": "NGBR_LOW_LAT_EMBB",
            "value": 0
          }
        ]
      },
      "ueBwpManager": {
        "attributes": [
          {
            "name": "NGBR_LOW_LAT_EMBB",
            "value": 0
          }
        ]
      },
      "uePhyAttributes": [
        {
          "name": "TxPower",
          "value": 180.0
        },
        {
          "name": "EnableUplinkPowerControl",
          "value": false
        }
      ],
      "gnbPhyAttributes": [
        {
          "name": "TxPower",
          "value": 180.0
        }
      ]
    }
]
```

## Layer MAC e di rete NR

NR include la gestione del layer MAC e di rete, aggregata da 1 unico modulo, che viene già configurato con il layer fisico, inoltre gli IP usati sono hardcoded all'interno del codice di NS3 pertanto non sono configurabili via JSON.

È sufficiente quindi aggiungere la seguente configurazione all'interno del dispositivo di rete NR per abilitare il layer MAC e di rete:
```json
"macLayer": [
  {
    "type": "nr"
  }
],
"networkLayer": [
  {
    "type": "ipv4",
    "address": "7.0.0.0",
    "mask": "255.0.0.0",
    "gateway": "7.0.0.1"
  }
]
```

Anche se si cercherà di impostare il network layer su indirizzi diversi da 7.0.0.0/8, questi verranno ignorati e sovrascritti con gli indirizzi hardcoded di 5g-lena.

## Configurazione di UE e gNB in IoD-Sim con NR

Quando andiamo a configurare i vari dispositivi di rete, è possibile configurare il netdevice NR tramite i seguenti parametri come nell'esempio qui sotto.

```json
"netDevices": [
  {
    "type": "nr",
    "networkLayer": 0, // ID del layer nr
    "role": "UE", // Ruolo: UE o gNB
    "bearers": [
      {
        "type": "NGBR_LOW_LAT_EMBB"
      }
    ]
  }
]
```

Come è possibile notare, è possibile specficare il tipo di bearer (flusso) che si vuole utilizzare tra quelli supportati da 5g-lena.

---

## Veicoli sulla Terra

### Struttura di base
```json
"vehicles": [
  {
    "netDevices": [...],
    "mobilityModel": {...},
    "applications": [...]
  }
]
```
I veicoli in generale agiscono come dei normali nodi, pertanto ne supportano tutti i parametri, ma vegono tracciati separatamente per comodità d'uso e per loggare separatamente le variazioni delle loro posizioni.

In particolare però concentriamo il dettagli sul modello geocentrico a velocità costante creato proprio per i veicoli terrestri:

### `vehicles[n].mobilityModel`
**Descrizione:** Modello di mobilità del veicolo sulla terra.

```json
"mobilityModel": {
  "name": "ns3::GeoConstantVelocityMobility",
  "attributes": [
    {"name": "InitialLatitude", "value": 19.5},
    {"name": "InitialLongitude", "value": 1.5},
    {"name": "Altitude", "value": 0.0},
    {"name": "Speed", "value": 30.0},
    {"name": "Azimuth", "value": 45.0}
  ]
}
```

| Parametro | Tipo | Descrizione |
|-----------|------|-------------|
| `InitialLatitude` | number | Latitudine iniziale in gradi (-90 a +90) |
| `InitialLongitude` | number | Longitudine iniziale in gradi (-180 a +180) |
| `Altitude` | number | Altitudine in metri (0 per superficie) |
| `Speed` | number | Velocità in m/s |
| `Azimuth` | number | Direzione di movimento in gradi (0-360) |

---

## Satelliti LEO

### Struttura di base
```json
"leo-sats": [
  {
    "netDevices": [...],
    "mobilityModel": {...},
    "applications": []
  }
]
```

Allo stesso modo, anche i satelliti LEO sono nodi con tutti i parametri standard, ma viene separatamente tracciata la loro mobilità orbitale.
Inoltre anche in questo caso vediamo nello specifico il modello di mobilità geocentrica per satelliti LEO:


### `leo-sats[n].mobilityModel`
**Descrizione:** Modello di mobilità orbitale LEO.

```json
"mobilityModel": {
  "name": "ns3::GeoLeoOrbitMobility",
  "attributes": [
    {"name": "Altitude", "value": 400.0},
    {"name": "Inclination", "value": 20.0},
    {"name": "Longitude", "value": 0.0},
    {"name": "Offset", "value": 0.0},
    {"name": "RetrogradeOrbit", "value": false}
  ]
}
```

| Parametro | Tipo | Descrizione |
|-----------|------|-------------|
| `Altitude` | number | Altitudine orbitale in km |
| `Inclination` | number | Inclinazione orbitale in gradi (0-180) |
| `Longitude` | number | Longitudine del nodo ascendente in gradi |
| `Offset` | number | Offset orbitale (phase offset) in gradi |
| `RetrogradeOrbit` | boolean | Se `true`, orbita retrograda |

---

## Esempi di Configurazione

### Esempio 1: Simulazione breve con 2 satelliti e 1 auto
```json
{
  "name": "leo-nr-short-test",
  "resultsPath": "../results/",
  "logOnFile": false,
  "duration": 10,
  "vehicles": [
    {
      "mobilityModel": {
        "name": "ns3::GeoConstantVelocityMobility",
        "attributes": [
          {"name": "InitialLatitude", "value": 45.0},
          {"name": "InitialLongitude", "value": 10.0},
          {"name": "Altitude", "value": 0.0},
          {"name": "Speed", "value": 20.0},
          {"name": "Azimuth", "value": 0.0}
        ]
      }
    }
  ],
  "leo-sats": [
    {
      "mobilityModel": {
        "name": "ns3::GeoLeoOrbitMobility",
        "attributes": [
          {"name": "Altitude", "value": 550.0},
          {"name": "Inclination", "value": 53.0},
          {"name": "Longitude", "value": 0.0},
          {"name": "Offset", "value": 0.0},
          {"name": "RetrogradeOrbit", "value": false}
        ]
      }
    },
    {
      "mobilityModel": {
        "name": "ns3::GeoLeoOrbitMobility",
        "attributes": [
          {"name": "Altitude", "value": 550.0},
          {"name": "Inclination", "value": 53.0},
          {"name": "Longitude", "value": 0.0},
          {"name": "Offset", "value": 180.0},
          {"name": "RetrogradeOrbit", "value": false}
        ]
      }
    }
  ]
}
```
