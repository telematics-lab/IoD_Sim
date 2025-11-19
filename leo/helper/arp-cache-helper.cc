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

#include "arp-cache-helper.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("ArpCacheHelper");

void
ArpCacheHelper::Install (NetDeviceContainer &devices, Ipv4InterfaceContainer &interfaces) const
{
  NS_LOG_FUNCTION (this);

  for (size_t i = 0; i < devices.GetN (); i ++)
    {
      Ptr<NetDevice> dev = devices.Get (i);
      Ptr<Node> node = dev->GetNode ();
      NS_LOG_INFO ("Preparing ARP cache of " << node);
      Ptr<Ipv4L3Protocol> ipv4 = node->GetObject<Ipv4L3Protocol> ();
      int32_t ifIndex = ipv4->GetInterfaceForDevice (dev);
      Ptr<Ipv4Interface> interface = ipv4->GetInterface (ifIndex);
      Ptr<ArpCache> cache = interface->GetArpCache ();

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
          Ipv4Address ipaddr = interfaces.GetAddress (otherIfIndex, 0); // IP

          // update cache
          ArpCache::Entry* entry = cache->Lookup (ipaddr);
          if (entry == 0)
            {
              entry = cache->Add (ipaddr);
            }
          entry->SetMacAddress (address);

	  NS_LOG_DEBUG ("Added entry for " << address);
      	}
    }
}

};
