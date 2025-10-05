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
#ifndef NR_PHY_SIMULATION_HELPER_H
#define NR_PHY_SIMULATION_HELPER_H

#include <ns3/cc-bwp-helper.h>
#include <ns3/ideal-beamforming-helper.h>
#include <ns3/model-configuration.h>
#include <ns3/nr-channel-helper.h>
#include <ns3/nr-helper.h>
#include <ns3/nr-point-to-point-epc-helper.h>
#include <ns3/object.h>
#include <ns3/type-id.h>

#include <tuple>
#include <vector>

namespace ns3
{

class NetDeviceContainer;
class NodeContainer;
struct OperationBandInfo;
class CcBwpCreator;

// BandwidthPartInfoPtrVector is already defined in cc-bwp-helper.h

/**
 * A data class to store information about a NR PHY layer configuration for a simulation.
 */
class NrPhySimulationHelper : public Object
{
  public:
    /** Default constructor */
    NrPhySimulationHelper(const size_t stackId);
    /** Default destructor */
    ~NrPhySimulationHelper();

    /**
     * \return The NR Helper used to configure this layer.
     */
    Ptr<NrHelper> GetNrHelper();
    /**
     * \return The NR EPC PHY Helper used to configure this layer.
     */
    Ptr<NrPointToPointEpcHelper> GetNrEpcHelper();
    /**
     * \return The Ideal Beamforming Helper used to configure Beamforming.
     */
    Ptr<IdealBeamformingHelper> GetIdealBeamformingHelper();
    /**
     * \return The NR Channel Helper used to configure the channel.
     */
    Ptr<NrChannelHelper> GetNrChannelHelper();

    /**
     * Set the beamforming method to be used in the simulation.
     * @param beamformingMethod The beamforming method to be used. It must be a valid TypeId name
     * accepted by IdealBeamformingHelper::SetBeamformingMethod.
     */
    void SetBeamformingMethod(const TypeId& beamformingMethod,
                              const std::vector<ModelConfiguration::Attribute>& attributes);

    /**
     * Set the attributes for the beamforming method.
     * @param attributes The attributes to be set for the beamforming method.
     */
    void SetBeamformingMethod(const TypeId& beamformingMethod);

    /**
     * Set the MAC scheduler to be used in the simulation.
     * @param schedulerType The scheduler type to be used. It must be a valid TypeId name
     * accepted by NrHelper::SetSchedulerTypeId.
     */
    void SetScheduler(const TypeId& schedulerType);

    OperationBandInfo CreateOperationBand(
        const std::vector<CcBwpCreator::SimpleOperationBandConf>& bandConf,
        const std::string& bandScenario,
        const std::string& bandCondition,
        const std::string& bandModel,
        bool contiguousCc,
        uint8_t channelConfigFlags = NrChannelHelper::INIT_PROPAGATION |
                                     NrChannelHelper::INIT_FADING);

    void SetGnbAntenna(const std::string& antennaType,
                       const std::vector<ModelConfiguration::Attribute>& antennaProps = {},
                       const std::vector<ModelConfiguration::Attribute>& antennaArrayProps = {});

    void SetUeAntenna(const std::string& antennaType,
                      const std::vector<ModelConfiguration::Attribute>& antennaProps = {},
                      const std::vector<ModelConfiguration::Attribute>& antennaArrayProps = {});

    void SetUePhyAttributes(const std::vector<ModelConfiguration::Attribute>& uePhyAttributes);
    void SetGnbPhyAttributes(const std::vector<ModelConfiguration::Attribute>& gnbPhyAttributes);

    BandwidthPartInfoPtrVector GetAllBwps() const;

    NetDeviceContainer InstallGnbDevices(NodeContainer& gnbNode,
                                         BandwidthPartInfoPtrVector allBwps);
    NetDeviceContainer InstallGnbDevices(NodeContainer& gnbNode);
    NetDeviceContainer InstallUeDevices(NodeContainer& ueNodes, BandwidthPartInfoPtrVector allBwps);
    NetDeviceContainer InstallUeDevices(NodeContainer& ueNodes);

    std::pair<NetDeviceContainer, NetDeviceContainer> InstallDevices(
        NodeContainer& gnbNode,
        NodeContainer& ueNodes,
        BandwidthPartInfoPtrVector allBwps);
    std::pair<NetDeviceContainer, NetDeviceContainer> InstallDevices(NodeContainer& gnbNode,
                                                                     NodeContainer& ueNodes);

  private:
    Ptr<NrHelper> m_nr;
    Ptr<NrPointToPointEpcHelper> m_nr_epc;
    Ptr<IdealBeamformingHelper> m_ideal_beam;
    Ptr<NrChannelHelper> m_nr_channel;
    std::vector<std::reference_wrapper<OperationBandInfo>> m_bands;
};

} // namespace ns3

#endif /* NR_PHY_SIMULATION_HELPER_H */
