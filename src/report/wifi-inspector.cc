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
 *
 * Authors: Giovanni Grieco <giovanni.grieco@poliba.it>
 */
#include "wifi-inspector.h"

#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/ssid.h>
#include <ns3/wifi-phy.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("WifiInspector");

WifiInspector::WifiInspector(Ptr<NetDevice> device)
    : m_dev{InitializeNetDevice(device)},
      m_phy{InitializePhy(m_dev)},
      m_channel{InitializeChannel(m_phy)},
      m_mac{InitializeMac(m_dev)}
{
}

const char*
WifiInspector::GetWifiStandard() const
{
    NS_LOG_FUNCTION_NOARGS();

    const auto standard = m_phy->GetStandard();

    switch (standard)
    {
    case WIFI_STANDARD_80211a:
        return "802.11a";
    case WIFI_STANDARD_80211b:
        return "802.11b";
    case WIFI_STANDARD_80211g:
        return "802.11g";
    case WIFI_STANDARD_80211n:
        return "802.11n";
    case WIFI_STANDARD_80211ac:
        return "802.11ac";
    case WIFI_STANDARD_80211ax:
        return "802.11ax";
    default:
        return "UNSUPPORTED";
    }
}

const uint16_t
WifiInspector::GetCarrierFrequency() const
{
    NS_LOG_FUNCTION_NOARGS();

    return m_phy->GetFrequency();
}

const std::string
WifiInspector::GetPropagationLossModel() const
{
    NS_LOG_FUNCTION_NOARGS();
    const TypeId channelTid = m_channel->GetInstanceTypeId();
    TypeId::AttributeInformation info;
    PointerValue attribute;

    if (!channelTid.LookupAttributeByName("PropagationLossModel", &info))
        NS_FATAL_ERROR("Wifi Channel was expected to have PropagationLossModel as "
                       "attribute, but instead it does not exist!");

    m_channel->GetAttribute(info.name, attribute);

    return attribute.GetObject()->GetInstanceTypeId().GetName();
}

const std::string
WifiInspector::GetPropagationDelayModel() const
{
    NS_LOG_FUNCTION_NOARGS();
    const TypeId channelTid = m_channel->GetInstanceTypeId();
    TypeId::AttributeInformation info;
    PointerValue attribute;

    if (!channelTid.LookupAttributeByName("PropagationDelayModel", &info))
        NS_FATAL_ERROR("Wifi Channel was expected to have PropagationDelayModel as "
                       "attribute, but instead it does not exist!");

    m_channel->GetAttribute(info.name, attribute);

    return attribute.GetObject()->GetInstanceTypeId().GetName();
}

const std::string
WifiInspector::GetWifiSsid() const
{
    NS_LOG_FUNCTION_NOARGS();

    return m_mac->GetSsid().PeekString();
}

const std::string
WifiInspector::GetWifiMode() const
{
    NS_LOG_FUNCTION_NOARGS();

    return m_dev->GetRemoteStationManager()->GetDefaultMode().GetUniqueName();
}

Ptr<WifiNetDevice>
WifiInspector::InitializeNetDevice(Ptr<NetDevice> dev)
{
    NS_LOG_FUNCTION(dev);
    Ptr<WifiNetDevice> wifiNetDevice = DynamicCast<WifiNetDevice>(dev);

    if (!wifiNetDevice)
        NS_FATAL_ERROR("Cannot inspect an object that is not a WifiNetDevice");

    return wifiNetDevice;
}

Ptr<WifiPhy>
WifiInspector::InitializePhy(Ptr<WifiNetDevice> dev)
{
    NS_LOG_FUNCTION_NOARGS();
    Ptr<WifiPhy> phy = dev->GetPhy();

    if (!phy)
        NS_FATAL_ERROR("Cannot inspect an object that is not a WifiPhy.");

    return phy;
}

Ptr<YansWifiChannel>
WifiInspector::InitializeChannel(Ptr<WifiPhy> phy)
{
    NS_LOG_FUNCTION_NOARGS();
    Ptr<YansWifiChannel> channel = DynamicCast<YansWifiChannel>(phy->GetChannel());

    if (!channel)
        NS_FATAL_ERROR("Unsupported channel type or channel not found");

    return channel;
}

Ptr<WifiMac>
WifiInspector::InitializeMac(Ptr<WifiNetDevice> dev)
{
    NS_LOG_FUNCTION_NOARGS();
    Ptr<WifiMac> mac = dev->GetMac();

    if (!mac)
        NS_FATAL_ERROR("Cannot inspect an object that is not a WifiMac.");

    return mac;
}

} // namespace ns3
