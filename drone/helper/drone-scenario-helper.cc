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

void
DroneScenarioHelper::Initialize(uint32_t argc, char **argv, const std::string name)
{
  NS_LOG_FUNCTION(argc << argv << name);
  NS_COMPMAN_ENSURE_UNIQUE();

  m_configurator = ScenarioConfigurationHelper::Get();
  m_configurator->Initialize(argc, argv, name);

  this->SetSimulationParameters(Seconds(m_configurator->GetDuration()));

  m_protocol = m_configurator->GetProtocol();

  this->SetNodesNumber();

  this->SetMobilityModels();

  this->InstallInternetStack();

  this->LoadProtocolGlobalSettings();

  this->SetupNetworkProtocol();

  this->LoadProtocolDeviceSettings();

  NS_COMPMAN_REGISTER_COMPONENT();
}

ScenarioConfigurationHelper*
DroneScenarioHelper::GetConfigurator()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");
  return ScenarioConfigurationHelper::Get();
}

void
DroneScenarioHelper::Run()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_ENSURE_UNIQUE();
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  Simulator::Run();
  Simulator::Destroy();

  NS_COMPMAN_REGISTER_COMPONENT();
}



// private

void
DroneScenarioHelper::SetSimulationParameters(Time duration)
{
  NS_LOG_FUNCTION(duration);

  Simulator::Stop(duration);
}

void
DroneScenarioHelper::SetNodesNumber()
{
  NS_LOG_FUNCTION_NOARGS();

  m_droneNodes.Create(m_configurator->GetDronesN());
  if (m_protocol == "lte") m_antennaNodes.Create(m_configurator->GetAntennasN());
  if (m_protocol == "wifi") m_zspNodes.Create(m_configurator->GetZspsN());
  m_remoteNodes.Create(m_configurator->GetRemotesN());
}

void
DroneScenarioHelper::SetMobilityModels()
{
  NS_LOG_FUNCTION_NOARGS();

  this->SetDronesMobility();
  if (m_protocol == "lte") this->SetAntennasPosition();
  if (m_protocol == "wifi") this->SetZspsPosition();
}

uint32_t
DroneScenarioHelper::MobilityToEnum(std::string mobilityModel)
{
  NS_LOG_FUNCTION_NOARGS();
  uint32_t mobilityModelValue = 0;
  while (mobilityModelValue < _MobilityModelName::ENUM_SIZE)
  {
    if (mobilityModel == _mobilityModels[mobilityModelValue]) break;
    ++mobilityModelValue;
  }

  NS_ASSERT_MSG(mobilityModelValue < _MobilityModelName::ENUM_SIZE,
      "No mobility model exists with name '" << mobilityModel << "'. Please check configuration file.");

  return mobilityModelValue;
}

void
DroneScenarioHelper::SetDronesMobility()
{
  NS_LOG_FUNCTION_NOARGS();

  std::string mobilityModel = m_configurator->GetDronesMobilityModel();

  MobilityHelper mobility;

  switch (this->MobilityToEnum(mobilityModel))
  {
    case CONSTANT_POSITION:
    {
      Ptr<ListPositionAllocator> position = CreateObject<ListPositionAllocator>();
      m_configurator->GetDronesPosition(position);
      mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
      mobility.SetPositionAllocator(position);
      mobility.Install(m_droneNodes);
    } break;

    case PARAMETRIC_SPEED:
    {
      for (uint32_t i = 0; i < m_droneNodes.GetN (); i++)
      {
        mobility.SetMobilityModel(mobilityModel,
            "SpeedCoefficients", SpeedCoefficientsValue(m_configurator->GetDroneSpeedCoefficients (i)),
            "FlightPlan", FlightPlanValue (m_configurator->GetDroneFlightPlan (i)),
            "CurveStep", DoubleValue (m_configurator->GetCurveStep ()));
        mobility.Install (m_droneNodes.Get (i));
      }
    } break;

    case CONSTANT_ACCELERATION:
    {
      for (uint32_t i = 0; i < m_droneNodes.GetN (); i++)
      {
        mobility.SetMobilityModel(mobilityModel,
            "Acceleration", DoubleValue (m_configurator->GetDroneAcceleration (i)),
            "MaxSpeed", DoubleValue (m_configurator->GetDroneMaxSpeed (i)),
            "FlightPlan", FlightPlanValue (m_configurator->GetDroneFlightPlan (i)),
            "CurveStep", DoubleValue (m_configurator->GetCurveStep ()));
        mobility.Install (m_droneNodes.Get (i));
      }
    } break;
  }
}

