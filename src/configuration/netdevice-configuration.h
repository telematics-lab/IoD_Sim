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
#ifndef NETDEVICE_CONFIGURATION_H
#define NETDEVICE_CONFIGURATION_H

#include <ns3/model-configuration.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/vector.h>

#include <optional>
#include <vector>

namespace ns3
{

struct DirectivityConfiguration
{
    std::string mode;
    std::string coordinates = "geocentric";
    Vector position = Vector(0, 0, 0);
    Time precision = MilliSeconds(100);
    std::string key = "";
    uint32_t index = 0;
};

struct OutputLinkConfiguration
{
    uint32_t sourceBwp;
    uint32_t targetBwp;
};

/**
 * \brief Data class to recnognize and configure a Network Device for an entity to be simulated.
 */
class NetdeviceConfiguration : public Object
{
  public:
    /**
     * Create a new object instance.
     *
     * \param type The type of the network device (e.g., "wifi" to use the underlying WiFi Protocol
     * Stack). \param macLayer The configuration of the MAC Layer to be simulated for this network
     * device. \param networkLayerId The identifier for the Network Layer that has been defined for
     * this simulation. It must be compatible with the given type and macLayer.
     */
    NetdeviceConfiguration(
        const std::string type,
        const std::optional<uint32_t> networkLayerId,
        const std::optional<ModelConfiguration> antennaModel,
        const std::optional<DirectivityConfiguration> directivity = std::nullopt);
    /**
     * Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    /** \return The type of the Network Device. */
    const std::string GetType() const;
    /** \return The reference network layer identifier. */
    virtual const std::optional<uint32_t> GetNetworkLayerId() const;
    /** \return The antenna model configuration for the Network Device. */
    std::optional<ModelConfiguration> GetAntennaModel() const;
    /** \return The directivity configuration for the Network Device. */
    std::optional<DirectivityConfiguration> GetDirectivity() const;

  private:
    const std::string m_type;
    const std::optional<uint32_t> m_networkLayerId;
    const std::optional<ModelConfiguration> m_antennaModel;
    const std::optional<DirectivityConfiguration> m_directivity;
};

} // namespace ns3

#endif /* NETDEVICE_CONFIGURATION_H */
