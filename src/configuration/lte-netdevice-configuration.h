/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2024 The IoD_Sim Authors.
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
#ifndef LTE_NETDEVICE_CONFIGURATION_H
#define LTE_NETDEVICE_CONFIGURATION_H

#include "lte-bearer-configuration.h"
#include "netdevice-configuration.h"

#include <ns3/model-configuration.h>

#include <optional>

namespace ns3
{

/**
 * \ingroup lte
 *
 * \brief Enum to represent the role of the Network Device in the LTE network.
 */
enum LteRole
{
    UE,
    eNB
};

/**
 * \ingroup lte
 *
 * \brief Data class to recnognize and configure an LTE Network Device for an entity to be
 * simulated.
 */
class LteNetdeviceConfiguration : public NetdeviceConfiguration
{
  public:
    /**
     * Create a new object instance.
     *
     * \param type The type of the network device (e.g., "wifi" to use the underlying WiFi Protocol
     * Stack). \param rawRole The role of the network device in the LTE RAN (e.g., UE or eNB). This
     * string will be parsed in the correct role. \param bearers The bearers to be initialised for
     * this network device. \param networkLayerId The identifier for the Network Layer that has been
     * defined for this simulation. It must be compatible with the given type and macLayer.
     */
    LteNetdeviceConfiguration(const std::string type,
                              const std::string rawRole,
                              const std::vector<LteBearerConfiguration> bearers,
                              const std::optional<uint32_t> networkLayerId,
                              const std::optional<ModelConfiguration> antennaModel,
                              const std::optional<ModelConfiguration> phyModel);
    /**
     * \return The role of the Network Device in the LTE network.
     */
    const LteRole GetRole() const;
    /**
     * \return The bearers configuration for the Network Device.
     */
    const std::vector<LteBearerConfiguration> GetBearers() const;
    /** \return The phy model configuration for the Network Device. */
    const std::optional<ModelConfiguration> GetPhyModel() const;

  private:
    const LteRole m_role;
    const std::vector<LteBearerConfiguration> m_bearers;
    const std::optional<ModelConfiguration> m_phyModel;
};

} // namespace ns3

#endif /* LTE_NETDEVICE_CONFIGURATION_H */
