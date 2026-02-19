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
#include "constant-acceleration-drone-mobility-model.h"

#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/integer.h>
#include <ns3/log.h>
#include <ns3/object-vector.h>
#include <ns3/proto-point.h>
#include <ns3/simulator.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ConstantAccelerationDroneMobilityModel");
NS_OBJECT_ENSURE_REGISTERED(ConstantAccelerationDroneMobilityModel);

TypeId
ConstantAccelerationDroneMobilityModel::GetTypeId()
{
    NS_LOG_FUNCTION_NOARGS();

    static TypeId tid =
        TypeId("ns3::ConstantAccelerationDroneMobilityModel")
            .SetParent<GeocentricMobilityModel>()
            .SetGroupName("Mobility")
            .AddConstructor<ConstantAccelerationDroneMobilityModel>()
            .AddAttribute(
                "UseGeodedicSystem",
                "Enable the use of Earth geographic coordinate system, instead of the classical "
                "cartesian one.",
                BooleanValue(false),
                MakeBooleanAccessor(&ConstantAccelerationDroneMobilityModel::m_useGeodedicSystem),
                MakeBooleanChecker())
            .AddAttribute(
                "Acceleration",
                "The constant acceleration of the drone, in m/s^2",
                DoubleValue(2.0),
                MakeDoubleAccessor(&ConstantAccelerationDroneMobilityModel::m_acceleration),
                MakeDoubleChecker<double>())
            .AddAttribute("MaxSpeed",
                          "Max speed of the drone, in m/s.",
                          DoubleValue(10.0),
                          MakeDoubleAccessor(&ConstantAccelerationDroneMobilityModel::m_maxSpeed),
                          MakeDoubleChecker<double>())
            .AddAttribute(
                "FlightPlan",
                "The ideal trajectory that the drone should run across.",
                FlightPlanValue(),
                MakeFlightPlanAccessor(&ConstantAccelerationDroneMobilityModel::GetFlightPlan,
                                       &ConstantAccelerationDroneMobilityModel::SetFlightPlan),
                MakeFlightPlanChecker())
            .AddAttribute("CurveStep",
                          "The step of the curve to generate. Lower step means more points "
                          "generated, hence higher resolution.",
                          DoubleValue(0.001),
                          MakeDoubleAccessor(&ConstantAccelerationDroneMobilityModel::m_curveStep),
                          MakeDoubleChecker<float>());

    return tid;
}

ConstantAccelerationDroneMobilityModel::ConstantAccelerationDroneMobilityModel()
    : m_acceleration{0.0},
      m_maxSpeed{0.0},
      m_flightParams{m_acceleration, m_maxSpeed},
      m_lastUpdate{-1},
      m_useGeodedicSystem{false}
{
}

Ptr<MobilityModel>
ConstantAccelerationDroneMobilityModel::Copy() const
{
    return CreateObject<ConstantAccelerationDroneMobilityModel>(*this);
}

FlightPlan
ConstantAccelerationDroneMobilityModel::ProjectedToGeographicCoordinates(
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
ConstantAccelerationDroneMobilityModel::GeographicToProjectedCoordinates(
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
ConstantAccelerationDroneMobilityModel::DoInitialize()
{
    NS_LOG_FUNCTION(this);
    GeocentricMobilityModel::DoInitialize();
    m_flightPlan = (m_useGeodedicSystem)
                       ? GeographicToProjectedCoordinates(m_flightPlan, GetEarthSpheroidType())
                       : m_flightPlan;
    m_flightParams = {m_acceleration, m_maxSpeed};
    m_planner = Planner<ConstantAccelerationParam, ConstantAccelerationFlight>(m_flightPlan,
                                                                               m_flightParams,
                                                                               m_curveStep);
}

void
ConstantAccelerationDroneMobilityModel::DoDispose()
{
    NS_LOG_FUNCTION(this);

    MobilityModel::DoDispose();
}

Vector
ConstantAccelerationDroneMobilityModel::DoGetPosition(PositionType type) const
{
    NS_LOG_FUNCTION(this << type);

    NS_LOG_LOGIC("Position before update: " << m_position);
    Update();
    NS_LOG_LOGIC("Position after update: " << m_position);

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
}

void
ConstantAccelerationDroneMobilityModel::DoSetPosition(const Vector& position, PositionType type)
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
ConstantAccelerationDroneMobilityModel::DoGetVelocity() const
{
    NS_LOG_FUNCTION(this);

    Update();

    NS_LOG_LOGIC("velocity: " << m_velocity);
    return m_velocity;
}

void
ConstantAccelerationDroneMobilityModel::Update() const
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
ConstantAccelerationDroneMobilityModel::GetFlightPlan() const
{
    NS_LOG_FUNCTION(this);
    return (m_useGeodedicSystem)
               ? ProjectedToGeographicCoordinates(m_flightPlan, GetEarthSpheroidType())
               : m_flightPlan;
}

void
ConstantAccelerationDroneMobilityModel::SetFlightPlan(const FlightPlan& flightPlan)
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

} // namespace ns3
