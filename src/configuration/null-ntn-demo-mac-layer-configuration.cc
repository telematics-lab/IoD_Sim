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
#include "null-ntn-demo-mac-layer-configuration.h"

namespace ns3
{

NullNtnDemoMacLayerConfiguration::NullNtnDemoMacLayerConfiguration(std::string macType,
                                                                   double timeResolution,
                                                                   double bandwidth,
                                                                   double rbBandwidth,
                                                                   double satEirpDensity,
                                                                   double ueAntennaNoiseFigure)
    : MacLayerConfiguration(macType),
      m_timeResolution(timeResolution),
      m_bandwidth(bandwidth),
      m_rbBandwidth(rbBandwidth),
      m_satEirpDensity(satEirpDensity),
      m_ueAntennaNoiseFigure(ueAntennaNoiseFigure)
{
}

NullNtnDemoMacLayerConfiguration::~NullNtnDemoMacLayerConfiguration()
{
}

double
NullNtnDemoMacLayerConfiguration::GetTimeResolution() const
{
    return m_timeResolution;
}

double
NullNtnDemoMacLayerConfiguration::GetBandwidth() const
{
    return m_bandwidth;
}

double
NullNtnDemoMacLayerConfiguration::GetRbBandwidth() const
{
    return m_rbBandwidth;
}

double
NullNtnDemoMacLayerConfiguration::GetSatEirpDensity() const
{
    return m_satEirpDensity;
}

double
NullNtnDemoMacLayerConfiguration::GetUeAntennaNoiseFigure() const
{
    return m_ueAntennaNoiseFigure;
}

} // namespace ns3