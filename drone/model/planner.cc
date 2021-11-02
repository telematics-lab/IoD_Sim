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
#include "planner.h"

#include <ns3/log.h>
#include "constant-acceleration-flight.h"
#include "constant-acceleration-param.h"
#include "parametric-speed-flight.h"
#include "parametric-speed-param.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE_MASK ("Planner", LOG_PREFIX_ALL);

template<typename FlightParam, typename FlightType>
Planner<FlightParam, FlightType>::Planner ()
{
}

template<typename FlightParam, typename FlightType>
Planner<FlightParam, FlightType>::Planner (FlightPlan flightPlan,
                                           FlightParam flightParam,
                                           float step) :
  m_step {step},
  m_flightParams {flightParam}
{
  for (auto point = flightPlan.Begin (); point != flightPlan.End (); point++)
    {
      if ((*point)->GetInterest () == 0)
        {
          if (!m_flightPlans.empty ())
            {
              m_flightPlans.back ().Add (*point);
            }
          m_flightPlans.push_back (FlightPlan (*point)); // Flight Plan to hover on the point
          m_flightPlans.push_back (FlightPlan ());
        }
      m_flightPlans.back ().Add (*point);
    }

  for (auto flightPlan : m_flightPlans)
    {
      // push new trajectory
      m_flights.push_back (FlightType (flightPlan,
                                       flightParam,
                                       step));
      // estimate timeWindow
      const Time time = m_flights.back ().GetTime ();
      const double startTime = (m_timeWindows.size () == 0)
                               ? 0
                               : m_timeWindows.back ().second.GetSeconds ();

      m_timeWindows.push_back ({Seconds (startTime),
                                Seconds (startTime + time.GetSeconds ())});

      NS_LOG_LOGIC ("Added flight " << flightPlan);
      NS_LOG_LOGIC ("TimeWindow: start at " << startTime << " for " << time);
    }

  NS_LOG_LOGIC ("Summary TimeWindow: " << m_timeWindows.size ());
  for (auto tw : m_timeWindows)
    NS_LOG_LOGIC ("    #: " << tw.first << "; " << tw.second);
}

template<typename FlightParam, typename FlightType>
Planner<FlightParam, FlightType>::~Planner ()
{
}

/**
 * Find and update the Trip corresponding to the correct timeWindow,
 * otherwise do not update
 **/
template<typename FlightParam, typename FlightType>
void
Planner<FlightParam, FlightType>::Update (const Time time) const
{
  NS_LOG_FUNCTION (time);

  const auto flightIndex = GetTimeWindow (time);
  if (flightIndex == -1)
    {
      NS_LOG_WARN ("flightIndex got -1 from GetTimeWindow (" << time << ")");
      return;
    }

  const double timeOffset = time.GetSeconds ()
                            - m_timeWindows[flightIndex].first.GetSeconds ();

  NS_LOG_LOGIC ("flightIndex: " << flightIndex << "; timeOffset: " << timeOffset);

  m_flights[flightIndex].Update (timeOffset);

  m_currentPosition = m_flights[flightIndex].GetPosition ();
  m_currentVelocity = m_flights[flightIndex].GetVelocity ();

  NS_LOG_LOGIC ("Drone updated to pos " << m_currentPosition
                << "; vel " << m_currentVelocity);
}

template<typename FlightParam, typename FlightType>
const Vector
Planner<FlightParam, FlightType>::GetPosition () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_currentPosition;
}

template<typename FlightParam, typename FlightType>
const Vector
Planner<FlightParam, FlightType>::GetVelocity () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_currentVelocity;
}

template<typename FlightParam, typename FlightType>
const int32_t
Planner<FlightParam, FlightType>::GetTimeWindow (const Time time) const
{
  NS_LOG_FUNCTION (time);

  for (uint32_t i = 0; i < m_timeWindows.size (); i++)
    {
      NS_LOG_LOGIC ("Checking if " << time << " < " << m_timeWindows[i].second);
      if (time.Compare(m_timeWindows[i].second) == -1)
        {
          return i;
        }
    }

  return -1;
}

template class Planner<ConstantAccelerationParam, ConstantAccelerationFlight>;
template class Planner<ParametricSpeedParam, ParametricSpeedFlight>;

} // namespace ns3
