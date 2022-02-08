/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2022 The IoD_Sim Authors.
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
#include "lte-phy-layer-configuration.h"

#include <ns3/abort.h>

namespace ns3 {

LtePhyLayerConfiguration::LtePhyLayerConfiguration(std::string phyType,
                                                   ModelConfiguration channelPropagationLossModel,
                                                   ModelConfiguration channelSpectrumModel) :
  PhyLayerConfiguration {phyType},
  m_channelPropagationLossModel {channelPropagationLossModel},
  m_channelSpectrumModel {channelSpectrumModel}
{

}

LtePhyLayerConfiguration::~LtePhyLayerConfiguration()
{

}

const ModelConfiguration
LtePhyLayerConfiguration::GetChannelPropagationLossModel ()
{
  return m_channelPropagationLossModel;
}

const ModelConfiguration
LtePhyLayerConfiguration::GetChannelSpectrumModel ()
{
  return m_channelSpectrumModel;
}

} // namespace ns3
