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
#ifndef WIFI_PHY_LAYER_CONFIGURATION_H
#define WIFI_PHY_LAYER_CONFIGURATION_H

#include <string>

#include <ns3/wifi-phy.h>

#include <ns3/model-configuration.h>

#include "phy-layer-configuration.h"

namespace ns3 {

/**
 * Data class to store information about the WiFi PHY Layer of a Scenario.
 */
class WifiPhyLayerConfiguration : public PhyLayerConfiguration
{
public:
  /**
   * Create a new object instance.
   *
   * \param phyType The type of the PHY Layer to be configured. It should be "wifi".
   * \param standard The reference WiFi Standard.
   * \param attributes Wi-Fi PHY Attributes.
   * \param mode The mode of WiFi PHY according to YANS.
   * \param channelPropagationDelayModel The Propagation Delay Model to be used for this Layer.
   * \param channelPropagationLossModel The Propagation Loss Model to be used for this Layer.
   */
  WifiPhyLayerConfiguration (std::string phyType,
                             std::string standard,
                             std::vector<ModelConfiguration::Attribute> attributes,
                             ModelConfiguration channelPropagationDelayModel,
                             ModelConfiguration channelPropagationLossModel);
  /** Default destructor */
  ~WifiPhyLayerConfiguration ();

  /**
   * \return The configured WiFi Standard.
   */
  const enum WifiStandard GetStandard ();
  /**
   * \return The Propagation Delay Model configuration.
   */
  const ModelConfiguration GetChannelPropagationDelayModel ();
  /**
   * \return The Propagation Loss Model configuration.
   */
  const ModelConfiguration GetChannelPropagationLossModel ();

private:
  const std::string m_standard;
  const ModelConfiguration m_channelPropagationDelayModel;
  const ModelConfiguration m_channelPropagationLossModel;
};

} // namespace ns3

#endif /* WIFI_PHY_LAYER_CONFIGURATION_H */
