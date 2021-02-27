/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2020 The IoD_Sim Authors.
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
#ifndef WIFI_PHY_LAYER_CONFIGURATION_H
#define WIFI_PHY_LAYER_CONFIGURATION_H

#include <string>

#include <ns3/wifi-phy.h>

#include "phy-layer-configuration.h"
#include "model-configuration.h"

namespace ns3 {

class WifiPhyLayerConfiguration : public PhyLayerConfiguration
{
public:
  WifiPhyLayerConfiguration (std::string phyType,
                             std::string standard,
                             double rxGain,
                             std::string mode,
                             ModelConfiguration channelPropagationDelayModel,
                             ModelConfiguration channelPropagationLossModel);
  ~WifiPhyLayerConfiguration ();

  const enum WifiStandard GetStandard ();
  const double GetRxGain ();
  const std::string GetMode ();
  const ModelConfiguration GetChannelPropagationDelayModel ();
  const ModelConfiguration GetChannelPropagationLossModel ();

private:
  const std::string m_standard;
  const double m_rxGain;
  const std::string m_mode;
  const ModelConfiguration m_channelPropagationDelayModel;
  const ModelConfiguration m_channelPropagationLossModel;
};

} // namespace ns3

#endif /* WIFI_PHY_LAYER_CONFIGURATION_H */
