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
class LeoWaypointFileEmptyTestCase : public TestCase
{
public:
  LeoWaypointFileEmptyTestCase ();
  virtual ~LeoWaypointFileEmptyTestCase ();

private:
  virtual void DoRun (void);
};

LeoWaypointFileEmptyTestCase::LeoWaypointFileEmptyTestCase ()
  : TestCase ("Test reading from empty file")
{
}

LeoWaypointFileEmptyTestCase::~LeoWaypointFileEmptyTestCase ()
{
}

void
LeoWaypointFileEmptyTestCase::DoRun (void)
{
  Ptr<LeoWaypointInputFileStreamContainer> container = CreateObject<LeoWaypointInputFileStreamContainer> ();
  container->SetAttribute("File", StringValue ("contrib/leo/data/test/empty"));
  container->SetAttribute("LastTime", TimeValue (Time (1)));
  Waypoint wp;

  uint32_t i = 0;
  while (container->GetNextSample (wp))
    {
      i ++;
    }

  NS_TEST_ASSERT_MSG_EQ ((i == 0), true, "Reading waypoints from empty");
}

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoWaypointSomeEntriesTestCase : public TestCase
{
public:
  LeoWaypointSomeEntriesTestCase ();
  virtual ~LeoWaypointSomeEntriesTestCase ();

private:
  virtual void DoRun (void);
};

LeoWaypointSomeEntriesTestCase::LeoWaypointSomeEntriesTestCase ()
  : TestCase ("Test reading from non-empty file")
{
}

LeoWaypointSomeEntriesTestCase::~LeoWaypointSomeEntriesTestCase ()
{
}

void
LeoWaypointSomeEntriesTestCase::DoRun (void)
{
  Ptr<LeoWaypointInputFileStreamContainer> container = CreateObject<LeoWaypointInputFileStreamContainer> ();
  container->SetAttribute("File", StringValue ("contrib/leo/data/test/waypoints.txt"));
  container->SetAttribute("LastTime", TimeValue (Time (1)));
  Waypoint wp;

  uint32_t i = 0;
  while (container->GetNextSample (wp))
    {
      i ++;
    }

  NS_TEST_ASSERT_MSG_EQ ((i > 0), true, "Reading from non-empty stream succeeds");
}

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoWaypointsTestSuite : public TestSuite
{
public:
  LeoWaypointsTestSuite ();
};

LeoWaypointsTestSuite::LeoWaypointsTestSuite ()
  : TestSuite ("leo-waypoints", TestSuite::Type::UNIT)
{
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new LeoWaypointFileEmptyTestCase, TestCase::Duration::QUICK);
  AddTestCase (new LeoWaypointSomeEntriesTestCase, TestCase::Duration::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static LeoWaypointsTestSuite leoWaypointsTestSuite;
