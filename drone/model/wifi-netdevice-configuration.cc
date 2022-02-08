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
#include "wifi-netdevice-configuration.h"

namespace ns3 {

WifiNetdeviceConfiguration::WifiNetdeviceConfiguration (const std::string type,
                                                        const ModelConfiguration macLayer,
                                                        const uint32_t networkLayerId) :
  NetdeviceConfiguration {type, networkLayerId},
  m_macLayer {macLayer}
{

}

WifiNetdeviceConfiguration::~WifiNetdeviceConfiguration ()
{

}

const ModelConfiguration
WifiNetdeviceConfiguration::GetMacLayer () const
{
  return m_macLayer;
}

} // namespace ns3
