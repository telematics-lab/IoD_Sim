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
#ifndef WIFI_MAC_SIMULATION_HELPER_H
#define WIFI_MAC_SIMULATION_HELPER_H

#include <ns3/object.h>
#include <ns3/wifi-mac-helper.h>

namespace ns3
{

/**
 * A data class to store information about a WiFi MAC layer configuration for a simulation.
 */
class WifiMacSimulationHelper : public Object
{
  public:
    /** Default constructor */
    WifiMacSimulationHelper();
    /** Default destructor */
    ~WifiMacSimulationHelper();

    /**
     * \return The WiFi MAC Helper used to configure this layer.
     */
    WifiMacHelper& GetMacHelper();

  private:
    WifiMacHelper m_macHelper;
};

} // namespace ns3

#endif /* WIFI_MAC_SIMULATION_HELPER_H */
