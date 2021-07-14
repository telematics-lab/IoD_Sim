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
#ifndef ENTITY_CONFIGURATION_H
#define ENTITY_CONFIGURATION_H

#include <ns3/object.h>

#include "model-configuration.h"
#include "netdevice-configuration.h"

namespace ns3 {

/**
 * \brief Describe the configuration of a generic entity to be simulated.
 */
class EntityConfiguration : public Object
{
  public:
    EntityConfiguration (std::vector<Ptr<NetdeviceConfiguration>> m_netDevices,
                         ModelConfiguration m_mobility,
                         std::vector<ModelConfiguration> m_applications);

    EntityConfiguration (std::vector<Ptr<NetdeviceConfiguration>> m_netDevices,
                         ModelConfiguration m_mobility,
                         std::vector<ModelConfiguration> m_applications,
                         ModelConfiguration m_mechanics,
                         ModelConfiguration m_battery);

    EntityConfiguration (std::vector<Ptr<NetdeviceConfiguration>> m_netDevices,
                         ModelConfiguration m_mobility,
                         std::vector<ModelConfiguration> m_applications,
                         ModelConfiguration m_mechanics,
                         ModelConfiguration m_battery,
                         std::vector<ModelConfiguration> m_peripherals);

    ~EntityConfiguration ();

  const std::vector<Ptr<NetdeviceConfiguration>>& GetNetDevices () const;
  const ModelConfiguration& GetMobilityModel () const;
  const std::vector<ModelConfiguration>& GetApplications () const;
  const ModelConfiguration& GetMechanics () const;
  const ModelConfiguration& GetBattery () const;
  const std::vector<ModelConfiguration>& GetPeripherals () const;

private:
  std::vector<Ptr<NetdeviceConfiguration>> m_netDevices;
  ModelConfiguration m_mobility;
  std::vector<ModelConfiguration> m_applications;
  ModelConfiguration m_mechanics;
  ModelConfiguration m_battery;
  std::vector<ModelConfiguration> m_peripherals;
};

} // namespace ns3


#endif /* ENTITY_CONFIGURATION_H */