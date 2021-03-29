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

#include <ns3/scenario-configuration-helper.h>
#include <ns3/drone-network.h>
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
#include <ns3/config-store-module.h>
#include <ns3/buildings-module.h>

namespace ns3
{

const std::string _mobilityModels[] = {
  "ns3::ConstantPositionMobilityModel",             // CONSTANT_POSITION
  "ns3::ConstantAccelerationDroneMobilityModel",    // CONSTANT_ACCELERATION
  "ns3::ParametricSpeedDroneMobilityModel",         // PARAMETRIC_SPEED
};

enum _MobilityModelName
{
  CONSTANT_POSITION,
  CONSTANT_ACCELERATION,
  PARAMETRIC_SPEED,
  ENUM_SIZE // Keep last, size of the enum
};

class DroneScenarioHelper : public Singleton<DroneScenarioHelper>
{
public:
  ScenarioConfigurationHelper* GetConfigurator();
  void Initialize(uint32_t argc, char **argv, std::string name="DroneScenario");

  void Run();

/*
  Ipv4InterfaceContainer GetDronesIpv4Interfaces();
  Ipv4InterfaceContainer GetRemotesIpv4Interfaces();
  Ipv4Address GetIpv4Address(Ipv4InterfaceContainer& ifaces, uint32_t id);
*/

  void SetDroneApplication(uint32_t id, Ptr<Application> app);
  void SetDronesApplication(Ptr<ApplicationContainer> apps);
  void SetRemoteApplication(uint32_t id, Ptr<Application> app);
  void UseUdpEchoApplications();
  //void EnableTrace(uint32_t id, std::string prefix);
  uint32_t GetRemoteId(uint32_t num);
  uint32_t GetAntennaId(uint32_t num);
  Ipv4Address GetIpv4Address(Ptr<Node> node);
  Ipv4Address GetDroneIpv4Address(uint32_t id);
  Ipv4Address GetRemoteIpv4Address(uint32_t id);

protected:

  uint32_t MobilityToEnum(std::string mobilityModel);
  void CreateNodes();
  void SetMobilityModels();
  void SetupNetworks();
  void LoadGlobalSettings();
  void LoadIndividualSettings();

  void SetDronesMobility();
  void SetAntennasPosition();

  void SetApplications(NodeContainer& nodes, Ptr<ApplicationContainer>& apps); // why should I pass apps by reference?
  void SetApplication(NodeContainer& nodes, uint32_t id, Ptr<Application> app);


  std::string m_protocol;
  ScenarioConfigurationHelper *m_configurator;
  Ptr<Node> m_backbone;
  NodeContainer m_droneNodes, m_antennaNodes, m_remoteNodes;
  NetDeviceContainer m_droneDevs, m_antennaDevs, m_remoteDevs;
  ApplicationContainer m_droneApps, m_remoteApps;
  //Ipv4InterfaceContainer m_droneIpv4, m_remoteIpv4;
  Ptr<LteHelper> m_lteHelper;
  Ptr<PointToPointEpcHelper> m_epcHelper;
  PointToPointHelper m_p2pHelper;
  InternetStackHelper m_internetHelper;
  std::vector<Ptr<Building>> m_buildings;
  DroneNetworkContainer m_networks;
  uint32_t m_globalIpv4AddressBase;
};

} // namespace ns3

#endif // DRONE_SCENARIO_HELPER_H