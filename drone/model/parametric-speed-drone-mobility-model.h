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
#ifndef PARAMETRIC_SPEED_DRONE_MOBILITY_MODEL_H
#define PARAMETRIC_SPEED_DRONE_MOBILITY_MODEL_H

#include <ns3/mobility-model.h>
#include <ns3/vector.h>

#include "parametric-speed-flight.h"
#include "parametric-speed-param.h"
#include "flight-plan.h"
#include "planner.h"
#include "proto-point.h"
#include "double-vector.h"

namespace ns3 {

class ParametricSpeedDroneMobilityModel : public MobilityModel
{
public:
  /**
   * Register the type using ns-3 TypeId System.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  ParametricSpeedDroneMobilityModel ();
  ~ParametricSpeedDroneMobilityModel ();

private:
  virtual void DoInitialize ();
  virtual void DoDispose ();

  virtual void Update () const;
  virtual void DoSetPosition (const Vector &position);
  virtual Vector DoGetPosition () const;
  virtual Vector DoGetVelocity () const;

  void SetSpeedCoefficients (const DoubleVector &a);
protected:
  mutable Vector m_position;
  mutable Vector m_velocity;

  double m_acceleration;
  double m_maxSpeed;
  double m_simulationDuration;

  FlightPlan m_flightPlan;
  ParametricSpeedParam m_flightParams;
  Planner<ParametricSpeedParam, ParametricSpeedFlight> m_planner;

  mutable Time m_lastUpdate;

  float m_curveStep;
};

} // namespace ns3

#endif /* PARAMETRIC_SPEED_DRONE_MOBILITY_MODEL_H */
