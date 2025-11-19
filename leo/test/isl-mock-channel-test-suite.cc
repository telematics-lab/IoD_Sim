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
class IslMockChannelTransmitUnknownTestCase : public TestCase
{
public:
  IslMockChannelTransmitUnknownTestCase () : TestCase ("transmission to broadcast address succeeds") {}
  virtual ~IslMockChannelTransmitUnknownTestCase () {}
private:
  virtual void DoRun (void)
  {
    Ptr<IslMockChannel> channel = CreateObject<IslMockChannel> ();
    Packet *packet = new Packet ();
    Ptr<Packet> p = Ptr<Packet>(packet);
    Ptr<MockNetDevice> dev = CreateObject<MockNetDevice> ();
    dev->SetAddress (Mac48Address::Allocate ());
    int32_t srcId = channel->Attach (dev);
    Address destAddr = Mac48Address::GetBroadcast ();
    Time txTime;
    bool result = channel->TransmitStart (p, srcId, destAddr, txTime);

    NS_TEST_ASSERT_MSG_EQ (result, true, "broadcast address does not transmit");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class IslMockChannelTransmitKnownTestCase : public TestCase
{
public:
  IslMockChannelTransmitKnownTestCase () : TestCase ("transmission to known destination succeeds") {}
  virtual ~IslMockChannelTransmitKnownTestCase () {}
private:
  virtual void DoRun (void)
  {
    Ptr<IslMockChannel> channel = CreateObject<IslMockChannel> ();

    Packet *packet = new Packet ();
    Ptr<Packet> p = Ptr<Packet>(packet);

    Ptr<Node> srcNode = CreateObject<Node> ();
    Ptr<MockNetDevice> srcDev = CreateObject<MockNetDevice> ();
    srcDev->SetNode (srcNode);
    srcDev->SetAddress (Mac48Address::Allocate ());
    int32_t srcId = channel->Attach (srcDev);

    Ptr<Node> dstNode = CreateObject<Node> ();
    Ptr<MockNetDevice> dstDev = CreateObject<MockNetDevice> ();
    dstDev->SetNode (dstNode);
    dstDev->SetAddress (Mac48Address::Allocate ());
    channel->Attach (dstDev);

    Address destAddr = dstDev->GetAddress ();
    Time txTime;
    bool result = channel->TransmitStart (p, srcId, destAddr, txTime);

    NS_TEST_ASSERT_MSG_EQ (result, true, "known destination did not deliver");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class IslMockChannelTestSuite : public TestSuite
{
public:
  IslMockChannelTestSuite ();
};

IslMockChannelTestSuite::IslMockChannelTestSuite ()
  : TestSuite ("leo-isl-mock-channel", TestSuite::Type::UNIT)
{
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new IslMockChannelTransmitUnknownTestCase, TestCase::Duration::QUICK);
  AddTestCase (new IslMockChannelTransmitKnownTestCase, TestCase::Duration::QUICK);
  // TODO more test
}

// Do not forget to allocate an instance of this TestSuite
static IslMockChannelTestSuite islMockChannelTestSuite;
