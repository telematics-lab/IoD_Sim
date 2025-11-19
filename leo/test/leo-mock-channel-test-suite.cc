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
class LeoMockChannelTransmitUnknownTestCase : public TestCase
{
public:
  LeoMockChannelTransmitUnknownTestCase () : TestCase ("transmission from unknown source fails") {}
  virtual ~LeoMockChannelTransmitUnknownTestCase () {}

private:
  virtual void DoRun (void)
  {
    Ptr<LeoMockChannel> channel = CreateObject<LeoMockChannel> ();
    Packet *packet = new Packet ();
    Ptr<Packet> p = Ptr<Packet>(packet);
    Address destAddr;
    Time txTime;
    channel->SetAttribute ("PropagationDelay", StringValue ("ns3::ConstantSpeedPropagationDelayModel"));
    channel->SetAttribute ("PropagationLoss", StringValue ("ns3::LeoPropagationLossModel"));
    bool result = channel->TransmitStart (p, 10000, destAddr, txTime);

    NS_TEST_ASSERT_MSG_EQ (result, false, "unknown source fails to deliver");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoMockChannelTransmitKnownTestCase : public TestCase
{
public:
  LeoMockChannelTransmitKnownTestCase () : TestCase ("transmission from ground to space succeeds") {}
  virtual ~LeoMockChannelTransmitKnownTestCase () {}
private:
  virtual void DoRun (void)
  {
    Ptr<LeoMockChannel> channel = CreateObject<LeoMockChannel> ();
    channel->SetAttribute ("PropagationDelay", StringValue ("ns3::ConstantSpeedPropagationDelayModel"));
    channel->SetAttribute ("PropagationLoss", StringValue ("ns3::LeoPropagationLossModel"));

    Packet *packet = new Packet ();
    Ptr<Packet> p = Ptr<Packet>(packet);

    Ptr<Node> srcNode = CreateObject<Node> ();
    Ptr<LeoMockNetDevice> srcDev = CreateObject<LeoMockNetDevice> ();
    srcDev->SetNode (srcNode);
    srcDev->SetDeviceType (LeoMockNetDevice::GND);
    srcDev->SetAddress (Mac48Address::Allocate ());
    int32_t srcId = channel->Attach (srcDev);

    Ptr<Node> dstNode = CreateObject<Node> ();
    Ptr<LeoMockNetDevice> dstDev = CreateObject<LeoMockNetDevice> ();
    dstDev->SetNode (dstNode);
    dstDev->SetDeviceType (LeoMockNetDevice::SAT);
    dstDev->SetAddress (Mac48Address::Allocate ());
    channel->Attach (dstDev);

    Address destAddr = dstDev->GetAddress ();
    Time txTime;
    bool result = channel->TransmitStart (p, srcId, destAddr, txTime);

    NS_TEST_ASSERT_MSG_EQ (result, true, "known source does not deliver");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoMockChannelTransmitSpaceGroundTestCase : public TestCase
{
public:
  LeoMockChannelTransmitSpaceGroundTestCase () : TestCase ("transmission from space to ground succeeds") {}
  virtual ~LeoMockChannelTransmitSpaceGroundTestCase () {}
private:
  virtual void DoRun (void)
  {
    Ptr<LeoMockChannel> channel = CreateObject<LeoMockChannel> ();
    channel->SetAttribute ("PropagationDelay", StringValue ("ns3::ConstantSpeedPropagationDelayModel"));
    channel->SetAttribute ("PropagationLoss", StringValue ("ns3::LeoPropagationLossModel"));

    Packet *packet = new Packet ();
    Ptr<Packet> p = Ptr<Packet>(packet);

    Ptr<Node> srcNode = CreateObject<Node> ();
    Ptr<LeoMockNetDevice> srcDev = CreateObject<LeoMockNetDevice> ();
    srcDev->SetNode (srcNode);
    srcDev->SetDeviceType (LeoMockNetDevice::SAT);
    srcDev->SetAddress (Mac48Address::Allocate ());
    int32_t srcId = channel->Attach (srcDev);

    Ptr<Node> dstNode = CreateObject<Node> ();
    Ptr<LeoMockNetDevice> dstDev = CreateObject<LeoMockNetDevice> ();
    dstDev->SetNode (dstNode);
    dstDev->SetDeviceType (LeoMockNetDevice::GND);
    dstDev->SetAddress (Mac48Address::Allocate ());
    channel->Attach (dstDev);

    Address destAddr = dstDev->GetAddress ();
    Time txTime;
    bool result = channel->TransmitStart (p, srcId, destAddr, txTime);

    NS_TEST_ASSERT_MSG_EQ (result, true, "space to ground transmission failed");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoMockChannelTransmitSpaceSpaceTestCase : public TestCase
{
public:
  LeoMockChannelTransmitSpaceSpaceTestCase () : TestCase ("transmission from space to space fails") {}
  virtual ~LeoMockChannelTransmitSpaceSpaceTestCase () {}
private:
  virtual void DoRun (void)
  {
    Ptr<LeoMockChannel> channel = CreateObject<LeoMockChannel> ();
    channel->SetAttribute ("PropagationDelay", StringValue ("ns3::ConstantSpeedPropagationDelayModel"));
    channel->SetAttribute ("PropagationLoss", StringValue ("ns3::LeoPropagationLossModel"));

    Packet *packet = new Packet ();
    Ptr<Packet> p = Ptr<Packet>(packet);

    Ptr<Node> srcNode = CreateObject<Node> ();
    Ptr<LeoMockNetDevice> srcDev = CreateObject<LeoMockNetDevice> ();
    srcDev->SetNode (srcNode);
    srcDev->SetDeviceType (LeoMockNetDevice::SAT);
    srcDev->SetAddress (Mac48Address::Allocate ());
    int32_t srcId = channel->Attach (srcDev);

    Ptr<Node> dstNode = CreateObject<Node> ();
    Ptr<LeoMockNetDevice> dstDev = CreateObject<LeoMockNetDevice> ();
    dstDev->SetNode (dstNode);
    dstDev->SetDeviceType (LeoMockNetDevice::SAT);
    dstDev->SetAddress (Mac48Address::Allocate ());
    channel->Attach (dstDev);

    Address destAddr = dstDev->GetAddress ();
    Time txTime;
    bool result = channel->TransmitStart (p, srcId, destAddr, txTime);

    NS_TEST_ASSERT_MSG_EQ (result, false, "space to space transmission succeeded");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoMockChannelTransmitGroundGroundTestCase : public TestCase
{
public:
  LeoMockChannelTransmitGroundGroundTestCase () : TestCase ("transmission from ground to ground fails") {}
  virtual ~LeoMockChannelTransmitGroundGroundTestCase () {}
private:
  virtual void DoRun (void)
  {
    Ptr<LeoMockChannel> channel = CreateObject<LeoMockChannel> ();
    channel->SetAttribute ("PropagationDelay", StringValue ("ns3::ConstantSpeedPropagationDelayModel"));
    channel->SetAttribute ("PropagationLoss", StringValue ("ns3::LeoPropagationLossModel"));

    Packet *packet = new Packet ();
    Ptr<Packet> p = Ptr<Packet>(packet);

    Ptr<Node> srcNode = CreateObject<Node> ();
    Ptr<LeoMockNetDevice> srcDev = CreateObject<LeoMockNetDevice> ();
    srcDev->SetNode (srcNode);
    srcDev->SetDeviceType (LeoMockNetDevice::GND);
    srcDev->SetAddress (Mac48Address::Allocate ());
    int32_t srcId = channel->Attach (srcDev);

    Ptr<Node> dstNode = CreateObject<Node> ();
    Ptr<LeoMockNetDevice> dstDev = CreateObject<LeoMockNetDevice> ();
    dstDev->SetNode (dstNode);
    dstDev->SetDeviceType (LeoMockNetDevice::GND);
    dstDev->SetAddress (Mac48Address::Allocate ());
    channel->Attach (dstDev);

    Address destAddr = dstDev->GetAddress ();
    Time txTime;
    bool result = channel->TransmitStart (p, srcId, destAddr, txTime);

    NS_TEST_ASSERT_MSG_EQ (result, false, "ground to ground transmission succeeded");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoMockChannelTestSuite : public TestSuite
{
public:
  LeoMockChannelTestSuite ();
};

LeoMockChannelTestSuite::LeoMockChannelTestSuite ()
  : TestSuite ("leo-mock-channel", TestSuite::Type::UNIT)
{
  AddTestCase (new LeoMockChannelTransmitUnknownTestCase, TestCase::Duration::QUICK);
  AddTestCase (new LeoMockChannelTransmitKnownTestCase, TestCase::Duration::QUICK);
  AddTestCase (new LeoMockChannelTransmitSpaceGroundTestCase, TestCase::Duration::QUICK);
  AddTestCase (new LeoMockChannelTransmitSpaceSpaceTestCase, TestCase::Duration::QUICK);
  AddTestCase (new LeoMockChannelTransmitGroundGroundTestCase, TestCase::Duration::QUICK);
}

static LeoMockChannelTestSuite islMockChannelTestSuite;
