/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2026 The IoD_Sim Authors.
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
#include <ns3/double.h>
#include <ns3/isotropic-antenna-model.h>
#include <ns3/net-device-container.h>
#include <ns3/node-container.h>
#include <ns3/nr-channel-helper.h>
#include <ns3/object-factory.h>
#include <ns3/pointer.h>
#include <ns3/string.h>
#include <ns3/type-id.h>
#include <ns3/uinteger.h>

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
NrPhySimulationHelper::SetScheduler(const TypeId& schedulerType,
                                    const std::vector<ModelConfiguration::Attribute>& attributes)
{
    m_nr->SetSchedulerTypeId(schedulerType);
    for (const auto& attr : attributes)
    {
        m_nr->SetSchedulerAttribute(attr.name, *attr.value);
    }
}

void
NrPhySimulationHelper::SetErrorModel(const TypeId& errorModelType,
                                     const std::vector<ModelConfiguration::Attribute>& attributes)
{
    // Set both DL and UL with the same model (backward compatibility)
    SetDlErrorModel(errorModelType, attributes);
    SetUlErrorModel(errorModelType, attributes);
}

void
NrPhySimulationHelper::SetDlErrorModel(const TypeId& dlErrorModelType,
                                       const std::vector<ModelConfiguration::Attribute>& attributes)
{
    m_nr->SetDlErrorModel(dlErrorModelType.GetName());
    for (const auto& attr : attributes)
    {
        m_nr->SetGnbDlAmcAttribute(attr.name, *attr.value);
    }
}

