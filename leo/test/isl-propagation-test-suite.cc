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
#include "ns3/test.h"

#include "ns3/leo-module.h"

using namespace ns3;

#define EARTH_RAD 6.3781e6

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
class IslPropagationAngleTestCase1 : public TestCase
{
public:
  IslPropagationAngleTestCase1 () : TestCase ("neighboring sats have line-of-sight") {}
  virtual ~IslPropagationAngleTestCase1 () {}
private:
  void DoRun (void)
  {
    Ptr<ConstantPositionMobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
    a->SetPosition (Vector3D (EARTH_RAD + 1.0e6, 10, 0));
    Ptr<ConstantPositionMobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();
    b->SetPosition (Vector3D (EARTH_RAD + 1.0e6, 0, 0));

    NS_TEST_ASSERT_MSG_EQ (IslPropagationLossModel::GetLos (a, b), true, "no line-of-sight");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class IslPropagationAngleTestCase2 : public TestCase
{
public:
  IslPropagationAngleTestCase2 () : TestCase ("satellites on opposing sites of earth have no line-of-sight") {}
  virtual ~IslPropagationAngleTestCase2 () {}
private:
  virtual void DoRun (void)
  {
    Ptr<ConstantPositionMobilityModel> a = CreateObject<ConstantPositionMobilityModel> ();
    a->SetPosition (Vector3D (EARTH_RAD + 1.0e6, 0, 0));
    Ptr<ConstantPositionMobilityModel> b = CreateObject<ConstantPositionMobilityModel> ();
    b->SetPosition (Vector3D (- (EARTH_RAD + 1.0e6), 0, 0));

    NS_TEST_ASSERT_MSG_EQ (IslPropagationLossModel::GetLos (a, b), false, "line-of-sight");
  }
};

/**
 * \ingroup leo-test
 * \ingroup tests
 *
 * \brief Unit tests
 */
class IslPropagationTestSuite : public TestSuite
{
public:
  IslPropagationTestSuite ();
};

IslPropagationTestSuite::IslPropagationTestSuite ()
  : TestSuite ("leo-isl-propagation", TestSuite::Type::UNIT)
{
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new IslPropagationAngleTestCase1, TestCase::Duration::QUICK);
  AddTestCase (new IslPropagationAngleTestCase2, TestCase::Duration::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static IslPropagationTestSuite leoTestSuite;
