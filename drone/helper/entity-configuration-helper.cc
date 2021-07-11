/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2021 The IoD_Sim Authors.
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
#include <ns3/integer.h>
#include <ns3/object-factory.h>
#include <ns3/string.h>
#include <ns3/vector.h>

#include <ns3/flight-plan.h>
#include <ns3/lte-netdevice-configuration.h>
#include <ns3/speed-coefficients.h>
#include <ns3/wifi-netdevice-configuration.h>

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

  if (json.HasMember("mechanics") && json.HasMember("battery"))
    {
      const auto mechanics = DecodeMechanicsConfiguration (json["mechanics"]);
      const auto battery = DecodeBatteryConfiguration (json["battery"]);
      if (json.HasMember("peripherals"))
        {
          const auto peripherals = DecodePeripheralConfigurations (json["peripherals"]);
          return CreateObject<EntityConfiguration> (netDevices, mobilityModel, applications, mechanics, battery, peripherals);
        }
      return CreateObject<EntityConfiguration> (netDevices, mobilityModel, applications, mechanics, battery);
    }
  else
    {
      return CreateObject<EntityConfiguration> (netDevices, mobilityModel, applications);
    }

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
  for (auto& netdev : json.GetArray ())
    {
      NS_ASSERT_MSG (netdev.IsObject (),
                    "Every Entity Network Device configuration must be an object.");
      NS_ASSERT_MSG (netdev.HasMember ("type"),
                    "Entity Network Device must have 'type' property defined.");
      NS_ASSERT_MSG (netdev["type"].IsString (),
                    "Entity Network Device 'type' property must be a string.");

      const std::string type = netdev["type"].GetString ();

      NS_ASSERT_MSG (netdev.HasMember ("networkLayer"),
                     "Entity Network Device must have 'networkLayer' property defined.");
      NS_ASSERT_MSG (netdev["networkLayer"].IsUint (),
                     "Entity Network Device 'networkLayer' property must be an unsigned integer.");

      const uint32_t networkLayerId = netdev["networkLayer"].GetUint ();

      if (type.compare("wifi") == 0)
        {
          NS_ASSERT_MSG (netdev.HasMember ("macLayer"),
                        "Entity WiFi Network Device must have 'macLayer' property defined.");
          NS_ASSERT_MSG (netdev["macLayer"].IsObject (),
                        "Entity WiFi Network Device 'macLayer' property must be an object.");

          const auto macLayer = DecodeModelConfiguration (netdev["macLayer"]);

          confs.push_back (CreateObject<WifiNetdeviceConfiguration> (type, macLayer, networkLayerId));
        }
      else if (type.compare ("lte") == 0)
        {
          NS_ASSERT_MSG (netdev.HasMember ("role"),
                         "Entity LTE Network Device must have 'role' property defined.");
          NS_ASSERT_MSG (netdev["role"].IsString (),
                         "Entity LTE Network Device 'role' property must be a string.");

          const std::string role = netdev["role"].GetString ();

          NS_ASSERT_MSG (netdev.HasMember ("bearers"),
                         "Entity LTE Network Device must have 'bearers' property defined.");
          NS_ASSERT_MSG (netdev["bearers"].IsArray (),
                         "Entity LTE Network Device 'bearers' must be an array.");

          const auto bearers = DecodeLteBearerConfigurations (netdev["bearers"].GetArray ());

          confs.push_back (CreateObject<LteNetdeviceConfiguration> (type, role, bearers, networkLayerId));
        }
      else
        {
          NS_FATAL_ERROR ("Entity Network Device of Type " << type << " is not supported!");
        }
    }

  return confs;
}

