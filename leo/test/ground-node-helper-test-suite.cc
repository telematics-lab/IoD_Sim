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
class EmptyGndNodeHelperTestCase : public TestCase
{
public:
  EmptyGndNodeHelperTestCase ();
  virtual ~EmptyGndNodeHelperTestCase ();

private:
  virtual void DoRun (void);
};

EmptyGndNodeHelperTestCase::EmptyGndNodeHelperTestCase ()
  : TestCase ("Empty list of positions")
{
}

EmptyGndNodeHelperTestCase::~EmptyGndNodeHelperTestCase ()
{
}

void
EmptyGndNodeHelperTestCase::DoRun (void)
{
  std::vector<std::string> gndWps = {};

  LeoGndNodeHelper gndHelper;
  NodeContainer nodes = gndHelper.Install ("hsksjhskjhs");

  NS_ASSERT_MSG (nodes.GetN () == 0, "Empty position file produces non-empty node container");
}

// ------------------------------------------------------------------------- //

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class SomeGndNodeHelperTestCase : public TestCase
{
public:
  SomeGndNodeHelperTestCase ();
  virtual ~SomeGndNodeHelperTestCase ();

private:
  virtual void DoRun (void);
};

SomeGndNodeHelperTestCase::SomeGndNodeHelperTestCase ()
  : TestCase ("Some ground stations")
{
}

SomeGndNodeHelperTestCase::~SomeGndNodeHelperTestCase ()
{
}

void
SomeGndNodeHelperTestCase::DoRun (void)
{
  LeoGndNodeHelper gndHelper;
  NodeContainer nodes = gndHelper.Install (LeoLatLong (50.1, 10.0),
  					   LeoLatLong (-70.1, -21.0));

  NS_ASSERT_MSG (nodes.GetN () == 2, "No ground stations");

  Ptr<MobilityModel> mob = nodes.Get (0)->GetObject<MobilityModel> ();
  NS_ASSERT_MSG (mob != Ptr<MobilityModel> (), "Mobility model is valid");
}

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class GndNodeHelperTestSuite : public TestSuite
{
public:
  GndNodeHelperTestSuite ();
};

GndNodeHelperTestSuite::GndNodeHelperTestSuite ()
  : TestSuite ("leo-gnd-node-helper", TestSuite::Type::UNIT)
{
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new EmptyGndNodeHelperTestCase, TestCase::Duration::QUICK);
  AddTestCase (new SomeGndNodeHelperTestCase, TestCase::Duration::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static GndNodeHelperTestSuite gndNodeHelperTestSuite;
