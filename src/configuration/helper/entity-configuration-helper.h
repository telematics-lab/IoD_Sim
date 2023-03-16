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
#ifndef ENTITY_CONFIGURATION_HELPER_H
#define ENTITY_CONFIGURATION_HELPER_H

#include <rapidjson/document.h>

#include <ns3/entity-configuration.h>
#include <ns3/lte-bearer-configuration.h>
#include <ns3/netdevice-configuration.h>

namespace ns3 {

using JsonArray = rapidjson::GenericArray<true, rapidjson::Value>;

/**
 * Helper to decode an Entity (i.e., Drone or ZSP) from a JSON configuration file and read the following properties:
 *   - Its network device(s)
 *   - Its associated mobility model
 *   - Its application(s)
 *   - Its mechanical properties
 */
class EntityConfigurationHelper
{
public:
  /**
   * Parse an entity configuration from a given JSON tree and map it on an EntityConfiguration data class.
   *
   * \param json The JSON tree to parse.
   * \return The configuration as a pointer to EntityConfiguration to easily retrieve parsed data.
   */
  static Ptr<EntityConfiguration> GetConfiguration (const rapidjson::Value& json);

private:
  EntityConfigurationHelper ();

  static const std::vector<Ptr<NetdeviceConfiguration>> DecodeNetdeviceConfigurations (const rapidjson::Value& json);
  static const std::vector<LteBearerConfiguration> DecodeLteBearerConfigurations (const JsonArray& json);
  static const MobilityModelConfiguration DecodeMobilityConfiguration (const rapidjson::Value& json);
  static const std::optional<Vector> DecodeInitialPosition (const rapidjson::Value& json);
  static const std::vector<ModelConfiguration> DecodeApplicationConfigurations (const rapidjson::Value& json);
  static const ModelConfiguration DecodeMechanicsConfiguration (const rapidjson::Value& json);
  static const ModelConfiguration DecodeBatteryConfiguration (const rapidjson::Value& json);
  static const std::vector<ModelConfiguration> DecodePeripheralConfigurations(const rapidjson::Value& json);
  static const ModelConfiguration DecodeModelConfiguration (const rapidjson::Value &json);
};

}

#endif /* ENTITY_CONFIGURATION_HELPER_H */
