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
#include "phy-layer-configuration-helper.h"

#include <ns3/assert.h>
#include <ns3/double.h>
#include <ns3/fatal-error.h>
#include <ns3/string.h>
#include <ns3/type-id.h>

#include <ns3/lte-phy-layer-configuration.h>
#include <ns3/wifi-phy-layer-configuration.h>

namespace ns3 {

Ptr<PhyLayerConfiguration>
PhyLayerConfigurationHelper::GetConfiguration (const rapidjson::Value& jsonPhyLayer)
{
  NS_ASSERT_MSG (jsonPhyLayer.IsObject (),
                 "PHY Layer definition must be an object, got " << jsonPhyLayer.GetType ());
  NS_ASSERT_MSG (jsonPhyLayer.HasMember ("type"),
                 "PHY Layer definition must have 'type' property.");
  NS_ASSERT_MSG (jsonPhyLayer["type"].IsString (),
                 "PHY Layer 'type' property must be a string.");

  const std::string phyType = jsonPhyLayer["type"].GetString ();
  if (phyType == "wifi")
    {
      NS_ASSERT_MSG (jsonPhyLayer.HasMember ("standard"),
                    "PHY Layer definition must have 'standard' property.");
      NS_ASSERT_MSG (jsonPhyLayer["standard"].IsString (),
                    "PHY Layer 'standard' property must be a string.");
      NS_ASSERT_MSG (jsonPhyLayer.HasMember ("rxGain"),
                    "PHY Layer definition must have 'rxGain' property.");
      NS_ASSERT_MSG (jsonPhyLayer["rxGain"].IsDouble (),
                    "PHY Layer 'rxGain' property must be a double.");
      NS_ASSERT_MSG (jsonPhyLayer.HasMember ("channel"),
                    "PHY Layer definition must have 'channel' property.");
      NS_ASSERT_MSG (jsonPhyLayer["channel"].IsObject (),
                    "PHY Layer 'mode' property must be an object.");
      NS_ASSERT_MSG (jsonPhyLayer["channel"].HasMember ("propagationDelayModel"),
                    "PHY Layer channel definition must have 'propagationDelayModel' property.");
      NS_ASSERT_MSG (jsonPhyLayer["channel"]["propagationDelayModel"].IsObject (),
                    "PHY Layer channel 'propagationDelayModel' must be an object");
      NS_ASSERT_MSG (jsonPhyLayer["channel"].HasMember ("propagationLossModel"),
                    "PHY Layer channel definition must have 'propagationLossModel' property.");
      NS_ASSERT_MSG (jsonPhyLayer["channel"]["propagationLossModel"].IsObject (),
                    "PHY Layer channel 'propagationLossModel' must be an object");

      const auto propagationDelayModel = DecodeModelConfiguration (jsonPhyLayer["channel"]["propagationDelayModel"]);
      const auto propagationLossModel = DecodeModelConfiguration (jsonPhyLayer["channel"]["propagationLossModel"]);

      return Create<WifiPhyLayerConfiguration> (phyType,
                                                jsonPhyLayer["standard"].GetString (),
                                                jsonPhyLayer["rxGain"].GetDouble (),
                                                propagationDelayModel,
                                                propagationLossModel);
    }
  else if (phyType == "lte")
    {
      NS_ASSERT_MSG (jsonPhyLayer.HasMember ("channel"),
                    "PHY Layer definition must have 'channel' property.");
      NS_ASSERT_MSG (jsonPhyLayer["channel"].IsObject (),
                    "PHY Layer 'mode' property must be an object.");
      NS_ASSERT_MSG (jsonPhyLayer["channel"].HasMember ("propagationLossModel"),
                    "PHY Layer channel definition must have 'propagationLossModel' property.");
      NS_ASSERT_MSG (jsonPhyLayer["channel"]["propagationLossModel"].IsObject (),
                    "PHY Layer channel 'propagationLossModel' must be an object");
      NS_ASSERT_MSG (jsonPhyLayer["channel"].HasMember ("spectrumModel"),
                    "PHY Layer channel definition must have 'spectrumModel' property.");
      NS_ASSERT_MSG (jsonPhyLayer["channel"]["spectrumModel"].IsObject (),
                    "PHY Layer channel 'spectrumModel' must be an object");

      const auto propagationLossModel = DecodeModelConfiguration (jsonPhyLayer["channel"]["propagationLossModel"]);
      const auto spectrumModel = DecodeModelConfiguration (jsonPhyLayer["channel"]["spectrumModel"]);

      return Create<LtePhyLayerConfiguration> (phyType,
                                               propagationLossModel,
                                               spectrumModel);
    }
  else
    {
      NS_FATAL_ERROR ("PHY Layer of Type " << phyType << " is not supported!");
    }
}

PhyLayerConfigurationHelper::PhyLayerConfigurationHelper ()
{

}

const ModelConfiguration
PhyLayerConfigurationHelper::DecodeModelConfiguration (const rapidjson::Value& jsonModel)
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

} // namespace ns3
