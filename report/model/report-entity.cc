/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2022 The IoD_Sim Authors.
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
#include <ns3/report-helper.h>
#include <rapidjson/document.h>
#include <ns3/lte-ue-net-device.h>
// #include <ns3/debug-helper.h>

#include "drone-control-layer.h"
#include "ipv4-layer.h"
#include "wifi-inspector.h"
#include "wifi-mac-layer.h"
#include "wifi-phy-layer.h"
#include "lte-ue-phy-layer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ReportEntity");
NS_OBJECT_ENSURE_REGISTERED (ReportEntity);

TypeId
ReportEntity::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ReportEntity")
    .AddConstructor<ReportEntity> ()
    .SetParent<Object> ()
    .AddAttribute ("Reference", "UID of reference",
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

  if (payloadSize > 0)
  {
    m_cumulativeDataRx[packetType]->Add (payloadSize);

    Ipv4Address ipv4dstaddr;
    ipv4dstaddr.Set(ipv4Dst.c_str());
    auto reportTransfer = CreateObjectWithAttributes<ReportTransfer>
      ("EntityID", IntegerValue (m_reference),
      "Interface", IntegerValue (ipv4->GetInterfaceForAddress(ipv4dstaddr)),
      "PacketType", PacketTypeValue (packetType),
      "TransferDirection", TransferDirectionValue (direction),
      "Time", TimeValue (Simulator::Now ()),
      "SourceAddress", StringValue (ipv4Src),
      "DestinationAddress", StringValue (ipv4Dst),
      "PacketLength", IntegerValue (payloadSize),
      "SequenceNumber", IntegerValue (sequenceNumber),
      "Payload", StringValue (payloadDecoded));
    m_dataRx.push_back (reportTransfer);
  }
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

  if (payloadSize > 0)
  {
    m_cumulativeDataTx[packetType]->Add (payloadSize);
    Ipv4Address ipv4srcaddr;
    ipv4srcaddr.Set(ipv4Src.c_str());
    auto reportTransfer = CreateObjectWithAttributes<ReportTransfer>
      ("EntityID", IntegerValue (m_reference),
      "Interface", IntegerValue (ipv4->GetInterfaceForAddress(ipv4srcaddr)),
      "PacketType", PacketTypeValue (packetType),
      "TransferDirection", TransferDirectionValue (direction),
      "Time", TimeValue (Simulator::Now ()),
      "SourceAddress", StringValue (ipv4Src),
      "DestinationAddress", StringValue (ipv4Dst),
      "PacketLength", IntegerValue (payloadSize),
      "SequenceNumber", IntegerValue (sequenceNumber),
      "Payload", StringValue (payloadDecoded));
    m_dataTx.push_back (reportTransfer);
  }
}

bool
ReportEntity::IsWifiNetDevice (Ptr<NetDevice> device)
{
  return device->GetInstanceTypeId ().GetName () == "ns3::WifiNetDevice";
}

const std::tuple<const int32_t, const std::string, const std::string>
ReportEntity::GetIpv4Address (Ptr<const NetDevice> dev)
{
  const auto node = NodeList::GetNode (m_reference);
  const auto nodeIpv4 = node->GetObject<Ipv4> ();

  if (nodeIpv4 == nullptr)
    return std::tuple<const int32_t, const std::string, const std::string> {-2, "", ""};

  const auto ifid = nodeIpv4->GetInterfaceForDevice(dev);
  // FIXME: we should bring all addresses available for all interfaces
  if (ifid == -1) return std::tuple<const int32_t, const std::string, const std::string> {-1, "", ""};

  const auto firstIpv4Addr = nodeIpv4->GetAddress (ifid, 0);
  std::stringstream localIpv4AddrBuilder, netmaskIpv4AddrBuilder;
  firstIpv4Addr.GetLocal ().Print (localIpv4AddrBuilder);
  firstIpv4Addr.GetBroadcast ().Print (netmaskIpv4AddrBuilder);

  return {ifid, localIpv4AddrBuilder.str (), netmaskIpv4AddrBuilder.str ()};


  // REMOVE FROM HERE
  // std::stringstream bPath;

  // bPath << "/NodeList/" << m_reference;
  // auto matches = Config::LookupMatches (bPath.str ());

  // for (std::size_t i = 0; i < matches.GetN (); i++)
  //   {
      // auto match = StaticCast<Node, Object> (matches.Get (i));
      // const auto matchClassName = match->GetInstanceTypeId ();

      /*
       * FIXME: this is not valid anymore!
       * Assumption: only one DroneClient is available on a Drone and
       *             only one DroneServer is available on a ZSP
       */
      // if (matchClassName == "ns3::DroneClientApplication"
      //     || matchClassName == "ns3::DroneServerApplication")
      //   {
      //     TypeId::AttributeInformation info;
      //     Ipv4AddressValue ipv4Address;
      //     Ipv4MaskValue ipv4NetMask;
      //     std::stringstream bIpv4Address,
      //                       bIpv4NetMask;

      //     match->GetInstanceTypeId ().LookupAttributeByName ("Ipv4Address",
      //                                                        &info);
      //     // This method should support Ptr<> type!
      //     info.accessor->Get (&(*match), ipv4Address);
      //     match->GetInstanceTypeId ().LookupAttributeByName ("Ipv4SubnetMask",
      //                                                        &info);
      //     info.accessor->Get (&(*match), ipv4NetMask);

      //     bIpv4Address << ipv4Address.Get ();
      //     bIpv4NetMask << ipv4NetMask.Get ();

      //     return {bIpv4Address.str (), bIpv4NetMask.str ()};
      //   }
    // }

  // NS_FATAL_ERROR ("Expected to find a Client/Server Application on "
  //                 "entity, but it was not there!");
}

