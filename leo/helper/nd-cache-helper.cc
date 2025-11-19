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
#include "ns3/log.h"
#include "../model/leo-mock-net-device.h"

#include "nd-cache-helper.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("NdCacheHelper");

void
NdCacheHelper::Install (NetDeviceContainer &devices, Ipv6InterfaceContainer &interfaces) const
{
  NS_LOG_FUNCTION (this);

  // prepare NDS cache
  for (uint32_t i = 0; i < devices.GetN (); i++)
    {
      Ptr<NetDevice> dev = devices.Get (i);
      Ptr<Node> node = dev->GetNode ();
      Ptr<Ipv6L3Protocol> ipv6 = node->GetObject<Ipv6L3Protocol> ();
      int32_t ifIndex = ipv6->GetInterfaceForDevice (dev);
      Ptr<Ipv6Interface> interface = ipv6->GetInterface (ifIndex);
      Ptr<NdiscCache> cache = interface->GetNdiscCache ();
      for (uint32_t j = 0; j < devices.GetN (); j++)
        {
          // every other device, that is not of same "type"
          Ptr<NetDevice> otherDevice = devices.Get (j);
          Ptr<LeoMockNetDevice> leoDev = DynamicCast<LeoMockNetDevice> (dev);
          Ptr<LeoMockNetDevice> otherLeoDev = DynamicCast<LeoMockNetDevice> (otherDevice);
          if (i == j || (leoDev != nullptr
              && otherLeoDev != nullptr
              && leoDev->GetDeviceType () == otherLeoDev->GetDeviceType ()))
            {
              continue;
            }
          Address address = otherDevice->GetAddress (); // MAC

          // and associated address
          uint32_t otherIfIndex = otherDevice->GetIfIndex ();
          Ipv6Address ipaddr = interfaces.GetAddress (otherIfIndex, 1); // IP

          // update cache
          NdiscCache::Entry* entry = cache->Lookup (ipaddr);
          if (entry == 0)
            {
              entry = cache->Add (ipaddr);
            }
          entry->SetMacAddress (address);

	  NS_LOG_DEBUG ("Added entry for " << address);
        }
    }
}

}; /* namespace ns3 */
