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
 *
 * Authors: Giovanni Grieco <giovanni.grieco@poliba.it>
 */
#include "simulation-duration.h"

#include <chrono>
#include <iomanip>

#include <ns3/log.h>
#include <ns3/simulator.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SimulationDuration");

SimulationDuration::SimulationDuration (const Time& realDuration,
                                        const Time& virtualDuration) :
  m_realDuration {realDuration},
  m_virtualDuration {virtualDuration}
{
  NS_LOG_FUNCTION (this << m_realDuration << m_virtualDuration);
}

SimulationDuration::SimulationDuration () :
  m_realDuration {GetRealTime ()},
  m_virtualDuration {GetVirtualTime ()}
{
  NS_LOG_FUNCTION (this);
}

SimulationDuration::~SimulationDuration ()
{
  NS_LOG_FUNCTION (this);
}

void
SimulationDuration::Write (xmlTextWriterPtr h) const
{
  NS_LOG_FUNCTION (h);
  if (h == nullptr)
    {
      NS_LOG_WARN ("Passed handler is not valid: " << h << ". "
                   "Data will be discarded.");
      return;
    }

  int rc;
  // use stream facilities to ease output conversion
  std::ostringstream bRealDuration,
                     bVirtualDuration;

  const Time real = GetRealTime () - m_realDuration;
  const Time virt = Simulator::Now () - m_virtualDuration;

  bRealDuration << real.GetSeconds ();
  bVirtualDuration << virt.GetSeconds ();

  rc = xmlTextWriterStartElement (h, BAD_CAST "duration");
  NS_ASSERT (rc >= 0);

  /* Nested Elements */
  rc = xmlTextWriterWriteElement (h,
                                  BAD_CAST "real",
                                  BAD_CAST bRealDuration.str ().c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteElement (h,
                                  BAD_CAST "virtual",
                                  BAD_CAST bVirtualDuration.str ().c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterEndElement (h);
  NS_ASSERT (rc >= 0);
}

Time
SimulationDuration::GetRealTime ()
{
  NS_LOG_FUNCTION_NOARGS ();

  const auto now = std::chrono::system_clock::now ();
  const auto epoch = now.time_since_epoch ();
  const auto seconds = std::chrono::duration_cast<std::chrono::seconds> (epoch);
  const uint64_t duration = seconds.count ();

  std::ostringstream bTime;
  bTime << duration << "s";

  return Time (bTime.str ());
}

Time
SimulationDuration::GetVirtualTime ()
{
  NS_LOG_FUNCTION_NOARGS ();

  return Simulator::Now ();
}

} // namespace ns3
