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
 *
 * Authors: Giovanni Grieco <giovanni.grieco@poliba.it>
 */
#ifndef WIFI_INSPECTOR_H
#define WIFI_INSPECTOR_H
#include <ns3/net-device.h>
#include <ns3/wifi-mac.h>
#include <ns3/wifi-net-device.h>
#include <ns3/wifi-phy.h>
#include <ns3/yans-wifi-channel.h>

#include <string>

namespace ns3
{

class WifiInspector
{
  public:
    /**
     * Initialize the inspector with a given Network Device
     */
    WifiInspector(Ptr<NetDevice> device);

    /**
     * Get a string representation of the Wifi Standard in use
     */
    const char* GetWifiStandard() const;
    /**
     * Get the carrier frequency used
     */
    const uint16_t GetCarrierFrequency() const;
    /**
     * Get the propagation loss model that has been set
     */
    const std::string GetPropagationLossModel() const;
    /**
     * Get the propagation delay model that has been set
     */
    const std::string GetPropagationDelayModel() const;
    /**
     * Get the wifi ssid that the device is connected to
     */
    const std::string GetWifiSsid() const;
    /**
     * Get the wifi mode of the device
     */
    const std::string GetWifiMode() const;

  private:
    /**
     * Internal helper to treat a NetDevice as a WifiNetDevice
     */
    static Ptr<WifiNetDevice> InitializeNetDevice(Ptr<NetDevice> dev);
    /**
     * Internal helper to get the Phy Layer of a given WifiNetDevice
     */
    static Ptr<WifiPhy> InitializePhy(Ptr<WifiNetDevice> dev);
    /**
     * Internal helper to get the Wifi Channel of a given WifiNetDevice
     */
    static Ptr<YansWifiChannel> InitializeChannel(Ptr<WifiNetDevice> dev);
    /**
     * Internal helper to get the MAC Layer of a given WifiNetDevice
     */
    static Ptr<WifiMac> InitializeMac(Ptr<WifiNetDevice> dev);

    Ptr<WifiNetDevice> m_dev;       /// Wifi Network Device to be inspected
    Ptr<WifiPhy> m_phy;             /// Wifi Phy Device, as cache
    Ptr<YansWifiChannel> m_channel; /// Wifi Channel, as cache
    Ptr<WifiMac> m_mac;             /// Wifi Mac, as cache
};

} // namespace ns3

#endif /* WIFI_INSPECTOR_H */
