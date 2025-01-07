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
#ifndef IPV4_NETWORK_LAYER_CONFIGURATION_H
#define IPV4_NETWORK_LAYER_CONFIGURATION_H

#include "network-layer-configuration.h"

#include <string>

namespace ns3
{

/**
 * \brief A data class to store configuration information related to the set up of an IPv4 Network
 * Layer.
 */
class Ipv4NetworkLayerConfiguration : public NetworkLayerConfiguration
{
  public:
    /**
     * Create a new object instance.
     *
     * \param type The type of network layer configuration. It must be "ipv4".
     * \param address The network address.
     * \param mask The network mask.
     * \param gatewayAddress The gateway address.
     */
    Ipv4NetworkLayerConfiguration(std::string type,
                                  std::string address,
                                  std::string mask,
                                  std::string gatewayAddress);
    /**
     * \return The configured network address.
     */
    const std::string GetAddress() const;
    /**
     * \return The configured network mask.
     */
    const std::string GetMask() const;
    /**
     * \return The configured network gateway address.
     */
    const std::string GetGatewayAddress() const;

  private:
    const std::string m_address;
    const std::string m_mask;
    const std::string m_gatewayAddress;
};

} // namespace ns3

#endif /* IPV4_NETWORK_LAYER_CONFIGURATION_H */
