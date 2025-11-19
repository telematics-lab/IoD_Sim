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
#include "ns3/aodv-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/udp-client-server-helper.h"

#include "ns3/leo-module.h"
#include "ns3/test.h"

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
class IslIcmpTestCase : public TestCase
{
public:
  IslIcmpTestCase ();
  virtual ~IslIcmpTestCase ();

private:
  virtual void DoRun (void);
};

IslIcmpTestCase::IslIcmpTestCase ()
  : TestCase ("Test connectivity using ICMP")
{
}

IslIcmpTestCase::~IslIcmpTestCase ()
{
}

void
IslIcmpTestCase::DoRun (void)
{
  Time::SetResolution (Time::NS);

  std::vector<std::string> satWps =
    {
      "contrib/leo/data/test/waypoints.txt",
    };

  LeoSatNodeHelper satHelper;
  NodeContainer satellites = satHelper.Install (satWps);

  IslHelper islCh;
  islCh.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  islCh.SetDeviceAttribute ("ReceiveErrorModel", StringValue ("ns3::BurstErrorModel"));
  islCh.SetChannelAttribute ("PropagationDelay", StringValue ("ns3::ConstantSpeedPropagationDelayModel"));
  islCh.SetDeviceAttribute ("InterframeGap", TimeValue (Seconds (0.001)));
  islCh.SetChannelAttribute ("PropagationLoss", StringValue ("ns3::IslPropagationLossModel"));
  NetDeviceContainer islNet = islCh.Install (satellites);

  // Install internet stack on nodes
  InternetStackHelper stack;
  AodvHelper aodv;
  stack.SetRoutingHelper (aodv);
  stack.Install (satellites);

  // Make all networks addressable for legacy protocol
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.0.0", "255.255.0.0");
  Ipv4InterfaceContainer islIp = ipv4.Assign (islNet);

  // install echo server on all nodes
  UdpEchoServerHelper echoServer (9);
  ApplicationContainer serverApps = echoServer.Install (satellites);

  ApplicationContainer clientApps;
  for (uint32_t i = 0; i < satellites.GetN (); i ++)
    {
      UdpEchoClientHelper echoClient (islIp.GetAddress ((i+3) % satellites.GetN (), 0), 9);
      echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
      echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
      echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
      echoClient.Install (satellites.Get (i));
    }

  serverApps.Start (Seconds (1.0));
  clientApps.Start (Seconds (5.0));
  clientApps.Stop (Seconds (20.0));
  serverApps.Stop (Seconds (21.0));

  Simulator::Stop (Seconds (22.0));
  Simulator::Run ();
  Simulator::Destroy ();
}

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class IslTestSuite : public TestSuite
{
public:
  IslTestSuite ();
};

IslTestSuite::IslTestSuite ()
  : TestSuite ("leo-isl", TestSuite::Type::EXAMPLE)
{
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new IslIcmpTestCase, TestCase::Duration::EXTENSIVE);
}

// Do not forget to allocate an instance of this TestSuite
static IslTestSuite leoTestSuite;
