# Guida LEO-NR Communication Scenario

Questo documento descrive tutte le opzioni configurabili nelle simulazioni che usano i moduli leo e i moduli nr in IoD-Sim.

## Indice
1. [Configurazione Globale](#configurazione-globale)
2. [Configurazione del Mondo](#configurazione-del-mondo)
3. [Configurazione di NR](#configurazione-di-nr)
  - [Configurazioni delle bande NR](#configurazioni-delle-bande-nr)
  - [Layer MAC e di rete NR](#layer-mac-e-di-rete-nr)
  - [Configurazione di UE e gNB in IoD-Sim con NR](#configurazione-di-ue-e-gnb-in-iod-sim-con-nr)
4. [Veicoli sulla Terra](#veicoli-sulla-terra)
5. [Satelliti LEO](#satelliti-leo)
6. [Esempi di Configurazione](#esempi-di-configurazione)

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

## Configurazione di NR

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

### `phyLayer[0].scheduler`
**Descrizione:** Configurazione del scheduler NR.

Esempio di configurazione del scheduler:

```json
"scheduler": : {
  "type": "ns3::NrMacSchedulerOfdmaPF",
  "attributes": [
    {
      "name": "HarqEnabled",
      "value": true
    }
  ]
}
```

In questo parametro va impostato come definire lo scheduler da usare per la gestione delle risorse radio. Possiamo scegliere tra i vari scheduler disponibili in 5g-lena:

| Tipo Scheduler (Classe ns-3)   | Famiglia | Logica dell'Algoritmo    | Descrizione                                                                                                                                                          |
|--------------------------------|----------|--------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| ns3::NrMacSchedulerOfdmaRR     | OFDMA    | Round Robin (RR)         | Assegna le risorse agli UE a turno, ciclicamente. Garantisce massima equità (fairness) ma non ottimizza il throughput totale né la qualità del canale.               |
| ns3::NrMacSchedulerOfdmaPF     | OFDMA    | Proportional Fair (PF)   | Cerca un compromesso tra equità e throughput. Assegna risorse basandosi sul rapporto tra la qualità del canale istantanea e il throughput medio storico dell'utente. |
| ns3::NrMacSchedulerOfdmaMR     | OFDMA    | Max Rate (MR)            | Assegna le risorse all'UE con il miglior canale in assoluto. Massimizza il throughput della cella ma affama gli utenti ai bordi (minima equità).                     |
| ns3::NrMacSchedulerOfdmaQos    | OFDMA    | Quality of Service (QoS) | Prioritizza gli utenti in base ai requisiti di QoS (es. priorità del bearer/flusso). Ideale per scenari misti con traffico voce, video e dati.                       |
| ns3::NrMacSchedulerOfdmaRandom | OFDMA    | Random                   | Assegna le risorse in modo puramente casuale. Usato principalmente per scopi di debug o baseline.                                                                    |
| ns3::NrMacSchedulerOfdmaAi     | OFDMA    | AI / External            | Interfaccia per collegare agenti di apprendimento esterni (es. tramite ns3-ai) per prendere decisioni di scheduling basate su Machine Learning.                      |
| ns3::NrMacSchedulerTdmaRR      | TDMA     | Round Robin (RR)         | Versione TDMA del Round Robin. Assegna l'intero slot agli utenti a turno.                                                                                            |
| ns3::NrMacSchedulerTdmaPF      | TDMA     | Proportional Fair (PF)   | Versione TDMA del Proportional Fair.                                                                                                                                 |
| ns3::NrMacSchedulerTdmaMR      | TDMA     | Max Rate (MR)            | Versione TDMA del Max Rate.                                                                                                                                          |
| ns3::NrMacSchedulerTdmaQos     | TDMA     | Quality of Service (QoS) | Versione TDMA dello scheduler QoS.                                                                                                                                   |
| ns3::NrMacSchedulerTdmaRandom  | TDMA     | Random                   | Versione TDMA dello scheduler casuale.                                                                                                                               |
| ns3::NrMacSchedulerTdmaAi      | TDMA     | AI / External            | Versione TDMA per agenti AI esterni.                                                                                                                                 |

#### Attributi comuni degli scheduler OFDMA

Gli scheduler OFDMA e TDMA condividono i seguenti attributi configurabili:

| Nome Attributo | Tipo | Range/Opzioni | Valore Iniziale | Descrizione |
|----------------|------|---------------|-----------------|-------------|
| `CqiTimerThreshold` | Time | -9.22337e+18ns:+9.22337e+18ns | +1e+09ns | Tempo di validità di un CQI |
| `FixedMcsDl` | boolean | - | false | Fissa l'MCS DL al valore impostato in StartingMcsDl |
| `FixedMcsUl` | boolean | - | false | Fissa l'MCS UL al valore impostato in StartingMcsUl |
| `StartingMcsDl` | uint8_t | 0:255 | 0 | MCS iniziale per DL |
| `StartingMcsUl` | uint8_t | 0:255 | 0 | MCS iniziale per UL |
| `DlCtrlSymbols` | uint8_t | 0:255 | 1 | Numero di simboli allocati per il controllo DL |
| `UlCtrlSymbols` | uint8_t | 0:255 | 1 | Numero di simboli allocati per il controllo UL |
| `SrsSymbols` | uint8_t | 0:255 | 1 | Numero di simboli allocati per SRS in UL |
| `EnableSrsInUlSlots` | boolean | - | true | Abilita la trasmissione SRS negli slot UL (oltre agli slot F) |
| `EnableSrsInFSlots` | boolean | - | true | Abilita la trasmissione SRS negli slot F |
| `DlAmc` | Ptr | - | 0 | Puntatore all'AMC DL dello scheduler |
| `UlAmc` | Ptr | - | 0 | Puntatore all'AMC UL dello scheduler |
| `MaxDlMcs` | int8_t | -1:30 | -1 | Indice MCS massimo per DL (-1 = nessun limite) |
| `EnableHarqReTx` | boolean | - | true | Abilita le ritrasmissioni HARQ (max 3 se true, 0 se false) |
| `SchedLcAlgorithmType` | TypeId | - | ns3::NrMacSchedulerLcRR | Tipo di algoritmo per assegnare byte ai diversi LC |
| `RachUlGrantMcs` | uint8_t | 0:255 | 0 | MCS del grant UL RACH (deve essere [0..15]) |
| `McsCsiSource` | enum | AVG_MCS\|AVG_SPEC_EFF\|AVG_SINR\|WIDEBAND_MCS | WIDEBAND_MCS | Fonte delle informazioni CSI per stimare l'MCS DL |

Solo per OFDMA:
| Nome Attributo | Tipo | Range/Opzioni | Valore Iniziale | Descrizione |
|----------------|------|---------------|-----------------|-------------|
| `SymPerBeamType` | enum | LOAD_BASED\|ROUND_ROBIN\|PROPORTIONAL_FAIR | LOAD_BASED | Tipo di allocazione dei simboli per beam |

**Note:**
- `SymPerBeamType`: Determina come vengono allocati i simboli tra i diversi beam (basato sul carico, round robin o proporzionalmente equo)
- `McsCsiSource`: Sceglie quale metrica CSI utilizzare per la stima dell'MCS DL (MCS medio, efficienza spettrale media, SINR medio o MCS wideband)

#### Attributi specifici di NrMacSchedulerOfdmaPF, NrMacSchedulerOfdmaQos, NrMacSchedulerTdmaPF

Lo scheduler Proportional Fair (PF) ha attributi aggiuntivi per controllare il bilanciamento tra equità e throughput:

| Nome Attributo | Tipo | Range | Valore Iniziale | Descrizione |
|----------------|------|-------|-----------------|-------------|
| `FairnessIndex` | float | 0:1 | 1 | Indice che definisce la metrica PF (1 = PF tradizionale 3GPP, 0 = Round Robin in throughput) |
| `LastAvgTPutWeight` | float | 0:3.40282e+38 | 99 | Peso dell'ultimo throughput medio nel calcolo del throughput medio |

**Note:**
- `FairnessIndex`: Controlla il bilanciamento tra equità e massimizzazione del throughput. Un valore di 1 implementa il Proportional Fair tradizionale secondo le specifiche 3GPP, mentre un valore di 0 si comporta come un Round Robin basato sul throughput
- `LastAvgTPutWeight`: Determina quanto peso dare al throughput medio storico rispetto alle misure recenti. Valori più alti danno più peso alla storia passata, rendendo la metrica più stabile ma meno reattiva ai cambiamenti

#### Attributi specifici di NrMacSchedulerOfdmaAi

Gli scheduler AI permettono l'integrazione con modelli di Machine Learning esterni (es. tramite ns3-ai) per decisioni di scheduling intelligenti. Ereditano tutti gli attributi da `NrMacSchedulerOfdmaQos` (o `NrMacSchedulerTdmaQos` per la versione TDMA) e aggiungono:

| Nome Attributo | Tipo | Valore Iniziale | Descrizione |
|----------------|------|-----------------|-------------|
| `NotifyCbDl` | Callback | NullCallback | Funzione di callback per notificare il modello AI per il downlink |
| `NotifyCbUl` | Callback | NullCallback | Funzione di callback per notificare il modello AI per l'uplink |
| `ActiveDlAi` | boolean | false | Flag per attivare il modello AI per il downlink |
| `ActiveUlAi` | boolean | false | Flag per attivare il modello AI per l'uplink |

**Note:**
- `NotifyCbDl` e `NotifyCbUl`: Devono essere configurati tramite codice C++ per collegare il modello AI esterno. Non possono essere impostati direttamente da JSON
- `ActiveDlAi` e `ActiveUlAi`: Controllano se le decisioni di scheduling vengono delegate al modello AI. Se `false`, lo scheduler si comporta come un normale scheduler QoS
- Gli scheduler AI sono progettati per integrarsi con framework come ns3-ai per implementare algoritmi di scheduling basati su reinforcement learning o altri approcci ML

### `phyLayer[0].error-model`
**Tipo:** `array`
**Descrizione:** Array di configurazioni dei modelli di errore per la simulazione del canale radio NR.

Il modello di errore simula le perdite di pacchetti e gli errori di trasmissione in base alle condizioni del canale (SINR, MCS, ecc.). È fondamentale per una simulazione realistica delle prestazioni del sistema.

**Esempio di configurazione singola (DL e UL con stesso modello):**

```json
"error-model": [
  {
    "type": "ns3::NrEesmIrT1",
    "direction": "both",
    "amcAttributes": [
      {
        "name": "McsTable",
        "value": "NrEesmIrT1"
      }
    ]
  }
]
```

#### Parametri del modello di errore

| Parametro | Tipo | Valori | Descrizione |
|-----------|------|--------|-------------|
| `type` | string | TypeId ns-3 | Tipo del modello di errore da utilizzare |
| `direction` | string | `both` \| `uplink` \| `downlink` | Direzione a cui applicare il modello (default: `both`) |
| `amcAttributes` | array | - | Array di attributi specifici per l'AMC (Adaptive Modulation and Coding) |

**Valori di `direction`:**
- `both`: Applica lo stesso modello sia a downlink che uplink
- `downlink`: Applica il modello solo al downlink (DL)
- `uplink`: Applica il modello solo all'uplink (UL)

#### Attributi AMC (Adaptive Modulation and Coding)

Gli attributi `amcAttributes` configurano il modulo AMC che determina il MCS (Modulation and Coding Scheme) basandosi sul CQI:

| Nome Attributo | Tipo | Range/Opzioni | Valore Iniziale | Descrizione |
|----------------|------|---------------|-----------------|-------------|
| `NumRefScPerRb` | uint8_t | 0:12 | 1 | Numero di sottoportanti che trasportano Reference Signals per Resource Block |
| `AmcModel` | enum | ErrorModel \| ShannonModel | ErrorModel | Modello AMC utilizzato per assegnare il CQI |

**Valori di `AmcModel`:**
- `ErrorModel`: Usa il modello di errore configurato (es. NrEesmIrT1) per calcolare il CQI in base alle tabelle BLER
- `ShannonModel`: Usa il limite teorico di Shannon per calcolare il CQI (più ottimistico, utile per analisi teoriche)

**Esempio con attributi AMC:**

```json
"error-model": [
  {
    "type": "ns3::NrEesmIrT1",
    "direction": "both",
    "amcAttributes": [
      {
        "name": "AmcModel",
        "value": "ErrorModel"
      },
      {
        "name": "NumRefScPerRb",
        "value": 1
      }
    ]
  }
]
```

**Nota:** Sia type che amcAttributes sono opzionali: Si possono usare insieme o separatamente in oggetti separati nell'array, a seconda delle esigenze di configurazione.

#### Modelli di errore disponibili

I modelli di errore disponibili in 5g-lena sono organizzati in due rami principali:

##### Ramo NrEesmErrorModel

Modelli basati su EESM (Exponential Effective SINR Mapping) con supporto per HARQ:

| Tipo Modello | Descrizione | Supporto HARQ |
|--------------|-------------|---------------|
| `ns3::NrEesmErrorModel` | Classe base astratta per modelli EESM | - |
| `ns3::NrEesmCc` | Error model con Chase Combining (classe base CC) | Chase Combining |
| `ns3::NrEesmCcT1` | EESM con Chase Combining e tabelle di tipo 1 (LDPC) | Chase Combining |
| `ns3::NrEesmCcT2` | EESM con Chase Combining e tabelle di tipo 2 (modulazioni elevate) | Chase Combining |
| `ns3::NrEesmIr` | Error model con Incremental Redundancy (classe base IR) | Incremental Redundancy |
| `ns3::NrEesmIrT1` | EESM con Incremental Redundancy e tabelle di tipo 1 (LDPC) | Incremental Redundancy |
| `ns3::NrEesmIrT2` | EESM con Incremental Redundancy e tabelle di tipo 2 (modulazioni elevate) | Incremental Redundancy |

##### Ramo NrLteMiErrorModel

Modelli basati su Mutual Information, compatibili con LTE:

| Tipo Modello | Descrizione |
|--------------|-------------|
| `ns3::NrLteMiErrorModel` | Classe base per modelli basati su Mutual Information |
| `ns3::LenaErrorModel` | Error model compatibile con LENA (LTE framework) |



**Esempio completo con configurazione DL/UL separata:**

Per configurare modelli diversi per downlink e uplink, aggiungere due elementi all'array `error-model` con `direction` appropriata:

```json
"error-model": [
  {
    "type": "ns3::NrEesmIrT1",
    "direction": "downlink",
    "amcAttributes": []
  },
  {
    "type": "ns3::NrEesmIrT2",
    "direction": "uplink",
    "amcAttributes": [
      {"name": "NumRefScPerRb", "value": 2}
    ]
  }
]
```

**Nota:** È possibile specificare più configurazioni `error-model` con direzioni diverse nello stesso array. Se si specificano più configurazioni con la stessa direzione, l'ultima sovrascriverà le precedenti.




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
    ],
    "phy": [
      {
        "bwpId": 0, // Optional, if not specified, this is applied to all BWPs
        "name": "TxPower",
        "value": 23.0
      }
    ]
  }
]
```

Come è possibile notare, è possibile specficare il tipo di bearer (flusso) che si vuole utilizzare tra quelli supportati da 5g-lena.

Inoltre è possibile specificare degli attributi nel layer fisico specifici per quel netdevice NR: l'attributo è applicato ad una particolare BWP se specificato, altrimenti a tutti i BWP del dispositivo.

Gli attributi supportati sono i medesimi di quelli elencati nella sezione precedente per `uePhyAttributes` e `gnbPhyAttributes`.

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
