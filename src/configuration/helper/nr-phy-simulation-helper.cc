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
#include "nr-phy-simulation-helper.h"

#include "scenario-configuration-helper.h"

#include <ns3/boolean.h>
#include <ns3/cc-bwp-helper.h>
#include <ns3/net-device-container.h>
#include <ns3/node-container.h>
#include <ns3/nr-channel-helper.h>
#include <ns3/object-factory.h>
#include <ns3/pointer.h>
#include <ns3/string.h>
#include <ns3/type-id.h>

namespace ns3
{

class NrPhySimulationHelperPriv
{
  public:
    static std::string GetS1uLinkPcapPrefix(const size_t stackId)
    {
        std::stringstream prefixBuilder;
        prefixBuilder << CONFIGURATOR->GetResultsPath() << "nr-" << stackId << "-s1u";

        return prefixBuilder.str();
    }

    static std::string GetX2LinkPcapPrefix(const size_t stackId)
    {
        std::stringstream prefixBuilder;
        prefixBuilder << CONFIGURATOR->GetResultsPath() << "nr-" << stackId << "-x2";

        return prefixBuilder.str();
    }
};

NrPhySimulationHelper::NrPhySimulationHelper(const size_t stackId)
    : m_nr{CreateObject<NrHelper>()}
{
    m_stackId = stackId;
}

void
NrPhySimulationHelper::SetEpcHelper(TypeId epc,
                                    std::vector<ModelConfiguration::Attribute> attributes)
{
    ObjectFactory factory;
    factory.SetTypeId(epc);
    factory.Set("S1uLinkEnablePcap", BooleanValue(CONFIGURATOR->GetLogOnFile()));
    factory.Set("S1uLinkPcapPrefix",
                StringValue(NrPhySimulationHelperPriv::GetS1uLinkPcapPrefix(m_stackId)));
    factory.Set("X2LinkEnablePcap", BooleanValue(CONFIGURATOR->GetLogOnFile()));
    factory.Set("X2LinkPcapPrefix",
                StringValue(NrPhySimulationHelperPriv::GetX2LinkPcapPrefix(m_stackId)));
    m_nr_epc = factory.Create<NrEpcHelper>();
    m_nr->SetEpcHelper(m_nr_epc);

    for (const auto& attr : attributes)
    {
        m_nr_epc->SetAttribute(attr.name, *attr.value);
    }
}

void
NrPhySimulationHelper::SetBeamformingHelper(TypeId beam,
                                            std::vector<ModelConfiguration::Attribute> attributes)
{
    ObjectFactory factory;
    factory.SetTypeId(beam);
    m_beamHelper = factory.Create<BeamformingHelperBase>();
    m_nr->SetBeamformingHelper(m_beamHelper);

    for (const auto& attr : attributes)
    {
        m_beamHelper->SetAttribute(attr.name, *attr.value);
    }
}

NrPhySimulationHelper::~NrPhySimulationHelper()
{
}

Ptr<NrHelper>
NrPhySimulationHelper::GetNrHelper()
{
    return m_nr;
}

Ptr<NrEpcHelper>
NrPhySimulationHelper::GetNrEpcHelper()
{
    return m_nr_epc;
}

Ptr<BeamformingHelperBase>
NrPhySimulationHelper::GetBeamformingHelper()
{
    return m_beamHelper;
}

void
NrPhySimulationHelper::SetScheduler(const TypeId& schedulerType)
{
    m_nr->SetSchedulerTypeId(schedulerType);
}

void
NrPhySimulationHelper::SetBeamformingMethod(
    const TypeId& beamformingMethod,
    const std::vector<ModelConfiguration::Attribute>& attributes)
{
    m_beamHelper->SetBeamformingMethod(beamformingMethod);
    for (size_t i = 0; i < attributes.size(); i++)
    {
        m_beamHelper->SetBeamformingAlgorithmAttribute(attributes[i].name, *attributes[i].value);
    }
}

void
NrPhySimulationHelper::SetBeamformingMethod(const TypeId& beamformingMethod)
{
    SetBeamformingMethod(beamformingMethod, {});
}

OperationBandInfo
NrPhySimulationHelper::CreateOperationBand(
    const std::vector<CcBwpCreator::SimpleOperationBandConf>& bandConf,
    const std::string& bandScenario,
    const std::string& bandCondition,
    const std::string& bandModel,
    bool contiguousCc,
    std::vector<ModelConfiguration::Attribute> channelAttributes,
    std::vector<ModelConfiguration::Attribute> pathlossAttributes,
    std::vector<ModelConfiguration::Attribute> phasedSpectrumAttributes,
    uint8_t channelConfigFlags)
{
    CcBwpCreator ccBwpCreator;
    OperationBandInfo res;
    auto nrChannel = CreateObject<NrChannelHelper>();
    nrChannel->ConfigureFactories(bandScenario, bandCondition, bandModel);
    if (contiguousCc)
    {
        NS_ASSERT_MSG(bandConf.size() == 1,
                      "When using contiguous CC, only one band configuration must be provided");
        res = ccBwpCreator.CreateOperationBandContiguousCc(bandConf[0]);
    }
    else
    {
        NS_ASSERT_MSG(
            bandConf.size() > 0,
            "When using non-contiguous CC, at least one band configuration must be provided");
        res = ccBwpCreator.CreateOperationBandNonContiguousCc(bandConf);
    }
    for (const auto& attr : channelAttributes)
    {
        nrChannel->SetChannelConditionModelAttribute(attr.name, *attr.value);
    }
    for (const auto& attr : pathlossAttributes)
    {
        nrChannel->SetPathlossAttribute(attr.name, *attr.value);
    }
    for (const auto& attr : phasedSpectrumAttributes)
    {
        nrChannel->SetPhasedArraySpectrumPropagationLossModelAttribute(attr.name, *attr.value);
    }

    nrChannel->AssignChannelsToBands({res}, channelConfigFlags);
    m_bands.push_back(std::move(res));
    return res;
}

BandwidthPartInfoPtrVector
NrPhySimulationHelper::GetAllBwps() const
{
    std::vector<std::reference_wrapper<OperationBandInfo>> refs;
    refs.reserve(m_bands.size());
    for (const auto& band : m_bands)
    {
        refs.push_back(std::ref(const_cast<OperationBandInfo&>(band)));
    }
    return CcBwpCreator::GetAllBwps(refs);
}

void
NrPhySimulationHelper::SetGnbAntenna(
    const std::string& antennaType,
    const std::vector<ModelConfiguration::Attribute>& antennaProps,
    const std::vector<ModelConfiguration::Attribute>& antennaArrayProps)
{
    for (const auto& attr : antennaArrayProps)
    {
        m_nr->SetGnbAntennaAttribute(attr.name, *attr.value);
    }

    ObjectFactory antennaFactory;
    antennaFactory.SetTypeId(TypeId::LookupByName(antennaType));
    for (const auto& attr : antennaProps)
    {
        antennaFactory.Set(attr.name, *attr.value);
    }
    Ptr<Object> antenna = antennaFactory.Create();
    m_nr->SetGnbAntennaAttribute("AntennaElement", PointerValue(antenna));
}

void
NrPhySimulationHelper::SetUeAntenna(
    const std::string& antennaType,
    const std::vector<ModelConfiguration::Attribute>& antennaProps,
    const std::vector<ModelConfiguration::Attribute>& antennaArrayProps)
{
    {
        for (const auto& attr : antennaArrayProps)
        {
            m_nr->SetUeAntennaAttribute(attr.name, *attr.value);
        }

        ObjectFactory antennaFactory;
        antennaFactory.SetTypeId(TypeId::LookupByName(antennaType));
        for (const auto& attr : antennaProps)
        {
            antennaFactory.Set(attr.name, *attr.value);
        }
        Ptr<Object> antenna = antennaFactory.Create();
        m_nr->SetUeAntennaAttribute("AntennaElement", PointerValue(antenna));
    }
}

NetDeviceContainer
NrPhySimulationHelper::InstallGnbDevices(NodeContainer& gnbNode, BandwidthPartInfoPtrVector allBwps)
{
    return m_nr->InstallGnbDevice(gnbNode, allBwps);
}

NetDeviceContainer
NrPhySimulationHelper::InstallGnbDevices(NodeContainer& gnbNode)
{
    return InstallGnbDevices(gnbNode, GetAllBwps());
}

NetDeviceContainer
NrPhySimulationHelper::InstallUeDevices(NodeContainer& ueNodes, BandwidthPartInfoPtrVector allBwps)
{
    return m_nr->InstallUeDevice(ueNodes, allBwps);
}

NetDeviceContainer
NrPhySimulationHelper::InstallUeDevices(NodeContainer& ueNodes)
{
    return InstallUeDevices(ueNodes, GetAllBwps());
}

std::pair<NetDeviceContainer, NetDeviceContainer>
NrPhySimulationHelper::InstallDevices(NodeContainer& gnbNode,
                                      NodeContainer& ueNodes,
                                      BandwidthPartInfoPtrVector allBwps)
{
    return std::make_pair(InstallGnbDevices(gnbNode, allBwps), InstallUeDevices(ueNodes, allBwps));
}

std::pair<NetDeviceContainer, NetDeviceContainer>
NrPhySimulationHelper::InstallDevices(NodeContainer& gnbNode, NodeContainer& ueNodes)
{
    return InstallDevices(gnbNode, ueNodes, GetAllBwps());
}

void
NrPhySimulationHelper::SetUePhyAttributes(
    const std::vector<ModelConfiguration::Attribute>& uePhyAttributes)
{
    for (const auto& attr : uePhyAttributes)
    {
        m_nr->SetUePhyAttribute(attr.name, *attr.value);
    }
}

void
NrPhySimulationHelper::SetGnbPhyAttributes(
    const std::vector<ModelConfiguration::Attribute>& gnbPhyAttributes)
{
    for (const auto& attr : gnbPhyAttributes)
    {
        m_nr->SetGnbPhyAttribute(attr.name, *attr.value);
    }
}

} // namespace ns3