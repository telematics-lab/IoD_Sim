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
#ifndef WIFI_MAC_FACTORY_HELPER_H
#define WIFI_MAC_FACTORY_HELPER_H

#include <ns3/model-configuration.h>
#include <ns3/wifi-helper.h>

namespace ns3
{

/**
 * \brief Helper to enhance ns-3 WifiHelper functionalities with additional
 * features, without modifying objects external to IoD_Sim.
 */
class WifiMacFactoryHelper
{
  public:
    /**
     * WifiMacFactoryHelper can't be instantiated, as it is an utility class.
     */
    WifiMacFactoryHelper() = delete;
    /**
     * Set the wifi model to be used from a ModelConfiguration data class.
     *
     * \param helper The WifiHelper instance.
     * \param modelConf The configuration data class that defines the WiFi model to be used and its
     * configuration.
     */
    static void SetRemoteStationManager(WifiHelper& helper, const ModelConfiguration& modelConf);
};

} // namespace ns3

#endif /* WIFI_MAC_FACTORY_HELPER_H */