const std::vector<LteBearerConfiguration>
EntityConfigurationHelper::DecodeLteBearerConfigurations (const JsonArray& jsonArray)
{
  auto bearers = std::vector<LteBearerConfiguration> ();

  for (auto& bearerConf : jsonArray)
    {
      NS_ASSERT_MSG (bearerConf.HasMember ("type"),
                     "Entity LTE Bearer configuration must have 'type' property defined.");
      NS_ASSERT_MSG (bearerConf["type"].IsString (),
                     "Entity LTE Bearer configuration 'type' must be an array.");
      NS_ASSERT_MSG (bearerConf.HasMember ("bitrate"),
                     "Entity LTE Bearer configuration must have 'bitrate' property defined.");
      NS_ASSERT_MSG (bearerConf["bitrate"].IsObject (),
                     "Entity LTE Bearer configuration 'bitrate' must be an object.");
      NS_ASSERT_MSG (bearerConf["bitrate"].HasMember ("guaranteed"),
                     "Entity LTE Bearer configuration bitrate must have 'guaranteed' property defined.");
      NS_ASSERT_MSG (bearerConf["bitrate"]["guaranteed"].IsObject (),
                     "Entity LTE Bearer configuration 'guaranteed' bitrate must be an object.");
      NS_ASSERT_MSG (bearerConf["bitrate"]["guaranteed"].HasMember ("downlink"),
                     "Entity LTE Bearer configuration guaranteed bitrate must have 'downlink' property defined.");
      NS_ASSERT_MSG (bearerConf["bitrate"]["guaranteed"]["downlink"].IsDouble (),
                     "Entity LTE Bearer configuration 'downlink' guaranteed bitrate must be an unsigned integer.");
      NS_ASSERT_MSG (bearerConf["bitrate"]["guaranteed"].HasMember ("uplink"),
                     "Entity LTE Bearer configuration guaranteed bitrate must have 'uplink' property defined.");
      NS_ASSERT_MSG (bearerConf["bitrate"]["guaranteed"]["uplink"].IsDouble (),
                     "Entity LTE Bearer configuration 'uplink' guaranteed bitrate must be an unsigned integer.");
      NS_ASSERT_MSG (bearerConf["bitrate"].HasMember ("maximum"),
                     "Entity LTE Bearer configuration bitrate must have 'maximum' property defined.");
      NS_ASSERT_MSG (bearerConf["bitrate"]["maximum"].IsObject (),
                     "Entity LTE Bearer configuration 'maximum' bitrate must be an object.");
      NS_ASSERT_MSG (bearerConf["bitrate"]["maximum"].HasMember ("downlink"),
                     "Entity LTE Bearer configuration maximum bitrate must have 'downlink' property defined.");
      NS_ASSERT_MSG (bearerConf["bitrate"]["maximum"]["downlink"].IsDouble (),
                     "Entity LTE Bearer configuration 'downlink' maximum bitrate must be an unsigned integer.");
      NS_ASSERT_MSG (bearerConf["bitrate"]["maximum"].HasMember ("uplink"),
                     "Entity LTE Bearer configuration maximum bitrate must have 'uplink' property defined.");
      NS_ASSERT_MSG (bearerConf["bitrate"]["maximum"]["uplink"].IsDouble (),
                     "Entity LTE Bearer configuration 'uplink' maximum bitrate must be an unsigned integer.");

      const std::string type = bearerConf["type"].GetString ();
      const double gbrDl = bearerConf["bitrate"]["guaranteed"]["downlink"].GetDouble ();
      const double gbrUl = bearerConf["bitrate"]["guaranteed"]["uplink"].GetDouble ();
      const double mbrDl = bearerConf["bitrate"]["maximum"]["downlink"].GetDouble ();
      const double mbrUl = bearerConf["bitrate"]["maximum"]["uplink"].GetDouble ();

      NS_ASSERT_MSG (gbrDl >= 0.0 && gbrUl >= 0.0 && mbrDl >= 0.0 && mbrUl >= 0.0 &&
                     floor(gbrDl) == gbrDl && floor(gbrUl) == gbrUl && floor(mbrDl) == mbrDl && floor(mbrUl) == mbrUl,
                     "Bitrate must be a positive integral number.");

      bearers.push_back (LteBearerConfiguration (type, (uint64_t) gbrDl, (uint64_t) gbrUl, (uint64_t) mbrDl, (uint64_t) mbrUl));
    }

  return bearers;
}

const ModelConfiguration
EntityConfigurationHelper::DecodeMobilityConfiguration (const rapidjson::Value& json)
{
  NS_ASSERT_MSG (json.IsObject (),
                 "Entity mobility model configuration must be an object.");

  return DecodeModelConfiguration (json);
}

const std::vector<ModelConfiguration>
EntityConfigurationHelper::DecodeApplicationConfigurations (const rapidjson::Value& json)
{
  NS_ASSERT_MSG (json.IsArray (),
                 "Entity configuration 'applications' property must be an array.");

  std::vector<ModelConfiguration> confs;
  for (auto& appl : json.GetArray ())
    {
      NS_ASSERT_MSG (appl.IsObject (),
                    "Application model configuration must be an object.");

      confs.push_back (DecodeModelConfiguration(appl));
    }

  return confs;
}

