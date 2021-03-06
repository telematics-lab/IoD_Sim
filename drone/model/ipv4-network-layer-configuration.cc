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
#include "ipv4-network-layer-configuration.h"

#include <ns3/abort.h>

namespace ns3 {

Ipv4NetworkLayerConfiguration::Ipv4NetworkLayerConfiguration (std::string type,
                                                              std::string address,
                                                              std::string mask):
  NetworkLayerConfiguration {type},
  m_address {address},
  m_mask {mask}
{

}

Ipv4NetworkLayerConfiguration::~Ipv4NetworkLayerConfiguration ()
{

}

const std::string
Ipv4NetworkLayerConfiguration::GetAddress () const
{
  return m_address;
}

const std::string
Ipv4NetworkLayerConfiguration::GetMask () const
{
  return m_mask;
}

} // namespace ns3
