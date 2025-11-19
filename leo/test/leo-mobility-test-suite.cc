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

#include "ns3/leo-module.h"
#include "ns3/test.h"
#include "ns3/log.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LeoMobilityTestSuite");

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
class LeoMobilityWaypointTestCase : public TestCase
{
public:
  LeoMobilityWaypointTestCase ();
  virtual ~LeoMobilityWaypointTestCase () {}

private:
  virtual void DoRun (void);
};

LeoMobilityWaypointTestCase::LeoMobilityWaypointTestCase ()
  : TestCase ("Feed waypoints to mobility model")
{
}

void
LeoMobilityWaypointTestCase::DoRun (void)
{
  Ptr<LeoWaypointInputFileStreamContainer> container = CreateObject<LeoWaypointInputFileStreamContainer> ();
  container->SetAttribute("File", StringValue ("contrib/leo/data/test/waypoints.txt"));
  container->SetAttribute("LastTime", TimeValue (Time (0)));

  Ptr<WaypointMobilityModel> mobility = CreateObject<WaypointMobilityModel> ();
  Waypoint wp;
  while (container->GetNextSample (wp))
    {
      mobility->AddWaypoint (wp);
    }
  NS_LOG_INFO ("Model has " << mobility->WaypointsLeft () << " waypoints left");

  NS_TEST_ASSERT_MSG_EQ ((mobility->WaypointsLeft () > 2), true, "Reading waypoints from empty");
}

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoMobilityTestSuite : public TestSuite
{
public:
  LeoMobilityTestSuite ();
};

LeoMobilityTestSuite::LeoMobilityTestSuite ()
  : TestSuite ("leo-mobility", TestSuite::Type::UNIT)
{
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new LeoMobilityWaypointTestCase, TestCase::Duration::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static LeoMobilityTestSuite leoMobilityTestSuite;
