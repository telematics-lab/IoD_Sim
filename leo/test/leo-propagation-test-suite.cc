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

#include "math.h"
#include "ns3/core-module.h"
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
class LeoPropagationRxNoLosTestCase : public TestCase
{
public:
  LeoPropagationRxNoLosTestCase () : TestCase ("no reception without line-of-sight") {}
  virtual ~LeoPropagationRxNoLosTestCase () {}
private:
  virtual void DoRun (void)
  {
    Ptr<ConstantPositionMobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
    a->SetPosition (Vector3D (6.7e6, 0, 0));
    Ptr<ConstantPositionMobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();
    b->SetPosition (Vector3D (-7.0e6, 0, 0));

    Ptr<LeoPropagationLossModel> model = CreateObject<LeoPropagationLossModel> ();
    model->SetAttribute ("ElevationAngle", DoubleValue (10.0));

    NS_TEST_ASSERT_MSG_LT (model->CalcRxPower (1.0, a, b), -500.0, "received transmission on destination without line-of-sight");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoPropagationRxLosTestCase : public TestCase
{
public:
  LeoPropagationRxLosTestCase () : TestCase ("reception of neighboring nodes") {}
  virtual ~LeoPropagationRxLosTestCase () {}
private:
  virtual void DoRun (void)
  {
    Ptr<ConstantPositionMobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
    a->SetPosition (Vector3D (7e6, 0, 0));
    Ptr<ConstantPositionMobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();
    b->SetPosition (Vector3D (7.1e6, 0, 0));

    Ptr<LeoPropagationLossModel> model = CreateObject<LeoPropagationLossModel> ();
    model->SetAttribute ("ElevationAngle", DoubleValue (10.0));
    model->SetAttribute ("AtmosphericLoss", DoubleValue (0.0));
    model->SetAttribute ("FreeSpacePathLoss", DoubleValue (0.0));
    model->SetAttribute ("LinkMargin", DoubleValue (0.0));

    NS_TEST_ASSERT_MSG_EQ (model->CalcRxPower (1.0, a, b), 1.0, "Tx not same as Rx power");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoPropagationBadAngleTestCase : public TestCase
{
public:
  LeoPropagationBadAngleTestCase () : TestCase ("no reception outside of beam") {}
  virtual ~LeoPropagationBadAngleTestCase () {}
private:
  void DoRun ()
  {
    Ptr<ConstantPositionMobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
    a->SetPosition (Vector3D (1000000.0, 0, 0));
    Ptr<ConstantPositionMobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();
    b->SetPosition (Vector3D (1000000.0, sin (M_PI / 9) * 1000000.0, 0));

    Ptr<LeoPropagationLossModel> model = CreateObject<LeoPropagationLossModel> ();
    model->SetAttribute ("ElevationAngle", DoubleValue (90));

    NS_TEST_ASSERT_MSG_LT (model->CalcRxPower (1.0, a, b), -500.0, "reception outside of beam");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoPropagationLossTestCase : public TestCase
{
public:
  LeoPropagationLossTestCase () : TestCase ("path losses are applied") {}
  virtual ~LeoPropagationLossTestCase () {}
private:
  void DoRun ()
  {
    Ptr<ConstantPositionMobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
    a->SetPosition (Vector3D (7e6, 0, 0));
    Ptr<ConstantPositionMobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();
    b->SetPosition (Vector3D (7.1e6, 0, 0));

    Ptr<LeoPropagationLossModel> model = CreateObject<LeoPropagationLossModel> ();
    model->SetAttribute ("AtmosphericLoss", DoubleValue (0.1));
    model->SetAttribute ("FreeSpacePathLoss", DoubleValue (0.1));
    model->SetAttribute ("LinkMargin", DoubleValue (0.3));

    NS_TEST_ASSERT_MSG_EQ (model->CalcRxPower (1.0, a, b), 0.5, "Rx power is incorrect");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class LeoPropagationTestSuite : public TestSuite
{
public:
  LeoPropagationTestSuite ();
};

LeoPropagationTestSuite::LeoPropagationTestSuite ()
  : TestSuite ("leo-propagation", TestSuite::Type::UNIT)
{
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new LeoPropagationRxNoLosTestCase, TestCase::Duration::QUICK);
  AddTestCase (new LeoPropagationRxLosTestCase, TestCase::Duration::QUICK);
  AddTestCase (new LeoPropagationBadAngleTestCase, TestCase::Duration::QUICK);
  AddTestCase (new LeoPropagationLossTestCase, TestCase::Duration::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static LeoPropagationTestSuite leoTestSuite;
