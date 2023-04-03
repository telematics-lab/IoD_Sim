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
#include "entity-configuration.h"

namespace ns3
{

EntityConfiguration::EntityConfiguration(std::vector<Ptr<NetdeviceConfiguration>> netDevices,
                                         MobilityModelConfiguration mobility,
                                         std::vector<ModelConfiguration> applications)
    : m_netDevices{netDevices},
      m_mobility{mobility},
      m_applications{applications}
{
}

EntityConfiguration::EntityConfiguration(std::vector<Ptr<NetdeviceConfiguration>> netDevices,
                                         MobilityModelConfiguration mobility,
                                         std::vector<ModelConfiguration> applications,
                                         ModelConfiguration mechanics,
                                         ModelConfiguration battery)
    : m_netDevices{netDevices},
      m_mobility{mobility},
      m_applications{applications},
      m_mechanics{mechanics},
      m_battery{battery}
{
}

EntityConfiguration::EntityConfiguration(std::vector<Ptr<NetdeviceConfiguration>> netDevices,
                                         MobilityModelConfiguration mobility,
                                         std::vector<ModelConfiguration> applications,
                                         ModelConfiguration mechanics,
                                         ModelConfiguration battery,
                                         std::vector<ModelConfiguration> peripherals)
    : m_netDevices{netDevices},
      m_mobility{mobility},
      m_applications{applications},
      m_mechanics{mechanics},
      m_battery{battery},
      m_peripherals{peripherals}
{
}

EntityConfiguration::~EntityConfiguration()
{
}

const std::vector<Ptr<NetdeviceConfiguration>>&
EntityConfiguration::GetNetDevices() const
{
    return m_netDevices;
}

const MobilityModelConfiguration&
EntityConfiguration::GetMobilityModel() const
{
    return m_mobility;
}

const std::vector<ModelConfiguration>&
EntityConfiguration::GetApplications() const
{
    return m_applications;
}

const ModelConfiguration&
EntityConfiguration::GetMechanics() const
{
    return m_mechanics;
}

const ModelConfiguration&
EntityConfiguration::GetBattery() const
{
    return m_battery;
}

const std::vector<ModelConfiguration>&
EntityConfiguration::GetPeripherals() const
{
    return m_peripherals;
}

} // namespace ns3
