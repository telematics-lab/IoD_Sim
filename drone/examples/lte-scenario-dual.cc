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
#include <ns3/drone-client.h>
#include <ns3/drone-server.h>
#include <ns3/config-store-module.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Scenario");

int main (int argc, char *argv[])
{
  LogComponentEnable("PointToPointEpcHelper", LOG_LEVEL_ALL);
  LogComponentEnable("NoBackhaulEpcHelper", LOG_LEVEL_ALL);

  NodeContainer enbNodes, ueNodes, hostNodes;
  enbNodes.Create (2);
  ueNodes.Create (3);
  hostNodes.Create (1);
  Ptr<Node> router = CreateObject<Node> ();

  MobilityHelper dronesMobility;
  dronesMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  dronesMobility.Install(ueNodes);
  for (uint32_t i=0; i<ueNodes.GetN(); ++i)
  {
    // drones placed on the x axis and spaced by 10 meters (offset of 10 meters from the origin)
    ueNodes.Get(i)->GetObject<MobilityModel>()->SetPosition(Vector(i*10-10, 0, 10));
  }

  MobilityHelper antennasMobility;
  antennasMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  antennasMobility.Install (enbNodes);
  for (uint32_t i=0; i<enbNodes.GetN(); ++i)
  {
    // antennas placed on the y axis and spaced by 100 meters (offset of 50 meter from the origin)
    enbNodes.Get(i)->GetObject<MobilityModel>()->SetPosition(Vector(0, i*100-50, 40));
  }

  Config::SetDefault("ns3::NoBackhaulEpcHelper::UePgwAddressBase", StringValue("100.0.0.0"));
  ConfigStore config;
  config.ConfigureDefaults();

  Ptr<LteHelper> lteHelper1 = CreateObject<LteHelper> ();
  /*
  Ptr<PointToPointEpcHelper> epcHelper1 = CreateObjectWithAttributes<PointToPointEpcHelper> (
      "UePgwAddressBase", Ipv4AddressValue("100.0.0.0"),
      "UePgwAddressMask", Ipv4MaskValue("255.255.255.0"));
  */
  Ptr<PointToPointEpcHelper> epcHelper1 = CreateObject<PointToPointEpcHelper> ();
  lteHelper1->SetEpcHelper (epcHelper1);

  Config::SetDefault("ns3::NoBackhaulEpcHelper::UePgwAddressBase", StringValue("200.0.0.0"));
  config.ConfigureDefaults();

  Ptr<LteHelper> lteHelper2 = CreateObject<LteHelper> ();
  /*
  Ptr<PointToPointEpcHelper> epcHelper2 = CreateObjectWithAttributes<PointToPointEpcHelper> (
      "UePgwAddressBase", Ipv4AddressValue("200.0.0.0"),
      "UePgwAddressMask", Ipv4MaskValue("255.255.255.0"));
  */
  Ptr<PointToPointEpcHelper> epcHelper2 = CreateObject<PointToPointEpcHelper> ();
  lteHelper2->SetEpcHelper (epcHelper2);

  InternetStackHelper internet;
  internet.Install (ueNodes);
  internet.Install (hostNodes);
  internet.Install (router);

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));
  NetDeviceContainer p2pDevs1 = p2ph.Install (epcHelper1->GetPgwNode (), router);
  NetDeviceContainer p2pDevs2 = p2ph.Install (epcHelper1->GetPgwNode (), router);
  NetDeviceContainer p2pDevsHost = p2ph.Install (hostNodes.Get(0), router);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.10.10.0", "255.255.255.0");
  Ipv4InterfaceContainer ipInterfaces1 = ipv4.Assign (p2pDevs1);
  ipv4.SetBase ("10.10.20.0", "255.255.255.0");
  Ipv4InterfaceContainer ipInterfaces2 = ipv4.Assign (p2pDevs2);
  ipv4.SetBase ("10.10.30.0", "255.255.255.0");
  Ipv4InterfaceContainer ipInterfacesHost = ipv4.Assign (p2pDevsHost);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

/*
  Ipv4StaticRoutingHelper ipv4RoutingH;
  internet.SetRoutingHelper(ipv4RoutingH);
  Ptr<Ipv4StaticRouting> hostStaticRoute = ipv4RoutingH.GetStaticRouting (hostNodes.Get(0)->GetObject<Ipv4> ());
  hostStaticRoute->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  */

  NodeContainer ueNodes1, ueNodes2;
  ueNodes1.Add(ueNodes.Get(0));
  ueNodes2.Add(ueNodes.Get(1));
  ueNodes2.Add(ueNodes.Get(2));

  NodeContainer enbNodes1, enbNodes2;
  enbNodes1.Add(enbNodes.Get(0));
  enbNodes2.Add(enbNodes.Get(1));



  NetDeviceContainer enbDevices1 = lteHelper1->InstallEnbDevice (enbNodes1);
  NetDeviceContainer ueDevices1 = lteHelper1->InstallUeDevice (ueNodes1);


  Ipv4InterfaceContainer lteDevsIfaces1 = epcHelper1->AssignUeIpv4Address (ueDevices1);

  /*
  for (uint32_t i = 0; i < ueNodes.GetN (); ++i)
    {
      Ptr<Ipv4StaticRouting> ueStaticRoute = ipv4RoutingH.GetStaticRouting (ueNodes.Get (i)->GetObject<Ipv4> ());
      ueStaticRoute->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }
  */

  lteHelper1->Attach (ueDevices1);



  NetDeviceContainer enbDevices2 = lteHelper2->InstallEnbDevice (enbNodes2);
  NetDeviceContainer ueDevices2 = lteHelper2->InstallUeDevice (ueNodes2);


  Ipv4InterfaceContainer lteDevsIfaces2 = epcHelper2->AssignUeIpv4Address (ueDevices2);

  lteHelper2->Attach (ueDevices2);



  Ipv4Address hostIp = ipInterfacesHost.GetAddress (1); // 0 is localhost

  Ptr<DroneServer> server = CreateObjectWithAttributes<DroneServer>(
        "Ipv4Address", Ipv4AddressValue(hostIp),
        "Ipv4SubnetMask", Ipv4MaskValue("255.0.0.0"));
    hostNodes.Get(0)->AddApplication(server);

  for (uint32_t i = 0; i < ueNodes.GetN (); ++i)
  {
    auto client = CreateObjectWithAttributes<DroneClient>(
        "Ipv4Address", Ipv4AddressValue(ueNodes.Get(i)->GetObject<Ipv4>()->GetAddress(0, 1).GetLocal()),
        "Ipv4SubnetMask", Ipv4MaskValue("255.0.0.0"),
        "DestinationIpv4Address", Ipv4AddressValue(hostIp));
    client->SetStartTime(Seconds(1 + i));

    ueNodes.Get(i)->AddApplication(client);
  }

  std::stringstream p2pPath, ipPath;

  p2pPath << "p2p";
  ipPath << "ip";

  //p2ph.EnableAscii (p2pPath.str (), p2pDevs.Get (0));
  //p2ph.EnablePcap (p2pPath.str (), p2pDevs.Get (0));

  internet.EnablePcapIpv4All (ipPath.str ());

  NS_LOG_DEBUG (hostIp);
  NS_LOG_DEBUG (lteDevsIfaces1.GetAddress(0));
  NS_LOG_DEBUG (lteDevsIfaces2.GetAddress(0));
  NS_LOG_DEBUG (lteDevsIfaces2.GetAddress(1));

  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}