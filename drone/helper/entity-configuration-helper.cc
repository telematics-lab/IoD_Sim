/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2020 The IoD_Sim Authors.
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
#include "entity-configuration-helper.h"

#include <ns3/double.h>
#include <ns3/string.h>

namespace ns3 {

Ptr<EntityConfiguration>
EntityConfigurationHelper::GetConfiguration (const rapidjson::Value& json)
{
  NS_ASSERT_MSG (json.IsObject (),
                 "Entity configuration must be an object.");
  NS_ASSERT_MSG (json.HasMember ("netDevices"),
                 "Entity configuration must have 'netDevices' property defined.");
  NS_ASSERT_MSG (json.HasMember ("mobilityModel"),
                 "Entity configuration must have 'mobilityModel'property.");
    NS_ASSERT_MSG (json.HasMember ("applications"),
                 "Entity configuration must have 'applications' property defined.");

  const auto netDevices = DecodeNetdeviceConfigurations (json["netDevices"]);
  const auto mobilityModel = DecodeMobilityConfiguration (json["mobilityModel"]);
  const auto applications = DecodeApplicationConfigurations (json["applications"]);

  return CreateObject<EntityConfiguration> (netDevices, mobilityModel, applications);
}

EntityConfigurationHelper::EntityConfigurationHelper ()
{

}

const std::vector<Ptr<NetdeviceConfiguration>>
EntityConfigurationHelper::DecodeNetdeviceConfigurations (const rapidjson::Value& json)
{
  NS_ASSERT_MSG (json.IsArray (),
                 "Entity configuration 'netDevices' property must be an array.");

  std::vector<Ptr<NetdeviceConfiguration>> confs;
  for (auto& netdev : json.GetArray ()) {
    NS_ASSERT_MSG (netdev.IsObject (),
                   "Every Entity Network Device configuration must be an object.");
    NS_ASSERT_MSG (netdev.HasMember ("type"),
                   "Entity Network Device must have 'type' property defined.");
    NS_ASSERT_MSG (netdev["type"].IsString (),
                   "Entity Network Device 'type' property must be a string.");

    const std::string type = netdev["type"].GetString ();
    if (type.compare("wifi") == 0) {
      NS_ASSERT_MSG (netdev.HasMember ("macLayer"),
                     "Entity WiFi Network Device must have 'macLayer' property defined.");
      NS_ASSERT_MSG (netdev["macLayer"].IsObject (),
                     "Entity WiFi Network Device 'macLayer' property must be an object.");

      const auto macLayer = DecodeModelConfiguration (netdev["macLayer"]);

      NS_ASSERT_MSG (netdev.HasMember ("networkLayer"),
                     "Entity WiFi Network Device must have 'networkLayer' property defined.");
      NS_ASSERT_MSG (netdev["networkLayer"].IsUint (),
                     "Entity WiFi Network Device 'networkLayer' property must be an unsigned integer.");

      const uint32_t networkLayerId = netdev["networkLayer"].GetUint ();

      confs.push_back (CreateObject<NetdeviceConfiguration> (type, macLayer, networkLayerId));
    } else {
      NS_FATAL_ERROR ("Entity Network Device of Type " << type << " is not supported!");
    }
  }

  return confs;
}

const ModelConfiguration
EntityConfigurationHelper::DecodeMobilityConfiguration (const rapidjson::Value& json)
{
  NS_ASSERT_MSG (json["mobilityModel"].IsObject (),
                 "Entity mobility model configuration must be an object.");

  return DecodeModelConfiguration (json["mobilityModel"]);
}

// TODO merge with other helpers
const ModelConfiguration
EntityConfigurationHelper::DecodeModelConfiguration (const rapidjson::Value& jsonModel)
{
  NS_ASSERT_MSG (jsonModel.HasMember ("name"),
                 "Model configuration must have 'name' property.");
  NS_ASSERT_MSG (jsonModel["name"].IsString (),
                 "Model configuration 'name' property must be a string.");
  NS_ASSERT_MSG (jsonModel.HasMember ("attributes"),
            	   "Model configuration must have 'attributes' property.");
  NS_ASSERT_MSG (jsonModel["attributes"].IsArray (),
                  "Model configuration 'attributes' property must be an array.");

  const std::string modelName = jsonModel["name"].GetString ();
  TypeId modelTid = TypeId::LookupByName (modelName);
  auto jsonAttributes = jsonModel["attributes"].GetArray ();
  std::vector<std::pair<std::string, Ptr<AttributeValue>>> attributes;

  for (auto& el : jsonAttributes) {
    NS_ASSERT_MSG (el.IsObject (),
                   "Attribute model definition must be an object, got " << el.GetType ());
    NS_ASSERT_MSG (el.HasMember ("name"),
                   "Attribute model must have 'name' property.");
    NS_ASSERT_MSG (el["name"].IsString (),
                   "Attribute model name must be a string.");
    NS_ASSERT_MSG (el.HasMember ("value"),
                   "Attribute model must have 'value' property.");

    const std::string attrName = el["name"].GetString();
    TypeId::AttributeInformation attrInfo = {};

    NS_ASSERT (modelTid.LookupAttributeByName (attrName, &attrInfo));

    const auto attrValueType = el["value"].GetType ();
    Ptr<AttributeValue> attrValue;
    switch (attrValueType)
    {
    case rapidjson::Type::kStringType:
      {
        const auto attrValueString = el["value"].GetString ();
        attrValue = attrInfo.checker->CreateValidValue (StringValue (attrValueString));
      }
      break;
    case rapidjson::Type::kNumberType:
      {
        const auto attrValueDouble = el["value"].GetDouble ();
        attrValue = attrInfo.checker->CreateValidValue (DoubleValue (attrValueDouble));
      }
      break;
    case rapidjson::Type::kArrayType:
    case rapidjson::Type::kFalseType:
    case rapidjson::Type::kTrueType:
    case rapidjson::Type::kNullType:
    case rapidjson::Type::kObjectType:
    default:
      NS_FATAL_ERROR ("Cannot determine attribute value type of " << attrName);
      break;
    }

    attributes.emplace_back(std::make_pair(attrName, attrValue));
  }

  return ModelConfiguration(modelName, attributes);
}

const std::vector<Ptr<ApplicationConfiguration>>
EntityConfigurationHelper::DecodeApplicationConfigurations (const rapidjson::Value& json)
{
  NS_ASSERT_MSG (json.IsArray (),
                 "Entity configuration 'applications' property must be an array.");

  std::vector<Ptr<ApplicationConfiguration>> confs;
  for (auto& appl : json.GetArray ()){
    NS_ASSERT_MSG (appl.HasMember ("type"),
                   "Application configuration must have 'type' property.");
    NS_ASSERT_MSG (appl["type"].IsString (),
                   "Application configuration 'type' property must be a string.");
    NS_ASSERT_MSG (appl.HasMember ("startTime"),
                   "Application configuration must have 'startTime' property.");
    NS_ASSERT_MSG (appl["startTime"].IsDouble (),
                   "Application configuration 'startTime' property must be a double.");
    NS_ASSERT_MSG (appl.HasMember ("stopTime"),
                   "Application configuration must have 'stopTime' property.");
    NS_ASSERT_MSG (appl["stopTime"].IsDouble (),
                   "Application configuration 'stopTime' property must be a double.");

    const std::string appType = appl["type"].GetString ();
    const double appStartTime = appl["startTime"].GetDouble ();
    const double appStopTime = appl["stopTime"].GetDouble ();

    confs.push_back (CreateObject<ApplicationConfiguration> (appType, appStartTime, appStopTime));
  }

  return confs;
}

} // namespace ns3
