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
#include <ns3/speed-coefficients.h>

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
  for (auto& appl : json.GetArray ()){
    NS_ASSERT_MSG (appl.IsObject (),
                   "Application model configuration must be an object.");

    confs.push_back (DecodeModelConfiguration(appl));
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
        if (el["value"].IsInt ()) {
          const int attrValueInt = el["value"].GetInt ();

          attrValue = attrInfo.checker->CreateValidValue (IntegerValue (attrValueInt));

        } else if (el["value"].IsDouble ()) {
          const double attrValueDouble = el["value"].GetDouble ();

          if (attrInfo.checker->Check (DoubleValue (attrValueDouble))) {
            attrValue = attrInfo.checker->CreateValidValue (DoubleValue (attrValueDouble));
          } else if (attrInfo.checker->Check (TimeValue ( Seconds (attrValueDouble)))) {
            attrValue = attrInfo.checker->CreateValidValue (TimeValue ( Seconds (attrValueDouble)));
          } else {
            NS_FATAL_ERROR ("Cannot read attribute " << attrName << " for model " << modelName);
          }
        }
      }
      break;
    case rapidjson::Type::kArrayType:
      {
        const auto arr = el["value"].GetArray ();

        if (arr.Size () == 3 && arr[0].IsDouble ()) {
          const Vector3D vec {arr[0].GetDouble (), arr[1].GetDouble (), arr[2].GetDouble ()};

          attrValue = attrInfo.checker->CreateValidValue (Vector3DValue (vec));
        } else if (attrName.compare("SpeedCoefficients") == 0 && arr[0].IsNumber ()) {
          std::vector<double> coeffs;

          for (auto& c : arr) {
            coeffs.push_back (c.GetDouble ());
          }
          attrValue = attrInfo.checker->CreateValidValue (SpeedCoefficientsValue (coeffs));
        } else if (attrName.compare("FlightPlan") == 0 && arr[0].IsObject ()) {
          FlightPlan fp {};

          for (auto& p : arr) {
            NS_ASSERT_MSG (p.IsObject () &&
                           p.HasMember ("position") &&
                           p["position"].IsArray () &&
                           p["position"][0].IsDouble () &&
                           p.HasMember ("interest") &&
                           p["interest"].IsUint (),
                           "FlightPlan contains invalid points.");

            const Vector3D position { p["position"][0].GetDouble (), p["position"][1].GetDouble (), p["position"][2].GetDouble () };
            const double restTime = (p.HasMember ("restTime") && p["restTime"].IsDouble ()) ? p["restTime"].GetDouble () : 0;
            fp.Add (CreateObjectWithAttributes<ProtoPoint> ("Position", VectorValue (position),
                                                            "Interest", IntegerValue (p["interest"].GetUint ()),
                                                            "RestTime", TimeValue (Seconds (restTime))));
          }

          attrValue = attrInfo.checker->CreateValidValue (FlightPlanValue (fp));
        } else {
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
