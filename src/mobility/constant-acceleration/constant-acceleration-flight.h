/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2024 The IoD_Sim Authors.
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
#ifndef CONSTANT_ACCELERATION_FLIGHT_H
#define CONSTANT_ACCELERATION_FLIGHT_H

#include "constant-acceleration-param.h"

#include <ns3/curve.h>
#include <ns3/flight-plan.h>
#include <ns3/vector.h>

namespace ns3
{

/**
 * \ingroup mobility
 * \brief A curve that represents a drone flight with constant acceleration.
 */
class ConstantAccelerationFlight : public Curve
{
  public:
    ConstantAccelerationFlight(FlightPlan flightPlan,
                               ConstantAccelerationParam flightParam,
                               double step);

    void Generate();
    void Update(const double& time) const;

    Time GetTime() const;
    Vector GetPosition() const;
    Vector GetVelocity() const;

  protected:
    void UpdatePosition() const;
    void UpdateVelocity() const;

    double m_length; /// Total length of the trajectory
    Time m_time;     /// Total time the drone would take to complete it

    double m_acceleration; // m/s^2
    double m_maxSpeed;     // m/s
    bool m_isHovering;

    double m_accelerationZoneLength;
    double m_accelerationZoneTime;

    mutable CurvePoint m_currentPosition;
    mutable CurvePoint m_pastPosition;
    mutable double m_currentDistance;
    mutable double m_currentT;

    mutable Vector m_currentVelocity;
    mutable double m_currentSpeed;
};

} // namespace ns3

#endif /* CONSTANT_ACCELERATION_FLIGHT_H */
