/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2023 The IoD_Sim Authors.
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
#include "nat-application.h"

#include <ns3/arp-cache.h>
#include <ns3/boolean.h>
#include <ns3/log.h>
#include <ns3/integer.h>
#include <ns3/ipv4.h>
#include <ns3/ipv4-header.h>
#include <ns3/ipv4-interface.h>
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/ipv4-routing-table-entry.h>
#include <ns3/lte-ue-net-device.h>
#include <ns3/epc-ue-nas.h>
#include <ns3/udp-header.h>
#include <ns3/uinteger.h>
#include <ns3/output-stream-wrapper.h>
#include <ns3/hash-fnv.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NatApplication");

NS_OBJECT_ENSURE_REGISTERED (NatApplication);

TypeId
NatApplication::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NatApplication")
    .SetParent<Application> ()
    .SetGroupName ("drone")
    .AddConstructor<NatApplication> ()
    .AddAttribute ("InternalNetDeviceId", "Identifier of the Internal Network Device",
                   IntegerValue (0),
                   MakeIntegerAccessor (&NatApplication::m_intNetDevId),
                   MakeIntegerChecker<uint32_t> ())
    .AddAttribute ("ExternalNetDeviceId", "Identifier of the External Network Device",
                   IntegerValue (1),
                   MakeIntegerAccessor (&NatApplication::m_extNetDevId),
                   MakeIntegerChecker<uint32_t> ())
    ;

  return tid;
}

NatApplication::NatApplication () :
  m_curNatPort {1}
{
  NS_LOG_FUNCTION_NOARGS ();
}

NatApplication::~NatApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
NatApplication::DoInitialize (void)
{
  NS_LOG_FUNCTION (m_intNetDevId << m_extNetDevId);

  auto node = GetNode ();

  // Set default route to loopback
  Ipv4StaticRoutingHelper routingHelper;
  Ptr<Ipv4StaticRouting> nodeStaticRouting = routingHelper.GetStaticRouting (node->GetObject<Ipv4> ());

  for (uint32_t i = 0; i < nodeStaticRouting->GetNRoutes (); i++)
    {
      auto route = nodeStaticRouting->GetRoute (i).GetDestNetwork ();
      if (route == Ipv4Address::GetAny ())
        {
          nodeStaticRouting->RemoveRoute (i);
          i--;
        }
    }

  nodeStaticRouting->SetDefaultRoute (Ipv4Address::GetLoopback (), m_extNetDevId);

  m_intNetDev = node->GetDevice(m_intNetDevId);
  m_extNetDev = node->GetDevice(m_extNetDevId);

  m_intNetDev->SetPromiscReceiveCallback (MakeCallback (&NatApplication::RecvPktFromIntNetDev, this));
  m_extNetDev->SetReceiveCallback (MakeCallback (&NatApplication::RecvPktFromExtNetDev, this));

  Application::DoInitialize ();
}

bool
NatApplication::RecvPktFromIntNetDev (Ptr<NetDevice> netdev,
                                      Ptr<const Packet> pkt,
                                      uint16_t protocol,
                                      const Address& sender,
                                      const Address& receiver,
                                      enum NetDevice::PacketType pktType)
{
  NS_LOG_FUNCTION (this << netdev << pkt << protocol << sender << receiver << pktType);
  return RecvPktFromNetDev (netdev, pkt, protocol, sender, receiver, pktType);
}

bool
NatApplication::RecvPktFromExtNetDev (Ptr<NetDevice> netdev,
                                      Ptr<const Packet> pkt,
                                      uint16_t protocol,
                                      const Address& sender)
{
  NS_LOG_FUNCTION (this << netdev << pkt << protocol << sender);
  //TODO: This is a fake address. How can we easily retrieve the receiver IP Address from the packet?
  auto receiver = Ipv4Address::GetZero ();

  return RecvPktFromNetDev (netdev, pkt, protocol, sender, receiver, NetDevice::PacketType::PACKET_OTHERHOST);
}

bool
NatApplication::RecvPktFromNetDev (Ptr<NetDevice> netdev,
                                   Ptr<const Packet> pkt,
                                   uint16_t protocol,
                                   const Address& senderMacAddr,
                                   const Address& receiverMacAddr,
                                   enum NetDevice::PacketType pktType)
{
  NS_LOG_FUNCTION (netdev << pkt << protocol << senderMacAddr << receiverMacAddr << pktType);

