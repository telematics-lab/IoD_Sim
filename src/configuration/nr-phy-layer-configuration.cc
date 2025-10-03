/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2023 The IoD_Sim Authors.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "nr-phy-layer-configuration.h"

#include <ns3/abort.h>
#include <ns3/cc-bwp-helper.h>
#include <ns3/double.h>
#include <ns3/ideal-beamforming-algorithm.h>
#include <ns3/ideal-beamforming-helper.h>
#include <ns3/isotropic-antenna-model.h>
#include <ns3/net-device-container.h>
#include <ns3/node-container.h>
#include <ns3/nr-channel-helper.h>
#include <ns3/nr-helper.h>
#include <ns3/nr-mac-scheduler-tdma-rr.h>
#include <ns3/nr-module.h>
#include <ns3/nr-point-to-point-epc-helper.h>
#include <ns3/object-factory.h>
#include <ns3/pointer.h>
#include <ns3/type-id.h>
#include <ns3/uinteger.h>

#include <functional>

/*
WHAT IS NEEDED IN THIS CLASS:

-

- OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc(bandConf); // also
non-contiguous with ns3::CcBwpCreator::SimpleOperationBandConf CcBwpCreator::SimpleOperationBandConf
bandConf(frequency, bandwidth, numCcPerBand); //Implement only

-    Ptr<NrChannelHelper> channelHelper = CreateObject<NrChannelHelper>();
// Set and configure the channel to the current band
channelHelper->ConfigureFactories(scenario, "Default", "ThreeGpp");
channelHelper->AssignChannelsToBands({band});

- // Configure ideal beamforming method
idealBeamformingHelper->SetAttribute("BeamformingMethod",
TypeIdValue(DirectPathBeamforming::GetTypeId()));

- // Configure scheduler
nrHelper->SetSchedulerTypeId(NrMacSchedulerTdmaRR::GetTypeId());

- Ue and gNB antennas
    nrHelper->SetUeAntennaAttribute("NumRows", UintegerValue(2));
    nrHelper->SetUeAntennaAttribute("NumColumns", UintegerValue(4));
    nrHelper->SetGnbAntennaAttribute("NumRows", UintegerValue(8));
    nrHelper->SetGnbAntennaAttribute("NumColumns", UintegerValue(8));
nrHelper->SetUeAntennaAttribute(
    "AntennaElement",
    PointerValue(CreateObjectWithAttributes<IsotropicAntennaModel>("Gain",
                                                                   DoubleValue(ueAntennaGainDb))));
nrHelper->SetGnbAntennaAttribute(
    "AntennaElement",
    PointerValue(CreateObjectWithAttributes<IsotropicAntennaModel>("Gain",
                                                                   DoubleValue(satAntennaGainDb))));

- Install
    NetDeviceContainer gnbNetDev = nrHelper->InstallGnbDevice(satellites, allBwps);
    NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice(cars, allBwps);

- Tx Power (possibly before here)
 nrHelper->GetGnbPhy(gnbNetDev.Get(i), 0)->SetTxPower(satTxPower);
 nrHelper->GetUePhy(ueNetDev.Get(i), 0)->SetTxPower(ueTxPower);


 - Attach nrHelper->AttachToClosestGnb(ueNetDev, gnbNetDev);


 - Logs? nrHelper->EnableTraces();
*/

