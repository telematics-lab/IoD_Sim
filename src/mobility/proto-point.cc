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
#include "proto-point.h"

#include <ns3/integer.h>
#include <ns3/vector.h>

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(ProtoPoint);

TypeId
ProtoPoint::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::ProtoPoint")
            .SetParent<Object>()
            .SetGroupName("ProtoTrajectory")
            .AddConstructor<ProtoPoint>()
            .AddAttribute("Position",
                          "The current position of the point of interest.",
                          VectorValue(Vector(0.0, 0.0, 0.0)),
                          MakeVectorAccessor(&ProtoPoint::m_position),
                          MakeVectorChecker())
            .AddAttribute("Interest",
                          "The grade of interest of this point. 0 for a point of destination.",
                          IntegerValue(0),
                          MakeIntegerAccessor(&ProtoPoint::m_interest),
                          MakeIntegerChecker<uint32_t>())
            .AddAttribute("RestTime",
                          "The time to rest, if the point is a destination one.",
                          TimeValue(Seconds(0)),
                          MakeTimeAccessor(&ProtoPoint::m_restTime),
                          MakeTimeChecker());

    return tid;
}

void
ProtoPoint::SetPosition(const Vector& position)
{
    m_position = position;
}

const Vector
ProtoPoint::GetPosition() const
{
    return m_position;
}

void
ProtoPoint::SetInterest(const uint32_t& interest)
{
    m_interest = interest;
}

const uint32_t
ProtoPoint::GetInterest() const
{
    return m_interest;
}

void
ProtoPoint::SetRestTime(const Time& seconds)
{
    m_restTime = seconds;
}

const Time
ProtoPoint::GetRestTime() const
{
    return m_restTime;
}

} // namespace ns3
