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
#ifndef PLANNER_H
#define PLANNER_H

#include "flight-plan.h"

#include <ns3/vector.h>

#include <utility>
#include <vector>

namespace ns3
{

/**
 * \ingroup mobility
 * \brief Plan trajectories
 **/
template <typename FlightParam, typename FlightType>
class Planner
{
  public:
    Planner() = default;
    Planner(FlightPlan flightPlan, FlightParam flightParam, float step);
    void Update(const Time t) const;

    const Vector GetPosition() const;
    const Vector GetVelocity() const;
    const int32_t GetTimeWindow(const Time t) const;

  private:
    mutable Vector m_currentVelocity;
    mutable Vector m_currentPosition;

    float m_step;

    std::vector<FlightParam> m_flightParams;
    std::vector<FlightPlan> m_flightPlans;
    std::vector<FlightType> m_flights;
    std::vector<std::pair<Time, Time>> m_timeWindows; /// Time windows for each flight plan
};

} // namespace ns3

#endif /* PLANNER_H */
