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
#ifndef NR_NETDEVICE_CONFIGURATION_H
#define NR_NETDEVICE_CONFIGURATION_H

#include "netdevice-configuration.h"
#include "nr-bearer-configuration.h"

#include <ns3/model-configuration.h>

#include <optional>

namespace ns3
{

enum NrRole
{
    nrUE,
    gNB
};

struct NrPhyProperty
{
    ModelConfiguration::Attribute attribute;
    std::optional<uint32_t> bwpId;
};

struct X2NeighborConfiguration
{
    std::string key;
    uint32_t index;
};

/**
 * Data class to recnognize and configure an NR Network Device for an entity to be simulated.
 */
class NrNetdeviceConfiguration : public NetdeviceConfiguration
{
  public:
    /**
     * Create a new object instance.
     *
     * \param type The type of the network device (e.g., "wifi" to use the underlying WiFi Protocol
     * Stack). \param rawRole The role of the network device in the NR RAN (e.g., UE or gNB). This
     * string will be parsed in the correct role. \param bearers The bearers to be initialised for
     * this network device. \param networkLayerId The identifier for the Network Layer that has been
     * defined for this simulation. It must be compatible with the given type and macLayer.
     */
    NrNetdeviceConfiguration(const std::string type,
                             const std::string rawRole,
                             const std::vector<NrBearerConfiguration> bearers,
                             const std::vector<NrPhyProperty> phyProperties,
                             const std::optional<uint32_t> networkLayerId,
                             const std::optional<ModelConfiguration> antennaModel,
                             const std::vector<OutputLinkConfiguration> outputLinks = {},
                             const std::optional<DirectivityConfiguration> directivity = std::nullopt,
                             const uint32_t channelId = 0,
                             const std::vector<uint32_t> channelBands = {},
                             const std::vector<NrPhyProperty> rrcProperties = {},
                             const std::vector<X2NeighborConfiguration> x2Neighbors = {});
    /** Default destructor */
    ~NrNetdeviceConfiguration();

    /**
     * Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * \return The role of the Network Device in the NR network.
     */
    const NrRole GetRole() const;
    /**
     * \return The bearers configuration for the Network Device.
     */
    const std::vector<NrBearerConfiguration> GetBearers() const;
    /** \return The phy properties configuration for the Network Device. */
    const std::vector<NrPhyProperty> GetPhyProperties() const;
    /** \return The rrc properties configuration for the Network Device. */
    const std::vector<NrPhyProperty> GetRrcProperties() const;
    /** \return The output links configuration for the Network Device. */
    const std::vector<OutputLinkConfiguration> GetOutputLinks() const;
    /** \return The X2 neighbors configuration for the Network Device. */
    const std::vector<X2NeighborConfiguration> GetX2Neighbors() const;

    /** \return The channel ID for the Network Device. */
    const uint32_t GetChannelId() const;
    /** \return The channel bands indices for the Network Device. */
    const std::vector<uint32_t> GetChannelBands() const;

  private:
    const NrRole m_role;
    const std::vector<NrBearerConfiguration> m_bearers;
    const std::vector<NrPhyProperty> m_phyProperties;
    const std::vector<NrPhyProperty> m_rrcProperties;
    const std::vector<OutputLinkConfiguration> m_outputLinks;
    const std::vector<X2NeighborConfiguration> m_x2Neighbors;
    const uint32_t m_channelId;
    const std::vector<uint32_t> m_channelBands;
};

} // namespace ns3

#endif /* NR_NETDEVICE_CONFIGURATION_H */
