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
                             const std::optional<ModelConfiguration> antennaModel);
    /** Default destructor */
    ~NrNetdeviceConfiguration();

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

  private:
    const NrRole m_role;
    const std::vector<NrBearerConfiguration> m_bearers;
    const std::vector<NrPhyProperty> m_phyProperties;
};

} // namespace ns3

#endif /* NR_NETDEVICE_CONFIGURATION_H */
