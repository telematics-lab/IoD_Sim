/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2020 The IoD_Sim Authors.
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

#include <ns3/core-module.h>
#include <ns3/internet-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/config-store-module.h>
#include <ns3/applications-module.h>
//#include <ns3/drone-client.h>
//#include <ns3/drone-server.h>
#include <ns3/scenario-configuration-helper.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Scenario");

int main (int argc, char *argv[])
{
  CONFIGURATOR->Initialize(argc, argv, "LTE_BASIC_SCENARIO");

  NodeContainer enbNodes, ueNodes, hostNodes;
  enbNodes.Create (CONFIGURATOR->GetAntennasN ());
  ueNodes.Create (CONFIGURATOR->GetDronesN ());
  hostNodes.Create (CONFIGURATOR->GetRemotesN ());

  MobilityHelper dronesMobility, antennasMobility;
  Ptr<ListPositionAllocator> antennasPosition, dronesPosition;
  dronesMobility.SetMobilityModel (CONFIGURATOR->GetDronesMobilityModel ());
  dronesPosition = CreateObject<ListPositionAllocator> ();
  CONFIGURATOR->GetDronesPosition (dronesPosition);
  dronesMobility.SetPositionAllocator (dronesPosition);
  dronesMobility.Install (ueNodes);
  antennasMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  antennasPosition = CreateObject<ListPositionAllocator> ();
  CONFIGURATOR->GetAntennasPosition (antennasPosition);
  antennasMobility.SetPositionAllocator (antennasPosition);
  antennasMobility.Install (enbNodes);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  Ptr<Node> host = hostNodes.Get (0);

  InternetStackHelper internet;
  internet.Install (ueNodes);
  internet.Install (hostNodes);

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));
  NetDeviceContainer p2pDevs = p2ph.Install (pgw, host);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer hostsIpInterfaces = ipv4.Assign (p2pDevs);
  Ipv4Address hostIp = hostsIpInterfaces.GetAddress (1); // 0 is localhost

  Ipv4StaticRoutingHelper ipv4RoutingH;
  internet.SetRoutingHelper(ipv4RoutingH);
  Ptr<Ipv4StaticRouting> hostStaticRoute = ipv4RoutingH.GetStaticRouting (host->GetObject<Ipv4> ());
  hostStaticRoute->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);


  NetDeviceContainer enbDevices = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueDevices = lteHelper->InstallUeDevice (ueNodes);

  Ipv4InterfaceContainer lteDevsIfaces = epcHelper->AssignUeIpv4Address (ueDevices);

  for (uint32_t i = 0; i < ueNodes.GetN (); ++i)
    {
      Ptr<Ipv4StaticRouting> ueStaticRoute = ipv4RoutingH.GetStaticRouting (ueNodes.Get (i)->GetObject<Ipv4> ());
      ueStaticRoute->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  lteHelper->Attach (ueDevices, enbDevices.Get (0));

  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  NS_LOG_INFO("> Creating applications for host.");
  UdpEchoServerHelper echoServer (9);
  ApplicationContainer serverApps = echoServer.Install (hostNodes);
  serverApps.Start (Seconds (CONFIGURATOR->GetRemoteApplicationStartTime (0)));
  serverApps.Stop (Seconds (CONFIGURATOR->GetRemoteApplicationStopTime (0)));

  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  NS_LOG_INFO("> Creating applications for drones.");
  UdpEchoClientHelper echoClient (hostIp, 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
  ApplicationContainer clientApps = echoClient.Install (ueNodes);
  for (uint32_t i = 0; i < ueNodes.GetN(); ++i)
  {
      clientApps.Get (i)->SetStartTime(Seconds (CONFIGURATOR->GetDroneApplicationStartTime (i)));
      clientApps.Get (i)->SetStopTime(Seconds (CONFIGURATOR->GetDroneApplicationStopTime (i)));
  }

/*
  Ptr<DroneServer> server = CreateObjectWithAttributes<DroneServer>(
        "Ipv4Address", Ipv4AddressValue(hostIp),
        "Ipv4SubnetMask", Ipv4MaskValue("255.0.0.0"));
    hostNodes.Get(0)->AddApplication(server);

  for (uint32_t i = 0; i < ueNodes.GetN (); ++i)
  {
    auto client = CreateObjectWithAttributes<DroneClient>(
        "Ipv4Address", Ipv4AddressValue(lteDevsIfaces.GetAddress(i)),
        "Ipv4SubnetMask", Ipv4MaskValue("255.0.0.0"),
        "DestinationIpv4Address", Ipv4AddressValue(hostIp));
    client->SetStartTime(Seconds(1 + i));

    ueNodes.Get(i)->AddApplication(client);
  }
*/

  std::stringstream p2pPath, ipPath;
  std::string path = CONFIGURATOR->GetResultsPath ();

  p2pPath << path << "p2p";
  ipPath << path << "ip";

  p2ph.EnableAscii (p2pPath.str (), p2pDevs.Get (0));
  p2ph.EnablePcap (p2pPath.str (), p2pDevs.Get (0));

  internet.EnablePcapIpv4All (ipPath.str ());

  NS_LOG_DEBUG (lteDevsIfaces.GetAddress(0));

  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}