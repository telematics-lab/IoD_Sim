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
#include <ns3/olsr-helper.h>

namespace ns3
{

/**
 * \brief an array of strings containing the names of all the implemented mobilityModels
 *        used to check if the ones written in the JSON config file are valid and supported.
*/
const std::string _mobilityModels[] = {
  "ns3::ConstantPositionMobilityModel",             // CONSTANT_POSITION
  "ns3::ConstantAccelerationDroneMobilityModel",    // CONSTANT_ACCELERATION
  "ns3::ParametricSpeedDroneMobilityModel",         // PARAMETRIC_SPEED
};

/**
 * \brief enumerator to convert a mobilityModel name string int an index in order to use
 *        a more elegant switch case when installing a mobilityModel on a drone.
 *        Convert using MobilityToEnum(string).
 */
enum _MobilityModelName
{
  CONSTANT_POSITION,
  CONSTANT_ACCELERATION,
  PARAMETRIC_SPEED,
  ENUM_SIZE // Keep last, size of the enum
};


/**
 * \brief An helper to automate the building and configuration of an IoD scenario with
 *        multiple networks, drones, antennas and remote host nodes. All the scenario
 *        parameters are to be specified in an appropriate configuration JSON file.
 */
class DroneScenarioHelper : public Singleton<DroneScenarioHelper>
{
public:

  /**
   * \return a pointer to the singleton instance of the ScenarioConfigurationHelper
   */
  ScenarioConfigurationHelper* GetConfigurator () const;

  /**
   * \brief main procedure to call after instanciation. It calls all the individual procedures
   *        to setup each part of the scenario.
   * \param argc the number of arguments to pass (forwarded from main)
   * \param argv the pointer to the list of arguments to pass (forwarded from main)
   * \param name the name of the scenario (used mostrly in log)
   */
  void Initialize (uint32_t argc, char **argv, const std::string& name = "DroneScenario");

  /**
   * \brief after initializiation, runs the simulation and terminates it after it's over.
   */
  void Run () const;

/*
  Ipv4InterfaceContainer GetDronesIpv4Interfaces();
  Ipv4InterfaceContainer GetRemotesIpv4Interfaces();
  Ipv4Address GetIpv4Address(Ipv4InterfaceContainer& ifaces, uint32_t id);
*/

  /**
   * \brief install an application on a single drone
   * \param id the id of the drone
   * \param app a pointer to the ns3::Application to install
   */
  void SetDroneApplication (uint32_t id, Ptr<Application> app) const;

  /**
   * \brief install all the apps in an ns3::ApplicationContainer into the
   *        corresponding drone. Size of apps must be equal to the number of drones.
   * \param apps the ns3::ApplicationContainer containing the pointers to ns3::Application
   *             to install in each drone.
   */
  void SetDronesApplication (ApplicationContainer apps) const;

  /**
   * \brief install an application on a single remote
   * \param id the id of the drone
   * \param app a pointer to the ns3::Application to install
   */
  void SetRemoteApplication (uint32_t id, Ptr<Application> app) const;

  /**
   * \brief quickly install on all the drones and on the first remote a simple UDP echo
   *        clients and server for debug purposes or if no special application is needed.
   */
  void UseUdpEchoApplications () const;

  /**
   * \brief enable the traces of a single network idenfied by the index in the list
   * \param net_id the index of the network in the list
   */
  void EnableTraces (uint32_t net_id) const;
  /**
   * \brief enable the traces of a single network idenfied by its name
   * \param net_name the name of the network
   */
  void EnableTraces (const std::string& net_name) const;

  /**
   * \brief enable the traces for all the networks in the list
   */
  void EnableTracesAll () const;

  /**
   * \param num the number of the remote in the remotes list
   * \return the node ID of that remote
   */
  uint32_t GetRemoteId (uint32_t num) const;

  /**
   * \param num the number of the antenna in the antennas list
   * \return the node ID of that antenna
   */
  uint32_t GetAntennaId (uint32_t num) const;

  /**
   * \param node the node of which the Ipv4Address is needed
   * \param index the index of the NetDevice (0 is localhost)
   * \return the ns3::Ipv4Address of that node
   */
  Ipv4Address GetIpv4Address (Ptr<Node> node, uint32_t index) const;

  /**
   * \param id the id of the drone of which the Ipv4Address is needed
   * \param index the index of the NetDevice (0 is localhost)
   * \return the ns3::Ipv4Address of that drone
   */
  Ipv4Address GetDroneIpv4Address (uint32_t id, uint32_t index) const ;

  /**
   * \param id the id of the remote of which the Ipv4Address is needed
   * \param index the index of the NetDevice (0 is localhost)
   * \return the ns3::Ipv4Address of that remote
   */
  Ipv4Address GetRemoteIpv4Address (uint32_t id, uint32_t index) const;

private:

  /**
   * \param mobilityModel the string identifying the mobility model
   * \return the index in the enumerator for that mobility model
   */
  uint32_t MobilityToEnum (const std::string& mobilityModel) const;

  /**
   * \brief creates the list of nodes for drones, antennas and remotes
   */
  void CreateNodes ();

  /**
   * \brief sets the positions of the antennas and the mobility model for each drone
   */
  void SetMobilityModels () const;

  /**
   * \brief sets the mobility model for each drone
   */
  void SetDronesMobility () const;

  /**
   * \brief sets the position for each antenna
   */
  void SetAntennasPosition () const;

  /**
   * \brief generates each network in the list and attaches its gateway in the backbone LAN
   *        with all the remotes and sets up the Ipv4 protocol and routing.
   */
  void SetupNetworks ();

  /**
   * \brief loads and applies the global settings for the scenario defined in the config file
   */
  void LoadGlobalSettings () const;

  /**
   * \brief Loads and applies the setting for individual entities of the scenario defined
   *        in the config file.
   */
  void LoadIndividualSettings () const;

  /**
   * \brief general purpose application setter using containers
   */
  void SetApplications (NodeContainer nodes, ApplicationContainer apps) const;
  /**
   * \brief general purpose application setter using a node and a pointer to application
   */
  void SetApplication (NodeContainer nodes, uint32_t id, Ptr<Application> app) const;

  /**
   * \brief configures and generater the LTE Radio Environment Map if enabled
   */
  void GenerateRadioMap () const;


  ScenarioConfigurationHelper *m_configurator;
  NodeContainer m_droneNodes, m_antennaNodes, m_remoteNodes, m_backbone;
  NetDeviceContainer m_droneDevs, m_antennaDevs, m_remoteDevs;
  DroneNetworkContainer m_networks;
  InternetStackHelper m_internetHelper;
  std::vector<Ptr<Building> > m_buildings;
  uint32_t m_globalIpv4AddressBase;
};

} // namespace ns3

#endif // DRONE_SCENARIO_HELPER_H