const ModelConfiguration
EntityConfigurationHelper::DecodeMechanicsConfiguration (const rapidjson::Value& json)
{
  NS_ASSERT_MSG (json.IsObject (),
                 "Entity mechanics configuration must be an object.");

  return DecodeModelConfiguration (json);
}

const ModelConfiguration
EntityConfigurationHelper::DecodeBatteryConfiguration (const rapidjson::Value& json)
{
  NS_ASSERT_MSG (json.IsObject (),
                 "Entity battery configuration must be an object.");

  return DecodeModelConfiguration (json);
}

const std::vector<ModelConfiguration>
EntityConfigurationHelper::DecodePeripheralConfigurations (const rapidjson::Value& json)
{
  NS_ASSERT_MSG (json.IsArray (),
                 "Entity configuration 'peripherals' property must be an array.");

  std::vector<ModelConfiguration> confs;
  for (auto& peripheral : json.GetArray ())
    {
      NS_ASSERT_MSG (peripheral.IsObject (),
                    "Peripheral model configuration must be an object.");

      confs.push_back (DecodeModelConfiguration(peripheral));
    }

  return confs;
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

  for (auto& el : jsonAttributes)
    {
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

      NS_ASSERT_MSG (modelTid.LookupAttributeByName (attrName, &attrInfo),
                     "Attribute '" << attrName << "' for model '" << modelName << "' does not exist.");

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
            if (el["value"].IsInt ())
              {
                const int attrValueInt = el["value"].GetInt ();
                attrValue = attrInfo.checker->CreateValidValue (IntegerValue (attrValueInt));
              }
            else if (el["value"].IsDouble ())
              {
                const double attrValueDouble = el["value"].GetDouble ();

                if (attrInfo.checker->Check (DoubleValue (attrValueDouble)))
                  attrValue = attrInfo.checker->CreateValidValue (DoubleValue (attrValueDouble));
                else if (attrInfo.checker->Check (TimeValue (Seconds (attrValueDouble))))
                  attrValue = attrInfo.checker->CreateValidValue (TimeValue (Seconds (attrValueDouble)));
                else
                  NS_FATAL_ERROR ("Cannot read attribute " << attrName << " for model " << modelName);
              }
          }
          break;
        case rapidjson::Type::kArrayType:
          {
            const auto arr = el["value"].GetArray ();

            if (arr.Size () == 3 && arr[0].IsDouble ())
              {
                const Vector3D vec {arr[0].GetDouble (), arr[1].GetDouble (), arr[2].GetDouble ()};
                attrValue = attrInfo.checker->CreateValidValue (Vector3DValue (vec));
              }
            else if (attrName.compare("SpeedCoefficients") == 0 && arr[0].IsNumber ())
              {
                std::vector<double> coeffs;

                for (auto& c : arr) {
                  coeffs.push_back (c.GetDouble ());
                }
                attrValue = attrInfo.checker->CreateValidValue (SpeedCoefficientsValue (coeffs));
              }
            else if (attrName.compare("FlightPlan") == 0 && arr[0].IsObject ())
              {
                FlightPlan fp {};

                for (auto& p : arr)
                  {
                    NS_ASSERT_MSG (p.IsObject () &&
                                   p.HasMember ("position") &&
                                   p["position"].IsArray () &&
                                   p["position"][0].IsDouble () &&
                                   p.HasMember ("interest") &&
                                   p["interest"].IsUint (),
                                   "FlightPlan contains invalid points.");

                    const Vector3D position { p["position"][0].GetDouble (),
                                              p["position"][1].GetDouble (),
                                              p["position"][2].GetDouble () };
                    const double restTime = (p.HasMember ("restTime") && p["restTime"].IsDouble ())
                                            ? p["restTime"].GetDouble ()
                                            : 0;
                    fp.Add (CreateObjectWithAttributes<ProtoPoint> ("Position", VectorValue (position),
                                                                    "Interest", IntegerValue (p["interest"].GetUint ()),
                                                                    "RestTime", TimeValue (Seconds (restTime))));
                  }

                attrValue = attrInfo.checker->CreateValidValue (FlightPlanValue (fp));
              }
            else
              {
                NS_FATAL_ERROR ("Unsupported attribute value type of " << attrName);
              }
          }
          break;
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

} // namespace ns3
