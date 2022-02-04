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
#include "remote-configuration-helper.h"

#include <ns3/double.h>
#include <ns3/integer.h>
#include <ns3/object-factory.h>
#include <ns3/string.h>
#include <ns3/vector.h>

#include <ns3/flight-plan.h>
#include <ns3/lte-netdevice-configuration.h>
#include <ns3/wifi-netdevice-configuration.h>
#include <ns3/double-vector.h>
#include <ns3/int-vector.h>

namespace ns3 {

Ptr<RemoteConfiguration>
RemoteConfigurationHelper::GetConfiguration (const rapidjson::Value& json)
{
  NS_ASSERT_MSG (json.IsObject (),
                 "Remote configuration must be an object.");
  NS_ASSERT_MSG (json.HasMember ("networkLayer"),
                 "Remote configuration must have 'networkLayer' property defined.");
  NS_ASSERT_MSG (json.HasMember ("applications"),
                 "Remote configuration must have 'applications' property defined.");

  const auto networkLayerId = DecodeNetworkLayerId (json["networkLayer"]);
  const auto applications = DecodeApplicationConfigurations (json["applications"]);

  return CreateObject<RemoteConfiguration> (networkLayerId, applications);

}

RemoteConfigurationHelper::RemoteConfigurationHelper ()
{

}

const uint32_t
RemoteConfigurationHelper::DecodeNetworkLayerId (const rapidjson::Value& json)
{
  NS_ASSERT_MSG (json.IsInt () && json.GetInt () >= 0,
                 "Remote configuration 'networkLayer' property must be a positive integer.");

  return json.GetInt ();
}

const std::vector<ModelConfiguration>
RemoteConfigurationHelper::DecodeApplicationConfigurations (const rapidjson::Value& json)
{
  NS_ASSERT_MSG (json.IsArray (),
                 "Remote configuration 'applications' property must be an array.");

  std::vector<ModelConfiguration> confs;
  for (auto& appl : json.GetArray ())
    {
      NS_ASSERT_MSG (appl.IsObject (),
                    "Application model configuration must be an object.");

      confs.push_back (DecodeModelConfiguration(appl));
    }

  return confs;
}

// TODO merge with other helpers
const ModelConfiguration
RemoteConfigurationHelper::DecodeModelConfiguration (const rapidjson::Value& jsonModel)
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

                // maybe an unsigned integer?
                if (attrValue == nullptr)
                  {
                    attrValue = attrInfo.checker->CreateValidValue (UintegerValue (attrValueInt));
                  }
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
            else if (attrName == "SpeedCoefficients" && arr[0].IsNumber ())
              {
                std::vector<double> coeffs;

                for (auto& c : arr) {
                  coeffs.push_back (c.GetDouble ());
                }
                attrValue = attrInfo.checker->CreateValidValue (DoubleVectorValue (coeffs));
              }
            else if (attrName == "FlightPlan" && arr[0].IsObject ())
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