void
DroneScenarioHelper::SetAntennasPosition()
{
  NS_LOG_FUNCTION_NOARGS();

  Ptr<ListPositionAllocator> position = CreateObject<ListPositionAllocator>();
  m_configurator->GetAntennasPosition(position);
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(position);
  mobility.Install(m_antennaNodes);
}

void
DroneScenarioHelper::SetZspsPosition()
{
  NS_LOG_FUNCTION_NOARGS();

  Ptr<ListPositionAllocator> position = CreateObject<ListPositionAllocator>();
  m_configurator->GetZspsPosition(position);
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(position);
  mobility.Install(m_zspNodes);
}



void
DroneScenarioHelper::LoadProtocolGlobalSettings()
{
  NS_LOG_FUNCTION_NOARGS();
  auto settings = m_configurator->GetProtocolGlobalSettings();
  for (auto s : settings)
  {
    Config::SetDefault(s.first, StringValue(s.second));
  }

  ConfigStore config;
  config.ConfigureDefaults();
}

void
DroneScenarioHelper::LoadProtocolDeviceSettings()
{
  NS_LOG_FUNCTION_NOARGS();
  auto settings = m_configurator->GetProtocolDeviceSettings();
  for (auto s : settings)
  {
    Config::Set(s.first, StringValue(s.second));
  }
}

void
DroneScenarioHelper::InstallInternetStack()
{
  NS_LOG_FUNCTION_NOARGS();

  m_internetHelper.Install(m_droneNodes);
  if (m_protocol == "lte") m_internetHelper.Install(m_remoteNodes);
  if (m_protocol == "wifi") m_internetHelper.Install(m_zspNodes);
}

void
DroneScenarioHelper::SetupNetworkProtocol()
{
  NS_LOG_FUNCTION_NOARGS();

  if (m_protocol == "lte")
  {
    this->SetupLte();
    this->SetupLteIpv4Routing();
  }
  if (m_protocol == "wifi")
  {
    this->SetupWifi();
    this->SetupWifiIpv4Routing();
  }
}

void
DroneScenarioHelper::SetupLte()
{
  NS_LOG_FUNCTION_NOARGS();

  m_lteHelper = CreateObject<LteHelper> ();
  m_epcHelper = CreateObject<PointToPointEpcHelper> ();
  m_lteHelper->SetEpcHelper (m_epcHelper);

  //m_lteHelper->SetAttribute("PathlossModel", StringValue("ns3::Cost231PropagationLossModel"));

  //m_lteHelper->SetSchedulerType("ns3::PfFfMacScheduler"); // Proportional Fair (FemtoForumAPI) Scheduler
  //m_lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler"); // Round Robin (FemtoForumAPI) Scheduler
  //m_lteHelper->SetSchedulerAttribute("HarqEnabled", BooleanValue(true));
  //m_lteHelper->SetSchedulerAttribute("CqiTimerThreshold", UintegerValue(1000));

  PointToPointHelper p2pHelper;
  p2pHelper.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2pHelper.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2pHelper.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));
  m_p2pDevs = p2pHelper.Install(m_epcHelper->GetPgwNode(), m_remoteNodes.Get(0));

/* JUST LIMIT AT 1 REMOTE WHILE DEBUGGING
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
*/
    m_remoteDevs.Add(m_p2pDevs.Get(1));

  m_antennaDevs = m_lteHelper->InstallEnbDevice(m_antennaNodes);
  if (m_antennaNodes.GetN() > 1)
    m_lteHelper->AddX2Interface(m_antennaNodes);
  m_droneDevs = m_lteHelper->InstallUeDevice(m_droneNodes);

  // Try to connect each drone to each antenna (?)
  for (uint32_t i=0; i<m_antennaDevs.GetN(); ++i)
    m_lteHelper->Attach(m_droneDevs, m_antennaDevs.Get(i));
}

void
DroneScenarioHelper::SetupLteIpv4Routing()
{
  NS_LOG_FUNCTION_NOARGS();

  Ipv4AddressHelper ipv4Helper;

  ipv4Helper.SetBase ("1.0.0.0", "255.0.0.0");
  m_remoteIpv4 = ipv4Helper.Assign(m_remoteDevs);

  //ipv4Helper.SetBase ("100.0.0.0", "255.0.0.0");
  //m_p2pIpv4 = ipv4Helper.Assign(m_p2pDevs);

  // assigning address 7.0.0.0/8
  m_droneIpv4 = m_epcHelper->AssignUeIpv4Address(m_droneDevs);

  Ipv4StaticRoutingHelper routingHelper;
  m_internetHelper.SetRoutingHelper(routingHelper);

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


  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
}

