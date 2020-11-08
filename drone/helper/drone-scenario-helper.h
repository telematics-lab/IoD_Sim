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

#ifndef DRONE_SCENARIO_HELPER_H
#define DRONE_SCENARIO_HELPER_H

#include <string>
#include <vector>

#include <scenario-configuration-helper.h>
#include <ns3/singleton.h>
#include <ns3/position-allocator.h>
#include <ns3/node-container.h>
#include <ns3/net-device-container.h>
#include <ns3/application-container.h>

#define DSH DroneScenarioHelper

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("DroneScenarioHelper");

class DroneScenarioHelper : public Singleton<DroneScenarioHelper>
{
public:
  static DroneScenarioHelper Create(int argc, char **argv, const std::string name);
  ScenarioConfigurationHelper& GetConfigurator();

  ~DroneScenarioHelper();
  DSH& SetDronesNumber(int num);
  DSH& SetDronesPosition(Ptr<PositionAllocator> pos);
  DSH& SetAntennasNumber(int num);
  DSH& SetAntennasPosition(Ptr<PositionAllocator> pos);
  DSH& SetRemotesNumber(int num);
  DSH& SetDronesApplication(ApplicationContainer apps);
  DSH& SetRemotesApplication(ApplicationContainer apps);

private:
  void Initialize(int argc, char **argv, const std::string name);
  void ComponentDefine(std::string def1);
  void ComponentRequire(std::string rq1, std::string rq2 = NULL, std::string rq3 = NULL);

  std::vector<std::string> m_components;

  NodeContainer m_droneNodes, m_antennaNodes, m_remoteNodes;
  NetDeviceContainer m_droneDevs, m_antennaDevs, m_remoteDevs;
  ApplicationContainer m_droneApps, m_remoteApps;

};

} // namespace ns3

#endif // DRONE_SCENARIO_HELPER_H