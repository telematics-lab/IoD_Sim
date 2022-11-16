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
#include "model-configuration-helper.h"

#include <ns3/assert.h>
#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/double-vector.h>
#include <ns3/fatal-error.h>
#include <ns3/flight-plan.h>
#include <ns3/int-vector.h>
#include <ns3/integer.h>
#include <ns3/nstime.h>
#include <ns3/object-factory.h>
#include <ns3/rectangle.h>
#include <ns3/string.h>
#include <ns3/uinteger.h>
#include <ns3/vector.h>

namespace ns3 {

const ModelConfiguration
ModelConfigurationHelper::Get (const JsonValue& jModel)
{
  NS_ASSERT_MSG (jModel.HasMember ("name"),
                 "Model configuration must have 'name' property.");
  NS_ASSERT_MSG (jModel["name"].IsString (),
                 "Model configuration 'name' property must be a string.");
  NS_ASSERT_MSG (jModel.HasMember ("attributes"),
            	   "Model configuration must have 'attributes' property.");
  NS_ASSERT_MSG (jModel["attributes"].IsArray (),
                  "Model configuration 'attributes' property must be an array.");

  const std::string modelName = jModel["name"].GetString ();
  const TypeId modelTid = TypeId::LookupByName (modelName);
  const auto jsonAttributes = jModel["attributes"].GetArray ();
  const auto attributes = GetAttributes (modelTid, jsonAttributes);

  return ModelConfiguration (modelName, attributes);
}

ModelConfigurationHelper::ModelConfigurationHelper ()
{

}

const std::vector<ModelConfiguration::Attribute>
ModelConfigurationHelper::GetAttributes (const TypeId& model, const rapidjson::Value::ConstArray& jAttrs)
{
  std::vector<ModelConfiguration::Attribute> attributes;
  attributes.reserve (jAttrs.Size ());
  for (auto& el : jAttrs)
      attributes.push_back (DecodeModelAttribute (model, el));

  return attributes;
}

const ModelConfiguration::Attribute
ModelConfigurationHelper::DecodeModelAttribute (const TypeId& model, const rapidjson::Value& el)
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

  NS_ASSERT_MSG (model.LookupAttributeByName (attrName, &attrInfo),
                "Attribute '" << attrName << "' for model '" << model.GetName () << "' does not exist!");

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
        if (el["value"].IsInt64 ())
          {
            const auto attrValueInt = el["value"].GetInt64 ();
            const auto acceptedType = attrInfo.checker->GetValueTypeName ();

            if (acceptedType == "ns3::IntegerValue")
              attrValue = attrInfo.checker->CreateValidValue (IntegerValue (attrValueInt));
            else if (acceptedType == "ns3::UintegerValue")
              attrValue = attrInfo.checker->CreateValidValue (UintegerValue (attrValueInt));
            else
              NS_FATAL_ERROR ("The attribute value for property " << attrName << " defined in model " << model.GetName ()
                              << " has incompatible type: " << acceptedType << " is needed.");
          }
        else if (el["value"].IsUint64 ())
          {
            const auto attrValueInt = el["value"].GetUint64 ();
            const auto acceptedType = attrInfo.checker->GetValueTypeName ();

            if (acceptedType == "ns3::IntegerValue")
              attrValue = attrInfo.checker->CreateValidValue (IntegerValue (attrValueInt));
            else if (acceptedType == "ns3::UintegerValue")
              attrValue = attrInfo.checker->CreateValidValue (UintegerValue (attrValueInt));
            else
              NS_FATAL_ERROR ("The attribute value for property " << attrName << " defined in model " << model.GetName ()
                              << " has incompatible type: " << acceptedType << " is needed.");
          }
        else if (el["value"].IsDouble ())
          {
            const double attrValueDouble = el["value"].GetDouble ();

            if (attrInfo.checker->Check (DoubleValue (attrValueDouble)))
              attrValue = attrInfo.checker->CreateValidValue (DoubleValue (attrValueDouble));
            else if (attrInfo.checker->Check (TimeValue (Seconds (attrValueDouble))))
              attrValue = attrInfo.checker->CreateValidValue (TimeValue (Seconds (attrValueDouble)));
            else
              NS_FATAL_ERROR ("Cannot read attribute " << attrName << " for model " << model.GetName ());
          }
        else
          {
            NS_FATAL_ERROR ("Cannot read attribute " << attrName << " for model " << model.GetName () << ". "
                            << "A number was detected, but it is not a valid Integer neither a Double.");
          }
      }
      break;
    case rapidjson::Type::kArrayType:
      {
        const auto arr = el["value"].GetArray ();

	if (arr.Size() <= 0)
	    break;

        if (arr.Size () == 3 && arr[0].IsDouble ())
          {
            const Vector3D vec {arr[0].GetDouble (), arr[1].GetDouble (), arr[2].GetDouble ()};
            attrValue = attrInfo.checker->CreateValidValue (Vector3DValue (vec));
          }
        else if ((attrName == "SpeedCoefficients" || attrName == "PowerConsumption") && arr[0].IsNumber ())
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
        else if (attrName == "RoITrigger" && arr[0].IsInt ())
          {
            std::vector<int> coeffs;

            for (auto& c : arr) {
              coeffs.push_back (c.GetDouble ());
            }
            attrValue = attrInfo.checker->CreateValidValue (IntVectorValue (coeffs));
          }
        else if (attrName == "Bounds" && arr[0].IsDouble() && arr.Size () == 4)
          {
            auto rect = Rectangle (arr[0].GetDouble (), arr[1].GetDouble (),
                                    arr[2].GetDouble (), arr[3].GetDouble ());
            attrValue = attrInfo.checker->CreateValidValue (RectangleValue (rect));
          }
        else
          {
            NS_FATAL_ERROR ("Unsupported attribute value type of " << attrName);
          }
      }
      break;
    case rapidjson::Type::kFalseType:
    case rapidjson::Type::kTrueType:
      {
        const auto attrValueBool = el["value"].GetBool ();
        attrValue = attrInfo.checker->CreateValidValue (BooleanValue (attrValueBool));
      }
      break;
    case rapidjson::Type::kNullType:
    case rapidjson::Type::kObjectType:
    default:
      NS_FATAL_ERROR ("Cannot determine attribute value type of " << attrName);
      break;
    }

  NS_ABORT_MSG_IF (attrValue == nullptr, "The attribute value for property " << attrName
                                          << " defined in model " << model.GetName () << " was not accepted. "
                                          << "Insert a valid value according to "
                                          << attrInfo.checker->GetUnderlyingTypeInformation ());

  return ModelConfiguration::Attribute (attrName, attrValue);
}

} // namespace ns3
