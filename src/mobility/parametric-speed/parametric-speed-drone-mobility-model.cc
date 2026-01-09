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
#include "parametric-speed-drone-mobility-model.h"

#include <ns3/boolean.h>
#include <ns3/double-vector.h>
#include <ns3/double.h>
#include <ns3/integer.h>
#include <ns3/log.h>
#include <ns3/object-vector.h>
#include <ns3/simulator.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ParametricSpeedDroneMobilityModel");
NS_OBJECT_ENSURE_REGISTERED(ParametricSpeedDroneMobilityModel);

TypeId
ParametricSpeedDroneMobilityModel::GetTypeId()
{
    NS_LOG_FUNCTION_NOARGS();

    static TypeId tid =
        TypeId("ns3::ParametricSpeedDroneMobilityModel")
            .SetParent<GeocentricMobilityModel>()
            .SetGroupName("Mobility")
            .AddConstructor<ParametricSpeedDroneMobilityModel>()
            .AddAttribute(
                "UseGeodedicSystem",
                "Enable the use of Earth geographic coordinate system, instead of the classical "
                "cartesian one.",
                BooleanValue(false),
                MakeBooleanAccessor(&ParametricSpeedDroneMobilityModel::m_useGeodedicSystem),
                MakeBooleanChecker())
            .AddAttribute(
                "SpeedCoefficients",
                "Coefficients to construct speed curve.",
                DoubleVectorValue(),
                MakeDoubleVectorAccessor(&ParametricSpeedDroneMobilityModel::SetSpeedCoefficients),
                MakeDoubleVectorChecker())
            .AddAttribute("FlightPlan",
                          "The ideal trajectory that the drone should run across.",
                          FlightPlanValue(),
                          MakeFlightPlanAccessor(&ParametricSpeedDroneMobilityModel::GetFlightPlan,
                                                 &ParametricSpeedDroneMobilityModel::SetFlightPlan),
                          MakeFlightPlanChecker())
            .AddAttribute("CurveStep",
                          "The step of the curve to generate. Lower step means more points "
                          "generated, hence higher resolution.",
                          DoubleValue(0.001),
                          MakeDoubleAccessor(&ParametricSpeedDroneMobilityModel::m_curveStep),
                          MakeDoubleChecker<float>());

    return tid;
}

ParametricSpeedDroneMobilityModel::ParametricSpeedDroneMobilityModel()
    : m_flightParams{{}},
      m_lastUpdate{-1},
      m_useGeodedicSystem{false}
{
}

FlightPlan
ParametricSpeedDroneMobilityModel::ProjectedToGeographicCoordinates(
    const FlightPlan& flightPlan,
    GeographicPositions::EarthSpheroidType earthType)
{
    NS_LOG_FUNCTION(flightPlan << earthType);

    FlightPlan fpOut;
    for (const auto& point : flightPlan)
    {
        const auto geographicPosition =
            GeographicPositions::ProjectedToGeographicCoordinates(point->GetPosition(), earthType);
        NS_LOG_LOGIC("Translated position from Projected "
                     << point->GetPosition() << " to Geographic " << geographicPosition);
        auto p = CreateObjectWithAttributes<ProtoPoint>("Position",
                                                        VectorValue(geographicPosition),
                                                        "Interest",
                                                        IntegerValue(point->GetInterest()),
                                                        "RestTime",
                                                        TimeValue(point->GetRestTime()));
        fpOut.Add(p);
    }

    return fpOut;
}

FlightPlan
ParametricSpeedDroneMobilityModel::GeographicToProjectedCoordinates(
    const FlightPlan& flightPlan,
    GeographicPositions::EarthSpheroidType earthType)
{
    NS_LOG_FUNCTION(flightPlan << earthType);

    FlightPlan fpOut;
    for (const auto& point : flightPlan)
    {
        const auto projectedPosition =
            GeographicPositions::GeographicToProjectedCoordinates(point->GetPosition(), earthType);
        NS_LOG_LOGIC("Translated position from Geographic "
                     << point->GetPosition() << " to Projected " << projectedPosition);
        auto p = CreateObjectWithAttributes<ProtoPoint>("Position",
                                                        VectorValue(projectedPosition),
                                                        "Interest",
                                                        IntegerValue(point->GetInterest()),
                                                        "RestTime",
                                                        TimeValue(point->GetRestTime()));
        fpOut.Add(p);
    }

    return fpOut;
}

void
ParametricSpeedDroneMobilityModel::DoInitialize()
{
    NS_LOG_FUNCTION(this);
    GeocentricMobilityModel::DoInitialize();
    m_flightPlan = (m_useGeodedicSystem)
                       ? GeographicToProjectedCoordinates(m_flightPlan, GetEarthSpheroidType())
                       : m_flightPlan;
    m_planner = Planner<ParametricSpeedParam, ParametricSpeedFlight>(m_flightPlan,
                                                                     m_flightParams,
                                                                     m_curveStep);
}

