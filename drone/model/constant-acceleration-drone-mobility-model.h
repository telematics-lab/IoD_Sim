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
#ifndef CONSTANT_ACCELERATION_DRONE_MOBILITY_MODEL_H
#define CONSTANT_ACCELERATION_DRONE_MOBILITY_MODEL_H

#include <ns3/mobility-model.h>
#include <ns3/vector.h>

#include "constant-acceleration-flight.h"
#include "constant-acceleration-param.h"
#include "flight-plan.h"
#include "planner.h"
#include "proto-point.h"

namespace ns3 {

class ConstantAccelerationDroneMobilityModel : public MobilityModel
{
public:
  /**
   * Register the type using ns-3 TypeId System.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  ConstantAccelerationDroneMobilityModel ();
  ~ConstantAccelerationDroneMobilityModel ();

private:
  virtual void DoInitialize ();
  virtual void DoDispose ();

  virtual void Update () const;
  virtual void DoSetPosition (const Vector &position);
  virtual Vector DoGetPosition () const;
  virtual Vector DoGetVelocity () const;

protected:
  mutable Vector m_position;
  mutable Vector m_velocity;

  double m_acceleration;
  double m_maxSpeed;

  FlightPlan m_flightPlan;
  ConstantAccelerationParam m_flightParams;
  Planner<ConstantAccelerationParam, ConstantAccelerationFlight> m_planner;

  mutable Time m_lastUpdate;

  float m_curveStep;
};

} // namespace ns3

#endif /* CONSTANT_ACCELERATION_DRONE_MOBILITY_MODEL_H */