void
DroneScenarioHelper::SetupWifi()
{
  // TODO
}

void
DroneScenarioHelper::SetupWifiIpv4Routing()
{
  // TODO
}

Ipv4InterfaceContainer
DroneScenarioHelper::GetDronesIpv4Interfaces()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  return m_droneIpv4;
}

Ipv4InterfaceContainer
DroneScenarioHelper::GetRemotesIpv4Interfaces()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

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
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  return this->GetIpv4Address(m_droneIpv4, id);
}

Ipv4Address
DroneScenarioHelper::GetRemoteIpv4Address(uint32_t id)
{
  NS_LOG_FUNCTION(id);
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  return this->GetIpv4Address(m_remoteIpv4, id);
}


// private
// why should I pass pointers to apps container by reference?
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

void
DroneScenarioHelper::SetDroneApplication(uint32_t id, Ptr<Application> app)
{
  NS_LOG_FUNCTION(id << app);
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  this->SetApplication(m_droneNodes, id, app);

/*
  NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetDroneApplication" + std::to_string(id));

  if (! NS_COMPMAN_CHECK_COMPONENT("SetDronesApplication") &&
      NS_COMPMAN_CHECK_MULTI_COMPONENT("SetDroneApplication", m_droneNodes.GetN()))
  {
    NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetDronesApplication");
    if (! NS_COMPMAN_CHECK_COMPONENT("CreateIpv4Routing"))
      NS_LOG_WARN("No internet routing has been created yet, apps may not work without IP addresses");
  }
*/
}

void
DroneScenarioHelper::SetDronesApplication(Ptr<ApplicationContainer> apps)
{
  NS_LOG_FUNCTION(&apps);
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");
  NS_COMPMAN_ENSURE_UNIQUE();

  this->SetApplications(m_droneNodes, apps);

  NS_COMPMAN_REGISTER_COMPONENT();
}

void
DroneScenarioHelper::SetRemoteApplication(uint32_t id, Ptr<Application> app)
{
  NS_LOG_FUNCTION(id << app);
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  this->SetApplication(m_remoteNodes, id, app);

/*
  NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetRemoteApplication" + std::to_string(id));
  if (! NS_COMPMAN_CHECK_COMPONENT("SetRemotesApplication") &&
      NS_COMPMAN_CHECK_MULTI_COMPONENT("SetRemoteApplication", m_remoteNodes.GetN()))
  {
    NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetRemotesApplication");
    if (! NS_COMPMAN_CHECK_COMPONENT("CreateIpv4Routing"))
      NS_LOG_WARN("No internet routing has been created yet, apps may not work without IP addresses");
  }
*/
}

void
DroneScenarioHelper::SetRemotesApplication(Ptr<ApplicationContainer> apps)
{
  NS_LOG_FUNCTION(&apps);
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");
  NS_COMPMAN_ENSURE_UNIQUE();

  this->SetApplications(m_remoteNodes, apps);

  NS_COMPMAN_REGISTER_COMPONENT();
}

void
DroneScenarioHelper::UseTestUdpEchoApplications()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");
  NS_COMPMAN_ENSURE_UNIQUE();

  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  UdpEchoServerHelper echoServer (9);
  if (m_protocol == "lte")
  {
    ApplicationContainer serverApps = echoServer.Install (m_remoteNodes);
    serverApps.Start (Seconds (m_configurator->GetRemoteApplicationStartTime (0)));
    serverApps.Stop (Seconds (m_configurator->GetRemoteApplicationStopTime (0)));
  }
  if (m_protocol == "wifi")
  {
    ApplicationContainer serverApps = echoServer.Install (m_zspNodes);
    serverApps.Start (Seconds (m_configurator->GetZspApplicationStartTime (0)));
    serverApps.Stop (Seconds (m_configurator->GetZspApplicationStopTime (0)));
  }

  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  UdpEchoClientHelper echoClient (this->GetRemoteIpv4Address(0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
  ApplicationContainer clientApps = echoClient.Install (m_droneNodes);
  for (uint32_t i = 0; i < m_droneNodes.GetN(); ++i)
  {
    clientApps.Get (i)->SetStartTime(Seconds (m_configurator->GetDroneApplicationStartTime (i)));
    clientApps.Get (i)->SetStopTime(Seconds (m_configurator->GetDroneApplicationStopTime (i)));
  }

  NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetRemotesApplication");
  NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetDronesApplication");
}


} // namespace ns3