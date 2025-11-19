/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Tim Schubert <ns-3-leo@timschubert.net>
 */

#ifndef ARP_CACHE_HELPER_H
#define ARP_CACHE_HELPER_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/node-container.h"

/**
 * \file
 * \ingroup leo
 * Declares ArpCacheHelper
 */

namespace ns3 {

/**
 * \ingroup leo
 * \brief Prepares the ARP cache, so the addresses do not have to be queried
 */
class ArpCacheHelper
{
public:
  /**
   * \brief Install the addresses of the interfaces into the ARP caches of the devices
   * \param devices devices
   * \param interfaces interfaces
   */
  void Install (NetDeviceContainer &devices, Ipv4InterfaceContainer &interfaces) const;

  /**
   * \brief Install the addresses of the interfaces into the ARP caches of the devices
   * \param deviceSrc devices
   * \param deviceDst devices
   * \param interfaces interfaces
   */
  void Install (NetDeviceContainer &devicesSrc, NetDeviceContainer &devicesDst, Ipv4InterfaceContainer &interfaces) const;
};

}; /* namespace ns3 */

#endif /* ARP_CACHE_HELPER_H */
