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
#ifndef NR_PHY_LAYER_CONFIGURATION_H
#define NR_PHY_LAYER_CONFIGURATION_H

#include "phy-layer-configuration.h"

#include <ns3/cc-bwp-helper.h>
#include <ns3/model-configuration.h>
#include <ns3/nr-channel-helper.h>
#include <ns3/object-factory.h>
#include <ns3/uinteger.h>
#include <ns3/wifi-phy.h>

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// Forward declarations for NR components
namespace ns3
{
class NrHelper;
class NrPointToPointEpcHelper;
class IdealBeamformingHelper;
struct OperationBandInfo; // struct, not class
class NetDeviceContainer;
class NodeContainer;
// BandwidthPartInfoPtr is a typedef, not a class - removing forward declaration
} // namespace ns3

namespace ns3
{

/**
 * Data class to store information about the NR PHY Layer of a Scenario.
 */
class NrPhyLayerConfiguration : public PhyLayerConfiguration
{
  public:
    /**
     * Create a new object instance.
     *
     * \param phyType The type of the PHY Layer to be configured. It should be "nr".
     * \param channelPropagationLossModel The Propagation Loss Model to be used for this Layer.
     * \param channelSpectrumModel The Spectrum Model type to be used.
     */
    NrPhyLayerConfiguration(
        std::string phyType,
        std::vector<ModelConfiguration::Attribute> attributes,
        std::string channelScenario,
        std::string ueAntennaType = "ns3::IsotropicAntennaModel",
        std::vector<ModelConfiguration::Attribute> ueAntennaProps = {},
        std::vector<ModelConfiguration::Attribute> ueAntennaArrayProps = {},
        std::string gnbAntennaType = "ns3::IsotropicAntennaModel",
        std::vector<ModelConfiguration::Attribute> gnbAntennaProps = {},
        std::vector<ModelConfiguration::Attribute> gnbAntennaArrayProps = {},
        std::string channelCondition = "Default",
        std::string channelModel = "ThreeGpp",
        std::string beamformingType = "ns3::DirectPathBeamforming",
        std::string schedulerType = "ns3::NrMacSchedulerTdmaRR",
        uint8_t channelConfigFlags = 0, // Will be set in constructor
        bool contiguousCc = true,
        std::vector<std::vector<CcBwpCreator::SimpleOperationBandConf>> bandConfs = {});
    /** Default destructor */
    ~NrPhyLayerConfiguration();

    /**
     * Get attribute value
     */
    std::string GetAttribute(const std::string& name) const;

    /**
     * Get channel propagation loss model configuration
     */
    std::shared_ptr<ModelConfiguration> GetChannelPropagationLossModel() const;

    /**
     * Get channel spectrum model configuration
     */
    std::shared_ptr<ModelConfiguration> GetChannelSpectrumModel() const;

  private:
};

} // namespace ns3

#endif /* NR_PHY_LAYER_CONFIGURATION_H */
