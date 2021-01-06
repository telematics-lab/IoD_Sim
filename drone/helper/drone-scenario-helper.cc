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


#include "drone-scenario-helper.h"
#include "scenario-configuration-helper.h"
#include <ns3/component-manager.h>

//#define CONFIGURATOR ScenarioConfigurationHelper::Get()


namespace ns3
{

NS_LOG_COMPONENT_DEFINE_MASK ("DroneScenarioHelper", LOG_PREFIX_ALL);

// private
void
DroneScenarioHelper::Initialize(uint32_t argc, char **argv, const std::string name)
{
  NS_LOG_FUNCTION(argc << argv << name);
  m_configurator = ScenarioConfigurationHelper::Get();
  m_configurator->Initialize(argc, argv, name);
}

DroneScenarioHelper*
DroneScenarioHelper::Create(uint32_t argc, char **argv, const std::string name)
{
  NS_LOG_FUNCTION(argc << argv << name);
  this->Initialize(argc, argv, name);

  NS_COMPMAN_REGISTER_COMPONENT();
  return this;
}

ScenarioConfigurationHelper*
DroneScenarioHelper::GetConfigurator()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("Create");
  return ScenarioConfigurationHelper::Get();
}

DroneScenarioHelper*
DroneScenarioHelper::SetSimulationParameters(ns3::Time duration)
{
  NS_LOG_FUNCTION(duration);
  NS_COMPMAN_REQUIRE_COMPONENT("Create");

  Simulator::Stop(duration);

  NS_COMPMAN_REGISTER_COMPONENT();
  return this;
}

void
DroneScenarioHelper::Run()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("SetSimulationParameters");
  NS_COMPMAN_REQUIRE_COMPONENT("CreateRemotesToEpcNetwork");
  NS_COMPMAN_REQUIRE_COMPONENT("CreateDronesToAntennasNetwork");
  NS_COMPMAN_REQUIRE_COMPONENT("CreateIpv4Routing");
  NS_COMPMAN_REQUIRE_COMPONENT("SetRemotesApplication");
  NS_COMPMAN_REQUIRE_COMPONENT("SetDronesApplication");

  Simulator::Run();
  Simulator::Destroy();

  NS_COMPMAN_REGISTER_COMPONENT();
  return;
}



// private
void
DroneScenarioHelper::SetNumber(NodeContainer& nodes, uint32_t num)
{
  nodes.Create(num);
}

DroneScenarioHelper*
DroneScenarioHelper::SetDronesNumber(uint32_t num)
{
  NS_LOG_FUNCTION(num);
  NS_COMPMAN_REQUIRE_COMPONENT("Create");

  this->SetNumber(m_droneNodes, num);

  NS_COMPMAN_REGISTER_COMPONENT();
  return this;
}

DroneScenarioHelper*
DroneScenarioHelper::SetAntennasNumber(uint32_t num)
{
  NS_LOG_FUNCTION(num);
  NS_COMPMAN_REQUIRE_COMPONENT("Create");

  this->SetNumber(m_antennaNodes, num);

  NS_COMPMAN_REGISTER_COMPONENT();
  return this;
}

DroneScenarioHelper*
DroneScenarioHelper::SetRemotesNumber(uint32_t num)
{
  NS_LOG_FUNCTION(num);
  NS_COMPMAN_REQUIRE_COMPONENT("Create");

  this->SetNumber(m_remoteNodes, num);

  NS_COMPMAN_REGISTER_COMPONENT();
  return this;
}



DroneScenarioHelper*
DroneScenarioHelper::SetDronesMobilityFromConfig()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("SetDronesNumber");

  std::string mobilityModel = m_configurator->GetDronesMobilityModel();

  NS_ASSERT_MSG(
      mobilityModel == "ns3::ParametricSpeedDroneMobilityModel" ||
      mobilityModel == "ns3::ConstantAccelerationDroneMobilityModel" ||
      mobilityModel == "ns3::ConstantPositionMobilityModel",
      "No mobility model exists with name '" << mobilityModel << "'. Please check configuration file.");

  MobilityHelper mobility;

  if (mobilityModel == "ns3::ConstantPositionMobilityModel")
  {
    Ptr<ListPositionAllocator> position = CreateObject<ListPositionAllocator>();
    m_configurator->GetDronesPosition(position);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator(position);
    mobility.Install(m_droneNodes);
  }
  else
  {
    for (uint32_t i = 0; i < m_droneNodes.GetN (); i++)
    {
      if (mobilityModel == "ns3::ParametricSpeedDroneMobilityModel")
      {
        mobility.SetMobilityModel(mobilityModel,
                                  "SpeedCoefficients", SpeedCoefficientsValue(m_configurator->GetDroneSpeedCoefficients (i)),
                                  "FlightPlan", FlightPlanValue (m_configurator->GetDroneFlightPlan (i)),
                                  "CurveStep", DoubleValue (m_configurator->GetCurveStep ()));
      }
      if (mobilityModel == "ns3::ConstantAccelerationDroneMobilityModel")
      {
        mobility.SetMobilityModel(mobilityModel,
                                  "Acceleration", DoubleValue (m_configurator->GetDroneAcceleration (i)),
                                  "MaxSpeed", DoubleValue (m_configurator->GetDroneMaxSpeed (i)),
                                  "FlightPlan", FlightPlanValue (m_configurator->GetDroneFlightPlan (i)),
                                  "CurveStep", DoubleValue (m_configurator->GetCurveStep ()));
      }

      mobility.Install (m_droneNodes.Get (i));
    }
  }

  NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetDronesMobility");
  return this;
}