void
ReportEntity::DoInitializeNetworkStacks ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<Node> node = NodeList::GetNode(m_reference);
  std::vector<uint32_t> ifaces;   // ifaces that needs to be registered

  if (node == nullptr)
    NS_FATAL_ERROR ("Report is trying to monitor NodeId " << m_reference << ", "
                    "but the object is null!");
  uint32_t netdevN = node->GetNDevices ();
  for (uint32_t i = 0; i < netdevN; i++)
    {
      auto dev = node->GetDevice (i);
      auto ipv4AddressMask = GetIpv4Address (dev);
      auto ifid = std::get<0>(ipv4AddressMask);
      if (ifid == -1) continue;
      while ((int) m_networkStacks.size()<= (int) ifid) m_networkStacks.push_back ({});
      if (IsWifiNetDevice (dev))
        {
          auto inspector = WifiInspector (dev);
          auto layer = CreateObjectWithAttributes<ProtocolLayer>
            ("InstanceType", StringValue (dev->GetInstanceTypeId ().GetName ()));
          auto phyLayer = CreateObjectWithAttributes<WifiPhyLayer>
            ("Frequency", IntegerValue (inspector.GetCarrierFrequency ()),
             "Standard", StringValue (inspector.GetWifiStandard ()),
             "PropagationDelayModel", StringValue (inspector.GetPropagationDelayModel ()),
             "PropagationLossModel", StringValue (inspector.GetPropagationLossModel ()),
             "NodeId", IntegerValue (m_reference),
             "NetdevId", IntegerValue (i));
          auto macLayer = CreateObjectWithAttributes<WifiMacLayer>
            ("Ssid", StringValue (inspector.GetWifiSsid ()),
             "Mode", StringValue (inspector.GetWifiMode ()));
          auto ipv4Layer = CreateObjectWithAttributes<Ipv4Layer>
            ("Ipv4Address", StringValue (std::get<1>(ipv4AddressMask)),
             "BroadcastAddress", StringValue (std::get<2>(ipv4AddressMask)));
          auto droneControlLayer = CreateObjectWithAttributes<DroneControlLayer>
            ("NotImplemented", StringValue ());

          phyLayer->Initialize ();

          ifaces.push_back (i);
          m_networkStacks[ifid].Add (layer);
          m_networkStacks[ifid].Add (phyLayer);
          m_networkStacks[ifid].Add (macLayer);
          m_networkStacks[ifid].Add (ipv4Layer);
          m_networkStacks[ifid].Add (droneControlLayer);
        } else if (dev->GetInstanceTypeId ().GetName () == "ns3::LteUeNetDevice") {
          auto lteuenetdev = StaticCast<LteUeNetDevice,NetDevice> (dev);
          NS_ASSERT(lteuenetdev->GetCcMap().begin() != lteuenetdev->GetCcMap().end());
          auto ccmap = (lteuenetdev->GetCcMap().begin())->second;

          auto pldltype = ccmap->GetPhy()->GetDlSpectrumPhy ()->GetChannel()->GetPropagationLossModel()->GetInstanceTypeId().GetName();
          auto plultype = ccmap->GetPhy()->GetUlSpectrumPhy ()->GetChannel()->GetPropagationLossModel()->GetInstanceTypeId().GetName();
          auto layer = CreateObjectWithAttributes<ProtocolLayer>
            ("InstanceType", StringValue (dev->GetInstanceTypeId ().GetName ()));
          auto phyLayer = CreateObjectWithAttributes<LteUEPhyLayer>
            ("IMSI", StringValue (std::to_string(lteuenetdev->GetImsi())),
              "PropagationLossModelDl", StringValue (pldltype),
              "PropagationLossModelUl", StringValue (plultype),
              "DlEarfcn", StringValue (std::to_string(ccmap->GetDlEarfcn())),
              "DlBandwidth", StringValue (std::to_string(ccmap->GetDlBandwidth())),
              "UlEarfcn", StringValue (std::to_string(ccmap->GetUlEarfcn())),
              "UlBandwidth", StringValue (std::to_string(ccmap->GetUlBandwidth())));

          auto ipv4Layer = CreateObjectWithAttributes<Ipv4Layer>
            ("Ipv4Address", StringValue (std::get<1>(ipv4AddressMask)),
             "BroadcastAddress", StringValue (std::get<2>(ipv4AddressMask)));
          ifaces.push_back (i);
          m_networkStacks[ifid].Add (layer);
          m_networkStacks[ifid].Add (phyLayer);
          m_networkStacks[ifid].Add (ipv4Layer);
        } else {
          auto layer = CreateObjectWithAttributes<ProtocolLayer>
            ("InstanceType", StringValue (dev->GetInstanceTypeId ().GetName ()));

          auto ipv4Layer = CreateObjectWithAttributes<Ipv4Layer>
            ("Ipv4Address", StringValue (std::get<1>(ipv4AddressMask)),
             "BroadcastAddress", StringValue (std::get<2>(ipv4AddressMask)));
          ifaces.push_back (i);
          m_networkStacks[ifid].Add (layer);
          m_networkStacks[ifid].Add (ipv4Layer);
        }
    }
}

} // namespace ns3
