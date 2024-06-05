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
#include "mac-layer-configuration-helper.h"

#include "model-configuration-helper.h"

#include <ns3/assert.h>
#include <ns3/double.h>
#include <ns3/fatal-error.h>
#include <ns3/null-ntn-demo-mac-layer-configuration.h>
#include <ns3/string.h>
#include <ns3/type-id.h>
#include <ns3/wifi-mac-layer-configuration.h>

namespace ns3
{

Ptr<MacLayerConfiguration>
MacLayerConfigurationHelper::GetConfiguration(const rapidjson::Value& jsonMacLayer)
{
    NS_ASSERT_MSG(jsonMacLayer.IsObject(),
                  "MAC Layer definition must be an object, got " << jsonMacLayer.GetType());
    NS_ASSERT_MSG(jsonMacLayer.HasMember("type"),
                  "MAC Layer definition must have 'type' property.");
    NS_ASSERT_MSG(jsonMacLayer["type"].IsString(), "MAC Layer 'type' property must be a string.");

    const std::string macType = jsonMacLayer["type"].GetString();
    Ptr<MacLayerConfiguration> macConfig{nullptr};

    if (macType == "wifi")
    {
        NS_ASSERT_MSG(jsonMacLayer.HasMember("ssid"),
                      "MAC Layer definition must have 'ssid' property.");
        NS_ASSERT_MSG(jsonMacLayer["ssid"].IsString(),
                      "MAC Layer 'ssid' property must be a string.");
        NS_ASSERT_MSG(jsonMacLayer["remoteStationManager"].IsObject(),
                      "MAC Layer 'remoteStationManager' property must be an object.");

        const auto remoteStationManagerConfiguration =
            ModelConfigurationHelper::Get(jsonMacLayer["remoteStationManager"]);

        macConfig = Create<WifiMacLayerConfiguration>(macType,
                                                      jsonMacLayer["ssid"].GetString(),
                                                      remoteStationManagerConfiguration);
    }
    else if (macType == "lte")
    {
        macConfig = Create<MacLayerConfiguration>(macType);
    }
    else if (macType == "NullNtnDemo")
    {
        NS_ASSERT_MSG(jsonMacLayer.HasMember("timeResolution"),
                      "MAC Layer definition must have 'timeResolution' property.");
        NS_ASSERT_MSG(jsonMacLayer["timeResolution"].IsNumber(),
                      "MAC Layer 'timeResolution' property must be a number.");
        NS_ASSERT_MSG(jsonMacLayer.HasMember("bandwidth"),
                      "MAC Layer definition must have 'bandwidth' property.");
        NS_ASSERT_MSG(jsonMacLayer["bandwidth"].IsNumber(),
                      "MAC Layer 'bandwidth' property must be a number.");
        NS_ASSERT_MSG(jsonMacLayer.HasMember("rbBandwidth"),
                      "MAC Layer definition must have 'rbBandwidth' property.");
        NS_ASSERT_MSG(jsonMacLayer["rbBandwidth"].IsNumber(),
                      "MAC Layer 'rbBandwidth' property must be a number.");
        NS_ASSERT_MSG(jsonMacLayer.HasMember("satEirpDensity"),
                      "MAC Layer definition must have 'satEirpDensity' property.");
        NS_ASSERT_MSG(jsonMacLayer["satEirpDensity"].IsNumber(),
                      "MAC Layer 'satEirpDensity' property must be a number.");
        NS_ASSERT_MSG(jsonMacLayer.HasMember("ueAntennaNoiseFigure"),
                      "MAC Layer definition must have 'ueAntennaNoiseFigure' property.");
        NS_ASSERT_MSG(jsonMacLayer["ueAntennaNoiseFigure"].IsNumber(),
                      "MAC Layer 'ueAntennaNoiseFigure' property must be a number.");

        macConfig = Create<NullNtnDemoMacLayerConfiguration>(
            macType,
            jsonMacLayer["timeResolution"].GetDouble(),
            jsonMacLayer["bandwidth"].GetDouble(),
            jsonMacLayer["rbBandwidth"].GetDouble(),
            jsonMacLayer["satEirpDensity"].GetDouble(),
            jsonMacLayer["ueAntennaNoiseFigure"].GetDouble());
    }
    else
    {
        NS_FATAL_ERROR("MAC Layer of Type " << macType << " is not supported!");
    }

    return macConfig;
}

MacLayerConfigurationHelper::MacLayerConfigurationHelper()
{
}

} // namespace ns3
