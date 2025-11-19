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
class EmptySatNodeHelperTestCase : public TestCase
{
public:
  EmptySatNodeHelperTestCase ();
  virtual ~EmptySatNodeHelperTestCase ();

private:
  virtual void DoRun (void);
};

EmptySatNodeHelperTestCase::EmptySatNodeHelperTestCase ()
  : TestCase ("Empty list of waypoint files")
{
}

EmptySatNodeHelperTestCase::~EmptySatNodeHelperTestCase ()
{
}

void
EmptySatNodeHelperTestCase::DoRun (void)
{
  std::vector<std::string> satWps = {};

  LeoSatNodeHelper satHelper;
  NodeContainer satellites = satHelper.Install (satWps);

  NS_ASSERT_MSG (satellites.GetN () == 0, "Empty list of waypoint files produces non-empty node container");
}

// ------------------------------------------------------------------------- //

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class SingleSatNodeHelperTestCase : public TestCase
{
public:
  SingleSatNodeHelperTestCase ();
  virtual ~SingleSatNodeHelperTestCase ();

private:
  virtual void DoRun (void);
};

SingleSatNodeHelperTestCase::SingleSatNodeHelperTestCase ()
  : TestCase ("Single waypoint file")
{
}

SingleSatNodeHelperTestCase::~SingleSatNodeHelperTestCase ()
{
}

void
SingleSatNodeHelperTestCase::DoRun (void)
{
  std::vector<std::string> satWps =
    {
      "contrib/leo/data/test/waypoints.txt"
    };

  LeoSatNodeHelper satHelper;
  NodeContainer satellites = satHelper.Install (satWps);

  NS_ASSERT_MSG (satellites.GetN () == 1, "No satellite nodes");

  Ptr<MobilityModel> mob = satellites.Get (0)->GetObject<MobilityModel> ();
  NS_ASSERT_MSG (mob != Ptr<MobilityModel> (), "Mobility model is valid");
}

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class SatNodeHelperTestSuite : public TestSuite
{
public:
  SatNodeHelperTestSuite ();
};

SatNodeHelperTestSuite::SatNodeHelperTestSuite ()
  : TestSuite ("leo-sat-node-helper", TestSuite::Type::UNIT)
{
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new EmptySatNodeHelperTestCase, TestCase::Duration::QUICK);
  AddTestCase (new SingleSatNodeHelperTestCase, TestCase::Duration::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static SatNodeHelperTestSuite satNodeHelperTestSuite;
