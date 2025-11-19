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

#include "nd-cache-helper.h"
#include "arp-cache-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "leo-helper.h"

namespace ns3 {

LeoHelper::LeoHelper()
{
}

// TODO use template?
NetDeviceContainer
LeoHelper::Install (NodeContainer &satellites, NodeContainer &gateways, NodeContainer &terminals)
{
  NetDeviceContainer container;

  // Create sets of net devices for individual networks
  NetDeviceContainer islNet, gwNet, utNet;
  islNet = m_islChannelHelper.Install (satellites);
  gwNet = m_gwChannelHelper.Install (satellites, gateways);
  utNet = m_utChannelHelper.Install (satellites, terminals);

  // Install internet stack on nodes
  m_stackHelper.Install (satellites);
  m_stackHelper.Install (gateways);
  m_stackHelper.Install (terminals);

  //// Make all networks addressable
  //Ipv6AddressHelper address;
  //Ipv6InterfaceContainer islAddrs = address.Assign (islNet);
  //Ipv6InterfaceContainer gwAddrs = address.Assign (gwNet);
  //Ipv6InterfaceContainer utAddrs = address.Assign (utNet);

  //// Pre-fill the ND caches of networks
  //NdCacheHelper ndCache;
  //ndCache.Install (islNet, islAddrs);
  //ndCache.Install (gwNet, gwAddrs);
  //ndCache.Install (utNet, utAddrs);

  // Make all networks addressable for legacy protocol
  Ipv4AddressHelper legacy;
  Ipv4InterfaceContainer islLegacy = legacy.Assign (islNet);
  Ipv4InterfaceContainer gwLegacy = legacy.Assign (gwNet);
  Ipv4InterfaceContainer utLegacy = legacy.Assign (utNet);

  ArpCacheHelper arpCache;
  arpCache.Install (islNet, islLegacy);
  arpCache.Install (gwNet, gwLegacy);
  arpCache.Install (utNet, utLegacy);

  // Add to resulting container of net devices
  container.Add (islNet);
  container.Add (gwNet);
  container.Add (utNet);

  return container;
}

void
LeoHelper::SetQueue (std::string type,
          std::string n1, const AttributeValue &v1,
          std::string n2, const AttributeValue &v2,
          std::string n3, const AttributeValue &v3,
          std::string n4, const AttributeValue &v4)
{
  SetGndGwQueue (type, n1, v1, n2, v2, n3, v3, n4, v4);
  SetGndUtQueue (type, n1, v1, n2, v2, n3, v3, n4, v4);
  SetSatGwQueue (type, n1, v1, n2, v2, n3, v3, n4, v4);
  SetSatUtQueue (type, n1, v1, n2, v2, n3, v3, n4, v4);
}

void
LeoHelper::SetGndGwQueue (std::string type,
          	       std::string n1, const AttributeValue &v1,
          	       std::string n2, const AttributeValue &v2,
          	       std::string n3, const AttributeValue &v3,
          	       std::string n4, const AttributeValue &v4)
{
  m_gwChannelHelper.SetGndQueue(type, n1, v1, n2, v2, n3, v3, n4, v4);
}

void
LeoHelper::SetGndUtQueue (std::string type,
          	       std::string n1, const AttributeValue &v1,
          	       std::string n2, const AttributeValue &v2,
          	       std::string n3, const AttributeValue &v3,
          	       std::string n4, const AttributeValue &v4)
{
  m_utChannelHelper.SetGndQueue(type, n1, v1, n2, v2, n3, v3, n4, v4);
}

void
LeoHelper::SetSatGwQueue (std::string type,
          		  std::string n1, const AttributeValue &v1,
          		  std::string n2, const AttributeValue &v2,
          		  std::string n3, const AttributeValue &v3,
          		  std::string n4, const AttributeValue &v4)
{
  m_gwChannelHelper.SetSatQueue(type, n1, v1, n2, v2, n3, v3, n4, v4);
}

void
LeoHelper::SetSatUtQueue (std::string type,
          		  std::string n1, const AttributeValue &v1,
          		  std::string n2, const AttributeValue &v2,
          		  std::string n3, const AttributeValue &v3,
          		  std::string n4, const AttributeValue &v4)
{
  m_utChannelHelper.SetSatQueue(type, n1, v1, n2, v2, n3, v3, n4, v4);
}

void
LeoHelper::SetDeviceAttribute (std::string name, const AttributeValue &value)
{
  m_islChannelHelper.SetDeviceAttribute (name, value);

  m_gwChannelHelper.SetGndDeviceAttribute (name, value);
  m_gwChannelHelper.SetSatDeviceAttribute (name, value);

  m_utChannelHelper.SetGndDeviceAttribute (name, value);
  m_utChannelHelper.SetSatDeviceAttribute (name, value);
}

void
LeoHelper::SetGndGwDeviceAttribute (std::string name, const AttributeValue &value)
{
  m_gwChannelHelper.SetGndDeviceAttribute (name, value);
}

void
LeoHelper::SetGndUtDeviceAttribute (std::string name, const AttributeValue &value)
{
  m_utChannelHelper.SetGndDeviceAttribute(name, value);
}

void
LeoHelper::SetSatUtDeviceAttribute (std::string name, const AttributeValue &value)
{
  m_utChannelHelper.SetSatDeviceAttribute(name, value);
}

void
LeoHelper::SetSatGwDeviceAttribute (std::string name, const AttributeValue &value)
{
  m_gwChannelHelper.SetSatDeviceAttribute(name, value);
}

void
LeoHelper::SetIslDeviceAttribute (std::string name, const AttributeValue &value)
{
  m_islChannelHelper.SetDeviceAttribute (name, value);
}

void LeoHelper::SetChannelAttribute (std::string name, const AttributeValue &value)
{
  SetIslChannelAttribute (name, value);
  SetUtChannelAttribute (name, value);
  SetGwChannelAttribute (name, value);
}

void
LeoHelper::SetIslChannelAttribute (std::string name, const AttributeValue &value)
{
  m_islChannelHelper.SetChannelAttribute (name, value);
}

void
LeoHelper::SetUtChannelAttribute (std::string name, const AttributeValue &value)
{
  m_utChannelHelper.SetChannelAttribute (name, value);
}

void
LeoHelper::SetGwChannelAttribute (std::string name, const AttributeValue &value)
{
  m_gwChannelHelper.SetChannelAttribute (name, value);
}

void
LeoHelper::EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename)
{
  m_islChannelHelper.EnablePcapInternal (prefix, nd, promiscuous, explicitFilename);
  m_gwChannelHelper.EnablePcapInternal (prefix, nd, promiscuous, explicitFilename);
  m_utChannelHelper.EnablePcapInternal (prefix, nd, promiscuous, explicitFilename);
}

void
LeoHelper::EnableAsciiInternal (
    Ptr<OutputStreamWrapper> stream,
    std::string prefix,
    Ptr<NetDevice> nd,
    bool explicitFilename)
{
  m_islChannelHelper.EnableAsciiInternal (stream, prefix, nd, explicitFilename);
  m_gwChannelHelper.EnableAsciiInternal (stream, prefix, nd, explicitFilename);
  m_utChannelHelper.EnableAsciiInternal (stream, prefix, nd, explicitFilename);
}

void
LeoHelper::SetRoutingHelper (const Ipv4RoutingHelper &routing)
{
  m_stackHelper.SetRoutingHelper (routing);
}

void
LeoHelper::SetRoutingHelper (const Ipv6RoutingHelper &routing)
{
  m_stackHelper.SetRoutingHelper (routing);
}

} /* namespace ns3 */
