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
#ifndef WIFI_NETDEVICE_CONFIGURATION_H
#define WIFI_NETDEVICE_CONFIGURATION_H

#include "netdevice-configuration.h"

#include <ns3/model-configuration.h>
#include <ns3/object.h>

namespace ns3
{

/**
 * \brief Data class to recnognize and configure a Network Device for an entity to be simulated.
 */
class WifiNetdeviceConfiguration : public NetdeviceConfiguration
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
    WifiNetdeviceConfiguration(const std::string type,
                               const ModelConfiguration macLayer,
                               const std::optional<uint32_t> networkLayerId,
                               const std::optional<ModelConfiguration> antennaModel);
    /**
     * \return The MAC Layer configuration.
     */
    const ModelConfiguration GetMacLayer() const;

  private:
    const ModelConfiguration m_macLayer;
};

} // namespace ns3

#endif /* WIFI_NETDEVICE_CONFIGURATION_H */
