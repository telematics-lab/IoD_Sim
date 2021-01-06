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
#include <ns3/point-to-point-module.h>
#include <ns3/csma-module.h>
#include <ns3/internet-module.h>
#include <ns3/applications-module.h>

#define DSH DroneScenarioHelper

namespace ns3
{

class DroneScenarioHelper : public Singleton<DroneScenarioHelper>
{
public:
  DroneScenarioHelper& Create(uint32_t argc, char **argv, const std::string name);
  ScenarioConfigurationHelper* GetConfigurator();

  ~DroneScenarioHelper();
  DSH& SetDronesNumber(uint32_t num);
  DSH& SetDronesMobilityFromConfig();
  DSH& SetDroneApplication(uint32_t id, Ptr<Application>& apps);
  DSH& SetDronesApplication(Ptr<ApplicationContainer>& apps);
  DSH& SetAntennasNumber(uint32_t num);
  DSH& SetAntennasPositionFromConfig();
  DSH& SetRemotesNumber(uint32_t num);
  DSH& SetRemoteApplication(uint32_t  id, Ptr<Application>& apps);
  DSH& SetRemotesApplication(Ptr<ApplicationContainer>& apps);

  Ipv4InterfaceContainer GetDronesIpv4Interfaces();
  Ipv4InterfaceContainer GetRemotesIpv4Interfaces();
  Ipv4Address GetDroneIpv4Address(uint32_t id);
  Ipv4Address GetRemoteIpv4Address(uint32_t id);

  DSH& CreateLteEpc();
  DSH& CreateRemotesToEpcNetwork();
  DSH& CreateDronesToAntennasNetwork();
  DSH& CreateIpv4Routing();

private:
  void Initialize(uint32_t argc, char **argv, const std::string name);
  void SetNumber(NodeContainer& nodes, uint32_t  num);
  Ipv4Address GetIpv4Address(Ipv4InterfaceContainer& ifaces, uint32_t id);
  //void SetPosition(NodeContainer& nodes, Ptr<PositionAllocator> pos);
  void SetApplications(NodeContainer& nodes, Ptr<ApplicationContainer>& apps);
  void SetApplication(NodeContainer& nodes, uint32_t  id, Ptr<Application>& app);

  ScenarioConfigurationHelper *m_configurator;
  NodeContainer m_droneNodes, m_antennaNodes, m_remoteNodes;
  NetDeviceContainer m_droneDevs, m_antennaDevs, m_remoteDevs, m_p2pDevs;
  ApplicationContainer m_droneApps, m_remoteApps;
  Ipv4InterfaceContainer m_droneIpv4, m_remoteIpv4, m_p2pIpv4;
  Ptr<LteHelper> m_lteHelper;
  Ptr<PointToPointEpcHelper> m_epcHelper;

};

} // namespace ns3

#endif // DRONE_SCENARIO_HELPER_H