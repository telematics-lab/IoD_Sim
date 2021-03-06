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
#include "application-configuration.h"

namespace ns3 {

ApplicationConfiguration::ApplicationConfiguration (const std::string type,
                                                    const double startTime,
                                                    const double stopTime) :
  m_type {type},
  m_startTime {startTime},
  m_stopTime {stopTime}
{

}

ApplicationConfiguration::~ApplicationConfiguration ()
{

}

const std::string
ApplicationConfiguration::GetType () const
{
  return m_type;
}

const double
ApplicationConfiguration::GetStartTime () const
{
  return m_startTime;
}

const double
ApplicationConfiguration::GetStopTime () const
{
  return m_stopTime;
}

} // namespace ns3
