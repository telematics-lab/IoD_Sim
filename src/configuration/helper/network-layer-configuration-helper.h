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
#ifndef NETWORK_LAYER_CONFIGURATION_HELPER_H
#define NETWORK_LAYER_CONFIGURATION_HELPER_H

#include <ns3/network-layer-configuration.h>

#include <rapidyyjson/document.h>

namespace ns3
{
/**
 * Helper to decode a Network Layer from a JSON configuration file.
 */
class NetworkLayerConfigurationHelper
{
  public:
    /**
     * NetworkLayerConfigurationHelper can't be instantiated, as it is an utility class.
     */
    NetworkLayerConfigurationHelper() = delete;
    /**
     * Parse a Network configuration from a given JSON tree and map it on a
     * NetworkLayerConfiguration data class.
     *
     * \param json The JSON tree to parse.
     * \return The configuration as a pointer to NetworkLayerConfiguration to easily retrieve
     * parsed data.
     */
    static Ptr<NetworkLayerConfiguration> GetConfiguration(const rapidyyjson::Value& json);
};

} // namespace ns3

#endif /* NETWORK_LAYER_CONFIGURATION_HELPER_H */