void
ParametricSpeedDroneMobilityModel::DoDispose()
{
    NS_LOG_FUNCTION(this);

    MobilityModel::DoDispose();
}

Vector
ParametricSpeedDroneMobilityModel::DoGetPosition(PositionType type) const
{
    NS_LOG_FUNCTION(this);
    NS_LOG_LOGIC("position before update: " << m_position);
    Update();
    NS_LOG_LOGIC("position after update: " << m_position);

    if (!m_useGeodedicSystem)
        return m_position;

    switch (type)
    {
    case PositionType::TOPOCENTRIC:
        return GeographicPositions::GeographicToTopocentricCoordinates(
            m_position,
            GetGeographicReferencePoint(),
            GetEarthSpheroidType());
    case PositionType::GEOCENTRIC:
        return GeographicPositions::GeographicToCartesianCoordinates(m_position.x,
                                                                     m_position.y,
                                                                     m_position.z,
                                                                     GetEarthSpheroidType());
    case PositionType::PROJECTED:
        return GeographicPositions::GeographicToProjectedCoordinates(m_position,
                                                                     GetEarthSpheroidType());
    case PositionType::GEOGRAPHIC:
    default:
        return m_position;
    }

    return m_position;
}

void
ParametricSpeedDroneMobilityModel::DoSetPosition(const Vector& position, PositionType type)
{
    NS_LOG_FUNCTION(this << position << type);

    if (m_useGeodedicSystem)
    {
        switch (type)
        {
        case PositionType::TOPOCENTRIC:
            m_position = GeographicPositions::TopocentricToGeographicCoordinates(
                position,
                GetGeographicReferencePoint(),
                GetEarthSpheroidType());
            break;
        case PositionType::GEOCENTRIC:
            m_position =
                GeographicPositions::CartesianToGeographicCoordinates(position,
                                                                      GetEarthSpheroidType());
            break;
        case PositionType::PROJECTED:
            m_position =
                GeographicPositions::ProjectedToGeographicCoordinates(position,
                                                                      GetEarthSpheroidType());
            break;
        case PositionType::GEOGRAPHIC:
        default:
            m_position = position;
            break;
        }

        NS_ASSERT_MSG((m_position.x >= -90) && (m_position.x <= 90),
                      "Latitude must be between -90 deg and +90 deg");
        NS_ASSERT_MSG((m_position.y >= -180) && (m_position.y <= 180),
                      "Longitude must be between -180 deg and +180 deg");
        NS_ASSERT_MSG(m_position.z >= 0, "Altitude must be higher or equal 0 meters");
    }
    else
    {
        m_position = position;
    }

    NotifyCourseChange();
}

Vector
ParametricSpeedDroneMobilityModel::DoGetVelocity() const
{
    NS_LOG_FUNCTION(this);

    Update();

    NS_LOG_LOGIC("velocity: " << m_velocity);
    return m_velocity;
}

void
ParametricSpeedDroneMobilityModel::Update() const
{
    NS_LOG_FUNCTION(this);

    const Time t = Simulator::Now();
    if (t.Compare(m_lastUpdate) <= 0)
    {
        NS_LOG_LOGIC("Update is being suppressed.");
        return;
    }

    m_lastUpdate = t;

    m_planner.Update(t);
    m_position =
        (m_useGeodedicSystem)
            ? GeographicPositions::ProjectedToGeographicCoordinates(m_planner.GetPosition(),
                                                                    GetEarthSpheroidType())
            : m_planner.GetPosition();
    m_velocity = m_planner.GetVelocity();

    NotifyCourseChange();
}

FlightPlan
ParametricSpeedDroneMobilityModel::GetFlightPlan() const
{
    NS_LOG_FUNCTION(this);
    return (m_useGeodedicSystem)
               ? ProjectedToGeographicCoordinates(m_flightPlan, GetEarthSpheroidType())
               : m_flightPlan;
}

void
ParametricSpeedDroneMobilityModel::SetFlightPlan(const FlightPlan& flightPlan)
{
    NS_LOG_FUNCTION(this << flightPlan);
    if (IsInitialized())
    {
        m_flightPlan = (m_useGeodedicSystem)
                           ? GeographicToProjectedCoordinates(flightPlan, GetEarthSpheroidType())
                           : flightPlan;
    }
    else
    {
        // Temporally store raw Flight Plan. Will convert at object inizialization.
        m_flightPlan = flightPlan;
    }
}

void
ParametricSpeedDroneMobilityModel::SetSpeedCoefficients(const DoubleVector& a)
{
    NS_LOG_FUNCTION(this << a);

    for (auto c = a.Begin(); c != a.End(); c++)
        m_flightParams.Add(*c);
}

} // namespace ns3
