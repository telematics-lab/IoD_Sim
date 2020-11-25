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

#ifndef DRONE_SCENARIO_HELPER_H
#define DRONE_SCENARIO_HELPER_H

#include <string>

#include "scenario-configuration-helper.h"
#include <ns3/singleton.h>
#include <ns3/position-allocator.h>
#include <ns3/node-container.h>
#include <ns3/net-device-container.h>
#include <ns3/application-container.h>

#include <ns3/core-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include <ns3/internet-module.h>
#include <ns3/applications-module.h>

#define DSH DroneScenarioHelper

namespace ns3
{

class DroneScenarioHelper : public Singleton<DroneScenarioHelper>
{
public:
  DroneScenarioHelper& Create(int argc, char **argv, const std::string name);
  ScenarioConfigurationHelper* GetConfigurator();

  ~DroneScenarioHelper();
  DSH& SetDronesNumber(int num);
  DSH& SetDronesMobilityFromConfig();
  DSH& SetDronesApplication(Ptr<ApplicationContainer> apps);
  DSH& SetAntennasNumber(int num);
  DSH& SetAntennasPositionFromConfig();
  DSH& SetRemotesNumber(int num);
  DSH& SetRemotesApplication(Ptr<ApplicationContainer> apps);

  Ipv4InterfaceContainer GetDronesIpv4Interfaces();
  Ipv4InterfaceContainer GetRemotesIpv4Interfaces();
  Ipv4Address GetDroneIpv4Address(int id);
  Ipv4Address GetRemoteIpv4Address(int id);

private:
  Ipv4InterfaceContainer GetIpv4Interfaces(NetDeviceContainer& dev);
  Ipv4Address GetIpv4Address(NetDeviceContainer& dev, int id);
  //void SetPosition(NodeContainer& nodes, Ptr<PositionAllocator> pos);
  void SetApplications(NodeContainer& nodes, Ptr<ApplicationContainer> apps);
  void Initialize(int argc, char **argv, const std::string name);

  ScenarioConfigurationHelper *m_configurator;
  NodeContainer m_droneNodes, m_antennaNodes, m_remoteNodes;
  NetDeviceContainer m_droneDevs, m_antennaDevs, m_remoteDevs;
  ApplicationContainer m_droneApps, m_remoteApps;

};

} // namespace ns3

#endif // DRONE_SCENARIO_HELPER_H