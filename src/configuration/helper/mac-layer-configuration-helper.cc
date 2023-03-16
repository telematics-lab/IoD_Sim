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
#include "mac-layer-configuration-helper.h"

#include <ns3/assert.h>
#include <ns3/double.h>
#include <ns3/fatal-error.h>
#include <ns3/string.h>
#include <ns3/type-id.h>

#include <ns3/wifi-mac-layer-configuration.h>

#include "model-configuration-helper.h"

namespace ns3 {

Ptr<MacLayerConfiguration>
MacLayerConfigurationHelper::GetConfiguration (const rapidjson::Value& jsonMacLayer)
{
  NS_ASSERT_MSG (jsonMacLayer.IsObject (),
                 "MAC Layer definition must be an object, got " << jsonMacLayer.GetType ());
  NS_ASSERT_MSG (jsonMacLayer.HasMember ("type"),
                 "MAC Layer definition must have 'type' property.");
  NS_ASSERT_MSG (jsonMacLayer["type"].IsString (),
                 "MAC Layer 'type' property must be a string.");

  const std::string macType = jsonMacLayer["type"].GetString ();
  if (macType == "wifi")
    {
      NS_ASSERT_MSG (jsonMacLayer.HasMember ("ssid"),
                    "MAC Layer definition must have 'ssid' property.");
      NS_ASSERT_MSG (jsonMacLayer["ssid"].IsString (),
                    "MAC Layer 'ssid' property must be a string.");
      NS_ASSERT_MSG (jsonMacLayer["remoteStationManager"].IsObject (),
                  "MAC Layer 'remoteStationManager' property must be an object.");

      const auto remoteStationManagerConfiguration = ModelConfigurationHelper::Get (jsonMacLayer["remoteStationManager"]);

      return Create<WifiMacLayerConfiguration> (macType,
                                                jsonMacLayer["ssid"].GetString (),
                                                remoteStationManagerConfiguration);
    }
  else if (macType == "lte")
    {
      return Create<MacLayerConfiguration> (macType);
    }
  else
    {
      NS_FATAL_ERROR ("MAC Layer of Type " << macType << " is not supported!");
    }
}

MacLayerConfigurationHelper::MacLayerConfigurationHelper ()
{

}

} // namespace ns3
