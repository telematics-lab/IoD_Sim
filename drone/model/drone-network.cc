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


#include "drone-network.h"
#include <ns3/config-store-module.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/trace-helper.h>
#include <ns3/scenario-configuration-helper.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE_MASK ("DroneNetwork", LOG_PREFIX_ALL);

const std::string&
DroneNetwork::GetName () const
{
  return m_name;
}

const std::string&
DroneNetwork::GetProtocol () const
{
  return m_protocol;
}

NodeContainer
DroneNetwork::GetDroneNodes () const
{
  return m_droneNodes;
}

NodeContainer
DroneNetwork::GetAntennaNodes () const
{
  return m_antennaNodes;
}

NetDeviceContainer
DroneNetwork::GetDroneNetDevices () const
{
  return m_droneDevs;
}

NetDeviceContainer
DroneNetwork::GetAntennaNetDevices () const
{
  return m_antennaDevs;
}

void
DroneNetwork::SetAttributes (const std::vector<std::pair<std::string, std::string> >& attributes)
{
  m_attributes = attributes;
}

void
DroneNetwork::AttachDrone (Ptr<Node> node)
{
  m_droneNodes.Add (node);
}

void
DroneNetwork::AttachDrones (NodeContainer& drones)
{
  m_droneNodes.Add (drones);
}

void
DroneNetwork::AttachAntenna (Ptr<Node> node)
{
  m_antennaNodes.Add (node);
}

void
DroneNetwork::AttachAntennas (NodeContainer& antennas)
{
  m_antennaNodes.Add (antennas);
}

void
DroneNetwork::SetIpv4AddressHelper (Ipv4AddressHelper& ipv4H)
{
  m_ipv4H = &ipv4H;
}





/* LTE DRONE NETWORK */

Ptr<Node>
LteDroneNetwork::GetGateway ()
{
  return m_epcHelper->GetPgwNode ();
}

void
LteDroneNetwork::Generate ()
{
  NS_LOG_FUNCTION_NOARGS ();

  //auto m_configurator = ScenarioConfigurationHelper::Get ();

  // setting up default values before generating the network
  for (auto s : m_attributes)
    {
      Config::SetDefault (s.first, StringValue (s.second));
    }

  ConfigStore config;
  config.ConfigureDefaults ();

  //m_internetHelper.Install (m_droneNodes);

  // create the LteHelper and assign the EpcHelper to it
  m_lteHelper = CreateObject<LteHelper> ();
  m_epcHelper = CreateObject<PointToPointEpcHelper> ();
  m_lteHelper->SetEpcHelper (m_epcHelper);

  m_lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::HybridBuildingsPropagationLossModel"));
  m_lteHelper->SetPathlossModelAttribute ("ShadowSigmaExtWalls", DoubleValue (0));
  m_lteHelper->SetPathlossModelAttribute ("ShadowSigmaOutdoor", DoubleValue (1));
  m_lteHelper->SetPathlossModelAttribute ("ShadowSigmaIndoor", DoubleValue (1.5));
  // always use NLos
  m_lteHelper->SetPathlossModelAttribute ("Los2NlosThr", DoubleValue (0));
  m_lteHelper->SetSpectrumChannelType ("ns3::MultiModelSpectrumChannel");

  // installing eNB devices and UE devices
  // if more than one eNB create X2 interfaces between them for auto handover
  m_antennaDevs = m_lteHelper->InstallEnbDevice (m_antennaNodes);
  if (m_antennaNodes.GetN () > 1)
    {
      m_lteHelper->AddX2Interface (m_antennaNodes);
    }
  m_droneDevs = m_lteHelper->InstallUeDevice (m_droneNodes);

  // assigning address 7.0.0.0/8 to UE devices
  // unfortunately this is hardwired into EpcHelper implementation
  Ipv4InterfaceContainer m_droneIpv4;
  m_droneIpv4 = m_epcHelper->AssignUeIpv4Address (m_droneDevs);

  // create a static route for each UE to the SGW/PGW in order to communicate
  // with the internet
  Ipv4StaticRoutingHelper routingHelper;
  m_internetHelper.SetRoutingHelper (routingHelper);
  // assign to each drone the default route to the SGW
  for (uint32_t i = 0; i < m_droneNodes.GetN (); ++i)
    {
      Ptr<Ipv4StaticRouting> dronesStaticRoute = routingHelper.GetStaticRouting (m_droneNodes.Get (i)->GetObject<Ipv4>());
      dronesStaticRoute->SetDefaultRoute (m_epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // auto attach each drone UE to the eNB with the strongest signal
  m_lteHelper->Attach (m_droneDevs);

  // activate a dedicated bearer for video trasmission on all the UEs
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VIDEO;
  GbrQosInformation qos;
  qos.gbrDl = 20000000;  // Downlink GBR (bit/s): 20 Mbps
  qos.gbrUl = 5000000;  // Uplink GBR: 5 Mbps
  qos.mbrDl = 20000000;  // Downlink MBR: 20 Mbps
  qos.mbrUl = 5000000;  // Uplink MBR: 5 Mbps
  EpsBearer bearer(q, qos);
  m_lteHelper->ActivateDedicatedEpsBearer (m_droneDevs, bearer, EpcTft::Default());
}

void
LteDroneNetwork::EnableTraces ()
{
  NS_LOG_FUNCTION_NOARGS ();

  AsciiTraceHelper ascii;

  std::string p2pPath, ipPath;
  std::string path = ScenarioConfigurationHelper::Get ()->GetResultsPath ();

  p2pPath = path + "REMOTE";
  ipPath = path + "IP";

  m_p2pHelper.EnableAsciiAll (ascii.CreateFileStream (p2pPath));
  m_p2pHelper.EnablePcapAll (p2pPath);

  m_internetHelper.EnableAsciiIpv4All (ascii.CreateFileStream (ipPath));
  m_internetHelper.EnablePcapIpv4All (ipPath);

  m_lteHelper->EnableTraces ();
}





// DRONE NETWORK CONTAINER

void
DroneNetworkContainer::Add (Ptr<DroneNetwork> network)
{
  m_droneNetworks.push_back (network);
}

Ptr<DroneNetwork>
DroneNetworkContainer::Get (uint32_t index) const
{
  return m_droneNetworks[index];
}

Ptr<DroneNetwork>
DroneNetworkContainer::Get (std::string name) const
{
  NS_LOG_FUNCTION (name);

  for (uint32_t i = 0; i < m_droneNetworks.size (); i++)
    {
      auto droneNet = m_droneNetworks[i];
      if (droneNet->GetName () == name)
        {
          return droneNet;
        }
    }
  NS_LOG_ERROR ("No DroneNetwork found with name \"" << name << "\".");
  return nullptr;
}

uint32_t
DroneNetworkContainer::GetN () const
{
  return m_droneNetworks.size ();
}

DroneNetworkContainer::Iterator
DroneNetworkContainer::Begin () const
{
  return m_droneNetworks.begin ();
}

DroneNetworkContainer::Iterator
DroneNetworkContainer::End () const
{
  return m_droneNetworks.end ();
}

} // namespace ns3
