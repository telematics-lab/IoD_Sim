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


#include "drone-network.h"
#include <ns3/config-store-module.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/buildings-helper.h>
#include <ns3/trace-helper.h>
#include <ns3/scenario-configuration-helper.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE_MASK ("DroneNetwork", LOG_PREFIX_ALL);

std::string
DroneNetwork::GetName()
{
  return m_name;
}

std::string
DroneNetwork::GetType()
{
  return m_type;
}

NodeContainer&
DroneNetwork::GetDroneNodes()
{
  return m_droneNodes;
}

NodeContainer&
DroneNetwork::GetAntennaNodes()
{
  return m_antennaNodes;
}

NetDeviceContainer&
DroneNetwork::GetDroneNetDevices()
{
  return m_droneDevs;
}

NetDeviceContainer&
DroneNetwork::GetAntennaNetDevices()
{
  return m_antennaDevs;
}

DroneNetwork
DroneNetwork::SetAttributes(const std::vector<std::pair<std::string, std::string>>& attributes)
{
  m_attributes = attributes;
  return *this;
}

DroneNetwork
DroneNetwork::SetName(std::string name)
{
  m_name = name;
  return *this;
}

DroneNetwork
DroneNetwork::AttachDrone(Ptr<Node> node)
{
  m_droneNodes.Add(node);
  return *this;
}

DroneNetwork
DroneNetwork::AttachDrones(NodeContainer& drones)
{
  m_droneNodes.Add(drones);
  return *this;
}

DroneNetwork
DroneNetwork::AttachAntenna(Ptr<Node> node)
{
  m_antennaNodes.Add(node);
  return *this;
}

DroneNetwork
DroneNetwork::AttachAntennas(NodeContainer& antennas)
{
  m_antennaNodes.Add(antennas);
  return *this;
}

DroneNetwork
DroneNetwork::ConnectToBackbone(Ptr<Node> backbone)
{
  m_backbone = backbone;
  return *this;
}


/* LTE DRONE NETWORK */

LteDroneNetwork
LteDroneNetwork::Generate()
{
  NS_LOG_FUNCTION_NOARGS();

  auto m_configurator = ScenarioConfigurationHelper::Get();

  for (auto s : m_attributes)
  {
    Config::SetDefault(s.first, StringValue(s.second));
  }

  ConfigStore config;
  config.ConfigureDefaults();

  m_internetHelper.Install(m_droneNodes);
  //m_internetHelper.Install(m_backbone);

  m_lteHelper = CreateObject<LteHelper> ();
  m_epcHelper = CreateObject<PointToPointEpcHelper> ();
  m_lteHelper->SetEpcHelper (m_epcHelper);

  //m_lteHelper->SetAttribute("PathlossModel", StringValue("ns3::Cost231PropagationLossModel"));
  //m_lteHelper->SetSchedulerType("ns3::PfFfMacScheduler"); // Proportional Fair (FemtoForumAPI) Scheduler
  //m_lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler"); // Round Robin (FemtoForumAPI) Scheduler
  //m_lteHelper->SetSchedulerAttribute("HarqEnabled", BooleanValue(true));
  //m_lteHelper->SetSchedulerAttribute("CqiTimerThreshold", UintegerValue(1000));

  //p2pHelper.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  //p2pHelper.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  //p2pHelper.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));
  m_backboneDevs = m_p2pHelper.Install(m_epcHelper->GetPgwNode(), m_backbone);

  Ipv4AddressHelper ipv4Helper;

  ipv4Helper.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer m_remoteIpv4 = ipv4Helper.Assign(m_backboneDevs);


  Ipv4StaticRoutingHelper routingHelper;
  m_internetHelper.SetRoutingHelper(routingHelper);

  // add to each remote a route to the PGW
  Ipv4Address pgwAddress = m_epcHelper->GetPgwNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
  Ptr<Ipv4StaticRouting> remoteStaticRoute = routingHelper.GetStaticRouting(m_backbone->GetObject<Ipv4>());
  remoteStaticRoute->AddNetworkRouteTo(pgwAddress, Ipv4Mask("255.0.0.0"), 1);

  m_antennaDevs = m_lteHelper->InstallEnbDevice(m_antennaNodes);
  if (m_antennaNodes.GetN() > 1)
    m_lteHelper->AddX2Interface(m_antennaNodes);
  m_droneDevs = m_lteHelper->InstallUeDevice(m_droneNodes);

  // assigning address 7.0.0.0/8
  Ipv4InterfaceContainer m_droneIpv4;
  m_droneIpv4 = m_epcHelper->AssignUeIpv4Address(m_droneDevs);

  // assign to each drone the default route to the SGW
  for (uint32_t i=0; i<m_droneNodes.GetN(); ++i)
  {
    Ptr<Ipv4StaticRouting> dronesStaticRoute = routingHelper.GetStaticRouting(m_droneNodes.Get(i)->GetObject<Ipv4>());
    dronesStaticRoute->SetDefaultRoute(m_epcHelper->GetUeDefaultGatewayAddress(), 1);
  }

/*
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VIDEO;
  GbrQosInformation qos;
  //qos.gbrDl = 132;  // bit/s, considering IP, UDP, RLC, PDCP header size
  //qos.gbrUl = 132;
  //qos.mbrDl = qos.gbrDl;
  //qos.mbrUl = qos.gbrUl;
  qos.gbrDl = 20000000; 	   // Downlink GBR (bit/s) ---> 20 Mbps
  qos.gbrUl = 5000000;	 	  // Uplink GBR ---> 5 Mbps
  qos.mbrDl = 20000000;		 // Downlink MBR
  qos.mbrUl = 5000000; 		// Uplink MBR,
  EpsBearer bearer(q, qos);
  m_lteHelper->ActivateDedicatedEpsBearer (m_droneDevs, bearer, EpcTft::Default());
*/

  // attach each drone ue to the antenna with the strongest signal
  m_lteHelper->Attach(m_droneDevs);

  m_buildings = m_configurator->GetBuildings();

  BuildingsHelper::Install (m_antennaNodes);
  BuildingsHelper::Install (m_droneNodes);
  return *this;
}

LteDroneNetwork
LteDroneNetwork::EnableTraces()
{
  AsciiTraceHelper ascii;

  std::string p2pPath, ipPath;
  std::string path = ScenarioConfigurationHelper::Get()->GetResultsPath();

  p2pPath = path + "REMOTE";
  ipPath = path + "IP";

  m_p2pHelper.EnableAsciiAll(ascii.CreateFileStream(p2pPath));
  m_p2pHelper.EnablePcapAll(p2pPath);

  m_internetHelper.EnableAsciiIpv4All(ascii.CreateFileStream(ipPath));
  m_internetHelper.EnablePcapIpv4All(ipPath);

  m_lteHelper->EnableTraces();

  return *this;
}


// DRONE NETWORK CONTAINER

void
DroneNetworkContainer::Add(Ptr<DroneNetwork> network)
{
  m_droneNetworks.push_back(network);
}

Ptr<DroneNetwork>
DroneNetworkContainer::Get(uint32_t index) const
{
  return m_droneNetworks[index];
}

Ptr<DroneNetwork>
DroneNetworkContainer::Get(std::string name) const
{
  for (uint32_t i = 0; i < m_droneNetworks.size(); i++)
  {
    auto droneNet = m_droneNetworks[i];
    if (droneNet->GetName() == name)
      return droneNet;
  }
  NS_LOG_ERROR("No DroneNetwork found with name \"" << name << "\".");
  return nullptr;
}

uint32_t
DroneNetworkContainer::GetN() const
{
  return m_droneNetworks.size();
}




} // namespace ns3