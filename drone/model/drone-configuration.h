/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2020 The IoD_Sim Authors.
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
#ifndef DRONE_CONFIGURATION_H
#define DRONE_CONFIGURATION_H

#include <ns3/object.h>

#include "application-configuration.h"
#include "model-configuration.h"
#include "netdevice-configuration.h"

namespace ns3 {

/**
 * \brief Describe the configuration of a drone to be simulated.
 */
class DroneConfiguration : public Object
{
  public:
    DroneConfiguration ();
    ~DroneConfiguration ();

  const std::vector<Ptr<NetDeviceConfiguration>>& GetNetDevices ();
  const ModelConfiguration& GetMobilityModel ();
  const std::vector<Ptr<ApplicationConfiguration>> GetApplications ();

private:
  std::vector<Ptr<NetDeviceConfiguration>> m_netDevices;
  ModelConfiguration m_mobility;
  std::vector<Ptr<ApplicationConfiguration>> m_applications;
};

} // namespace ns3


#endif /* DRONE_CONFIGURATION_H */
