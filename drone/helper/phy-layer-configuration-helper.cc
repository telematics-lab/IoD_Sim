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
#include "phy-layer-configuration-helper.h"

#include <ns3/assert.h>
#include <ns3/fatal-error.h>
#include <ns3/string.h>
#include <ns3/type-id.h>
#include <ns3/wifi-phy-layer-configuration.h>

namespace ns3 {

Ptr<PhyLayerConfiguration>
PhyLayerConfigurationHelper::GetConfiguration (const rapidjson::Value& jsonPhyLayer)
{
  NS_ASSERT (jsonPhyLayer.IsObject () &&
             jsonPhyLayer.HasMember ("type") &&
             jsonPhyLayer["type"].IsString ());

  const std::string phyType = jsonPhyLayer["type"].GetString ();
  if (phyType.compare("wifi") == 0) {
    // TODO use NS_ASSERT_MSG to output clear error message
    NS_ASSERT (jsonPhyLayer.HasMember ("standard") &&
               jsonPhyLayer["standard"].IsString () &&
               jsonPhyLayer.HasMember ("rxGain") &&
               jsonPhyLayer["rxGain"].IsDouble () &&
               jsonPhyLayer.HasMember ("mode") &&
               jsonPhyLayer["mode"].IsString () &&
               jsonPhyLayer.HasMember ("channel") &&
               jsonPhyLayer["channel"].IsObject () &&
               jsonPhyLayer["channel"].HasMember ("propagationDelayModel") &&
               jsonPhyLayer["channel"]["propagationDelayModel"].IsObject () &&
               jsonPhyLayer["channel"].HasMember ("propagationLossModel") &&
               jsonPhyLayer["channel"]["propagationLossModel"].IsObject ());

    const auto propagationDelayModel = DecodeModelConfiguration (jsonPhyLayer["channel"]["propagationDelayModel"]);
    const auto propagationLossModel = DecodeModelConfiguration (jsonPhyLayer["channel"]["propagationLossModel"]);

    return Create<WifiPhyLayerConfiguration> (jsonPhyLayer["type"].GetString (),
                                              jsonPhyLayer["standard"].GetString (),
                                              jsonPhyLayer["rxGain"].GetDouble (),
                                              jsonPhyLayer["mode"].GetString (),
                                              propagationDelayModel,
                                              propagationLossModel);
  } else {
    NS_FATAL_ERROR ("PHY Layer of Type " << phyType << " is not supported!");
  }
}

PhyLayerConfigurationHelper::PhyLayerConfigurationHelper ()
{

}

const ModelConfiguration
PhyLayerConfigurationHelper::DecodeModelConfiguration (const rapidjson::Value& jsonModel)
{
  NS_ASSERT(jsonModel.HasMember ("name") &&
            jsonModel["name"].IsString () &&
            jsonModel.HasMember ("attributes") &&
            jsonModel["attributes"].IsArray ());

  const std::string modelName = jsonModel["name"].GetString ();
  TypeId modelTid = TypeId::LookupByName (modelName);
  auto jsonAttributes = jsonModel["attributes"].GetArray ();
  std::vector<std::pair<std::string, Ptr<AttributeValue>>> attributes;

  for (auto& el : jsonAttributes) {
    NS_ASSERT (el.IsObject () &&
               el.HasMember ("name") &&
               el["name"].IsString () &&
               el.HasMember ("value"));

    const std::string attrName = el["name"].GetString();
    const auto attrValueString = el["value"].GetString ();
    auto attrInfo = Create<TypeId::AttributeInformation> ();

    NS_ASSERT (modelTid.LookupAttributeByName (attrName, PeekPointer (attrInfo)));

    Ptr<AttributeValue> attrValue = attrInfo->checker->CreateValidValue (StringValue (attrValueString));

    attributes.emplace_back(std::make_pair(attrName, attrValue));
  }

  return ModelConfiguration(modelName, attributes);
}

} // namespace ns3