DroneScenarioHelper*
DroneScenarioHelper::SetAntennasPositionFromConfig()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("SetAntennasNumber");

  Ptr<ListPositionAllocator> position = CreateObject<ListPositionAllocator>();
  m_configurator->GetAntennasPosition(position);
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(position);
  mobility.Install(m_antennaNodes);

  NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetAntennasPosition");
  return this;
}



DroneScenarioHelper*
DroneScenarioHelper::CreateLteEpc()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("Create");

  // Makes the LTE Helper to manage LTE connections
  m_lteHelper = CreateObject<LteHelper> ();
  // Makes the Evolved Packet Core that manages the connections
  // between UEs, eNBs and the internet network
  // EPC mainly provides the PGW to where remotes connect to the LTE network
  // And the SGW and MME to where eNBs (and so UEs) connect.
  m_epcHelper = CreateObject<PointToPointEpcHelper> ();
  m_lteHelper->SetEpcHelper (m_epcHelper);

  NS_COMPMAN_REGISTER_COMPONENT();
  return this;
}

DroneScenarioHelper*
DroneScenarioHelper::CreateRemotesToEpcNetwork()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("SetRemotesNumber");
  NS_COMPMAN_REQUIRE_COMPONENT("CreateLteEpc");

  // The first remote node is connected to the PGW with a point to point link
  PointToPointHelper p2pHelper;
  p2pHelper.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2pHelper.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2pHelper.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));
  m_p2pDevs = p2pHelper.Install(m_epcHelper->GetPgwNode(), m_remoteNodes.Get(0));

  if (m_remoteNodes.GetN() > 1)
  {
    // if more than 1 remote is used, connect them via LAN. Only the first
    // of them will be connected to the PGW with a point to point link.
    CsmaHelper csmaHelper;
    csmaHelper.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
    csmaHelper.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
    m_remoteDevs = csmaHelper.Install (m_remoteNodes);
  }
  else
    m_remoteDevs.Add(m_p2pDevs.Get(1));

  NS_COMPMAN_REGISTER_COMPONENT();
  return this;
}

DroneScenarioHelper*
DroneScenarioHelper::CreateDronesToAntennasNetwork()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("SetDronesMobility");
  NS_COMPMAN_REQUIRE_COMPONENT("SetAntennasPosition");
  NS_COMPMAN_REQUIRE_COMPONENT("CreateLteEpc");

  m_antennaDevs = m_lteHelper->InstallEnbDevice(m_antennaNodes);
  m_droneDevs = m_lteHelper->InstallUeDevice(m_droneNodes);

  /* Try to connect all the drones to each antenna
  for (uint32_t i=0; i<m_antennaDevs.GetN(); ++i)
    m_lteHelper->Attach(m_droneDevs, m_antennaDevs.Get(i));
  */

  NS_COMPMAN_REGISTER_COMPONENT();
  return this;
}

DroneScenarioHelper*
DroneScenarioHelper::CreateIpv4Routing()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("CreateRemotesToEpcNetwork");
  NS_COMPMAN_REQUIRE_COMPONENT("CreateDronesToAntennasNetwork");

  InternetStackHelper internetHelper;
  internetHelper.Install(m_droneNodes);
  internetHelper.Install(m_remoteNodes);

  Ipv4AddressHelper ipv4Helper;

  ipv4Helper.SetBase ("200.0.0.0", "255.0.0.0");
  m_remoteIpv4 = ipv4Helper.Assign(m_remoteDevs);

  ipv4Helper.SetBase ("100.0.0.0", "255.0.0.0");
  m_p2pIpv4 = ipv4Helper.Assign(m_p2pDevs);

  // assigning address 7.0.0.0/8
  m_droneIpv4 = m_epcHelper->AssignUeIpv4Address(m_droneDevs);

  Ipv4StaticRoutingHelper routingHelper;
  internetHelper.SetRoutingHelper(routingHelper);

  // add to each remote a route to the PGW
  Ipv4Address pgwAddress = m_epcHelper->GetPgwNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
  for (uint32_t i=0; i<m_remoteNodes.GetN(); ++i)
  {
    Ptr<Ipv4StaticRouting> remoteStaticRoute = routingHelper.GetStaticRouting(m_remoteNodes.Get(i)->GetObject<Ipv4>());
    remoteStaticRoute->AddNetworkRouteTo(pgwAddress, Ipv4Mask("255.0.0.0"), 1);
  }

  // assign to each drone the default route to the SGW
  for (uint32_t i=0; i<m_droneNodes.GetN(); ++i)
  {
    Ptr<Ipv4StaticRouting> dronesStaticRoute = routingHelper.GetStaticRouting(m_droneNodes.Get(i)->GetObject<Ipv4>());
    dronesStaticRoute->SetDefaultRoute(m_epcHelper->GetUeDefaultGatewayAddress(), 1);
  }

  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  NS_COMPMAN_REGISTER_COMPONENT();
  return this;
}

