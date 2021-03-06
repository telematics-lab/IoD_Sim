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
#ifndef WIFI_MAC_LAYER_CONFIGURATION_H
#define WIFI_MAC_LAYER_CONFIGURATION_H

#include <string>

#include "mac-layer-configuration.h"
#include "model-configuration.h"

namespace ns3 {

class WifiMacLayerConfiguration : public MacLayerConfiguration
{
public:
  WifiMacLayerConfiguration (std::string macType,
                             std::string ssid,
                             ModelConfiguration remoteStationManagerConfiguration);
  ~WifiMacLayerConfiguration ();

  const std::string GetSsid () const;
  const ModelConfiguration GetRemoteStationManagerConfiguration () const;

private:
  const std::string m_ssid;
  const ModelConfiguration m_remoteStationManagerConfiguration;
};

} // namespace ns3

#endif /* WIFI_MAC_LAYER_CONFIGURATION_H */
