/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2023 The IoD_Sim Authors.
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

#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/object-vector.h>
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
            .SetParent<MobilityModel>()
            .SetGroupName("Mobility")
            .AddConstructor<ConstantAccelerationDroneMobilityModel>()
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
                MakeFlightPlanAccessor(&ConstantAccelerationDroneMobilityModel::m_flightPlan),
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
    : m_flightParams{m_acceleration, m_maxSpeed},
      m_lastUpdate{-1}
{
    NS_LOG_FUNCTION(this);
}

ConstantAccelerationDroneMobilityModel::~ConstantAccelerationDroneMobilityModel()
{
    NS_LOG_FUNCTION(this);
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
    m_position = m_planner.GetPosition();
    m_velocity = m_planner.GetVelocity();

    NotifyCourseChange();
}

Vector
ConstantAccelerationDroneMobilityModel::DoGetPosition() const
{
    NS_LOG_FUNCTION(this);
    NS_LOG_LOGIC("position: " << m_position);

    Update();
    return m_position;
}

void
ConstantAccelerationDroneMobilityModel::DoSetPosition(const Vector& position)
{
    NS_LOG_FUNCTION(this << position);

    m_position = position;
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
ConstantAccelerationDroneMobilityModel::DoInitialize()
{
    NS_LOG_FUNCTION(this);

    m_flightParams = {m_acceleration, m_maxSpeed};

    m_planner = Planner<ConstantAccelerationParam, ConstantAccelerationFlight>(m_flightPlan,
                                                                               m_flightParams,
                                                                               m_curveStep);

    MobilityModel::DoInitialize();
}

void
ConstantAccelerationDroneMobilityModel::DoDispose()
{
    NS_LOG_FUNCTION(this);

    MobilityModel::DoDispose();
}

} // namespace ns3
