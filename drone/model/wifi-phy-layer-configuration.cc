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
#include "wifi-phy-layer-configuration.h"

#include <ns3/abort.h>

namespace ns3 {

WifiPhyLayerConfiguration::WifiPhyLayerConfiguration(std::string phyType,
                                                     std::string standard,
                                                     double rxGain,
                                                     ModelConfiguration channelPropagationDelayModel,
                                                     ModelConfiguration channelPropagationLossModel) :
  PhyLayerConfiguration {phyType},
  m_standard {standard},
  m_rxGain {rxGain},
  m_channelPropagationDelayModel {channelPropagationDelayModel},
  m_channelPropagationLossModel {channelPropagationLossModel}
{

}

WifiPhyLayerConfiguration::~WifiPhyLayerConfiguration()
{

}

const enum WifiStandard
WifiPhyLayerConfiguration::GetStandard ()
{
  if (m_standard.compare("802.11a") == 0)
    return WIFI_STANDARD_80211a;
  else if (m_standard.compare("802.11b") == 0)
    return WIFI_STANDARD_80211b;
  else if (m_standard.compare("802.11g") == 0)
    return WIFI_STANDARD_80211g;
  else if (m_standard.compare("802.11p") == 0)
    return WIFI_STANDARD_80211p;
  else if (m_standard.compare("802.11n-2.4GHz") == 0)
    return WIFI_STANDARD_80211n_2_4GHZ;
  else if (m_standard.compare("802.11n-5GHz") == 0)
    return WIFI_STANDARD_80211n_5GHZ;
  else if (m_standard.compare("802.11ac") == 0)
    return WIFI_STANDARD_80211ac;
  else if (m_standard.compare("802.11ax-2.4GHz") == 0)
    return WIFI_STANDARD_80211ax_2_4GHZ;
  else if (m_standard.compare("802.11ax-5GHz") == 0)
    return WIFI_STANDARD_80211ax_5GHZ;
  else if (m_standard.compare("802.11ax-6GHz") == 0)
    return WIFI_STANDARD_80211ax_6GHZ;
  else
    NS_FATAL_ERROR ("Cannot decode Wifi Standard: " << m_standard);
}

const double
WifiPhyLayerConfiguration::GetRxGain ()
{
  return m_rxGain;
}

const ModelConfiguration
WifiPhyLayerConfiguration::GetChannelPropagationDelayModel ()
{
  return m_channelPropagationDelayModel;
}

const ModelConfiguration
WifiPhyLayerConfiguration::GetChannelPropagationLossModel ()
{
  return m_channelPropagationLossModel;
}

} // namespace ns3
