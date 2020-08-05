/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2020 The IoD_Sim Authors.
 *
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
 */
#include "flight-plan.h"

#include <ns3/integer.h>
#include <ns3/names.h>
#include <ns3/object-factory.h>

namespace ns3 {

ATTRIBUTE_HELPER_CPP (FlightPlan);

FlightPlan::FlightPlan ()
{
}

FlightPlan::FlightPlan (Ptr<ProtoPoint> point)
{
  m_protoPoints.push_back (point);
}

FlightPlan::FlightPlan (const FlightPlan &a,
                        const FlightPlan &b)
{
  Add (a);
  Add (b);
}

FlightPlan::FlightPlan (const FlightPlan &a,
                        const FlightPlan &b,
                        const FlightPlan &c)
{
  Add (a);
  Add (b);
  Add (c);
}

FlightPlan::FlightPlan (const FlightPlan &a,
                        const FlightPlan &b,
                        const FlightPlan &c,
                        const FlightPlan &d)
{
  Add (a);
  Add (b);
  Add (c);
  Add (d);
}

FlightPlan::FlightPlan (const FlightPlan &a,
                        const FlightPlan &b,
                        const FlightPlan &c,
                        const FlightPlan &d,
                        const FlightPlan &e)
{
  Add (a);
  Add (b);
  Add (c);
  Add (d);
  Add (e);
}

FlightPlan::Iterator
FlightPlan::Begin () const
{
  return m_protoPoints.begin ();
}

FlightPlan::Iterator
FlightPlan::End () const
{
  return m_protoPoints.end ();
}

uint32_t
FlightPlan::GetN () const
{
  return m_protoPoints.size ();
}

Ptr<ProtoPoint>
FlightPlan::Get (const uint32_t i) const
{
  return m_protoPoints[i];
}

Ptr<ProtoPoint>
FlightPlan::GetFront () const
{
  return m_protoPoints.front ();
}

Ptr<ProtoPoint>
FlightPlan::GetBack () const
{
  return m_protoPoints.back ();
}

void
FlightPlan::Add (const FlightPlan other)
{
  for (Iterator i = other.Begin (); i != other.End (); i++)
    m_protoPoints.push_back(*i);
}

void
FlightPlan::Add (const Ptr<ProtoPoint> point)
{
  m_protoPoints.push_back (point);
}

void
FlightPlan::Add (const std::string pointName)
{
  Ptr<ProtoPoint> point = Names::Find<ProtoPoint> (pointName);

  m_protoPoints.push_back (point);
}

std::ostream &
operator<< (std::ostream &os, const FlightPlan &prototrajectory)
{
  os << prototrajectory.GetN () << ";";

  for (auto protoPoint = prototrajectory.Begin ();
       protoPoint != prototrajectory.End ();
       protoPoint++)
    {
      os << (*protoPoint)->GetPosition () << "$"
         << (*protoPoint)->GetInterest () << "$"
         << (*protoPoint)->GetRestTime () << ";";
    }

  return os;
}

std::istream &
operator>> (std::istream &is, FlightPlan &prototrajectory)
{
  char sepInPoint1 = '\0';
  char sepInPoint2 = '\0';
  char sepInPoint3 = '\0';
  char sepInterPoint = '\0';

  uint32_t n;
  Vector position;
  uint32_t interest;
  double restTime;

  is >> n >> sepInterPoint;
  if (sepInterPoint != ';')
    is.setstate (std::ios::failbit);

  for (uint32_t i = 0; i < n; i++)
    {
      is >> position >> sepInPoint1
         >> interest >> sepInPoint2
         >> restTime >> sepInterPoint;

      if (sepInPoint1 != '$'
          || sepInPoint2 != '$'
          || sepInPoint3 != '$'
          || sepInterPoint != ';')
        {
          is.setstate (std::ios::failbit);
          break;
        }

      auto protoPoint = CreateObjectWithAttributes<ProtoPoint>
        ("Position", VectorValue (position),
         "Interest", IntegerValue (interest),
         "RestTime", TimeValue (Seconds (restTime)));

      prototrajectory.Add (protoPoint);
    }

  return is;
}

} // namespace ns3
