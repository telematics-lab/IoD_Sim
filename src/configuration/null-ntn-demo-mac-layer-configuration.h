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
#ifndef NULL_NTN_MAC_LAYER_CONFIGURATION_H
#define NULL_NTN_MAC_LAYER_CONFIGURATION_H

#include "mac-layer-configuration.h"

#include <ns3/model-configuration.h>
#include <ns3/wifi-phy.h>

#include <optional>
#include <string>
#include <vector>

namespace ns3
{

/// Data class to store information about the Null NTN Demo (i.e., no MAC, just PHY analysis).
class NullNtnDemoMacLayerConfiguration : public MacLayerConfiguration
{
  public:
    /**
     * Create a new object instance.
     *
     * \param macType The type of the MAC Layer to be configured. It should be "NullNtnDemo".
     * \param timeResolution Time resolution to evaluate NTN communications.
     */
    NullNtnDemoMacLayerConfiguration(std::string macType,
                                     double timeResolution,
                                     double bandwidth,
                                     double rbBandwidth,
                                     double satEirpDensity,
                                     double ueAntennaNoiseFigure);
    /// Default destructor.
    ~NullNtnDemoMacLayerConfiguration();

    /// \return The configured time resolution.
    double GetTimeResolution() const;
    /// \return The configured bandwidth.
    double GetBandwidth() const;
    /// \return The configured RB bandwidth.
    double GetRbBandwidth() const;
    /// \return The configured satellite EIRP density.
    double GetSatEirpDensity() const;
    /// \return The configured UE antenna noise figure.
    double GetUeAntennaNoiseFigure() const;

  private:
    const double m_timeResolution;       /// Configured time resolution, in seconds
    const double m_bandwidth;            /// Configured bandwidth, in Hz
    const double m_rbBandwidth;          /// Configured RB bandwidth, in Hz
    const double m_satEirpDensity;       /// Configured satellite EIRP density, in dBW/MHz
    const double m_ueAntennaNoiseFigure; /// Configured UE antenna noise figure, in dB
};

} // namespace ns3

#endif /* NULL_NTN_MAC_LAYER_CONFIGURATION_H */
