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
#ifndef CONSTANT_ACCELERATION_DRONE_MOBILITY_MODEL_H
#define CONSTANT_ACCELERATION_DRONE_MOBILITY_MODEL_H

#include "constant-acceleration-flight.h"
#include "constant-acceleration-param.h"

#include <ns3/flight-plan.h>
#include <ns3/geocentric-mobility-model.h>
#include <ns3/geographic-positions.h>
#include <ns3/planner.h>
#include <ns3/proto-point.h>
#include <ns3/vector.h>

namespace ns3
{

/**
 * \ingroup mobility
 * \brief A geocentric mobility model for drones that follows a constant acceleration.
 */
class ConstantAccelerationDroneMobilityModel : public GeocentricMobilityModel
{
  public:
    /**
     * Register the type using ns-3 TypeId System.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    ConstantAccelerationDroneMobilityModel();

    static FlightPlan ProjectedToGeographicCoordinates(
        const FlightPlan& flightPlan,
        GeographicPositions::EarthSpheroidType earthType);
    static FlightPlan GeographicToProjectedCoordinates(
        const FlightPlan& flightPlan,
        GeographicPositions::EarthSpheroidType earthType);

  private:
    /// Initizalize the object instance.
    virtual void DoInitialize();
    /// Destroy the object instance.
    virtual void DoDispose();

    virtual Vector DoGetPosition(PositionType type) const;
    virtual void DoSetPosition(const Vector& position, PositionType type);
    virtual Vector DoGetVelocity() const;

    virtual void Update() const;
    FlightPlan GetFlightPlan() const;
    void SetFlightPlan(const FlightPlan& flightPlan);

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
    bool m_useGeodedicSystem;
};

} // namespace ns3

#endif /* CONSTANT_ACCELERATION_DRONE_MOBILITY_MODEL_H */
