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
#include "network-layer-configuration-helper.h"

#include <ns3/assert.h>
#include <ns3/double.h>
#include <ns3/fatal-error.h>
#include <ns3/ipv4-network-layer-configuration.h>
#include <ns3/string.h>
#include <ns3/type-id.h>

namespace ns3
{

Ptr<NetworkLayerConfiguration>
NetworkLayerConfigurationHelper::GetConfiguration(const rapidjson::Value& json)
{
    NS_ASSERT_MSG(json.IsObject(),
                  "Network Layer definition must be an object, got " << json.GetType());
    NS_ASSERT_MSG(json.HasMember("type"), "Network Layer definition must have 'type' property.");
    NS_ASSERT_MSG(json["type"].IsString(), "Network Layer 'type' property must be a string.");

    const std::string type = json["type"].GetString();
    Ptr<NetworkLayerConfiguration> netConfig{nullptr};

    if (type == "ipv4")
    {
        NS_ASSERT_MSG(json.HasMember("address"),
                      "Network Layer definition must have 'address' property.");
        NS_ASSERT_MSG(json["address"].IsString(),
                      "Network Layer 'address' property must be a string.");
        NS_ASSERT_MSG(json.HasMember("mask"),
                      "Network Layer definition must have 'mask' property.");
        NS_ASSERT_MSG(json["mask"].IsString(), "Network Layer 'mask' property must be a string.");
        NS_ASSERT_MSG(json["gateway"].IsString(),
                      "Network Layer 'gateway' property must be a string.");

        netConfig = Create<Ipv4NetworkLayerConfiguration>(json["type"].GetString(),
                                                          json["address"].GetString(),
                                                          json["mask"].GetString(),
                                                          json["gateway"].GetString());
    }
    else
    {
        NS_FATAL_ERROR("Network Layer of Type " << type << " is not supported!");
    }

    return netConfig;
}

NetworkLayerConfigurationHelper::NetworkLayerConfigurationHelper()
{
}

} // namespace ns3