namespace ns3
{

NrPhyLayerConfiguration::NrPhyLayerConfiguration(
    std::string phyType,
    std::vector<ModelConfiguration::Attribute> attributes,
    std::string channelScenario,
    std::string ueAntennaType,
    std::vector<ModelConfiguration::Attribute> ueAntennaProps,
    std::vector<ModelConfiguration::Attribute> ueAntennaArrayProps,
    std::string gnbAntennaType,
    std::vector<ModelConfiguration::Attribute> gnbAntennaProps,
    std::vector<ModelConfiguration::Attribute> gnbAntennaArrayProps,
    std::string channelCondition,
    std::string channelModel,
    std::string beamformingType,
    std::string schedulerType,
    uint8_t channelConfigFlags,
    bool contiguousCc,
    std::vector<std::vector<CcBwpCreator::SimpleOperationBandConf>> bandConfs)
    : PhyLayerConfiguration{phyType, attributes}
{
    // Set default values if not provided
    if (channelConfigFlags == 0)
    {
        channelConfigFlags = NrChannelHelper::INIT_PROPAGATION | NrChannelHelper::INIT_FADING;
    }

    if (bandConfs.empty())
    {
        bandConfs = {{CcBwpCreator::SimpleOperationBandConf(28e9, 100e6, 1)}};
    }

    if (ueAntennaProps.empty())
    {
        ueAntennaProps = {{"NumRows", Create<UintegerValue>(2)},
                          {"NumColumns", Create<UintegerValue>(4)}};
    }

    if (gnbAntennaProps.empty())
    {
        gnbAntennaProps = {{"NumRows", Create<UintegerValue>(8)},
                           {"NumColumns", Create<UintegerValue>(8)}};
    }

    NS_ASSERT_MSG(bandConfs.size() > 0, "At least one band configuration must be provided");

    Ptr<NrPointToPointEpcHelper> nrEpcHelper = CreateObject<NrPointToPointEpcHelper>();
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    Ptr<NrChannelHelper> channelHelper = CreateObject<NrChannelHelper>();
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();

    // Configure beamforming
    idealBeamformingHelper->SetAttribute("BeamformingMethod",
                                         TypeIdValue(IdealBeamformingAlgorithm::GetTypeId()));

    nrHelper->SetBeamformingHelper(idealBeamformingHelper);
    nrHelper->SetEpcHelper(nrEpcHelper);

    // Configure scheduler
    nrHelper->SetSchedulerTypeId(NrMacSchedulerTdmaRR::GetTypeId());

    BandwidthPartInfoPtrVector allBwps;
    CcBwpCreator ccBwpCreator;

    // Configure channel
    channelHelper->ConfigureFactories(channelScenario, channelCondition, channelModel);

    // Create bands from configuration
    std::vector<OperationBandInfo> bands;

    for (const auto& bandConf : bandConfs)
    {
        if (contiguousCc)
        {
            NS_ASSERT_MSG(bandConf.size() == 1,
                          "When using contiguous CC, only one band configuration must be provided");
            bands.push_back(ccBwpCreator.CreateOperationBandContiguousCc(bandConf[0]));
        }
        else
        {
            NS_ASSERT_MSG(
                bandConf.size() > 0,
                "When using non-contiguous CC, at least one band configuration must be provided");
            bands.push_back(ccBwpCreator.CreateOperationBandNonContiguousCc(bandConf));
        }
    }

    // Create reference wrapper vector for NR API
    std::vector<std::reference_wrapper<OperationBandInfo>> band_refs;
    band_refs.reserve(bands.size());
    for (OperationBandInfo& band : bands)
    {
        band_refs.push_back(std::ref(band));
    }

    channelHelper->AssignChannelsToBands(band_refs, channelConfigFlags);
    allBwps = CcBwpCreator::GetAllBwps(band_refs);

    // antenna array properties setup
    for (const auto& attr : ueAntennaArrayProps)
    {
        nrHelper->SetUeAntennaAttribute(attr.name, *attr.value);
    }
    for (const auto& attr : gnbAntennaArrayProps)
    {
        nrHelper->SetGnbAntennaAttribute(attr.name, *attr.value);
    }

    // Ue Antenna setup
    ObjectFactory ueAntennaFactory;
    ueAntennaFactory.SetTypeId(TypeId::LookupByName(ueAntennaType));
    for (const auto& attr : ueAntennaProps)
    {
        ueAntennaFactory.Set(attr.name, *attr.value);
    }
    Ptr<Object> ueAntenna = ueAntennaFactory.Create();
    nrHelper->SetUeAntennaAttribute("AntennaElement", PointerValue(ueAntenna));

    // gNB Antenna setup
    ObjectFactory gnbAntennaFactory;
    gnbAntennaFactory.SetTypeId(TypeId::LookupByName(gnbAntennaType));
    for (const auto& attr : gnbAntennaProps)
    {
        gnbAntennaFactory.Set(attr.name, *attr.value);
    }
    Ptr<Object> gnbAntenna = gnbAntennaFactory.Create();
    nrHelper->SetGnbAntennaAttribute("AntennaElement", PointerValue(gnbAntenna));

    // TODO: Tx power is settable via NrGnbPhy::TxPower and NrUePhy::TxPower attributes
    // TODO: Consider calling AttachToClosestGnb every CourseChange

    // ueIpIface = nrEpcHelper->AssignUeIpv4Address(NetDeviceContainer(ueNetDev));
    // IPs are hardcoded on 5g-lena ...........
    // nrEpcHelper->SetupRemoteHost() ?????? how manage
}

NrPhyLayerConfiguration::~NrPhyLayerConfiguration()
{
}

std::string
NrPhyLayerConfiguration::GetAttribute(const std::string& name) const
{
    // Return default beamforming method for now
    if (name == "BeamformingMethod")
    {
        return "ns3::IdealBeamformingAlgorithm";
    }
    return "";
}

std::shared_ptr<ModelConfiguration>
NrPhyLayerConfiguration::GetChannelPropagationLossModel() const
{
    // Return nullptr for now - not implemented
    return nullptr;
}

std::shared_ptr<ModelConfiguration>
NrPhyLayerConfiguration::GetChannelSpectrumModel() const
{
    // Return nullptr for now - not implemented
    return nullptr;
}

} // namespace ns3
