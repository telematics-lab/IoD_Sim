/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2024 The IoD_Sim Authors.
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

#include "model-configuration-helper.h"

#include <ns3/assert.h>
#include <ns3/double.h>
#include <ns3/fatal-error.h>
#include <ns3/lte-phy-layer-configuration.h>
#include <ns3/none-phy-layer-configuration.h>
#include <ns3/nr-phy-layer-configuration.h>
#include <ns3/string.h>
#include <ns3/three-gpp-phy-layer-configuration.h>
#include <ns3/type-id.h>
#include <ns3/wifi-phy-layer-configuration.h>

namespace ns3
{

Ptr<PhyLayerConfiguration>
PhyLayerConfigurationHelper::GetConfiguration(const rapidjson::Value& jsonPhyLayer)
{
    NS_ASSERT_MSG(jsonPhyLayer.IsObject(),
                  "PHY Layer definition must be an object, got " << jsonPhyLayer.GetType());
    NS_ASSERT_MSG(jsonPhyLayer.HasMember("type"),
                  "PHY Layer definition must have 'type' property.");
    NS_ASSERT_MSG(jsonPhyLayer["type"].IsString(), "PHY Layer 'type' property must be a string.");

    const std::string phyType = jsonPhyLayer["type"].GetString();
    Ptr<PhyLayerConfiguration> phyConfig{nullptr};

    if (phyType == "wifi")
    {
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("standard"),
                      "Wi-Fi PHY Layer definition must have 'standard' property.");
        NS_ASSERT_MSG(jsonPhyLayer["standard"].IsString(),
                      "Wi-Fi PHY Layer 'standard' property must be a string.");
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("attributes"),
                      "Wi-Fi PHY Layer definition must have 'attributes' property.");
        NS_ASSERT_MSG(jsonPhyLayer["attributes"].IsArray(),
                      "Wi-Fi PHY Layer 'attributes' must be an object.");
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("channel"),
                      "Wi-Fi PHY Layer definition must have 'channel' property.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"].IsObject(),
                      "Wi-Fi PHY Layer 'mode' property must be an object.");
        NS_ASSERT_MSG(
            jsonPhyLayer["channel"].HasMember("propagationDelayModel"),
            "Wi-Fi PHY Layer channel definition must have 'propagationDelayModel' property.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"]["propagationDelayModel"].IsObject(),
                      "Wi-Fi PHY Layer channel 'propagationDelayModel' must be an object");
        NS_ASSERT_MSG(
            jsonPhyLayer["channel"].HasMember("propagationLossModel"),
            "Wi-Fi PHY Layer channel definition must have 'propagationLossModel' property.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"]["propagationLossModel"].IsObject(),
                      "Wi-Fi PHY Layer channel 'propagationLossModel' must be an object");

        const auto phyAttributes =
            ModelConfigurationHelper::GetAttributes(TypeId::LookupByName("ns3::WifiPhy"),
                                                    jsonPhyLayer["attributes"].GetArray());
        const auto propagationDelayModel =
            ModelConfigurationHelper::Get(jsonPhyLayer["channel"]["propagationDelayModel"]);
        const auto propagationLossModel =
            ModelConfigurationHelper::Get(jsonPhyLayer["channel"]["propagationLossModel"]);

        return Create<WifiPhyLayerConfiguration>(phyType,
                                                 jsonPhyLayer["standard"].GetString(),
                                                 phyAttributes,
                                                 propagationDelayModel,
                                                 propagationLossModel);
    }
    if (phyType == "none")
    {
        return Create<NonePhyLayerConfiguration>(phyType,
                                                 std::vector<ModelConfiguration::Attribute>());
    }
    else if (phyType == "lte")
    {
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("attributes"),
                      "LTE PHY Layer definition must have 'attributes' property.");
        NS_ASSERT_MSG(jsonPhyLayer["attributes"].IsArray(),
                      "LTE PHY Layer 'attributes' must be an object.");
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("channel"),
                      "LTE PHY Layer definition must have 'channel' property.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"].IsObject(),
                      "LTE PHY Layer 'mode' property must be an object.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"].HasMember("spectrumModel"),
                      "LTE PHY Layer channel definition must have 'spectrumModel' property.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"]["spectrumModel"].IsObject(),
                      "LTE PHY Layer channel 'spectrumModel' must be an object");

        const auto phyAttributes =
            ModelConfigurationHelper::GetAttributes(TypeId::LookupByName("ns3::LteHelper"),
                                                    jsonPhyLayer["attributes"].GetArray());
        const auto spectrumModel =
            ModelConfigurationHelper::Get(jsonPhyLayer["channel"]["spectrumModel"]);
        const auto propagationLossModel =
            ModelConfigurationHelper::GetOptional(jsonPhyLayer["channel"].GetObject(),
                                                  "propagationLossModel");

        phyConfig = Create<LtePhyLayerConfiguration>(phyType,
                                                     phyAttributes,
                                                     propagationLossModel,
                                                     spectrumModel);
    }
    else if (phyType == "3GPP")
    {
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("attributes"),
                      "3GPP PHY Layer definition must have 'attributes' property.");
        NS_ASSERT_MSG(jsonPhyLayer["attributes"].IsArray(),
                      "3GPP PHY Layer 'attributes' must be an object.");
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("channel"),
                      "3GPP PHY Layer definition must have 'channel' property.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"].IsObject(),
                      "3GPP PHY Layer 'mode' property must be an object.");
        NS_ASSERT_MSG(
            jsonPhyLayer["channel"].HasMember("propagationLossModel"),
            "3GPP PHY Layer channel definition must have 'propagationLossModel' property.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"]["propagationLossModel"].IsObject(),
                      "3GPP PHY Layer channel 'propagationLossModel' must be an object");
        NS_ASSERT_MSG(jsonPhyLayer["channel"].HasMember("conditionModel"),
                      "3GPP PHY Layer channel definition must have 'conditionModel' property.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"]["conditionModel"].IsObject(),
                      "3GPP PHY Layer channel 'conditionModel' must be an object");
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("environment"),
                      "3GPP PHY Layer definition must have 'environment' property.");

        const auto phyAttributes = ModelConfigurationHelper::GetAttributes(
            TypeId::LookupByName("ns3::ThreeGppChannelModel"),
            jsonPhyLayer["attributes"].GetArray());
        const auto propagationLossModel =
            ModelConfigurationHelper::Get(jsonPhyLayer["channel"]["propagationLossModel"]);
        const auto conditionModel =
            ModelConfigurationHelper::Get(jsonPhyLayer["channel"]["conditionModel"]);
        const auto environment = jsonPhyLayer["environment"].GetString();

        phyConfig = Create<ThreeGppPhyLayerConfiguration>(phyType,
                                                          phyAttributes,
                                                          propagationLossModel,
                                                          conditionModel,
                                                          environment);
    }
    else if (phyType == "nr")
    {
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("beamforming"),
                      "NR PHY Layer definition must have 'beamforming' property.");

        const auto phyAttributes =
            ModelConfigurationHelper::GetAttributes(TypeId::LookupByName("ns3::NrHelper"),
                                                    jsonPhyLayer["attributes"].GetArray());

        // For NR, use simplified configuration with default parameters
        phyConfig = Create<NrPhyLayerConfiguration>(phyType, phyAttributes, "UMi-StreetCanyon");
    }
    else
    {
        NS_FATAL_ERROR("PHY Layer of Type " << phyType << " is not supported!");
    }

    return phyConfig;
}

} // namespace ns3
