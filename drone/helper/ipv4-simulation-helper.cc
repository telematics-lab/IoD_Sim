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
#include "ipv4-simulation-helper.h"

namespace ns3 {

Ipv4SimulationHelper::Ipv4SimulationHelper (const std::string mask, const std::string gatewayAddress) :
  m_mask {mask},
  m_gatewayAddress {gatewayAddress.c_str ()}
{

}

Ipv4SimulationHelper::~Ipv4SimulationHelper ()
{

}

Ipv4InterfaceContainer&
Ipv4SimulationHelper::GetIpv4Interfaces ()
{
 return m_ifacesIps;
}

InternetStackHelper&
Ipv4SimulationHelper::GetInternetHelper ()
{
  return m_internetHelper;
}

Ipv4AddressHelper&
Ipv4SimulationHelper::GetIpv4Helper ()
{
  return m_ipv4Helper;
}

const std::string&
Ipv4SimulationHelper::GetMask ()
{
  return m_mask;
}

const Ipv4Address&
Ipv4SimulationHelper::GetGatewayAddress ()
{
  return m_gatewayAddress;
}

} // namespace ns3
