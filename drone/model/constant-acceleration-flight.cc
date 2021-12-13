/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2021 The IoD_Sim Authors.
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
#include "constant-acceleration-flight.h"

#include <cmath>
#include <vector>

#include <ns3/log.h>
#include <ns3/vector.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ConstantAccelerationFlight");

ConstantAccelerationFlight::ConstantAccelerationFlight (FlightPlan flightPlan,
                                                        ConstantAccelerationParam flightParam,
                                                        double step) :
  Curve (flightPlan, step),
  m_acceleration {flightParam.GetAcceleration ()},
  m_maxSpeed {flightParam.GetMaxSpeed ()},
  m_isHovering {flightPlan.GetN () == 1},
  m_currentDistance {0.0},
  m_currentT {0.0},
  m_currentSpeed {0.0}
{
  NS_LOG_FUNCTION (this << m_acceleration << m_maxSpeed);

  Generate ();
}

ConstantAccelerationFlight::~ConstantAccelerationFlight ()
{
  NS_LOG_FUNCTION (this);
}

void
ConstantAccelerationFlight::Generate ()
{
  NS_LOG_FUNCTION (this);

  if (m_isHovering)
    {
      // this is a dummy flight plan to hover on a specific point.
      m_length = 0;
      m_currentVelocity = Vector3D ();
      m_time = m_knots.GetBack ()->GetRestTime ();
    }
  else
    {
      m_length = Curve::Generate ();
      m_accelerationZoneLength = 0.5 * std::pow (m_maxSpeed, 2) / m_acceleration;
      m_accelerationZoneTime = m_maxSpeed / m_acceleration;

      const double aZonesL = 2 * m_accelerationZoneLength; // total length of acceleration zones
      const double aZonesT = 2 * m_accelerationZoneTime;   // total time of acceleration zones

      m_time = Seconds (aZonesT + (m_length - aZonesL) / m_maxSpeed);

      if (m_time.GetSeconds () <= aZonesT) // in case the trajectory is too short to never reach m_maxSpeed
        m_time = Seconds (std::sqrt (4.0 * m_length / m_acceleration));
    }

  m_currentPosition = CurvePoint (m_knots.GetFront ()->GetPosition ());
}

void
ConstantAccelerationFlight::Update (const double &t) const
{
  NS_LOG_FUNCTION (this << t);

  if (m_isHovering)
    return;

  double a = (m_currentSpeed < m_maxSpeed) ? m_acceleration : 0.0;

  if (m_currentDistance >= m_length - 0.5 * std::pow (m_currentSpeed, 2) / m_acceleration)
    a = -m_acceleration;

  m_currentDistance += m_currentSpeed * (t - m_currentT) + 0.5 * a * std::pow (t - m_currentT, 2);
  m_currentSpeed += a * (t - m_currentT);
  m_currentT = t;

  UpdatePosition ();
  UpdateVelocity ();

  NS_LOG_LOGIC ("Current speed: " << m_currentSpeed);
  NS_LOG_LOGIC ("Current distance: " << m_currentDistance);
}


Time
ConstantAccelerationFlight::GetTime () const
{
  return m_time;
}

Vector
ConstantAccelerationFlight::GetPosition () const
{
  return m_currentPosition.GetPosition ();
}

Vector
ConstantAccelerationFlight::GetVelocity () const
{
  return m_currentVelocity;
}

void
ConstantAccelerationFlight::UpdatePosition () const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("old pos vector: " << m_currentPosition.GetPosition ());
  m_pastPosition = m_currentPosition;

  for (auto i = m_curve.begin (); i != m_curve.end (); i++)
    {
      if ((*i).GetAbsoluteDistance () > m_currentDistance)
        {
          m_currentPosition = (*i);
          break;
        }
    }

  NS_LOG_LOGIC ("new pos vector: " << m_currentPosition.GetPosition ());
}

void
ConstantAccelerationFlight::UpdateVelocity () const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("old vel: " << m_currentVelocity);

  const Vector relativeDistance = m_currentPosition.GetRelativeDistanceVector (m_pastPosition);
  const double relativeDistScalar = m_currentPosition.GetRelativeDistance (m_pastPosition);

  if (relativeDistScalar == 0.0)
    return;

  const double velX = (relativeDistance.x != 0)
                      ? (relativeDistance.x / relativeDistScalar) * m_currentSpeed
                      : 0;
  const double velY = (relativeDistance.y != 0)
                      ? (relativeDistance.y / relativeDistScalar) * m_currentSpeed
                      : 0;
  const double velZ = (relativeDistance.z != 0)
                      ? (relativeDistance.z / relativeDistScalar) * m_currentSpeed
                      : 0;

  NS_LOG_LOGIC ("relative distance: " << relativeDistance
                << "; current speed: " << m_currentSpeed);

  m_currentVelocity = {velX, velY, velZ};

  NS_LOG_LOGIC ("new vel: " << m_currentVelocity);
}

} // namespace ns3