Ipv4InterfaceContainer
DroneScenarioHelper::GetDronesIpv4Interfaces()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("CreateIpv4Routing");

  return m_droneIpv4;
}

Ipv4InterfaceContainer
DroneScenarioHelper::GetRemotesIpv4Interfaces()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("CreateIpv4Routing");

  return m_remoteIpv4;
}

// private
Ipv4Address
DroneScenarioHelper::GetIpv4Address(Ipv4InterfaceContainer& ifaces, uint32_t id)
{
  return ifaces.GetAddress(id);
}

Ipv4Address
DroneScenarioHelper::GetDroneIpv4Address(uint32_t id)
{
  NS_LOG_FUNCTION(id);
  NS_COMPMAN_REQUIRE_COMPONENT("CreateIpv4Routing");

  return this->GetIpv4Address(m_droneIpv4, id);
}

Ipv4Address
DroneScenarioHelper::GetRemoteIpv4Address(uint32_t id)
{
  NS_LOG_FUNCTION(id);
  NS_COMPMAN_REQUIRE_COMPONENT("CreateIpv4Routing");

  return this->GetIpv4Address(m_remoteIpv4, id);
}


// private
// why should I pass apps by reference?
void
DroneScenarioHelper::SetApplications(NodeContainer& nodes, Ptr<ApplicationContainer>& apps)
{
  for (uint32_t i=0; i<apps->GetN(); ++i)
    nodes.Get(i)->AddApplication(apps->Get(i));
}

// private
void
DroneScenarioHelper::SetApplication(NodeContainer& nodes, uint32_t id, Ptr<Application> app)
{
  nodes.Get(id)->AddApplication(app);
}

DroneScenarioHelper*
DroneScenarioHelper::SetDroneApplication(uint32_t id, Ptr<Application> app)
{
  NS_LOG_FUNCTION(id << app);
  NS_COMPMAN_REQUIRE_COMPONENT("SetDronesNumber");

  this->SetApplication(m_droneNodes, id, app);

  NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetDroneApplication" + std::to_string(id));

  if (! NS_COMPMAN_CHECK_COMPONENT("SetDronesApplication") &&
      NS_COMPMAN_CHECK_MULTI_COMPONENT("SetDroneApplication", m_droneNodes.GetN()))
  {
    NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetDronesApplication");
    if (! NS_COMPMAN_CHECK_COMPONENT("CreateIpv4Routing"))
      NS_LOG_WARN("No internet routing has been created yet, apps may not work without IP addresses");
  }
  return this;
}

DroneScenarioHelper*
DroneScenarioHelper::SetDronesApplication(Ptr<ApplicationContainer> apps)
{
  NS_LOG_FUNCTION(&apps);
  NS_COMPMAN_REQUIRE_COMPONENT("SetDronesNumber");
  if (! NS_COMPMAN_CHECK_COMPONENT("CreateIpv4Routing"))
    NS_LOG_WARN("No internet routing has been created yet, apps may not work without IP addresses");

  this->SetApplications(m_droneNodes, apps);

  NS_COMPMAN_REGISTER_COMPONENT();
  return this;
}

DroneScenarioHelper*
DroneScenarioHelper::SetRemoteApplication(uint32_t id, Ptr<Application> app)
{
  NS_LOG_FUNCTION(id << app);
  NS_COMPMAN_REQUIRE_COMPONENT("SetRemotesNumber");

  this->SetApplication(m_remoteNodes, id, app);

  NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetRemoteApplication" + std::to_string(id));
  if (! NS_COMPMAN_CHECK_COMPONENT("SetRemotesApplication") &&
      NS_COMPMAN_CHECK_MULTI_COMPONENT("SetRemoteApplication", m_remoteNodes.GetN()))
  {
    NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetRemotesApplication");
    if (! NS_COMPMAN_CHECK_COMPONENT("CreateIpv4Routing"))
      NS_LOG_WARN("No internet routing has been created yet, apps may not work without IP addresses");
  }
  return this;
}

DroneScenarioHelper*
DroneScenarioHelper::SetRemotesApplication(Ptr<ApplicationContainer> apps)
{
  NS_LOG_FUNCTION(&apps);
  NS_COMPMAN_REQUIRE_COMPONENT("SetRemotesNumber");
  if (! NS_COMPMAN_CHECK_COMPONENT("CreateIpv4Routing"))
    NS_LOG_WARN("No internet routing has been created yet, apps may not work without IP addresses");

  this->SetApplications(m_remoteNodes, apps);

  NS_COMPMAN_REGISTER_COMPONENT();
  return this;
}


} // namespace ns3