void
NrPhySimulationHelper::SetUlErrorModel(const TypeId& ulErrorModelType,
                                       const std::vector<ModelConfiguration::Attribute>& attributes)
{
    m_nr->SetUlErrorModel(ulErrorModelType.GetName());
    for (const auto& attr : attributes)
    {
        m_nr->SetGnbUlAmcAttribute(attr.name, *attr.value);
    }
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

void
NrPhySimulationHelper::CreateChannel(
    const std::vector<ChannelOperationBandConf>& freqBands,
    const std::string& bandScenario,
    const std::string& bandCondition,
    const std::string& bandModel,
    std::vector<ModelConfiguration::Attribute> channelAttributes,
    std::vector<ModelConfiguration::Attribute> pathlossAttributes,
    std::vector<ModelConfiguration::Attribute> phasedSpectrumAttributes,
    uint8_t channelConfigFlags)
{
    CcBwpCreator ccBwpCreator;
    std::vector<OperationBandInfo> opBands;
    auto nrChannel = CreateObject<NrChannelHelper>();
    nrChannel->ConfigureFactories(bandScenario, bandCondition, bandModel);

    for (const auto& fb : freqBands)
    {
        if (fb.contiguous)
        {
            NS_ASSERT_MSG(fb.carrierConfs.size() == 1,
                          "When using contiguous CC, one carrier configuration must be provided");
            opBands.push_back(ccBwpCreator.CreateOperationBandContiguousCc(fb.carrierConfs[0]));
        }
        else
        {
            NS_ASSERT_MSG(fb.carrierConfs.size() > 0,
                          "When using non-contiguous CC, at least one carrier configuration must "
                          "be provided");
            opBands.push_back(ccBwpCreator.CreateOperationBandNonContiguousCc(fb.carrierConfs));
        }
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

    std::vector<std::reference_wrapper<OperationBandInfo>> refBands;
    refBands.reserve(opBands.size());
    for (auto& band : opBands)
    {
        refBands.emplace_back(band);
    }

    nrChannel->AssignChannelsToBands(refBands, channelConfigFlags);
    m_channelsBands.push_back(std::move(opBands));
}

BandwidthPartInfoPtrVector
NrPhySimulationHelper::GetBwps(uint32_t channelId, const std::vector<uint32_t>& bandIndices) const
{
    NS_ASSERT_MSG(channelId < m_channelsBands.size(), "Channel ID out of range");

    std::vector<std::reference_wrapper<OperationBandInfo>> refs;
    const auto& channel = m_channelsBands[channelId];

    if (bandIndices.empty())
    {
        // If no specific bands are requested, return all bands in the channel
        for (const auto& band : channel)
        {
            refs.push_back(std::ref(const_cast<OperationBandInfo&>(band)));
        }
    }
    else
    {
        for (const auto& index : bandIndices)
        {
            NS_ASSERT_MSG(index < channel.size(), "Band index out of range");
            refs.push_back(std::ref(const_cast<OperationBandInfo&>(channel[index])));
        }
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

void
NrPhySimulationHelper::ResetGnbAntenna()
{
    // Reset type to UniformPlanarArray
    m_nr->SetGnbAntennaTypeId("ns3::UniformPlanarArray");
    m_nr->SetGnbAntennaAttribute("NumRows", UintegerValue(4));
    m_nr->SetGnbAntennaAttribute("NumColumns", UintegerValue(4));
    m_nr->SetGnbAntennaAttribute("NumHorizontalPorts", UintegerValue(1));
    m_nr->SetGnbAntennaAttribute("NumVerticalPorts", UintegerValue(1));
    m_nr->SetGnbAntennaAttribute("IsDualPolarized", BooleanValue(false));
    m_nr->SetGnbAntennaAttribute("PolSlantAngle", DoubleValue(0.0));
    m_nr->SetGnbAntennaAttribute("DowntiltAngle", DoubleValue(0.0));
    m_nr->SetGnbAntennaAttribute("BearingAngle", DoubleValue(0.0));
    m_nr->SetGnbAntennaAttribute("AntennaHorizontalSpacing", DoubleValue(0.5));
    m_nr->SetGnbAntennaAttribute("AntennaVerticalSpacing", DoubleValue(0.5));
    // Reset AntennaElement to Isotropic
    Ptr<Object> iso = CreateObject<IsotropicAntennaModel>();
    m_nr->SetGnbAntennaAttribute("AntennaElement", PointerValue(iso));
}

void
NrPhySimulationHelper::ResetUeAntenna()
{
    // Reset type to UniformPlanarArray
    m_nr->SetUeAntennaTypeId("ns3::UniformPlanarArray");
    m_nr->SetUeAntennaAttribute("NumRows", UintegerValue(4));
    m_nr->SetUeAntennaAttribute("NumColumns", UintegerValue(4));
    m_nr->SetUeAntennaAttribute("NumHorizontalPorts", UintegerValue(1));
    m_nr->SetUeAntennaAttribute("NumVerticalPorts", UintegerValue(1));
    m_nr->SetUeAntennaAttribute("IsDualPolarized", BooleanValue(false));
    m_nr->SetUeAntennaAttribute("PolSlantAngle", DoubleValue(0.0));
    m_nr->SetUeAntennaAttribute("DowntiltAngle", DoubleValue(0.0));
    m_nr->SetUeAntennaAttribute("BearingAngle", DoubleValue(0.0));
    m_nr->SetUeAntennaAttribute("AntennaHorizontalSpacing", DoubleValue(0.5));
    m_nr->SetUeAntennaAttribute("AntennaVerticalSpacing", DoubleValue(0.5));
    // Reset AntennaElement to Isotropic
    Ptr<Object> iso = CreateObject<IsotropicAntennaModel>();
    m_nr->SetUeAntennaAttribute("AntennaElement", PointerValue(iso));
}

NetDeviceContainer
NrPhySimulationHelper::InstallGnbDevices(NodeContainer& gnbNode, BandwidthPartInfoPtrVector allBwps)
{
    return m_nr->InstallGnbDevice(gnbNode, allBwps);
}

NetDeviceContainer
NrPhySimulationHelper::InstallUeDevices(NodeContainer& ueNodes, BandwidthPartInfoPtrVector allBwps)
{
    return m_nr->InstallUeDevice(ueNodes, allBwps);
}

std::pair<NetDeviceContainer, NetDeviceContainer>
NrPhySimulationHelper::InstallDevices(NodeContainer& gnbNode,
                                      NodeContainer& ueNodes,
                                      BandwidthPartInfoPtrVector allBwps)
{
    return std::make_pair(InstallGnbDevices(gnbNode, allBwps), InstallUeDevices(ueNodes, allBwps));
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