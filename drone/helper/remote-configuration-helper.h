/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2022 The IoD_Sim Authors.
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
#ifndef REMOTE_CONFIGURATION_HELPER_H
#define REMOTE_CONFIGURATION_HELPER_H

#include <ns3/remote-configuration.h>

#include <rapidjson/document.h>

namespace ns3 {

using JsonArray = rapidjson::GenericArray<true, rapidjson::Value>;

/**
 * Helper to decode an Remote from a JSON configuration file and read the following properties:
 *   - Its Network Layer Identifier
 *   - Its Applications
 */
class RemoteConfigurationHelper
{
public:
  /**
   * Parse an entity configuration from a given JSON tree and map it on an RemoteConfiguration data class.
   *
   * \param json The JSON tree to parse.
   * \return The configuration as a pointer to RemoteConfiguration to easily retrieve parsed data.
   */
  static Ptr<RemoteConfiguration> GetConfiguration (const rapidjson::Value& json);

private:
  RemoteConfigurationHelper ();

  static const uint32_t DecodeNetworkLayerId (const rapidjson::Value& json);
  static const std::vector<ModelConfiguration> DecodeApplicationConfigurations (const rapidjson::Value& json);
};

}

#endif /* REMOTE_CONFIGURATION_HELPER_H */
