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
#ifndef DEBUG_HELPER_H
#define DEBUG_HELPER_H

#include <iostream>

#include <ns3/config.h>
#include <ns3/ipv4.h>
#include <ns3/node.h>
#include <ns3/node-list.h>
#include <ns3/object.h>

namespace ns3 {

class DebugHelper
{
public:
  static void ProbeNodes ()
  {
    std::cout << "********************************************************************************" << std::endl;

    std::cout << "There are " << Config::GetRootNamespaceObjectN () << " root objects available." << std::endl;
    for (size_t i = 0; i < Config::GetRootNamespaceObjectN (); i++)
      {
        auto obj = Config::GetRootNamespaceObject (i);
        std::cout << std::endl << "TypeId " << obj->GetInstanceTypeId () << std::endl;
      }

    const uint32_t nNodes = NodeList::GetNNodes ();
    std::cout << "There are " << nNodes << " registered nodes." << std::endl;
    for (auto iNode = NodeList::Begin (); iNode != NodeList::End (); iNode++)
      {
        std::cout << std::endl << (*iNode)->GetInstanceTypeId () << std::endl;

        auto aggregateIter = (*iNode)->GetAggregateIterator ();
        std::cout << "Aggregate objects " << std::endl;
        while (aggregateIter.HasNext ())
          {
            auto o = aggregateIter.Next ();
            std::cout << "Object " << o->GetInstanceTypeId () << std::endl;
          }
        std::cout << std::endl;

        auto nDevices = (*iNode)->GetNDevices ();
        std::cout << "There are " << nDevices << " devices" << std::endl;

        for (uint32_t iDev = 0; iDev < nDevices; iDev++)
          {
            auto dev = (*iNode)->GetDevice (iDev);
            std::cout << "Device TypeId " << dev->GetInstanceTypeId () << std::endl;
          }

        auto ipv4 = (*iNode)->GetObject<Ipv4> ();
        auto ipv4Ifaces = ipv4->GetNInterfaces ();
        for (uint32_t ifaceId = 0; ifaceId < ipv4Ifaces; ifaceId++)
          {
            std::cout << "Device ID " << ifaceId << std::endl;
            auto nAddresses = ipv4->GetNAddresses (ifaceId);

            for (uint32_t iAddr = 0; iAddr < nAddresses; iAddr++)
              {
                auto ipv4Addr = ipv4->GetAddress (ifaceId, iAddr);
                std::cout << "IPv4: " << ipv4Addr << std::endl;
              }
          }
      }

    std::cout << "********************************************************************************" << std::endl;
  }

  static void ObjectAttributes(Ptr<Object> o)
  {
      for (size_t n=0; n < o->GetInstanceTypeId ().GetParent().GetAttributeN(); n++)
      {
          TypeId::AttributeInformation attrInfo = o->GetInstanceTypeId ().GetParent().GetAttribute(n);
          Ptr<AttributeValue> attrValue;
          std::string attrName = o->GetInstanceTypeId ().GetParent().GetAttribute(n).name;
          std::string typeidvalue = attrInfo.checker->GetValueTypeName();
          std::cout<<attrName<<" "<<typeidvalue<<" "<<attrInfo.initialValue->SerializeToString(attrInfo.checker)<<std::endl;
      }
      for (size_t n=0; n < o->GetInstanceTypeId ().GetAttributeN(); n++)
      {
          TypeId::AttributeInformation attrInfo = o->GetInstanceTypeId ().GetAttribute(n);
          Ptr<AttributeValue> attrValue;
          std::string attrName = o->GetInstanceTypeId ().GetAttribute(n).name;
          std::string typeidvalue = attrInfo.checker->GetValueTypeName();
          std::cout<<attrName<<" "<<typeidvalue<<" "<<attrInfo.initialValue->SerializeToString(attrInfo.checker)<<std::endl;
      }
}
};



} // namespace ns3

#endif /* DEBUG_HELPER_H */
