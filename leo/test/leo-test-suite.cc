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
#include "ns3/applications-module.h"
#include "ns3/node-container.h"
#include "ns3/core-module.h"
#include "ns3/aodv-module.h"
#include "ns3/test.h"

#include "ns3/leo-module.h"

using namespace ns3;

/**
 * \ingroup leo
 * \defgroup leo-test LEO module tests
 */


/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoTestCase1 : public TestCase
{
public:
  LeoTestCase1 ();
  virtual ~LeoTestCase1 ();

private:
  virtual void DoRun (void);
  NodeContainer MakeSomeNodes (Vector position, size_t amount);
};

LeoTestCase1::LeoTestCase1 ()
  : TestCase ("Leo test case (does nothing)")
{
}

LeoTestCase1::~LeoTestCase1 ()
{
}

NodeContainer
LeoTestCase1::MakeSomeNodes (Vector position, size_t amount)
{
  NodeContainer nodes;
  for (uint32_t i = 0; i < amount; i ++)
    {
      Ptr<ConstantPositionMobilityModel> mob = CreateObject<ConstantPositionMobilityModel> ();
      mob->SetPosition (Vector (i, 0, 0) + position);
      Ptr<Node> node = CreateObject<Node> ();
      node->AggregateObject (mob);
      nodes.Add (node);
    }

  return nodes;
}

void
LeoTestCase1::DoRun (void)
{
  Time::SetResolution (Time::NS);

  //std::vector<std::string> satWps =
  //  {
  //    "contrib/leo/data/test/waypoints.txt",
  //    "contrib/leo/data/test/waypoints.txt",
  //    "contrib/leo/data/test/waypoints.txt",
  //    "contrib/leo/data/test/waypoints.txt",
  //  };

  //LeoSatNodeHelper satHelper;
  //NodeContainer satellites = satHelper.Install (satWps);
  //LeoGndNodeHelper gndHelper;
  //NodeContainer gateways = gndHelper.Install ("contrib/leo/data/test/ground-stations.txt");
  //NodeContainer terminals = gndHelper.Install ("contrib/leo/data/test/ground-stations.txt");
  NodeContainer satellites = MakeSomeNodes (Vector (0, 0, 1), 2);
  // TODO disable forwarding for terminals, only for gateways
  NodeContainer terminals = MakeSomeNodes (Vector (0, 0, 0), 2);

  NetDeviceContainer islNet, utNet;

  IslHelper islCh;
  islCh.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  islCh.SetDeviceAttribute ("ReceiveErrorModel", StringValue ("ns3::BurstErrorModel"));
  islCh.SetChannelAttribute ("PropagationDelay", StringValue ("ns3::ConstantSpeedPropagationDelayModel"));
  islCh.SetDeviceAttribute ("InterframeGap", TimeValue (Seconds (0.001)));
  islCh.SetChannelAttribute ("PropagationLoss", StringValue ("ns3::IslPropagationLossModel"));
  islNet = islCh.Install (satellites);

  LeoChannelHelper utCh;
  utCh.SetGndDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  utCh.SetGndDeviceAttribute ("ReceiveErrorModel", StringValue ("ns3::BurstErrorModel"));
  utCh.SetSatDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  utCh.SetSatDeviceAttribute ("ReceiveErrorModel", StringValue ("ns3::BurstErrorModel"));
  utCh.SetChannelAttribute ("PropagationDelay", StringValue ("ns3::ConstantSpeedPropagationDelayModel"));
  utCh.SetSatDeviceAttribute ("InterframeGap", TimeValue (Seconds (0.001)));
  utCh.SetGndDeviceAttribute ("InterframeGap", TimeValue (Seconds (0.001)));
  utCh.SetChannelAttribute ("PropagationLoss", StringValue ("ns3::LeoPropagationLossModel"));
  utCh.SetChannelAttribute ("PropagationDelay", StringValue ("ns3::ConstantSpeedPropagationDelayModel"));
  utNet = utCh.Install (satellites, terminals);

  // Install internet stack on nodes
  InternetStackHelper stack;
  AodvHelper aodv;
  stack.SetRoutingHelper (aodv);
  stack.Install (satellites);
  stack.Install (terminals);

  // Make all networks addressable for legacy protocol
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.0.0", "255.255.0.0");
  Ipv4InterfaceContainer islIp = ipv4.Assign (islNet);
  ipv4.SetBase ("10.3.0.0", "255.255.0.0");
  Ipv4InterfaceContainer utIp = ipv4.Assign (utNet);

  // we want to ping terminals
  UdpEchoServerHelper echoServer (9);
  ApplicationContainer serverApps = echoServer.Install (terminals.Get (1));

  // install a client on one of the terminals
  ApplicationContainer clientApps;
  Address remote = utIp.GetAddress (3, 0);
  UdpEchoClientHelper echoClient (remote, 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (2.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
  clientApps.Add (echoClient.Install (terminals.Get (0)));

  serverApps.Start (Seconds (1.0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (60));
  serverApps.Stop (Seconds (60));

  Simulator::Stop (Seconds (60));
  Simulator::Run ();
  Simulator::Destroy ();
}

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoTestSuite : public TestSuite
{
public:
  LeoTestSuite ();
};

LeoTestSuite::LeoTestSuite ()
  : TestSuite ("leo", TestSuite::Type::EXAMPLE)
{
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new LeoTestCase1, TestCase::Duration::EXTENSIVE);
}

// Do not forget to allocate an instance of this TestSuite
static LeoTestSuite leoTestSuite;
