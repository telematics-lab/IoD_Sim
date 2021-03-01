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
#ifndef IPV4_SIMULATION_HELPER_H
#define IPV4_SIMULATION_HELPER_H

#include <ns3/object.h>
#include <ns3/ipv4-interface-container.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/ipv4-address-helper.h>

namespace ns3 {

class Ipv4SimulationHelper : public Object
{
public:
  Ipv4SimulationHelper ();
  ~Ipv4SimulationHelper ();

  Ipv4InterfaceContainer& GetIpv4Interfaces ();
  InternetStackHelper& GetInternetHelper ();
  Ipv4AddressHelper& GetIpv4Helper ();

private:
  Ipv4InterfaceContainer m_ifacesIps;
  InternetStackHelper m_internetHelper;
  Ipv4AddressHelper m_ipv4Helper;
};

} // namespace ns3

#endif /* IPV4_SIMULATION_HELPER_H */
