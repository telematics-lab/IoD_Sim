/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2022 The IoD_Sim Authors.
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
#ifndef PARAMETRIC_SPEED_FLIGHT_H
#define PARAMETRIC_SPEED_FLIGHT_H

#include "curve.h"
#include "parametric-speed-param.h"

namespace ns3 {

/**
 * Handle a partial curvilinear trajectory between two rest points (i.e., Interest Points with I. Level set to 0).
 */
class ParametricSpeedFlight : public Curve
{
public:
  /**
   * Default constructor
   *
   * \param flightPlan      The Flight Plan to construct the flight path.
   * \param speedParameters The coefficients to model the speed of the drone.
   */
  ParametricSpeedFlight (FlightPlan flightPlan,
                         ParametricSpeedParam speedParams,
                         double step);
  /**
   * default destructor
   */
  virtual ~ParametricSpeedFlight ();

  void Generate ();
  void Update (const double &time) const;

  Time   GetTime ()     const;
  Vector GetPosition () const;
  Vector GetVelocity () const;

protected:
  void UpdateDistance (const double &time) const;
  void UpdateSpeed (const double &time) const;
  void UpdatePosition () const;
  void UpdateVelocity () const;
  const double FindTime () const;

  mutable CurvePoint m_currentPosition;
  mutable Curve::Iterator m_currentPositionPtr;
  mutable CurvePoint m_pastPosition;
  mutable double     m_currentDistance;

  mutable Vector m_currentVelocity;
  mutable double m_currentSpeed;

  double m_length;         /// Total length of the flight path
  Time   m_time;           /// Total time the drone would take to complete it

  std::vector<double> m_speedParams;
  bool m_isHovering;
};

} // namespace ns3

#endif /* PARAMETRIC_SPEED_FLIGHT_H */
