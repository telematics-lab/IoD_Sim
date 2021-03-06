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
#include "report-entity.h"

#include <ns3/config.h>
#include <ns3/drone-communications.h>
#include <ns3/integer.h>
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/log.h>
#include <ns3/node.h>
#include <ns3/node-list.h>
#include <ns3/simulator.h>
#include <ns3/string.h>
#include <ns3/udp-header.h>

#include <rapidjson/document.h>

#include "drone-control-layer.h"
#include "ipv4-layer.h"
#include "wifi-inspector.h"
#include "wifi-mac-layer.h"
#include "wifi-phy-layer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ReportEntity");
NS_OBJECT_ENSURE_REGISTERED (ReportEntity);

TypeId
ReportEntity::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ReportEntity")
    .AddConstructor<ReportEntity> ()
    .SetParent<Object> ()
    .AddAttribute ("Reference", "Drone UID of reference",
                   IntegerValue (0),
                   MakeIntegerAccessor (&ReportEntity::m_reference),
                   MakeIntegerChecker<uint32_t> ())
    ;

  return tid;
}

ReportEntity::ReportEntity ()
{
  NS_LOG_FUNCTION (this);
}

ReportEntity::~ReportEntity ()
{
  NS_LOG_FUNCTION (this);
}

void
ReportEntity::Write (xmlTextWriterPtr h)
{
  NS_LOG_FUNCTION (h);

  DoWrite (h);
}

void
ReportEntity::DoWrite (xmlTextWriterPtr handle)
{
}

void
ReportEntity::DoInitializeTrajectoryMonitor ()
{
}

void
ReportEntity::DoInitializeNetworkStacks ()
{
}

void
ReportEntity::DoMonitorTrajectory (const Ptr<const MobilityModel> mobility)
{
}

void
ReportEntity::DoInitialize ()
{
  NS_LOG_FUNCTION_NOARGS ();

  DoInitializeTrajectoryMonitor ();
  DoInitializeNetworkStacks ();
  DoInitializeDataStats ();
  DoInitializeTrafficMonitors ();
}

void
ReportEntity::DoInitializeDataStats ()
{
  NS_LOG_FUNCTION_NOARGS ();

  for (uint8_t i = 0; i < PacketType::numValues; i++)
    {
      auto objRx = CreateObjectWithAttributes<ReportDataStats>
        ("kind", PacketTypeValue (PacketType (i)));
      auto objTx = CreateObjectWithAttributes<ReportDataStats>
        ("kind", PacketTypeValue (PacketType (i)));

      m_cumulativeDataRx.push_back (objRx);
      m_cumulativeDataTx.push_back (objTx);
    }
}

void
ReportEntity::DoInitializeTrafficMonitors ()
{
  DoInitializeTrafficMonitor (TransferDirection ("Rx"));
  DoInitializeTrafficMonitor (TransferDirection ("Tx"));
}

void
ReportEntity::DoInitializeTrafficMonitor (TransferDirection d)
{
  NS_LOG_FUNCTION (d);
  /* set IPv4 Rx callback using ns-3 XPath addressing system */
  std::stringstream xPathCallback;

  xPathCallback << "/NodeList/" << m_reference << "/"
                "$ns3::Ipv4L3Protocol/" << d.ToString ();

  switch (d)
    {
      case TransferDirection::Value::Received:
        Config::ConnectWithoutContext
          (xPathCallback.str (), MakeCallback (&ReportEntity::DoMonitorRxTraffic,
                                               this));
        break;
      case TransferDirection::Value::Transmitted:
        Config::ConnectWithoutContext
          (xPathCallback.str (), MakeCallback (&ReportEntity::DoMonitorTxTraffic,
                                               this));
        break;
    }
}

void
ReportEntity::DoMonitorRxTraffic (Ptr<const Packet> packet,
                                  Ptr<Ipv4> ipv4,
                                  uint32_t interface)
{
  NS_LOG_FUNCTION (packet << ipv4 << interface);
  constexpr const char *direction = "Rx";
  std::stringstream builderIpv4Src,
                    builderIpv4Dst;
  std::string ipv4Src,
              ipv4Dst;
  std::uint32_t payloadSize = 0;
  std::uint32_t sequenceNumber = 0;
  PacketType packetType;

  constexpr const uint8_t bufferLength = 128;
  rapidjson::Document d;
  auto pCopy = packet->Copy ();
  uint8_t *buf = (uint8_t *) calloc (bufferLength, sizeof (uint8_t));

  if (buf == nullptr)
    NS_FATAL_ERROR ("Cannot allocate " << bufferLength << " bytes "
                    "to manage packet payload.");

  Ipv4Header ipv4Header;
  pCopy->RemoveHeader (ipv4Header);
  UdpHeader udpHeader;
  pCopy->RemoveHeader (udpHeader);

  builderIpv4Src << ipv4Header.GetSource ();
  ipv4Src = builderIpv4Src.str ();

  builderIpv4Dst << ipv4Header.GetDestination ();
  ipv4Dst = builderIpv4Dst.str ();

  auto i = pCopy->BeginItem ();
  while (i.HasNext ())
    {
      auto item = i.Next ();

      if (!item.isFragment && item.type == PacketMetadata::Item::PAYLOAD)
        {
          payloadSize = item.currentSize;

          pCopy->CopyData (buf,
                           (item.currentSize > bufferLength)
                            ? bufferLength
                            : item.currentSize);

          d.Parse ((char *) buf);
          if (!d.IsObject ())
            return;

          const char *commandStr = d["cmd"].GetString ();
          sequenceNumber = d["sn"].GetUint ();

          packetType = PacketType (commandStr);

          break;
        }
    }

  const std::string payloadDecoded ((char *) buf);
  free (buf);

  m_cumulativeDataRx[packetType]->Add (payloadSize);

  auto reportTransfer = CreateObjectWithAttributes<ReportTransfer>
    ("PacketType", PacketTypeValue (packetType),
     "TransferDirection", TransferDirectionValue (direction),
     "Time", TimeValue (Simulator::Now ()),
     "SourceAddress", StringValue (ipv4Src),
     "DestinationAddress", StringValue (ipv4Dst),
     "PacketLength", IntegerValue (payloadSize),
     "SequenceNumber", IntegerValue (sequenceNumber),
     "Payload", StringValue (payloadDecoded));
  m_dataRx.push_back (reportTransfer);
}

