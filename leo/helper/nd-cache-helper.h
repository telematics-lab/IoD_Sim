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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/node-container.h"

#ifndef NDS_CACHE_HELPER_
#define NDS_CACHE_HELPER_

/**
 * \file
 * \ingroup leo
 * Declares NdCacheHelper
 */

namespace ns3 {

/**
 * \ingroup leo
 * \brief Neighbor Cache helper fills the neighbor cache
 */
class NdCacheHelper
{
public:
  /**
   * \brief Fill the cache of devices with addresses
   * \param devices devices
   * \param interfaces interfaces that have addresses
   */
  void Install (NetDeviceContainer &devices, Ipv6InterfaceContainer &interfaces) const;

  /**
   * \brief Fill the cache of devices with addresses
   * \param devicesSrc devices
   * \param devicesDst devices
   * \param interfaces interfaces that have addresses
   */
  void Install (NetDeviceContainer &devicesSrc, NetDeviceContainer &devicesDst, Ipv6InterfaceContainer &interfaces) const;
};

}; /* namespace ns3 */

#endif /* NDS_CACHE_HELPER */
