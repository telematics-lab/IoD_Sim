# LEO-NR Communication Scenario Guide

This document describes all configurable options in simulations using the leo and nr modules in IoD-Sim.

## Index
1. [Global Configuration](#global-configuration)
2. [World Configuration](#world-configuration)
3. [NR Configuration](#nr-configuration)
  - [NR Band Configurations](#nr-band-configurations)
  - [NR MAC and Network Layer](#nr-mac-and-network-layer)
  - [UE and gNB Configuration in IoD-Sim with NR](#ue-and-gnb-configuration-in-iod-sim-with-nr)
4. [Vehicles on Earth](#vehicles-on-earth)
5. [LEO Satellites](#leo-satellites)
6. [Configuration Examples](#configuration-examples)
7. [NR Radio Environment Map](#nr-radio-environment-map)

---

## Global Configuration

### `staticNs3Config`
**Type:** `array[object]`
**Description:** Array of static ns-3 configurations to apply before the simulation. Each object has `name` (attribute name) and `value`.

It is advisable to set some global parameters useful for configuring certain nr settings and configurations on the earth model used in LEO and NTN propagation models.

For Nr, in the lena examples, the default value of the RLC UM buffer is "overridden" to avoid packet loss in high-latency scenarios such as satellite ones.
```json
{
  "name": "ns3::NrRlcUm::MaxTxBufferSize",
  "value": 999999999
}
```

While for LEO configurations, it is important to specify the type of earth spheroid used in the geocentric mobility model and in the NTN propagation models. Usually, it is convenient to use a perfect sphere (`SPHERE`) for the positioning of satellites and ground vehicles.
```json
{
  "name": "ns3::GeocentricMobilityModel::EarthSpheroidType",
  "value": "SPHERE"
}
```

Furthermore, we can specify the precision of the mobility models, setting the position update frequency for the various models (scheduling the updates in which CourseChange will be called).

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

In this example, we set the frequency to 1 second in the LEO orbital mobility models and in the constant velocity mobility models used by ground vehicles.

## World Configuration

### `world.size`
**Description:** Dimensions of the simulated space in meters.

| Parameter | Type | Description |
|-----------|------|-------------|
| `X` | string | Dimension along the X axis |
| `Y` | string | Dimension along the Y axis |
| `Z` | string | Dimension along the Z axis (altitude) |

**Example:**
```json
"world": {
  "size": {
    "X": "40000000",
    "Y": "40000000",
    "Z": "40000000"
  }
}
```

In these models, it is necessary to set a very large world dimension because we will have the earth at the center of the coordinates and the satellites in LEO orbits around it, so we must consider a dimension that can encompass everything like the one provided in this example.

---

## NR Configuration

In the `phyLayer` settings, we can configure a physical layer of type `nr` which will be supported by 5g-lena and allows configuration with the following options:


### `type`
**Type:** `string`
**Options:** `nr`, `lte`, `wifi`, `none`
**Description:** Type of physical layer. In our case, it is `nr` for 5G NR (New Radio) communications.

### `attachMethod`
**Type:** `string`
**Options:** `closest`, `max-rsrp`, `none`
**Default:** `max-rsrp`
**Description:** Method used to attach UEs to gNBs.
- `closest`: Attaches the UE to the geographically closest gNB.
- `max-rsrp`: Attaches the UE to the gNB with the strongest signal (RSRP).
- `none`: Does not perform attachment.

### `fullMeshX2Links`
**Optional**
**Type:** `boolean`
**Default:** `true`
**Description:** If set to `true`, the simulation will automatically create X2 interfaces between all pairs of gNBs configured within this PHY layer, creating a full mesh topology. This enables X2 handovers between any pair of gNBs.

### `channels`
**Description:** Array of channels (Operation Bands) available for communication.

## NR Channel Configurations

We have the possibility to configure various NR channels with different scenarios, propagation models, and frequencies.

In particular, for the single channel, we have the following configurable parameters:

### `channels[i].scenario`
**Type**: `string`
**Options:** `InF`, `InH`, `UMa`, `UMi`, `RMa`, `InH-OfficeMixed`, `InH-OfficeOpen`, `V2V-Highway`, `V2V-Urban`, `NTN-DenseUrban`, `NTN-Urban`, `NTN-Suburban`, `NTN-Rural`
**Description:** Propagation scenario for the NR channel.

In the scenarios of our interest for LEO-satellite communications, it is convenient to use `NTN-*` scenarios such as `NTN-Suburban`, `NTN-Urban`, `NTN-DenseUrban`, or `NTN-Rural` depending on the type of terrestrial environment where the vehicles are located.

### `channels[i].propagationModel`
**Type:** `string`
**Options:** `NYU`, `TwoRay`, `ThreeGpp`
**Description:** Propagation model for the NR channel.

The choice of scenario, model conditions, and propagation model type will influence the calculation of path loss and channel quality between nodes, defining the ns3 ChannelModel according to a table.

In particular, with this parameter, we define the "Spectrum Propagation Loss Model" used to calculate the path loss between nodes.
| ChannelModel | SpectrumPropagationLossModel |
|---------------|------------------------------|
| ChannelModel::ThreeGpp | ThreeGppSpectrumPropagationLossModel |
| ChannelModel::TwoRay | TwoRaySpectrumPropagationLossModel |
| ChannelModel::NYU | NYUSpectrumPropagationLossModel |

### `channels[i].conditionModel`
**Type:** `string`
**Options:** `NLOS`, `LOS`, `Buildings`, `Default`
**Description:** Channel condition model for the NR channel.

With this option, we can specify a specific Channel Condition Model. If we select 'Default', this will be automatically chosen based on the selected scenario and propagation model.

#### Lookup Table: Available Propagation Models

The following table shows the valid combinations of **Propagation Model** and **Scenario**:

##### ThreeGpp and TwoRay Models

In case we set TwoRay as the propagation model, the same scheme as ThreeGpp will be used for the available scenarios.

The Channel Condition Model will be used if conditionModel is "Default".

| Scenario | Propagation Loss Model | Channel Condition Model (Default conditions) | Description |
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

The Channel Condition Model will be used if conditionModel is "Default".

| Scenario | Propagation Loss Model | Channel Condition Model (Default conditions) | Description |
|----------|------------------------|------------------------|-------------|
| `RMa` | NYURmaPropagationLossModel | NYURmaChannelConditionModel | Rural Macrocell |
| `UMa` | NYUUmaPropagationLossModel | NYUUmaChannelConditionModel | Urban Macrocell |
| `UMi` | NYUUmiPropagationLossModel | NYUUmiChannelConditionModel | Urban Microcell |
| `InH` | NYUInHPropagationLossModel | NYUInHChannelConditionModel | Indoor |
| `InF` | NYUInFPropagationLossModel | NYUInFChannelConditionModel | In-Factory |


#### Channel Condition Model on conditionModel != "Default"

If conditionModel is set to a value other than "Default", the Channel Condition Model will be chosen based on the following table:

| Condition | Channel Condition Model |
|------------|-------------------------|
| NLOS       | NeverLosChannelConditionModel |
| LOS        | AlwaysLosChannelConditionModel |
| Buildings  | BuildingsChannelConditionModel |

### `channels[i].pathlossAttributes`
**Type:** `array`
**Description:** List of path loss attributes for the specified channel. Each path loss attribute is defined by an object specifying the attribute name and value.
**Notes:** These attributes are passed to the selected PropagationLoss model, so the available parameters depend on the chosen propagation model.

In general, for ThreeGpp (or TwoRay) models, we have these attributes taken from the generic ThreeGppPropagationLossModel:

| Name | Type | Initial Value | Description |
|------|------|----------------|-------------|
| `BuildingPenetrationLossesEnabled` | boolean | true | Enable/disable building penetration losses |
| `EnforceParameterRanges` | boolean | false | Whether to strictly enforce TR38.901 applicability ranges |
| `ShadowingEnabled` | boolean | true | Enable/disable shadowing effect |

While for NYU models, we have these attributes taken from the generic NYUPropagationLossModel:

| Name | Type | Initial Value | Description |
|------|------|----------------|-------------|
| `FoliageLoss` | double | 0.4 | Foliage loss in dB |
| `ShadowingEnabled` | boolean | true | Enable/disable shadowing effect |
| `O2ILosstype` | string | "Low Loss" | Outdoor-to-Indoor (O2I) penetration loss type - "Low Loss" or "High Loss" |
| `FoliageLossEnabled` | boolean | false | Enable/disable foliage loss |
| `AtmosphericLossEnabled` | boolean | false | Enable/disable atmospheric loss |
| `Pressure` | double | 1013.25 | Barometric pressure in mbar |
| `Humidity` | double | 50 | Humidity in percentage |
| `Temperature` | double | 20 | Temperature in degrees Celsius |
| `RainRate` | double | 0 | Rain rate in mm/hr |

For specific models, there might be additional more specific attributes available, for which reference is made to the ns-3 and 5g-lena documentation.

### `channels[i].channelConditionAttributes`
**Description:** Channel condition attributes for the specified channel. Each channel condition attribute is defined by an object specifying the attribute name and value.

Also in this case, the available attributes depend on the selected channel condition model.

For example, for "Default" channelCondition with ThreeGpp models, we have these common attributes (from ):

| Name | Type | Initial Value | Description |
|------|------|----------------|-------------|
| `LinkO2iConditionToAntennaHeight` | boolean | false | Specifies if the O2I condition will be determined based on the UE antenna height (if UE height is 1.5m then it is O2O, otherwise it is O2I) |
| `O2iLowLossThreshold` | double (0:1) | 1 | Specifies the ratio of low vs. high O2I penetration losses. Default value 1.0 means all losses will be low |
| `O2iThreshold` | double (0:1) | 0 | Specifies the ratio of O2I channel conditions. Default value 0 corresponds to 0 O2I losses |
| `UpdatePeriod` | Time | 0ns | Specifies the time period after which the channel condition is recalculated. If set to 0, the channel condition is never updated |

In the case of NYU models, we have these common attributes (from NYUPropagationLossModel):

| Name | Type | Initial Value | Description |
|------|------|----------------|-------------|
| `UpdatePeriod` | Time | 0ms | Specifies the time period after which the channel condition is recalculated. If set to 0, the channel condition is never updated |

While for NLOS, LOS, and Buildings models, we do not have specifiable attributes.


### `channels[i].phasedSpectrumAttributes`
**Description:** Phased spectrum attributes for the specified channel. Each phased spectrum attribute is defined by an object specifying the attribute name and value.

Here we can specify attributes for the Spectrum PropagationLossModel. In particular, however, there are no useful attributes associated with these models.
The possibility to use this functionality derived from the 5g-lena nr helper is still left open.

### `channels[i].bands`
**Type:** `array`
**Description:** List of operation bands for the specified NR channel. Each band can be configured as contiguous or non-contiguous.

Each band object structure depends on its `type`:

#### Contiguous Band
**Type:** `contiguous`

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | string | Must be `contiguous`. |
| `carrier` | object | Configuration of the single carrier. |

#### Non-Contiguous Band
**Type:** `non-contiguous`

| Parameter | Type | Description |
|-----------|------|-------------|
| `type` | string | Must be `non-contiguous`. |
| `carriers` | array | List of carrier configurations. |

#### Carrier Configuration
Each carrier object (inside `carrier` or `carriers`) has the following parameters:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `centralFrequency` | double | 28e9 | Central frequency in Hz. |
| `bandwidth` | double | 100e6 | Bandwidth in Hz. |
| `numComponentCarriers` | uint8_t | 1 | Number of component carriers. |
| `numBandwidthParts` | uint8_t | 1 | Number of bandwidth parts. |

**Example:**
```json
"bands": [
  {
    "type": "contiguous",
    "carrier": {
      "centralFrequency": 28e9,
      "bandwidth": 100e6,
      "numComponentCarriers": 1,
      "numBandwidthParts": 2
    }
  },
  {
    "type": "non-contiguous",
    "carriers": [
      {
         "centralFrequency": 30e9,
         "bandwidth": 50e6
      },
      {
         "centralFrequency": 30.1e9,
         "bandwidth": 50e6
      }
    ]
  }
]
```

### `phyLayer[0].scheduler`
**Description:** NR scheduler configuration.

Example of scheduler configuration:

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

In this parameter, we set how to define the scheduler to use for radio resource management. We can choose from various schedulers available in 5g-lena:

| Scheduler Type (ns-3 Class)   | Family | Algorithm Logic    | Description                                                                                                                                                          |
|--------------------------------|----------|--------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| ns3::NrMacSchedulerOfdmaRR     | OFDMA    | Round Robin (RR)         | Assigns resources to UEs in turn, cyclically. Guarantees maximum fairness but does not optimize total throughput or channel quality.               |
| ns3::NrMacSchedulerOfdmaPF     | OFDMA    | Proportional Fair (PF)   | Seeks a compromise between fairness and throughput. Assigns resources based on the ratio between instantaneous channel quality and the user's historical average throughput. |
| ns3::NrMacSchedulerOfdmaMR     | OFDMA    | Max Rate (MR)            | Assigns resources to the UE with the absolute best channel. Maximizes cell throughput but starves edge users (minimum fairness).                     |
| ns3::NrMacSchedulerOfdmaQos    | OFDMA    | Quality of Service (QoS) | Prioritizes users based on QoS requirements (e.g., bearer/flow priority). Ideal for mixed scenarios with voice, video, and data traffic.                       |
| ns3::NrMacSchedulerOfdmaRandom | OFDMA    | Random                   | Assigns resources purely randomly. Mainly used for debug or baseline purposes.                                                                    |
| ns3::NrMacSchedulerOfdmaAi     | OFDMA    | AI / External            | Interface to connect external learning agents (e.g., via ns3-ai) to make scheduling decisions based on Machine Learning.                      |
| ns3::NrMacSchedulerTdmaRR      | TDMA     | Round Robin (RR)         | TDMA version of Round Robin. Assigns the entire slot to users in turn.                                                                                            |
| ns3::NrMacSchedulerTdmaPF      | TDMA     | Proportional Fair (PF)   | TDMA version of Proportional Fair.                                                                                                                                 |
| ns3::NrMacSchedulerTdmaMR      | TDMA     | Max Rate (MR)            | TDMA version of Max Rate.                                                                                                                                          |
| ns3::NrMacSchedulerTdmaQos     | TDMA     | Quality of Service (QoS) | TDMA version of QoS scheduler.                                                                                                                                   |
| ns3::NrMacSchedulerTdmaRandom  | TDMA     | Random                   | TDMA version of random scheduler.                                                                                                                               |
| ns3::NrMacSchedulerTdmaAi      | TDMA     | AI / External            | TDMA version for external AI agents.                                                                                                                                 |

#### Common Attributes of OFDMA Schedulers

OFDMA and TDMA schedulers share the following configurable attributes:

| Attribute Name | Type | Range/Options | Initial Value | Description |
|----------------|------|---------------|-----------------|-------------|
| `CqiTimerThreshold` | Time | -9.22337e+18ns:+9.22337e+18ns | +1e+09ns | Validity time of a CQI |
| `FixedMcsDl` | boolean | - | false | Fixes DL MCS to the value set in StartingMcsDl |
| `FixedMcsUl` | boolean | - | false | Fixes UL MCS to the value set in StartingMcsUl |
| `StartingMcsDl` | uint8_t | 0:255 | 0 | Initial MCS for DL |
| `StartingMcsUl` | uint8_t | 0:255 | 0 | Initial MCS for UL |
| `DlCtrlSymbols` | uint8_t | 0:255 | 1 | Number of symbols allocated for DL control |
| `UlCtrlSymbols` | uint8_t | 0:255 | 1 | Number of symbols allocated for UL control |
| `SrsSymbols` | uint8_t | 0:255 | 1 | Number of symbols allocated for SRS in UL |
| `EnableSrsInUlSlots` | boolean | - | true | Enables SRS transmission in UL slots (in addition to F slots) |
| `EnableSrsInFSlots` | boolean | - | true | Enables SRS transmission in F slots |
| `DlAmc` | Ptr | - | 0 | Pointer to the scheduler's DL AMC |
| `UlAmc` | Ptr | - | 0 | Pointer to the scheduler's UL AMC |
| `MaxDlMcs` | int8_t | -1:30 | -1 | Maximum MCS index for DL (-1 = no limit) |
| `EnableHarqReTx` | boolean | - | true | Enables HARQ retransmissions (max 3 if true, 0 if false) |
| `SchedLcAlgorithmType` | TypeId | - | ns3::NrMacSchedulerLcRR | Type of algorithm to assign bytes to different LCs |
| `RachUlGrantMcs` | uint8_t | 0:255 | 0 | MCS of the RACH UL grant (must be [0..15]) |
| `McsCsiSource` | enum | AVG_MCS\|AVG_SPEC_EFF\|AVG_SINR\|WIDEBAND_MCS | WIDEBAND_MCS | Source of CSI information to estimate DL MCS |

Only for OFDMA:
| Attribute Name | Type | Range/Options | Initial Value | Description |
|----------------|------|---------------|-----------------|-------------|
| `SymPerBeamType` | enum | LOAD_BASED\|ROUND_ROBIN\|PROPORTIONAL_FAIR | LOAD_BASED | Type of symbol allocation per beam |

**Notes:**
- `SymPerBeamType`: Determines how symbols are allocated among different beams (load-based, round robin, or proportionally fair)
- `McsCsiSource`: Chooses which CSI metric to use for DL MCS estimation (average MCS, average spectral efficiency, average SINR, or wideband MCS)

#### Specific Attributes of NrMacSchedulerOfdmaPF, NrMacSchedulerOfdmaQos, NrMacSchedulerTdmaPF

The Proportional Fair (PF) scheduler has additional attributes to control the balance between fairness and throughput:

| Attribute Name | Type | Range | Initial Value | Description |
|----------------|------|-------|-----------------|-------------|
| `FairnessIndex` | float | 0:1 | 1 | Index defining the PF metric (1 = traditional 3GPP PF, 0 = Round Robin in throughput) |
| `LastAvgTPutWeight` | float | 0:3.40282e+38 | 99 | Weight of the last average throughput in the average throughput calculation |

**Notes:**
- `FairnessIndex`: Controls the balance between fairness and throughput maximization. A value of 1 implements traditional Proportional Fair according to 3GPP specifications, while a value of 0 behaves like a Round Robin based on throughput
- `LastAvgTPutWeight`: Determines how much weight to give to historical average throughput versus recent measurements. Higher values give more weight to past history, making the metric more stable but less reactive to changes

#### Specific Attributes of NrMacSchedulerOfdmaAi

AI schedulers allow integration with external Machine Learning models (e.g., via ns3-ai) for intelligent scheduling decisions. They inherit all attributes from `NrMacSchedulerOfdmaQos` (or `NrMacSchedulerTdmaQos` for the TDMA version) and add:

| Attribute Name | Type | Initial Value | Description |
|----------------|------|-----------------|-------------|
| `NotifyCbDl` | Callback | NullCallback | Callback function to notify the AI model for downlink |
| `NotifyCbUl` | Callback | NullCallback | Callback function to notify the AI model for uplink |
| `ActiveDlAi` | boolean | false | Flag to activate the AI model for downlink |
| `ActiveUlAi` | boolean | false | Flag to activate the AI model for uplink |

**Notes:**
- `NotifyCbDl` and `NotifyCbUl`: Must be configured via C++ code to connect the external AI model. They cannot be set directly via JSON
- `ActiveDlAi` and `ActiveUlAi`: Control whether scheduling decisions are delegated to the AI model. If `false`, the scheduler behaves like a normal QoS scheduler
- AI schedulers are designed to integrate with frameworks like ns3-ai to implement scheduling algorithms based on reinforcement learning or other ML approaches

### `phyLayer[0].error-model`
**Type:** `array`
**Description:** Array of error model configurations for NR radio channel simulation.

The error model simulates packet losses and transmission errors based on channel conditions (SINR, MCS, etc.). It is fundamental for a realistic simulation of system performance.

**Example of single configuration (DL and UL with same model):**

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

#### Error Model Parameters

| Parameter | Type | Values | Description |
|-----------|------|--------|-------------|
| `type` | string | ns-3 TypeId | Type of error model to use |
| `direction` | string | `both` \| `uplink` \| `downlink` | Direction to apply the model to (default: `both`) |
| `amcAttributes` | array | - | Array of specific attributes for AMC (Adaptive Modulation and Coding) |

**Values of `direction`:**
- `both`: Applies the same model to both downlink and uplink
- `downlink`: Applies the model only to downlink (DL)
- `uplink`: Applies the model only to uplink (UL)

#### AMC (Adaptive Modulation and Coding) Attributes

The `amcAttributes` configure the AMC module which determines the MCS (Modulation and Coding Scheme) based on the CQI:

| Attribute Name | Type | Range/Options | Initial Value | Description |
|----------------|------|---------------|-----------------|-------------|
| `NumRefScPerRb` | uint8_t | 0:12 | 1 | Number of subcarriers carrying Reference Signals per Resource Block |
| `AmcModel` | enum | ErrorModel \| ShannonModel | ErrorModel | AMC model used to assign CQI |

**Values of `AmcModel`:**
- `ErrorModel`: Uses the configured error model (e.g., NrEesmIrT1) to calculate CQI based on BLER tables
- `ShannonModel`: Uses the theoretical Shannon limit to calculate CQI (more optimistic, useful for theoretical analysis)

**Example with AMC attributes:**

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

**Note:** Both type and amcAttributes are optional: They can be used together or separately in separate objects in the array, depending on configuration needs.

#### Available Error Models

The error models available in 5g-lena are organized into two main branches:

##### NrEesmErrorModel Branch

Models based on EESM (Exponential Effective SINR Mapping) with HARQ support:

| Model Type | Description | HARQ Support |
|--------------|-------------|---------------|
| `ns3::NrEesmErrorModel` | Abstract base class for EESM models | - |
| `ns3::NrEesmCc` | Error model with Chase Combining (CC base class) | Chase Combining |
| `ns3::NrEesmCcT1` | EESM with Chase Combining and Type 1 tables (LDPC) | Chase Combining |
| `ns3::NrEesmCcT2` | EESM with Chase Combining and Type 2 tables (high modulations) | Chase Combining |
| `ns3::NrEesmIr` | Error model with Incremental Redundancy (IR base class) | Incremental Redundancy |
| `ns3::NrEesmIrT1` | EESM with Incremental Redundancy and Type 1 tables (LDPC) | Incremental Redundancy |
| `ns3::NrEesmIrT2` | EESM with Incremental Redundancy and Type 2 tables (high modulations) | Incremental Redundancy |

##### NrLteMiErrorModel Branch

Models based on Mutual Information, compatible with LTE:

| Model Type | Description |
|--------------|-------------|
| `ns3::NrLteMiErrorModel` | Base class for models based on Mutual Information |
| `ns3::LenaErrorModel` | Error model compatible with LENA (LTE framework) |



**Complete example with separate DL/UL configuration:**

To configure different models for downlink and uplink, add two elements to the `error-model` array with appropriate `direction`:

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

**Note:** It is possible to specify multiple `error-model` configurations with different directions in the same array. If multiple configurations with the same direction are specified, the last one will overwrite the previous ones.




### `phyLayer[0].epc`
**Description:** Configuration of the EPC (Evolved Packet Core).

In general, as helpers for the epc, we have available classes: `ns3::NrPointToPointEpcHelper`, `ns3::NrPointToPointEpcHelperBase`, `ns3::NrNoBackhaulEpcHelper`.

Note that `ns3::NrPointToPointEpcHelper` and `ns3::NrPointToPointEpcHelperBase` derive from `ns3::NrNoBackhaulEpcHelper` and therefore inherit all its attributes.

Example of EPC configuration:

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

Attributes available for `ns3::NrPointToPointEpcHelper` and `ns3::NrPointToPointEpcHelperBase` (in addition to those inherited from `ns3::NrNoBackhaulEpcHelper`):

| Name | Type | Initial Value | Description |
|------|------|----------------|-------------|
| `S1uLinkDataRate` | DataRate | 10000000000bps | The data rate to be used for the next S1-U link to be created |
| `S1uLinkDelay` | Time | +0ns | The delay to be used for the next S1-U link to be created |
| `S1uLinkMtu` | uint16_t (0:65535) | 2000 | The MTU of the next S1-U link to be created. Note: due to the additional overhead of GTP/UDP/IP tunneling, an MTU larger than the end-to-end MTU you want to support is required |
| `S1uLinkPcapPrefix` | string | "s1u" | Prefix for Pcap files generated by the S1-U link |
| `S1uLinkEnablePcap` | boolean | false | Enable Pcap file generation for the S1-U link |

Here are the attributes inherited from `ns3::NrNoBackhaulEpcHelper`:

| Name | Type | Initial Value | Description |
|------|------|----------------|-------------|
| `S5LinkDataRate` | DataRate | 10000000000bps | The data rate to be used for the next S5 link to be created |
| `S5LinkDelay` | Time | +0ns | The delay to be used for the next S5 link to be created |
| `S5LinkMtu` | uint16_t (0:65535) | 2000 | The MTU of the next S5 link to be created |
| `S11LinkDataRate` | DataRate | 10000000000bps | The data rate to be used for the next S11 link to be created |
| `S11LinkDelay` | Time | +0ns | The delay to be used for the next S11 link to be created |
| `S11LinkMtu` | uint16_t (0:65535) | 2000 | The MTU of the next S11 link to be created |
| `X2LinkDataRate` | DataRate | 10000000000bps | The data rate to be used for the next X2 link to be created |
| `X2LinkDelay` | Time | +0ns | The delay to be used for the next X2 link to be created |
| `X2LinkMtu` | uint16_t (0:65535) | 3000 | The MTU of the next X2 link to be created. Note: due to some large X2 messages, a large MTU is required |
| `X2LinkPcapPrefix` | string | "x2" | Prefix for Pcap files generated by the X2 link |
| `X2LinkEnablePcap` | boolean | false | Enable Pcap file generation for the X2 link |

Note that some Prefixes are already overwritten by default in IoD-Sim to avoid conflicts between the various generated links: replacing them again could compromise the generation of reporting files.

### `phyLayer[0].beamforming`
**Description:** Beamforming configuration.

In general, as helpers for beamforming, we have available classes: `ns3::IdealBeamformingHelper`, `ns3::RealisticBeamformingHelper`.
For these helpers, we do not have particular specific attributes, but we can choose the beamforming method (or algorithm) from `ns3::IdealBeamformingAlgorithm`, `ns3::DirectPathBeamforming`, `ns3::DirectPathQuasiOmniBeamforming`, and `ns3::QuasiOmniDirectPathBeamforming`.
Also for beamforming algorithms, we do not have particular specific attributes.

Example of beamforming configuration:

```json
"beamforming": {
  "helper": "ns3::IdealBeamformingHelper",
  "method": "ns3::DirectPathBeamforming",
  "algorithmAttributes": [],
  "attributes": []
}
```

### `phyLayer[0].ueAntenna` and `gnbAntenna`
**Description:** Antenna configuration for UE and gNB.

Example of UE antenna configuration:

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

In Type, we can insert any antenna present on ns-3, which will be inserted into an antenna array (as defined by 5g-lena), whose configuration parameters can be specified in arrayProperties.

Specific antenna properties (other than NumRows and NumColumns) can be specified in the `properties` array and depend on the selected antenna type.

In particular, the Antenna Array consists of a `UniformPlanarArray` of antennas of the type specified in `type` and can have these attributes (in arrayProperties):

| Name | Type | Initial Value | Range | Description |
|------|------|----------------|--------|-------------|
| `AntennaHorizontalSpacing` | double | 0.5 | 0:1.79769e+308 | Horizontal spacing between antenna elements, in multiples of wavelength |
| `AntennaVerticalSpacing` | double | 0.5 | 0:1.79769e+308 | Vertical spacing between antenna elements, in multiples of wavelength |
| `BearingAngle` | double | 0 | -π:π | Orientation angle in radians |
| `DowntiltAngle` | double | 0 | -π:π | Downtilt angle in radians |
| `IsDualPolarized` | boolean | false | - | If true, dual-polarized antenna |
| `NumColumns` | uint32_t | 4 | 1:4294967295 | Horizontal dimension of the array |
| `NumHorizontalPorts` | uint32_t | 1 | 0:4294967295 | Number of horizontal ports |
| `NumRows` | uint32_t | 4 | 1:4294967295 | Vertical dimension of the array |
| `NumVerticalPorts` | uint32_t | 1 | 0:4294967295 | Number of vertical ports |
| `PolSlantAngle` | double | 0 | -π:π | Polarization slant angle in radians |

while below are the available antenna types with relative parameters:

#### ns3::IsotropicAntennaModel

Isotropic antenna that radiates uniformly in all directions. It is the simplest to configure.

| Name | Type | Initial Value | Range | Description |
|------|------|----------------|--------|-------------|
| `Gain` | double | 0 | -∞:+∞ | Antenna gain in dB |

#### ns3::CosineAntennaModel

Antenna with cosine radiation pattern, useful for modeling directional antennas with defined main lobes.

| Name | Type | Initial Value | Range | Description |
|------|------|----------------|--------|-------------|
| `HorizontalBeamwidth` | double | 120 | 0:360 | Horizontal beamwidth at 3 dB (degrees). 360° = constant gain |
| `VerticalBeamwidth` | double | 360 | 0:360 | Vertical beamwidth at 3 dB (degrees). 360° = constant gain |
| `MaxGain` | double | 0 | -∞:+∞ | Maximum gain (dB) in the pointing direction |
| `Orientation` | double | 0 | -360:360 | Antenna orientation on the x-y plane with respect to the x-axis (degrees) |


#### ns3::ParabolicAntennaModel

Parabolic antenna with directional radiation pattern, ideal for high-directivity point-to-point communications.

| Name | Type | Initial Value | Range | Description |
|------|------|----------------|--------|-------------|
| `Beamwidth` | double | 60 | 0:180 | Beamwidth at 3 dB (degrees) |
| `MaxAttenuation` | double | 20 | -∞:+∞ | Maximum attenuation (dB) of the radiation pattern |
| `Orientation` | double | 0 | -360:360 | Antenna orientation on the x-y plane with respect to the x-axis (degrees) |


#### ns3::CircularApertureAntennaModel

Circular aperture antenna that simulates the behavior of reflector antennas or arrays with defined physical aperture.

| Name | Type | Initial Value | Range | Description |
|------|------|----------------|--------|-------------|
| `AntennaCircularApertureRadius` | double | 0.5 | ≥0 | Antenna aperture radius (meters) |
| `OperatingFrequency` | double | 2e9 | >0 | Antenna operating frequency (Hz) |
| `AntennaMinGainDb` | double | -100.0 | -∞:+∞ | Minimum antenna gain (dB) |
| `AntennaMaxGainDb` | double | 1.0 | ≥0 | Maximum antenna gain (dB) |
| `ForceGainBounds` | boolean | true | - | Forces GetGainDb into the interval [AntennaMinGainDb, AntennaMaxGainDb] |

#### ns3::ThreeGppAntennaModel

Antenna based on a parabolic approximation of the main lobe radiation pattern, compliant with 3GPP standards.

| Name | Type | Initial Value | Description |
|------|------|----------------|-------------|
| `RadiationPattern` | string | "Outdoor" | 3GPP antenna radiation pattern |

**Possible values for RadiationPattern:**
- `Outdoor` - Pattern for outdoor environments
- `Indoor` - Pattern for indoor environments

### `gnbBwpManager` and `ueBwpManager`
**Description:** Bandwidth Part (BWP) Manager configuration for gNB and UE.

It is possible to specify the BWP Manager Algorithm type with the "type" field, although currently only `ns3::BwpManagerAlgorithmStatic` is available, and related attributes such as:

**Attributes of `ns3::BwpManagerAlgorithmStatic`:**

| Attribute Name | Type | Range | Initial Value | Description |
|----------------|------|-------|-----------------|-------------|
| `GBR_CONV_VOICE` | uint8_t | 0:5 | 0 | BWP index for GBR_CONV_VOICE flows (conversational voice) |
| `GBR_CONV_VIDEO` | uint8_t | 0:5 | 0 | BWP index for GBR_CONV_VIDEO flows (conversational video) |
| `GBR_GAMING` | uint8_t | 0:5 | 0 | BWP index for GBR_GAMING flows (gaming) |
| `GBR_NON_CONV_VIDEO` | uint8_t | 0:5 | 0 | BWP index for GBR_NON_CONV_VIDEO flows (non-conversational video) |
| `GBR_MC_PUSH_TO_TALK` | uint8_t | 0:5 | 0 | BWP index for GBR_MC_PUSH_TO_TALK flows (mission critical push-to-talk) |
| `GBR_NMC_PUSH_TO_TALK` | uint8_t | 0:5 | 0 | BWP index for GBR_NMC_PUSH_TO_TALK flows (non-mission critical push-to-talk) |
| `GBR_MC_VIDEO` | uint8_t | 0:5 | 0 | BWP index for GBR_MC_VIDEO flows (mission critical video) |
| `GBR_V2X` | uint8_t | 0:5 | 0 | BWP index for GBR_V2X flows (Vehicle-to-Everything) |
| `NGBR_IMS` | uint8_t | 0:5 | 0 | BWP index for NGBR_IMS flows (IP Multimedia Subsystem) |
| `NGBR_VIDEO_TCP_OPERATOR` | uint8_t | 0:5 | 0 | BWP index for NGBR_VIDEO_TCP_OPERATOR flows (operator TCP video) |
| `NGBR_VOICE_VIDEO_GAMING` | uint8_t | 0:5 | 0 | BWP index for NGBR_VOICE_VIDEO_GAMING flows (voice/video/gaming) |
| `NGBR_VIDEO_TCP_PREMIUM` | uint8_t | 0:5 | 0 | BWP index for NGBR_VIDEO_TCP_PREMIUM flows (premium TCP video) |
| `NGBR_VIDEO_TCP_DEFAULT` | uint8_t | 0:5 | 0 | BWP index for NGBR_VIDEO_TCP_DEFAULT flows (default TCP video) |
| `NGBR_MC_DELAY_SIGNAL` | uint8_t | 0:5 | 0 | BWP index for NGBR_MC_DELAY_SIGNAL flows (delay-sensitive MC signaling) |
| `NGBR_MC_DATA` | uint8_t | 0:5 | 0 | BWP index for NGBR_MC_DATA flows (mission critical data) |
| `NGBR_V2X` | uint8_t | 0:5 | 0 | BWP index for NGBR_V2X flows (non-GBR Vehicle-to-Everything) |
| `NGBR_LOW_LAT_EMBB` | uint8_t | 0:5 | 0 | BWP index for NGBR_LOW_LAT_EMBB flows (low latency eMBB) |
| `DGBR_DISCRETE_AUT_SMALL` | uint8_t | 0:5 | 0 | BWP index for DGBR_DISCRETE_AUT_SMALL flows (discrete automation small) |
| `DGBR_DISCRETE_AUT_LARGE` | uint8_t | 0:5 | 0 | BWP index for DGBR_DISCRETE_AUT_LARGE flows (discrete automation large) |
| `DGBR_ITS` | uint8_t | 0:5 | 0 | BWP index for DGBR_ITS flows (Intelligent Transport Systems) |
| `DGBR_ELECTRICITY` | uint8_t | 0:5 | 0 | BWP index for DGBR_ELECTRICITY flows (electricity distribution) |
| `GBR_LIVE_UL_71` | uint8_t | 0:5 | 0 | BWP index for GBR_LIVE_UL_71 flows (live uplink 71) |
| `GBR_LIVE_UL_72` | uint8_t | 0:5 | 0 | BWP index for GBR_LIVE_UL_72 flows (live uplink 72) |
| `GBR_LIVE_UL_73` | uint8_t | 0:5 | 0 | BWP index for GBR_LIVE_UL_73 flows (live uplink 73) |
| `GBR_LIVE_UL_74` | uint8_t | 0:5 | 0 | BWP index for GBR_LIVE_UL_74 flows (live uplink 74) |
| `GBR_LIVE_UL_76` | uint8_t | 0:5 | 0 | BWP index for GBR_LIVE_UL_76 flows (live uplink 76) |
| `DGBR_INTER_SERV_87` | uint8_t | 0:5 | 0 | BWP index for DGBR_INTER_SERV_87 flows (interactive services 87) |
| `DGBR_INTER_SERV_88` | uint8_t | 0:5 | 0 | BWP index for DGBR_INTER_SERV_88 flows (interactive services 88) |
| `DGBR_VISUAL_CONTENT_89` | uint8_t | 0:5 | 0 | BWP index for DGBR_VISUAL_CONTENT_89 flows (visual content 89) |
| `DGBR_VISUAL_CONTENT_90` | uint8_t | 0:5 | 0 | BWP index for DGBR_VISUAL_CONTENT_90 flows (visual content 90) |

**Note:** Each attribute specifies the index of the Bandwidth Part (BWP) to which flows of the respective QCI (Quality of Service Class Identifier) type should be forwarded. The value must be between 0 and 5, corresponding to the configured BWP index.

Example of BWP Manager configuration:

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

### `phyLayer[0].handover`
**Description:** Handover algorithm configuration.

It is possible to specify the handover algorithm type and its attributes.

**Example:**
```json
"handover": {
  "algorithm": "ns3::NrNoOpHandoverAlgorithm",
  "attributes": []
}
```

### `phyLayer[0].channelAccessManager`
**Description:** Channel Access Manager configuration for UE and gNB.

It is possible to specify the channel access manager type and its attributes for both UE and gNB.

**Example:**
```json
"channelAccessManager": {
  "ue": {
    "type": "ns3::NrAlwaysOnAccessManager",
    "attributes": []
  },
  "gnb": {
    "type": "ns3::NrAlwaysOnAccessManager",
    "attributes": []
  }
}
```

### `phyLayer[0].uePhyAttributes` and `gnbPhyAttributes`
**Description:** Physical attributes specific to UE and gNB.

Attributes available for `uePhyAttributes`:

| Name | Type | Range | Initial Value | Description |
|------|------|-------|-----------------|-------------|
| `TxPower` | double | -1.79769e+308:1.79769e+308 | 2 | Transmission power in dBm |
| `NoiseFigure` | double | -1.79769e+308:1.79769e+308 | 5 | Loss (dB) in signal-to-noise ratio due to non-idealities in the receiver |
| `PowerAllocationType` | enum | UniformPowerAllocBw\|UniformPowerAllocUsed | UniformPowerAllocUsed | Type of power allocation on RBs |
| `LBTThresholdForCtrl` | Time | -9.22337e+18ns:+9.22337e+18ns | +25000ns | Threshold to consider the channel free for control transmission |
| `TbDecodeLatency` | Time | -9.22337e+18ns:+9.22337e+18ns | +100000ns | Transport block decoding latency |
| `EnableUplinkPowerControl` | boolean | - | false | Enables uplink power control |
| `WbPmiUpdateInterval` | Time | -9.22337e+18ns:+9.22337e+18ns | +1e+07ns | Wideband PMI update interval |
| `SbPmiUpdateInterval` | Time | -9.22337e+18ns:+9.22337e+18ns | +2e+06ns | Subband PMI update interval |
| `AlphaCovMat` | double | 0:1 | 1 | Alpha parameter for interference covariance matrix calculation |
| `CsiImDuration` | uint8_t | 1:12 | 1 | CSI-IM duration in number of OFDM symbols |
| `UeMeasurementsFilterPeriod` | Time | -9.22337e+18ns:+9.22337e+18ns | +2e+08ns | Filtering period for UE measurements |
| `EnableRlfDetection` | boolean | - | true | Enables RLF (Radio Link Failure) detection |

Attributes available for `gnbPhyAttributes`:

| Name | Type | Range | Initial Value | Description |
|------|------|-------|-----------------|-------------|
| `RbOverhead` | double | 0:0.5 | 0.04 | Overhead in calculating the number of usable RBs |
| `TxPower` | double | -1.79769e+308:1.79769e+308 | 4 | Transmission power in dBm |
| `NoiseFigure` | double | -1.79769e+308:1.79769e+308 | 5 | Loss (dB) in signal-to-noise ratio due to non-idealities in the receiver |
| `PowerAllocationType` | enum | UniformPowerAllocBw\|UniformPowerAllocUsed | UniformPowerAllocUsed | Type of power allocation: uniform over the entire band or on used RBs |
| `N0Delay` | uint32_t | 0:1 | 0 | Minimum processing delay to decode DL DCI and DL data |
| `N1Delay` | uint32_t | 0:4 | 2 | Minimum delay (UE side) from end of DL data reception to start of ACK/NACK transmission |
| `N2Delay` | uint32_t | 0:4 | 2 | Minimum processing delay to decode UL DCI and prepare UL data |
| `TbDecodeLatency` | Time | -9.22337e+18ns:+9.22337e+18ns | +100000ns | Transport block decoding latency |
| `Numerology` | uint16_t | 0:65535 | 0 | The 3GPP numerology to use |
| `SymbolsPerSlot` | uint16_t | 0:65535 | 14 | Number of symbols in a slot |
| `Pattern` | string | - | F\|F\|F\|F\|F\|F\|F\|F\|F\|F\| | Slot pattern |
| `CsiRsModel` | enum | CsiRsPerUe\|CsiRsPerBeam | CsiRsPerUe | Type of CSI-RS model: per UE or per beam |
| `CsiRsPeriodicity` | uint16_t | 0:65535 | 10 | Default CSI periodicity in number of slots |

**Note:** From the `Pattern` attribute we can define the slot pattern in terms of Downlink (D), Uplink (U), and Flexible (F). For example, a pattern of "DDFU" indicates that the first two slots are Downlink, the third is Flexible, and the fourth is Uplink.

**Example:**
```json
"uePhyAttributes": [
  {"name": "TxPower", "value": 23.0},
  {"name": "EnableUplinkPowerControl", "value": false}
]
```

### Complete NR PHY Layer Configuration Example
Here is a complete example of NR physical layer configuration in JSON format:
```json
"phyLayer": [
   {
      "type": "nr",
      "channels": [
        {
          "scenario": "NTN-Suburban",
          "conditionModel": "Default",
          "propagationModel": "ThreeGpp",
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
          "bands": [
            {
              "type": "contiguous",
              "carrier": {
                "centralFrequency": 28e9,
                "bandwidth": 10e6,
                "numComponentCarriers": 1
              }
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

## NR MAC and Network Layer

NR includes the management of the MAC and network layer, aggregated by 1 single module, which is already configured with the physical layer, moreover the IPs used are hardcoded within the NS3 code therefore they are not configurable via JSON.

It is therefore sufficient to add the following configuration inside the NR network device to enable the MAC and network layer:
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

Even if you try to set the network layer to addresses other than 7.0.0.0/8, these will be ignored and overwritten with the hardcoded addresses of 5g-lena.

## UE and gNB Configuration in IoD-Sim with NR

When we go to configure the various network devices, it is possible to configure the NR netdevice through the following parameters as in the example below.

```json
"netDevices": [
  {
    "type": "nr",
    "networkLayer": 0, // ID of the nr layer
    "role": "UE", // Role: UE or gNB
    "channelId": 0, // Optional: Channel ID to use (default: 0)
    "channelBands": [0, 1], // Optional: List of band indices to use (default: all)
    "bearers": [
      {
        "type": "NGBR_LOW_LAT_EMBB"
      }
    ],
    "antennaModel": {
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
      ],
      "properties": [
        {
          "name": "Gain",
          "value": 10
        }
      ]
    },
    "phy": [
      {
        "bwpId": 0, // Optional, if not specified, this is applied to all BWPs
        "name": "TxPower",
        "value": 23.0
      }
    ],
    "directivity": {
        "mode": "nearest-gnb"
    }
  }
]
```

As you can see, it is possible to specify the type of bearer (flow) you want to use among those supported by 5g-lena.

It is possible to specify attributes in the physical layer specific to that NR netdevice: the attribute is applied to a particular BWP if specified, otherwise to all BWPs of the device. The supported attributes are the same as those listed in the previous section for `uePhyAttributes` and `gnbPhyAttributes`.

Furthermore, using the `antennaModel` parameter, it is possible to specify the antenna model of the device with the same kind of attributes as the `ueAntenna` and `gnbAntenna` parameters.

### `netDevices[i].channelId`
**Type:** `uint32_t`
**Default:** `0`
**Description:** The ID of the channel to which the device will tune. This corresponds to the index of the channel configuration in the `bands` array.

### `netDevices[i].channelBands`
**Type:** `array[uint32_t]`
**Default:** `[]` (all bands)
**Description:** A list of indices of the bands within the selected channel that the device should use. If empty or not specified, the device will be configured to use all available bands in the channel.

### `netDevices[i].directivity`
**Description:** Configuration of the initial antenna pointing direction.
- `mode`: Directivity mode string. Options:
  - `nearest-gnb`: Points towards the nearest gNB (only for UE).
  - `serving-gnb`: Points towards the currently connected gNB (only for UE).
  - `nearest-ue`: Points towards the nearest UE (only for gNB).
  - `earth-centered`: Points towards the center of the Earth (0,0,0).
  - `point`: Points towards a specific fixed coordinate.
  - `node`: Points towards a specific node in the simulation at each instant.
- `key`: (Only for `mode="node"`) The type of node target. Options: `leo-sats`, `nodes`, `vehicles`, `drones`, `zsps`, `remote-nodes`, `backbone`.
- `index`: (Only for `mode="node"`) The index of the node in the container (0, 1, ...).
- `coordinates`: Coordinate system for `point` mode (default: `geocentric`, optional `geographic`).
- `position`: Coordinates [x, y, z] for `point` mode.
- `precision`: Update interval for directivity (e.g., "100ms").

**Example (Node Mode):**
```json
"directivity": {
  "mode": "node",
  "key": "leo-sats",
  "index": 0,
  "precision": "100ms"
}
```

**Properties:**

| Name | Type | Options | Description |
|------|------|---------|-------------|
| `mode` | string | `nearest-gnb`, `serving-gnb`, `nearest-ue`, `earth-centered`, `point` | The directivity mode. |
| `coordinates` | string | `geocentric`, `geographic` | Coordinate system for `point` mode (optional). |
| `position` | array | `[x, y, z]` | Target position for `point` mode (optional). |
| `precision` | string | - | The update interval (e.g., "100ms", "1s"). Default: "100ms". |

**Modes:**
- `serving-gnb`: The antenna points to the currently connected (serving) gNB. If the UE is not connected or the serving gNB cannot be found, it falls back to `nearest-gnb` behavior. This mode is specific to `NR` UEs and ensures beam alignment after handovers.
- `nearest-gnb`: The antenna constantly points to the nearest gNB (considering only gNBs of the same `networkLayer` ID). This mode is specific to `NR` devices.
- `nearest-ue`: The antenna constantly points to the nearest UE (considering only UEs of the same `networkLayer` ID). This mode is specific to `NR` devices.
- `earth-centered`: The antenna points towards the center of the earth (0, 0, 0).
- `point`: The antenna points towards a fixed point specified by `position` and `coordinates`.

### `netDevices[i].outputLinks`
**Description:** Configuration of output links for BWP routing.

This optional parameter allows defining a mapping for routing control messages between Bandwidth Parts (BWPs). It is useful when control messages generated in one BWP need to be transmitted on another BWP.

**Example:**
```json
"outputLinks": [
  {
    "sourceBwp": 0,
    "targetBwp": 1
  }
]
```

**Properties:**

| Name | Type | Description |
|------|------|-------------|
| `sourceBwp` | uint32_t | The ID of the source BWP from which messages originate. |
| `targetBwp` | uint32_t | The ID of the target BWP to which messages are routed. |

**Note:** This configuration applies to both gNB and UE devices. It utilizes the `SetOutputLink` method of the `BwpManager`.

---
### `netDevices[i].rrcAttributes`
**Description:** List of attributes to configure on the RRC layer of the NetDevice.

**Example:**
```json
"rrcAttributes": [
  {
    "name": "AdmitHandoverRequest",
    "value": "true"
  }
]
```

**Properties:**

| Name | Type | Description |
|------|------|-------------|
| `name` | string | The name of the RRC attribute to set. |
| `value` | string | The value of the attribute. |

---
### `netDevices[i].x2Links`
**Description:** List of other gNB nodes to establish an X2 interface with. This is useful for configuring neighbor relations for handovers and coordination.

**Example:**
```json
"x2Links": [
  {
    "key": "leo-sats",
    "index": 1
  },
  {
    "key": "nodes",
    "index": 0
  }
]
```

**Properties:**

| Name | Type | Description |
|------|------|-------------|
| `key` | string | The key of the node container (e.g., "leo-sats", "nodes", "vehicles"). |
| `index` | uint32_t | The index of the node within the specified container. |

---

## Vehicles on Earth

### Basic Structure
```json
"vehicles": [
  {
    "netDevices": [...],
    "mobilityModel": {...},
    "applications": [...]
  }
]
```
Vehicles in general act like normal nodes, therefore they support all parameters, but are tracked separately for convenience of use and to log changes in their positions separately.

In particular, however, we focus the detail on the constant velocity geocentric model created specifically for ground vehicles:

### `vehicles[n].mobilityModel`
**Description:** Mobility model of the vehicle on earth.

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

| Parameter | Type | Description |
|-----------|------|-------------|
| `InitialLatitude` | number | Initial latitude in degrees (-90 to +90) |
| `InitialLongitude` | number | Initial longitude in degrees (-180 to +180) |
| `Altitude` | number | Altitude in meters (0 for surface) |
| `Speed` | number | Speed in m/s |
| `Azimuth` | number | Direction of movement in degrees (0-360) |

---

## LEO Satellites

### Basic Structure
```json
"leo-sats": [
  {
    "netDevices": [...],
    "mobilityModel": {...},
    "applications": []
  }
]
```

Similarly, LEO satellites are also nodes with all standard parameters, but their orbital mobility is tracked separately.
Furthermore, also in this case we see specifically the geocentric mobility model for LEO satellites:


### `leo-sats[n].mobilityModel`
**Description:** LEO orbital mobility model.

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

| Parameter | Type | Description |
|-----------|------|-------------|
| `Altitude` | number | Orbital altitude in km |
| `Inclination` | number | Orbital inclination in degrees (0-180) |
| `Longitude` | number | Longitude of the ascending node in degrees |
| `Offset` | number | Orbital offset (phase offset) in degrees |
| `RetrogradeOrbit` | boolean | If `true`, retrograde orbit |

---

## Configuration Examples

### Example 1: Short simulation with 2 satellites and 1 car
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

---

## NR Radio Environment Map

It is possible to generate Radio Environment Maps (REM) for the NR scenarios to visualize the SINR coverage. To enable the generation you need to specify the `--radioMaps` flag in the command line of the scenario executable.

### `radioMaps`
**Type**: `array[object]`
**Description**: List of Radio Map configurations. It allows generating multiple maps for different PHY layers (lte or nr).

Each object in the list contains:
- `phyLayerId` (integer): ID of the PHY layer (usually 0).
- `bwpId` (integer): ID of the Bandwidth Part to analyze (only nr).
- `parameters` (object): Key-value pairs matching the `NrRadioEnvironmentMapHelper` attributes.

**Common Parameters:**
- `XMin`, `XMax`, `YMin`, `YMax`: Coordinates of the map boundaries (in meters).
- `Z`: Altitude of the map plane (in meters).
- `XRes`, `YRes`: Resolution (number of points) along X and Y axes.

**Example:**
```json
"radioMaps": [
  {
    "phyLayerId": 0,
    "bwpId": 0,
    "parameters": {
      "XMin": "0",
      "XMax": "1000",
      "YMin": "0",
      "YMax": "1000",
      "Z": "1.5",
      "XRes": "50",
      "YRes": "50"
    }
  }
]
```

#### Geocentric Radio Map

For LEO satellite scenarios, it is often necessary to generate a global radio map using geocentric coordinates (latitude/longitude). This can be achieved by specifying the `coordinates` parameter and configuring the map boundaries in degrees.

**Additional Parameters:**
- `coordinates` (string): Set to `"geocentric"` to enable geocentric mode.
- `logGeocentricREM` (boolean): If `true`, the geocentric REM (ECEF coordinates) will be saved to a file.

**Geocentric Parameters:**
- `MinLon`, `MaxLon`: Longitude range in degrees.
- `MinLat`, `MaxLat`: Latitude range in degrees.
- `Altitude`: Altitude in meters from the earth surface.
- `XRes`, `YRes`: Resolution (number of points) along Longitude and Latitude.

**Example:**
```json
"radioMaps": [
  {
    "phyLayerId": 0,
    "bwpId": 0,
    "coordinates": "geocentric",
    "logGeocentricREM": true,
    "parameters": {
      "MinLon": "-180.0",
      "MaxLon": "180.0",
      "MinLat": "-90.0",
      "MaxLat": "90.0",
      "Altitude": "2.0",
      "XRes": "100",
      "YRes": "50",
      "RemMode": "CoverageArea",
      "InstallationDelay": "0.1s"
    }
  }
]
```
