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
#include "ns3/node-container.h"
#include "ns3/mobility-helper.h"
#include "ns3/test.h"
#include "ns3/integer.h"
#include "ns3/nstime.h"
#include "ns3/geographic-positions.h"
#include "../model/leo-circular-orbit-mobility-model.h"

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
class LeoOrbitSpeedTestCase : public TestCase
{
public:
  LeoOrbitSpeedTestCase () : TestCase ("speed at 0 altitude is earth rotation speed") {}
  virtual ~LeoOrbitSpeedTestCase () {}
private:
  virtual void DoRun (void)
  {
    Ptr<LeoCircularOrbitMobilityModel> mob = CreateObject<LeoCircularOrbitMobilityModel> ();
    mob->SetAttribute ("Altitude", DoubleValue (0.0));

    NS_TEST_ASSERT_MSG_EQ ((uint64_t) mob->GetSpeed (), (uint64_t) 7909.79, "postion at 0 altitude is on surface");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoOrbitPositionTestCase : public TestCase
{
public:
  LeoOrbitPositionTestCase () : TestCase ("position is on surface") {}
  virtual ~LeoOrbitPositionTestCase () {}
private:
  virtual void DoRun (void)
  {
    Ptr<LeoCircularOrbitMobilityModel> mob = CreateObject<LeoCircularOrbitMobilityModel> ();
    mob->SetAttribute ("Altitude", DoubleValue (0.0));
    mob->SetAttribute ("Inclination", DoubleValue (1.0));

    NS_TEST_ASSERT_MSG_EQ_TOL (mob->GetPosition ().GetLength() / 1000, GeographicPositions::EARTH_SPHERE_RADIUS/1000.0, 0.1, "unexpected position on earths surface for 1 deg inclination");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoOrbitProgressTestCase : public TestCase
{
public:
  LeoOrbitProgressTestCase () : TestCase ("position changes over time") {}
  virtual ~LeoOrbitProgressTestCase () {}
private:
  void TestLengthPosition (double expl, double expx, Ptr<LeoCircularOrbitMobilityModel> mob)
  {
    Vector pos = mob->GetPosition ();
    NS_TEST_EXPECT_MSG_EQ_TOL (pos.GetLength () / 1000, expl, 0.001, "Distance to earth should be the same");
    NS_TEST_EXPECT_MSG_NE (pos.x, expx, "position should not be equal");
  }

  virtual void DoRun (void)
  {
    Ptr<LeoCircularOrbitMobilityModel> mob = CreateObject<LeoCircularOrbitMobilityModel> ();
    mob->SetAttribute ("Altitude", DoubleValue (0.0));
    mob->SetAttribute ("Inclination", DoubleValue (20.0));

    Vector pos = mob->GetPosition ();
    Simulator::Schedule (Seconds (100.0), &LeoOrbitProgressTestCase::TestLengthPosition, this, GeographicPositions::EARTH_SPHERE_RADIUS/1000.0, pos.x, mob);
    Simulator::Stop (Seconds (101.0));
    Simulator::Run ();
    Simulator::Destroy ();
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoOrbitLatitudeTestCase : public TestCase
{
public:
  LeoOrbitLatitudeTestCase () : TestCase ("neighboring satellite planes have offset") {}
  virtual ~LeoOrbitLatitudeTestCase () {}
private:
  virtual void DoRun (void)
  {
    Ptr<LeoCircularOrbitMobilityModel> mob = CreateObject<LeoCircularOrbitMobilityModel> ();
    mob->SetAttribute ("Altitude", DoubleValue (1000.0));
    mob->SetAttribute ("Inclination", DoubleValue (20.0));

    Vector pos1 = mob->GetPosition ();

    mob->SetPosition (Vector3D (20.0, 0, 0));
    Vector pos2 = mob->GetPosition ();

    NS_TEST_ASSERT_MSG_NE (pos1.x, pos2.x, "neighboring satellite planes should have different position");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoOrbitOffsetTestCase : public TestCase
{
public:
  LeoOrbitOffsetTestCase () : TestCase ("neighboring satellites have offset") {}
  virtual ~LeoOrbitOffsetTestCase () {}
private:
  virtual void DoRun (void)
  {
    Ptr<LeoCircularOrbitMobilityModel> mob = CreateObject<LeoCircularOrbitMobilityModel> ();
    mob->SetAttribute ("Altitude", DoubleValue (1000.0));
    mob->SetAttribute ("Inclination", DoubleValue (20.0));

    Vector pos1 = mob->GetPosition ();

    mob->SetPosition (Vector3D (0, 20.0, 0));
    Vector pos2 = mob->GetPosition ();

    NS_TEST_ASSERT_MSG_NE (pos1.x, pos2.x, "neighboring satellite should have different position");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoOrbitTracingTestCase : public TestCase
{
public:
  LeoOrbitTracingTestCase () : TestCase ("position changes are traced") {}
  virtual ~LeoOrbitTracingTestCase () {}

  static uint64_t traced;

  static void CourseChange (std::string context, Ptr<const MobilityModel> position);
private:
  virtual void DoRun (void)
  {
    Ptr<LeoCircularOrbitMobilityModel> mob = CreateObject<LeoCircularOrbitMobilityModel> ();
    mob->SetAttribute ("Altitude", DoubleValue (1000.0));
    mob->SetAttribute ("Inclination", DoubleValue (20.0));
    mob->SetAttribute ("Precision", TimeValue (Seconds (10)));

    NodeContainer c;
    c.Create (10000);

    MobilityHelper mobility;
    mobility.SetPositionAllocator ("ns3::LeoCircularOrbitPostionAllocator",
                                   "NumOrbits", IntegerValue (100),
                                   "NumSatellites", IntegerValue (100));
    mobility.SetMobilityModel ("ns3::LeoCircularOrbitMobilityModel");
    mobility.Install (c);

    Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
                     MakeCallback (&CourseChange));
    traced = 0;

    Simulator::Stop (Seconds (100.0));
    Simulator::Run ();

    NS_TEST_ASSERT_MSG_GT (traced, 8, "position not every time");

    Simulator::Destroy ();
  }
};

uint64_t LeoOrbitTracingTestCase::traced = 0;

void LeoOrbitTracingTestCase::CourseChange (std::string context, Ptr<const MobilityModel> position)
  {
    traced ++;
    Vector pos = position->GetPosition ();
    std::cout << Simulator::Now () << ", pos=" << position << ", x=" << pos.x << ", y=" << pos.y
      << ", z=" << pos.z << std::endl;
  }


/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoOrbitTestSuite : TestSuite
{
public:
  LeoOrbitTestSuite() : TestSuite ("leo-orbit", TestSuite::Type::UNIT) {
      AddTestCase (new LeoOrbitSpeedTestCase, TestCase::Duration::QUICK);
      AddTestCase (new LeoOrbitPositionTestCase, TestCase::Duration::QUICK);
      AddTestCase (new LeoOrbitProgressTestCase, TestCase::Duration::QUICK);
      AddTestCase (new LeoOrbitLatitudeTestCase, TestCase::Duration::QUICK);
      AddTestCase (new LeoOrbitOffsetTestCase, TestCase::Duration::QUICK);
      AddTestCase (new LeoOrbitTracingTestCase, TestCase::Duration::EXTENSIVE);
  }
};

static LeoOrbitTestSuite leoOrbitTestSuite;
