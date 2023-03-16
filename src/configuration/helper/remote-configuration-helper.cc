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

#include "model-configuration-helper.h"

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

      confs.push_back (ModelConfigurationHelper::Get (appl));
    }

  return confs;
}

} // namespace ns3
