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
#include "parametric-speed-param.h"

namespace ns3 {

ParametricSpeedParam::ParametricSpeedParam (std::vector<double> speedCoefficients) :
  m_speedCoefficients {speedCoefficients}
{
}

ParametricSpeedParam::~ParametricSpeedParam ()
{
}

const std::vector<double>
ParametricSpeedParam::GetSpeedCoefficients () const
{
  return m_speedCoefficients;
}

void
ParametricSpeedParam::Add (double c)
{
  m_speedCoefficients.push_back (c);
}

} // namespace ns3
