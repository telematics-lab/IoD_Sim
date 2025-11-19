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

#include <iostream>

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/leo-module.h"
#include "ns3/network-module.h"
#include "ns3/aodv-module.h"
#include "ns3/udp-server.h"
#include "ns3/epidemic-routing-module.h"

using namespace ns3;

// Note: TCP tracing disabled due to incompatibility with ns-3.45
// The following callback paths are not valid in this version
//static void
//EchoTxRx (std::string context, const Ptr< const Packet > packet, const TcpHeader &header, const Ptr< const TcpSocketBase > socket)
//{
//  std::cout << Simulator::Now () << ":" << context << ":" << packet->GetUid() << ":" << socket->GetNode () << ":" << header.GetSequenceNumber () << std::endl;
//}

NS_LOG_COMPONENT_DEFINE ("LeoBulkSendTracingExample");

int main (int argc, char *argv[])
{

  CommandLine cmd;
  std::string orbitFile;
  std::string traceFile;
  LeoLatLong source (51.399, 10.536);
  LeoLatLong destination (40.76, -73.96);
  std::string islRate = "2Gbps";
  std::string constellation = "TelesatGateway";
  uint16_t port = 9;
  uint32_t latGws = 20;
  uint32_t lonGws = 20;
  double duration = 100;
  bool islEnabled = true;
  bool pcap = false;
  uint64_t ttlThresh = 0;
  std::string routingProto = "aodv";

  cmd.AddValue("orbitFile", "CSV file with orbit parameters", orbitFile);
  cmd.AddValue("traceFile", "CSV file to store mobility trace in", traceFile);
  cmd.AddValue("precision", "ns3::GeoLeoOrbitMobility::Precision");
  cmd.AddValue("duration", "Duration of the simulation in seconds", duration);
  cmd.AddValue("source", "Traffic source", source);
  cmd.AddValue("destination", "Traffic destination", destination);
  cmd.AddValue("islRate", "ns3::MockNetDevice::DataRate");
  cmd.AddValue("constellation", "LEO constellation link settings name", constellation);
  cmd.AddValue("routing", "Routing protocol", routingProto);
  cmd.AddValue("islEnabled", "Enable inter-satellite links", islEnabled);
  cmd.AddValue("latGws", "Latitudal rows of gateways", latGws);
  cmd.AddValue("lonGws", "Longitudinal rows of gateways", lonGws);
  cmd.AddValue("ttlThresh", "TTL threshold", ttlThresh);
  cmd.AddValue("destOnly", "ns3::aodv::RoutingProtocol::DestinationOnly");
  cmd.AddValue("routeTimeout", "ns3::aodv::RoutingProtocol::ActiveRouteTimeout");
  cmd.AddValue("pcap", "Enable packet capture", pcap);
  cmd.Parse (argc, argv);

  std::streambuf *coutbuf = std::cout.rdbuf();
  // redirect cout if traceFile
  std::ofstream out;
  out.open (traceFile);
  if (out.is_open ())
    {
      std::cout.rdbuf(out.rdbuf());
    }

  LeoOrbitNodeHelper orbit;
  NodeContainer satellites;
  if (!orbitFile.empty())
    {
      satellites = orbit.Install (orbitFile);
    }
  else
    {
      satellites = orbit.Install ({ LeoOrbit (1200, 20, 32, 16),
      				  LeoOrbit (1180, 30, 12, 10) });
    }

  LeoGndNodeHelper ground;
  NodeContainer stations = ground.Install (latGws, lonGws);

  NodeContainer users = ground.Install (source, destination);
  stations.Add (users);

  LeoChannelHelper utCh;
  utCh.SetConstellation (constellation);
  NetDeviceContainer utNet = utCh.Install (satellites, stations);

  InternetStackHelper stack;
  if (routingProto == "epidemic")
    {
      EpidemicHelper epidemic;
      //epidemic.Set ("BeaconInterval", TimeValue (MilliSeconds (100)));
      stack.SetRoutingHelper (epidemic);
    }
  else
    {
      AodvHelper aodv;
      aodv.Set ("EnableHello", BooleanValue (false));
      //aodv.Set ("HelloInterval", TimeValue (Seconds (10)));
      if (ttlThresh != 0)
        {
        aodv.Set ("TtlThreshold", UintegerValue (ttlThresh));
        aodv.Set ("NetDiameter", UintegerValue (2*ttlThresh));
        }
      stack.SetRoutingHelper (aodv);
    }

  // Install internet stack on nodes
  stack.Install (satellites);
  stack.Install (stations);

  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("10.1.0.0", "255.255.0.0");
  ipv4.Assign (utNet);

  if (islEnabled)
    {
      std::cerr << "ISL enabled" << std::endl;
      IslHelper islCh;
      NetDeviceContainer islNet = islCh.Install (satellites);
      ipv4.SetBase ("10.2.0.0", "255.255.0.0");
      ipv4.Assign (islNet);
    }

  Ipv4Address remote = users.Get (1)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
  BulkSendHelper sender ("ns3::TcpSocketFactory",
                         InetSocketAddress (remote, port));
  // Set the amount of data to send in bytes.  Zero is unlimited.
  sender.SetAttribute ("MaxBytes", UintegerValue (0));
  ApplicationContainer sourceApps = sender.Install (users.Get (0));
  sourceApps.Start (Seconds (0.0));

//
// Create a PacketSinkApplication and install it on node 1
//
  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (users.Get (1));
  sinkApps.Start (Seconds (0.0));

  // Note: TCP tracing disabled due to incompatibility with ns-3.45
  // The following callback paths are not valid in this version
  // Config::Connect ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/Tx",
  // 		   MakeCallback (&EchoTxRx));
  // Config::Connect ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/Rx",
  // 		   MakeCallback (&EchoTxRx));

  //
  // Set up tracing if enabled
  //
  if (pcap)
    {
      AsciiTraceHelper ascii;
      utCh.EnableAsciiAll (ascii.CreateFileStream ("tcp-bulk-send.tr"));
      utCh.EnablePcapAll ("tcp-bulk-send", false);
    }

  std::cerr << "LOCAL =" << users.Get (0)->GetId () << std::endl;
  std::cerr << "REMOTE=" << users.Get (1)->GetId () << ",addr=" << Ipv4Address::ConvertFrom (remote) << std::endl;

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (duration));
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps.Get (0));
  std::cout << users.Get (0)->GetId () << ":" << users.Get (1)->GetId () << ": " << sink1->GetTotalRx () << std::endl;

  out.close ();
  std::cout.rdbuf(coutbuf);

  return 0;
}
