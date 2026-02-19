/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2026 The IoD_Sim Authors.
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

#include "parametric-speed-flight.h"
#include "parametric-speed-param.h"

#include <ns3/double-vector.h>
#include <ns3/flight-plan.h>
#include <ns3/geocentric-mobility-model.h>
#include <ns3/planner.h>
#include <ns3/proto-point.h>
#include <ns3/vector.h>

namespace ns3
{

/**
 * \ingroup mobility
 * \brief A geocentric mobility model for drones that follows a parametric speed.
 */
class ParametricSpeedDroneMobilityModel : public GeocentricMobilityModel
{
  public:
    /**
     * Register the type using ns-3 TypeId System.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    ParametricSpeedDroneMobilityModel();
    virtual ~ParametricSpeedDroneMobilityModel() = default;

    /**
     * \brief Copy the mobility model
     * \return a copy of the mobility model
     */
    virtual Ptr<MobilityModel> Copy() const override;

    static FlightPlan ProjectedToGeographicCoordinates(
        const FlightPlan& flightPlan,
        GeographicPositions::EarthSpheroidType earthType);
    static FlightPlan GeographicToProjectedCoordinates(
        const FlightPlan& flightPlan,
        GeographicPositions::EarthSpheroidType earthType);

  private:
    /// Initialize the object instance.
    virtual void DoInitialize() override;
    /// Destroy the object instance.
    virtual void DoDispose() override;

    virtual Vector DoGetPosition(PositionType type) const override;
    virtual void DoSetPosition(const Vector& position, PositionType type) override;
    virtual Vector DoGetVelocity() const override;

    virtual void Update() const;
    FlightPlan GetFlightPlan() const;
    void SetFlightPlan(const FlightPlan& fp);

    void SetSpeedCoefficients(const DoubleVector& a);

  protected:
    mutable Vector m_position;
    mutable Vector m_velocity;

    FlightPlan m_flightPlan;
    ParametricSpeedParam m_flightParams;
    Planner<ParametricSpeedParam, ParametricSpeedFlight> m_planner;

    mutable Time m_lastUpdate;

    float m_curveStep;
    bool m_useGeodedicSystem;
};

} // namespace ns3

#endif /* PARAMETRIC_SPEED_DRONE_MOBILITY_MODEL_H */
