/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2026 The IoD_Sim Authors.
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

#include "mac-layer-configuration.h"

#include <ns3/model-configuration.h>

#include <string>

namespace ns3
{

/**
 * \brief Data class to store information about the WiFi MAC Layer of a Scenario.
 */
class WifiMacLayerConfiguration : public MacLayerConfiguration
{
  public:
    /**
     * Create a new object instance.
     *
     * \param macType The type of the MAC Layer to be configured. It should be "wifi".
     * \param ssid The name of the ssid to configure the BSS.
     * \param remoteStationManagerConfiguration The configuration of the Remote Station Manager to
     * set up the BSS.
     */
    WifiMacLayerConfiguration(std::string macType,
                              std::string ssid,
                              ModelConfiguration remoteStationManagerConfiguration);
    /**
     * \return The configured SSID.
     */
    const std::string GetSsid() const;
    /**
     * \return The Remote Station Manager model configuration.
     */
    const ModelConfiguration GetRemoteStationManagerConfiguration() const;

  private:
    const std::string m_ssid;
    const ModelConfiguration m_remoteStationManagerConfiguration;
};

} // namespace ns3

#endif /* WIFI_MAC_LAYER_CONFIGURATION_H */
