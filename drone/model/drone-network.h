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

#ifndef DRONE_NETWORK_H
#define DRONE_NETWORK_H

#include <ns3/node-container.h>
#include <ns3/net-device-container.h>
#include <string>
#include <vector>

#include <ns3/internet-stack-helper.h>
#include <ns3/lte-helper.h>
#include <ns3/point-to-point-epc-helper.h>
#include <ns3/point-to-point-helper.h>


namespace ns3
{

/**
 * \brief An interface that defines all the elements of a DroneNetwork in order to be
 *        used with the DroneScenarioHelper and with DroneNetworkContainer.
 *        Inherits from ns3::Object so that it can be wrapped in a ns3::Ptr
 */
class DroneNetwork : public Object
{
public:
  DroneNetwork ()
  {}
  ~DroneNetwork ()
  {}
  void DoInitialize ()
  {}
  void DoDispose ()
  {}

  /**
   * \return the name of the network
   */
  std::string GetName ();

  /**
   * \return the name of the general protocol used
   */
  std::string GetProtocol ();

  /** \brief sets the attributes for the network
   *  \param attributes a vector of key/value string pairs for each attribute */
  void SetAttributes (const std::vector<std::pair<std::string, std::string> >& attributes);

  /**
   * \return a reference to the node container for the drones in this network
   */
  NodeContainer& GetDroneNodes ();

  /**
   * \return a reference to the node container for the antennas in this network
   */
  NodeContainer& GetAntennaNodes ();

  /**
   * \return a reference to the net devices container for the drones in this network
   */
  NetDeviceContainer& GetDroneNetDevices ();

  /**
   * \return a reference to the net devices container for the drones in this network
   */
  NetDeviceContainer& GetAntennaNetDevices ();

  /**
   * \brief add a drone to the drone list of this network
   * \param node a pointer to the drone node
   */
  void AttachDrone (Ptr<Node> node);

  /**
   * \brief add all the drones in a node container to the drone list of this network
   * \param nodes the node container with the drones to add to the network
   */
  void AttachDrones (NodeContainer& nodes);

  /**
   * \brief add an antenna to the antenna list of this network
   * \param node a pointer to the antenna node
   */
  void AttachAntenna (Ptr<Node> node);
  /**
   * \brief add all the antennas in a node container to the antenna list of this network
   * \param nodes the node container with the antennas to add to the network
   */
  void AttachAntennas (NodeContainer& antennas);

  /**
   * \brief sets the global Ipv4 address helper to keep track of used Ipv4 int the scenario
   * \param ipv4H the Ipv4AddressHelper of the scenario
   */
  void SetIpv4AddressHelper (Ipv4AddressHelper& ipv4H);

  // PURE VIRTUAL METHODS, each network specialization has to implement them.

  virtual Ptr<Node> GetGateway () = 0;
  virtual void Generate () = 0;
  virtual void EnableTraces () = 0;

protected:
  std::vector<std::pair<std::string, std::string> > m_attributes;
  NodeContainer m_droneNodes, m_antennaNodes;
  NetDeviceContainer m_droneDevs, m_antennaDevs;
  Ipv4AddressHelper* m_ipv4H;
  std::string m_name;
  std::string m_protocol;
};


/**
 * \brief defines and generates a Lte network with drones as UEs and antennas as eNB
 *        connected to an EPC. Also presents the PGW as the gateway to be connected
 *        to the scenario backbone.
 */
class LteDroneNetwork : public DroneNetwork
{
public:

  /**
   * \brief instanciate the object with a name and "lte" as protocol
   * \param name the name of the network
   */
  LteDroneNetwork (std::string name)
  {
    m_name = name;
    m_protocol = "lte";
  }

  /**
   * \brief generate the topology, the links, the routing and all the setting for the LTE-EPC network
   */
  void Generate ();

  /**
   * \brief enables all the LTE traces for all the nodes in the network
   */
  void EnableTraces ();

  /**
   * \return the PGW to be connected to the backbone
   */
  Ptr<Node> GetGateway ();

private:
  InternetStackHelper m_internetHelper;
  Ptr<LteHelper> m_lteHelper;
  Ptr<PointToPointEpcHelper> m_epcHelper;
  PointToPointHelper m_p2pHelper;
};

/* TO BE IMPLEMENTED

class WifiDroneNetwork : public DroneNetwork
{
public:
  WifiDroneNetwork() { m_protocol = "wifi"; };
  ~WifiDroneNetwork();
  WifiDroneNetwork SetName(std::string name);
  WifiDroneNetwork AttachNodes(NodeContainer nodes);
  WifiDroneNetwork SetAttribute(std::string attr_name, std::string attr_value);
  WifiDroneNetwork Generate();
}

*/


/**
 * \brief A container class to collect all the DroneNetwork of the simulation
 */
class DroneNetworkContainer
{
public:
  // define an Iterator type for brevity
  typedef std::vector<Ptr<DroneNetwork> >::const_iterator Iterator;
  DroneNetworkContainer ()
  {}
  ~DroneNetworkContainer ()
  {}

  /**
   * \return an iterator to the beginning of the vector
   */
  Iterator Begin (void) const;

  /**
   * \return an iterator to the end of the vector
   */
  Iterator End (void) const;

  /**
   * \brief adds a DroneNetwork to the container
   * \param element a ns3::Ptr to the DroneNetwork to add
   */
  void Add (Ptr<DroneNetwork> element);

  /**
   * \return the size of the container
   */
  uint32_t GetN (void) const;

  /**
   * \param index the index of the DroneNetwork to get
   * \return the pointer to the DroneNetwork
   */
  Ptr<DroneNetwork> Get (uint32_t index) const;

  /**
   * \param name the name of the DroneNetwork to get
   * \return the pointer to the DroneNetwork
   */
  Ptr<DroneNetwork> Get (std::string name) const;

private:
  std::vector<Ptr<DroneNetwork> > m_droneNetworks;
};

} // namespace ns3

#endif // DRONE_NETWORK_H