  Ipv4Header ipHdr;
  UdpHeader udpHdr;
  uint8_t ipv4Protocol; // https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml

  // https://www.iana.org/assignments/ieee-802-numbers/ieee-802-numbers.xhtml
  if (m_intNetDev == netdev && protocol == 2048 /* IPv4 */)
    {
      auto toBeSent = pkt->Copy ();
      toBeSent->RemoveHeader (ipHdr);
      ipv4Protocol = ipHdr.GetProtocol ();

      //TODO: if (ipv4Protocol == 6 /* TCP */)
      if (ipv4Protocol == 17 /* UDP */)
        {
          toBeSent->RemoveHeader (udpHdr);

          NatEntry entry = {
            .ipv4Addr = ipHdr.GetSource (),
            .port = udpHdr.GetSourcePort (),
            .macAddr = senderMacAddr
          };

          auto outPort = m_natTable[entry];
          if (!outPort)
            {
              // TODO: TCP could break!
              if (m_curNatPort <= 0 || m_curNatPort >= UINT16_MAX)
                m_curNatPort = 1;

              m_natTable[entry] = m_curNatPort;
              outPort = m_curNatPort;

              m_curNatPort++;
            }

          // substitute source IP and port
          auto ifId = m_extNetDev->GetIfIndex (); /* LTE does something strange with interface index */
          auto extIpv4Addr = GetNode ()->GetObject<Ipv4> ()->GetAddress (ifId + 1, 0).GetLocal ();

          NS_LOG_LOGIC (ipHdr.GetSource () << ":" << udpHdr.GetSourcePort () << " -> " << extIpv4Addr << ":" << outPort);
          ipHdr.SetSource (extIpv4Addr);
          udpHdr.SetSourcePort (outPort);

          toBeSent->AddHeader (udpHdr);
          toBeSent->AddHeader (ipHdr);

          // check if we have LTE UE as external net device
          auto netdevObjName = m_extNetDev->GetInstanceTypeId ().GetName ();
          if (netdevObjName == "ns3::LteUeNetDevice")
            {
              auto netdevLteUe = StaticCast<LteUeNetDevice, NetDevice> (m_extNetDev);
              netdevLteUe->GetNas ()->Send (toBeSent, protocol);
            }
        }
    }
  else if (m_extNetDev == netdev && protocol == 2048 /* IPv4 */)
    {
      auto toBeSent = pkt->Copy ();
      toBeSent->RemoveHeader (ipHdr);
      ipv4Protocol = ipHdr.GetProtocol ();

      //TODO: if (ipv4Protocol == 6 /* TCP */)
      if (ipv4Protocol == 17 /* UDP */)
        {
          toBeSent->RemoveHeader (udpHdr);

          auto currentDestPort = udpHdr.GetDestinationPort ();

          auto matchedRule = InverseLookup (currentDestPort);
          if (matchedRule == m_natTable.end ())
            {
              NS_LOG_WARN ("Got a packet on the external interface that was never send from the internal network. DROP.");
              return false;
            }

          auto destIpv4Addr = matchedRule->first.ipv4Addr;
          auto destPort = matchedRule->first.port;
          auto destMacAddr = matchedRule->first.macAddr;

          ipHdr.SetDestination (destIpv4Addr);
          udpHdr.SetDestinationPort (destPort);
          toBeSent->AddHeader (udpHdr);
          toBeSent->AddHeader (ipHdr);

          m_intNetDev->Send (toBeSent, destMacAddr, protocol);
        }
    }

  return true;
}

NatApplication::NatTableI
NatApplication::InverseLookup (uint16_t port)
{
  for (auto rule = m_natTable.begin(); rule != m_natTable.end(); rule++)
    {
      if (rule->second == port)
        return rule;
    }

  return m_natTable.end();
}

size_t
NatApplication::NatEntryHash::operator() (NatApplication::NatEntry const &x) const
{
  auto hashf = Hash::Function::Fnv1a();
  uint32_t raw_buf[2] = {'\0'};

  raw_buf[0] = x.ipv4Addr.Get ();
  raw_buf[1] = x.port;

  return hashf.GetHash32 ((const char*) &raw_buf, 2);
};

bool operator== (const NatApplication::NatEntry x, const NatApplication::NatEntry y)
{
  return x.ipv4Addr == y.ipv4Addr && x.macAddr == y.macAddr && x.port == y.port;
}

} // namespace ns3
