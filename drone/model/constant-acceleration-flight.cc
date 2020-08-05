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
  m_maxSpeed {flightParam.GetMaxSpeed ()}
{
  NS_LOG_FUNCTION (m_acceleration << m_maxSpeed); // TODO: ostreams and istreams
                                                  // implementations for complex
                                                  // objects

  Generate ();
}

ConstantAccelerationFlight::~ConstantAccelerationFlight ()
{
}

void
ConstantAccelerationFlight::Generate ()
{
  NS_LOG_FUNCTION_NOARGS ();

  m_length = Curve::Generate ();

  m_accelerationZoneLength = (0.5 * std::pow (m_maxSpeed, 2)) / m_acceleration;
  m_accelerationZoneTime = m_maxSpeed / m_acceleration;

  const double aZonesT = 2 * m_accelerationZoneTime;   // total time of acceleration zones
  const double aZonesL = 2 * m_accelerationZoneLength; // total length of acceleration zones

  m_time = Seconds (aZonesT + (m_length - aZonesL) / m_maxSpeed);
  NS_LOG_LOGIC ("Time: " << m_time << " by " << aZonesT
                << " + (" << m_length << " - " << aZonesL
                << ") / " << m_maxSpeed);
}

void
ConstantAccelerationFlight::Update (const double &t) const
{
  NS_LOG_FUNCTION (t);

  if (m_length > 2.0 * m_accelerationZoneLength)
    {
      if (t <= m_accelerationZoneTime)
        {
          m_currentSpeed = m_acceleration * t;
          m_currentDistance = 0.5 * m_acceleration * std::pow (t, 2);
        }
      else if (t < m_time - m_accelerationZoneTime)
        {
          m_currentSpeed = m_maxSpeed;
          m_currentDistance = 0.5 * (m_maxSpeed / m_acceleration)
                              + m_maxSpeed * (t - m_accelerationZoneTime);
        }
      else
        {
          m_currentSpeed = m_maxSpeed
                           - m_acceleration * (m_time.GetSeconds () - t);
          m_currentDistance = m_length
                              + m_maxSpeed * (t - m_time.GetSeconds ()
                                              + m_accelerationZoneTime)
                              - 0.5 * m_acceleration
                                * std::pow (t - m_time.GetSeconds ()
                                            + m_accelerationZoneTime, 2);
        }
    }
  else
    {
      if (t <= m_time / 2.0)
        {
          m_currentSpeed = m_acceleration * t;
          m_currentDistance = 0.5 * m_acceleration * std::pow (t, 2);
        }
      else
        {
          m_currentSpeed = m_acceleration * 0.5 * (2 * t - m_time.GetSeconds ());
          m_currentDistance = 0.125 * m_acceleration
                                * std::pow (m_time.GetSeconds (), 2)
                              + 0.5 * m_acceleration * m_time.GetSeconds ()
                                * (t - 0.5 * m_time.GetSeconds ())
                              - 0.5 * m_acceleration
                                * std::pow (t - 0.5 * m_time.GetSeconds (), 2);
        }
    }

  UpdatePosition ();
  UpdateVelocity ();

  NS_LOG_LOGIC ("current speed: " << m_currentSpeed);
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
  NS_LOG_FUNCTION_NOARGS ();
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
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC ("old vel: " << m_currentVelocity);

  const Vector relativeDistance = m_currentPosition.GetRelativeDistanceVector (m_pastPosition);
  const double relativeDistScalar = m_currentPosition.GetRelativeDistance (m_pastPosition);

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
