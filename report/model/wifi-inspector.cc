/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 The IoD_Sim Authors.
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

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WifiInspector");

WifiInspector::WifiInspector (Ptr<NetDevice> device) :
  m_dev {InitializeNetDevice (device)},
  m_phy {InitializePhy (m_dev)},
  m_channel {InitializeChannel (m_dev)},
  m_mac {InitializeMac (m_dev)}
{
  NS_LOG_FUNCTION (this << device);
}

WifiInspector::~WifiInspector ()
{
  NS_LOG_FUNCTION (this);
}

const char*
WifiInspector::GetWifiStandard () const
{
  NS_LOG_FUNCTION_NOARGS ();

  const auto standard = m_phy->GetStandard ();

  switch (standard)
    {
      case WIFI_PHY_STANDARD_80211a:
        return "802.11a";
      case WIFI_PHY_STANDARD_80211b:
        return "802.11b";
      case WIFI_PHY_STANDARD_80211g:
        return "802.11g";
      case WIFI_PHY_STANDARD_80211n_2_4GHZ:
      case WIFI_PHY_STANDARD_80211n_5GHZ:
        return "802.11n";
      case WIFI_PHY_STANDARD_80211ac:
        return "802.11ac";
      case WIFI_PHY_STANDARD_80211ax_2_4GHZ:
      case WIFI_PHY_STANDARD_80211ax_5GHZ:
        return "802.11ax";
      default:
        return "UNSUPPORTED";
    }
}

const uint16_t
WifiInspector::GetCarrierFrequency () const
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_phy->GetFrequency ();
}

const std::string
WifiInspector::GetPropagationLossModel () const
{
  NS_LOG_FUNCTION_NOARGS ();
  const TypeId channelTid = m_channel->GetInstanceTypeId ();
  TypeId::AttributeInformation info;
  PointerValue attribute;

  if (!channelTid.LookupAttributeByName ("PropagationLossModel", &info))
    NS_FATAL_ERROR ("Wifi Channel was expected to have PropagationLossModel as "
                    "attribute, but instead it does not exist!");

  m_channel->GetAttribute (info.name, attribute);

  return attribute.GetObject ()->GetInstanceTypeId ().GetName ();
}

const std::string
WifiInspector::GetPropagationDelayModel () const
{
  NS_LOG_FUNCTION_NOARGS ();
  const TypeId channelTid = m_channel->GetInstanceTypeId ();
  TypeId::AttributeInformation info;
  PointerValue attribute;

  if (!channelTid.LookupAttributeByName ("PropagationDelayModel", &info))
    NS_FATAL_ERROR ("Wifi Channel was expected to have PropagationDelayModel as "
                    "attribute, but instead it does not exist!");

  m_channel->GetAttribute (info.name, attribute);

  return attribute.GetObject ()->GetInstanceTypeId ().GetName ();
}

const std::string
WifiInspector::GetWifiSsid () const
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_mac->GetSsid ().PeekString ();
}

const std::string
WifiInspector::GetWifiMode () const
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_dev->GetRemoteStationManager ()->GetDefaultMode ().GetUniqueName ();
}

Ptr<WifiNetDevice>
WifiInspector::InitializeNetDevice (Ptr<NetDevice> dev)
{
  NS_LOG_FUNCTION (dev);
  Ptr<WifiNetDevice> wifiNetDevice = DynamicCast<WifiNetDevice> (dev);

  if (wifiNetDevice == nullptr)
    NS_FATAL_ERROR ("Cannot inspect an object that is not a WifiNetDevice");

  return wifiNetDevice;
}

Ptr<WifiPhy>
WifiInspector::InitializePhy (Ptr<WifiNetDevice> dev)
{
  NS_LOG_FUNCTION_NOARGS ();
  const TypeId devTid = dev->GetInstanceTypeId ();
  TypeId::AttributeInformation info;
  PointerValue attribute;
  Ptr<WifiPhy> phy;

  if (!devTid.LookupAttributeByName ("Phy", &info))
    NS_FATAL_ERROR ("Wifi Device was expected to have Phy as attribute, but "
                    "instead it does not exist!");

  dev->GetAttribute (info.name, attribute);
  phy = DynamicCast<WifiPhy> (attribute.GetObject ());
  if (phy == nullptr)
    NS_FATAL_ERROR ("Cannot inspect and object that is not a WifiPhy.");

  return phy;
}

Ptr<YansWifiChannel>
WifiInspector::InitializeChannel (Ptr<WifiNetDevice> dev)
{
  NS_LOG_FUNCTION_NOARGS ();
  const TypeId devTid = dev->GetInstanceTypeId ();
  TypeId channelTid;
  TypeId::AttributeInformation info;
  PointerValue attribute;
  Ptr<Object> channelObject;

  if (!devTid.LookupAttributeByName ("Channel", &info))
    NS_FATAL_ERROR ("Device was expected to have Channel as attribute, but "
                    "instead it does not exist!");

  dev->GetAttribute (info.name, attribute);
  channelObject = attribute.GetObject ();
  channelTid = channelObject->GetInstanceTypeId ();

  if (channelTid.GetName () != "ns3::YansWifiChannel")
    NS_FATAL_ERROR ("Unsupported channel type " << channelTid.GetName ());

  return DynamicCast<YansWifiChannel> (channelObject);
}

Ptr<WifiMac>
WifiInspector::InitializeMac (Ptr<WifiNetDevice> dev)
{
  NS_LOG_FUNCTION_NOARGS ();
  const TypeId devTid = dev->GetInstanceTypeId ();
  TypeId::AttributeInformation info;
  PointerValue attribute;
  Ptr<WifiMac> mac;

  if (!devTid.LookupAttributeByName ("Mac", &info))
    NS_FATAL_ERROR ("Wifi Device was expected to have Mac as attribute, but "
                    "instead it does not exist!");

  dev->GetAttribute (info.name, attribute);
  mac = DynamicCast<WifiMac> (attribute.GetObject ());
  if (mac == nullptr)
    NS_FATAL_ERROR ("Cannot inspect and object that is not a WifiMac.");

  return mac;
}

} // namespace ns3