void
ReportEntity::DoMonitorTxTraffic (Ptr<const Packet> packet,
                                  Ptr<Ipv4> ipv4,
                                  uint32_t interface)
{
  NS_LOG_FUNCTION (packet << ipv4 << interface);
  constexpr const char *direction = "Tx";
  std::stringstream builderIpv4Src,
                    builderIpv4Dst;
  std::string ipv4Src,
              ipv4Dst;
  std::uint32_t payloadSize = 0;
  std::uint32_t sequenceNumber = 0;
  PacketType packetType;

  constexpr const uint8_t bufferLength = 128;
  rapidjson::Document d;
  auto pCopy = packet->Copy ();
  uint8_t *buf = (uint8_t *) calloc (bufferLength, sizeof (uint8_t));

  if (buf == nullptr)
    NS_FATAL_ERROR ("Cannot allocate " << bufferLength << " bytes "
                    "to manage packet payload.");

  Ipv4Header ipv4Header;
  pCopy->RemoveHeader (ipv4Header);
  UdpHeader udpHeader;
  pCopy->RemoveHeader (udpHeader);

  builderIpv4Src << ipv4Header.GetSource ();
  ipv4Src = builderIpv4Src.str ();

  builderIpv4Dst << ipv4Header.GetDestination ();
  ipv4Dst = builderIpv4Dst.str ();

  auto i = pCopy->BeginItem ();
  while (i.HasNext ())
    {
      auto item = i.Next ();
      if (!item.isFragment && item.type == PacketMetadata::Item::PAYLOAD)
        {
          payloadSize = item.currentSize;

          pCopy->CopyData (buf,
                           (item.currentSize > bufferLength)
                            ? bufferLength
                            : item.currentSize);

          d.Parse ((char *) buf);
          if (!d.IsObject ())
            return;

          const char *commandStr = d["cmd"].GetString ();
          sequenceNumber = d["sn"].GetUint ();

          packetType = PacketType (commandStr);

          break;
        }
    }

  const std::string payloadDecoded ((char *) buf);
  free (buf);

  m_cumulativeDataTx[packetType]->Add (payloadSize);

  auto reportTransfer = CreateObjectWithAttributes<ReportTransfer>
    ("PacketType", PacketTypeValue (packetType),
     "TransferDirection", TransferDirectionValue (direction),
     "Time", TimeValue (Simulator::Now ()),
     "SourceAddress", StringValue (ipv4Src),
     "DestinationAddress", StringValue (ipv4Dst),
     "PacketLength", IntegerValue (payloadSize),
     "SequenceNumber", IntegerValue (sequenceNumber),
     "Payload", StringValue (payloadDecoded));
  m_dataTx.push_back (reportTransfer);
}

bool
ReportEntity::IsWifiNetDevice (Ptr<NetDevice> device)
{
  return device->GetInstanceTypeId ().GetName () == "ns3::WifiNetDevice";
}

const std::tuple<const std::string, const std::string>
ReportEntity::GetIpv4Address ()
{
  std::stringstream bPath;

  bPath << "/NodeList/" << m_reference << "/ApplicationList/*";
  auto matches = Config::LookupMatches (bPath.str ());

  for (std::size_t i = 0; i < matches.GetN (); i++)
    {
      auto match = matches.Get (i);
      const auto matchClassName = match->GetInstanceTypeId ().GetName ();

      /*
       * Assumption: only one DroneClient is available on a Drone and
       *             only one DroneServer is available on a ZSP
       */
      if (matchClassName == "ns3::DroneClient"
          || matchClassName == "ns3::DroneServer")
        {
          TypeId::AttributeInformation info;
          Ipv4AddressValue ipv4Address;
          Ipv4MaskValue ipv4NetMask;
          std::stringstream bIpv4Address,
                            bIpv4NetMask;

          match->GetInstanceTypeId ().LookupAttributeByName ("Ipv4Address",
                                                             &info);
          // This method should support Ptr<> type!
          info.accessor->Get (&(*match), ipv4Address);
          match->GetInstanceTypeId ().LookupAttributeByName ("Ipv4SubnetMask",
                                                             &info);
          info.accessor->Get (&(*match), ipv4NetMask);

          bIpv4Address << ipv4Address.Get ();
          bIpv4NetMask << ipv4NetMask.Get ();

          return {bIpv4Address.str (), bIpv4NetMask.str ()};
        }
    }

  NS_FATAL_ERROR ("Expected to find a DroneClient/Server Application on "
                  "entity, but it was not there!");
}

} // namespace ns3
