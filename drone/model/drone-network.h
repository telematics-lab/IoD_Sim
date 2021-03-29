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
#include <ns3/building.h>


namespace ns3
{

// make a derivate class of Object
class DroneNetwork : public Object
{
public:
  DroneNetwork() {}
  ~DroneNetwork() {}
  void DoDispose() {}
  void DoInitialize() {}
  std::string GetName();
  std::string GetProtocol();
  void SetAttributes(const std::vector<std::pair<std::string, std::string>>& attributes);
  NodeContainer& GetDroneNodes();
  NodeContainer& GetAntennaNodes();
  NetDeviceContainer& GetDroneNetDevices();
  NetDeviceContainer& GetAntennaNetDevices();
  void AttachDrone(Ptr<Node> node);
  void AttachDrones(NodeContainer& nodes);
  void AttachAntenna(Ptr<Node> node);
  void AttachAntennas(NodeContainer& antennas);
  void ConnectToBackbone(Ptr<Node> backbone);
  virtual void Generate() {}
  virtual void EnableTraces() {}
protected:
  std::vector<std::pair<std::string, std::string>> m_attributes;
  Ptr<Node> m_backbone;
  NodeContainer m_droneNodes, m_antennaNodes;
  NetDeviceContainer m_droneDevs, m_antennaDevs, m_backboneDevs;
  std::string m_name;
  std::string m_protocol;
};


class LteDroneNetwork : public DroneNetwork
{
public:
  LteDroneNetwork(std::string name) { m_name = name; m_protocol = "lte"; }
  //~LteDroneNetwork();


  void Generate();
  void EnableTraces();

private:
  InternetStackHelper m_internetHelper;
  Ptr<LteHelper> m_lteHelper;
  Ptr<PointToPointEpcHelper> m_epcHelper;
  PointToPointHelper m_p2pHelper;
  std::vector<Ptr<Building>> m_buildings;
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

class DroneNetworkContainer
{
public:
  DroneNetworkContainer() {}
  ~DroneNetworkContainer() {}
  typedef std::vector<Ptr<DroneNetwork>>::const_iterator Iterator;
  Iterator Begin(void) const;
  Iterator End(void) const;
  void Add(Ptr<DroneNetwork> element);
  uint32_t GetN(void) const;
  Ptr<DroneNetwork> Get(uint32_t index) const;
  Ptr<DroneNetwork> Get(std::string name) const;


private:
  std::vector<Ptr<DroneNetwork>> m_droneNetworks;
};

} // namespace ns3

#endif // DRONE_NETWORK_H