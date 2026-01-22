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
#ifndef NR_PHY_SIMULATION_HELPER_H
#define NR_PHY_SIMULATION_HELPER_H

#include <ns3/beamforming-helper-base.h>
#include <ns3/cc-bwp-helper.h>
#include <ns3/model-configuration.h>
#include <ns3/nr-channel-helper.h>
#include <ns3/nr-epc-helper.h>
#include <ns3/nr-helper.h>
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
    Ptr<NrEpcHelper> GetNrEpcHelper();
    /**
     * \return The Beamforming Helper used to configure Beamforming.
     */
    Ptr<BeamformingHelperBase> GetBeamformingHelper();

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
     * @param attributes The list of attributes to configure the scheduler (optional).
     */
    void SetScheduler(const TypeId& schedulerType,
                      const std::vector<ModelConfiguration::Attribute>& attributes = {});

    /**
     * Set the error model to be used in the simulation.
     * @param errorModelType The error model type to be used.
     * @param attributes The list of attributes to configure the error model (optional).
     */
    void SetErrorModel(const TypeId& errorModelType,
                       const std::vector<ModelConfiguration::Attribute>& attributes = {});

    /**
     * Set the DL error model to be used in the simulation.
     * @param dlErrorModelType The DL error model type to be used.
     * @param attributes The list of attributes to configure the DL error model (optional).
     */
    void SetDlErrorModel(const TypeId& dlErrorModelType,
                         const std::vector<ModelConfiguration::Attribute>& attributes = {});

    /**
     * Set the UL error model to be used in the simulation.
     * @param ulErrorModelType The UL error model type to be used.
     * @param attributes The list of attributes to configure the UL error model (optional).
     */
    void SetUlErrorModel(const TypeId& ulErrorModelType,
                         const std::vector<ModelConfiguration::Attribute>& attributes = {});

    struct ChannelOperationBandConf
    {
        bool contiguous;
        std::vector<CcBwpCreator::SimpleOperationBandConf> carrierConfs;
    };

    void CreateChannel(
        const std::vector<ChannelOperationBandConf>& freqBands,
        const std::string& bandScenario,
        const std::string& bandCondition,
        const std::string& bandModel,
        std::vector<ModelConfiguration::Attribute> channelAttributes,
        std::vector<ModelConfiguration::Attribute> pathlossAttributes,
        std::vector<ModelConfiguration::Attribute> phasedSpectrumAttributes,
        uint8_t channelConfigFlags = NrChannelHelper::INIT_PROPAGATION |
                                     NrChannelHelper::INIT_FADING);

    void SetGnbAntenna(const std::string& antennaType,
                       const std::vector<ModelConfiguration::Attribute>& antennaProps = {},
                       const std::vector<ModelConfiguration::Attribute>& antennaArrayProps = {});

    void SetUeAntenna(const std::string& antennaType,
                      const std::vector<ModelConfiguration::Attribute>& antennaProps = {},
                      const std::vector<ModelConfiguration::Attribute>& antennaArrayProps = {});

    void ResetGnbAntenna();
    void ResetUeAntenna();

    void SetUePhyAttributes(const std::vector<ModelConfiguration::Attribute>& uePhyAttributes);
    void SetGnbPhyAttributes(const std::vector<ModelConfiguration::Attribute>& gnbPhyAttributes);

    BandwidthPartInfoPtrVector GetBwps(uint32_t channelId, const std::vector<uint32_t>& bandIndices) const;

    NetDeviceContainer InstallGnbDevices(NodeContainer& gnbNode,
                                         BandwidthPartInfoPtrVector allBwps);
    NetDeviceContainer InstallUeDevices(NodeContainer& ueNodes, BandwidthPartInfoPtrVector allBwps);

    std::pair<NetDeviceContainer, NetDeviceContainer> InstallDevices(
        NodeContainer& gnbNode,
        NodeContainer& ueNodes,
        BandwidthPartInfoPtrVector allBwps);

    void SetEpcHelper(TypeId epc, std::vector<ModelConfiguration::Attribute> attributes);
    void SetBeamformingHelper(TypeId beam, std::vector<ModelConfiguration::Attribute> attributes);

  private:
    Ptr<NrHelper> m_nr;
    Ptr<NrEpcHelper> m_nr_epc;
    Ptr<BeamformingHelperBase> m_beamHelper;
    std::vector<std::vector<OperationBandInfo>> m_channelsBands;
    size_t m_stackId = 0;
};

} // namespace ns3

#endif /* NR_PHY_SIMULATION_HELPER_H */
