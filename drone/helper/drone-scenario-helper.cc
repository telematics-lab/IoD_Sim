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
#include <ns3/component-manager.h>

//#define CONFIGURATOR ScenarioConfigurationHelper::Get()


namespace ns3
{

NS_LOG_COMPONENT_DEFINE_MASK ("DroneScenarioHelper", LOG_PREFIX_ALL);

void
DroneScenarioHelper::Initialize(uint32_t argc, char **argv, std::string name /*="DroneScenario"*/)
{
  NS_LOG_FUNCTION(argc << argv << name);
  NS_COMPMAN_ENSURE_UNIQUE();

  m_configurator = ScenarioConfigurationHelper::Get();
  m_configurator->Initialize(argc, argv, name);

  this->LoadGlobalSettings();

  this->CreateNodes();

  this->SetMobilityModels();

  this->SetupNetworks();

  this->LoadIndividualSettings();

  NS_COMPMAN_REGISTER_COMPONENT();
}

void
DroneScenarioHelper::EnableTraces(uint32_t net_id)
{
  NS_LOG_FUNCTION(net_id);
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  m_networks.Get(net_id)->EnableTraces();
}

void
DroneScenarioHelper::EnableTraces(std::string net_name)
{
  NS_LOG_FUNCTION(net_name);
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  m_networks.Get(net_name)->EnableTraces();
}

void
DroneScenarioHelper::EnableTracesAll()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  for (auto net = m_networks.Begin(); net != m_networks.End(); net++)
  {
    (*net)->EnableTraces();
  }
}

ScenarioConfigurationHelper*
DroneScenarioHelper::GetConfigurator()
{
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");
  return ScenarioConfigurationHelper::Get();
}

void
DroneScenarioHelper::Run()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_ENSURE_UNIQUE();
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  Simulator::Stop(Seconds(m_configurator->GetDuration()));
  Simulator::Run();
  Simulator::Destroy();

  /*
  Since ns3::LteHelper generates its traces in the root folder of ns3 by default
  and there's no way to change that, here if it finds any "lte" type network the
  helper manually moves them to the current result folder using "mv" with a system call.
  */
  for (auto i = m_networks.Begin(); i != m_networks.End(); i++)
  {
    if ((*i)->GetProtocol() == "lte")
    {
      system(("mv Dl* " + m_configurator->GetResultsPath()).c_str());
      system(("mv Ul* " + m_configurator->GetResultsPath()).c_str());
      break;
    }
  }

  NS_COMPMAN_REGISTER_COMPONENT();
}

uint32_t
DroneScenarioHelper::GetRemoteId(uint32_t num)
{
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");
  return m_remoteNodes.Get(num)->GetId();
}

uint32_t
DroneScenarioHelper::GetAntennaId(uint32_t num)
{
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");
  NS_ASSERT_MSG(num < m_antennaNodes.GetN(), "Antenna index out of bound");
  return m_antennaNodes.Get(num)->GetId();
}



void
DroneScenarioHelper::CreateNodes()
{
  NS_LOG_FUNCTION_NOARGS();

  m_droneNodes.Create(m_configurator->GetDronesN());
  m_antennaNodes.Create(m_configurator->GetAntennasN());
  m_remoteNodes.Create(m_configurator->GetRemotesN());
}

void
DroneScenarioHelper::SetMobilityModels()
{
  NS_LOG_FUNCTION_NOARGS();

  this->SetDronesMobility();
  this->SetAntennasPosition();
}

uint32_t
DroneScenarioHelper::MobilityToEnum(std::string mobilityModel)
{
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

  std::string globalMobilityModel = m_configurator->GetDronesMobilityModel();

  for (uint32_t i = 0; i < m_droneNodes.GetN (); i++)
  {
    MobilityHelper mobility;

    std::string mobilityModel = globalMobilityModel;
    if (globalMobilityModel == "mixed")
      mobilityModel = m_configurator->GetDroneMobilityModel(i);

    switch (this->MobilityToEnum(mobilityModel))
    {
      case CONSTANT_POSITION:
      {
        mobility.SetMobilityModel(mobilityModel);
        mobility.Install(m_droneNodes.Get(i));
        m_droneNodes.Get(i)->GetObject<MobilityModel>()->SetPosition(m_configurator->GetDronePosition(i));
      } break;

      case PARAMETRIC_SPEED:
      {
        mobility.SetMobilityModel(mobilityModel,
            "SpeedCoefficients", SpeedCoefficientsValue(m_configurator->GetDroneSpeedCoefficients (i)),
            "FlightPlan", FlightPlanValue (m_configurator->GetDroneFlightPlan (i)),
            "CurveStep", DoubleValue (m_configurator->GetCurveStep ()));
        mobility.Install (m_droneNodes.Get (i));
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
DroneScenarioHelper::LoadGlobalSettings()
{
  NS_LOG_FUNCTION_NOARGS();
  auto settings = m_configurator->GetGlobalSettings();
  for (auto s : settings)
  {
    Config::SetDefault(s.first, StringValue(s.second));
  }

  ConfigStore config;
  config.ConfigureDefaults();
}

void
DroneScenarioHelper::LoadIndividualSettings()
{
  NS_LOG_FUNCTION_NOARGS();
  auto settings = m_configurator->GetIndividualSettings();
  for (auto s : settings)
  {
    Config::Set(s.first, StringValue(s.second));
  }
}

void
DroneScenarioHelper::SetupNetworks()
{
  NS_LOG_FUNCTION_NOARGS();

  m_networks = m_configurator->GetNetworks();

  Ipv4AddressHelper ipv4H;
  ipv4H.SetBase("100.1.0.0", "255.255.0.0");

  m_backbone.Create(m_networks.GetN());
  m_backbone.Add(m_remoteNodes);

  m_internetHelper.Install (m_backbone);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  NetDeviceContainer backboneDevs = csma.Install (m_backbone);

  ipv4H.SetBase(Ipv4Address("250.1.0.0"), Ipv4Mask("255.255.0.0"));
  ipv4H.Assign(backboneDevs);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  for (uint32_t i = 0; i < m_networks.GetN(); i++)
  {
    Ptr<DroneNetwork> droneNetwork = m_networks.Get(i);
    std::string netName = droneNetwork->GetName();
    for (uint32_t id : m_configurator->GetDronesInNetwork(netName))
      droneNetwork->AttachDrone(m_droneNodes.Get(id));
    for (uint32_t id : m_configurator->GetAntennasInNetwork(netName))
      droneNetwork->AttachAntenna(m_antennaNodes.Get(id));
    droneNetwork->SetIpv4AddressHelper(ipv4H);

    droneNetwork->ConnectToBackbone(m_backbone.Get(i));
    droneNetwork->Generate();
  }

  for (uint32_t i = 0; i < m_backbone.GetN(); i++)
  {
    NS_LOG_LOGIC("BackBone Interface " << i << ": " << m_backbone.Get(i)->GetObject<Ipv4>()->GetAddress(1, 0));
  }

}

Ipv4Address
DroneScenarioHelper::GetIpv4Address(Ptr<Node> node)
{
  return node->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
}

Ipv4Address
DroneScenarioHelper::GetDroneIpv4Address(uint32_t id)
{
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  return this->GetIpv4Address(m_droneNodes.Get(id));
}

Ipv4Address
DroneScenarioHelper::GetRemoteIpv4Address(uint32_t id)
{
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  // add 1 since index 0 is the PGW (for LTE with EPC network)
  return this->GetIpv4Address(m_remoteNodes.Get(id));
}

/*
Ipv4InterfaceContainer
DroneScenarioHelper::GetDronesIpv4Interfaces()
{
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  return m_droneIpv4;
}

Ipv4InterfaceContainer
DroneScenarioHelper::GetRemotesIpv4Interfaces()
{
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
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  return this->GetIpv4Address(m_droneIpv4, id);
}

Ipv4Address
DroneScenarioHelper::GetRemoteIpv4Address(uint32_t id)
{
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  // add 1 since index 0 is the PGW (for LTE with EPC network)
  return this->GetIpv4Address(m_remoteIpv4, id + 1);
}
*/


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
  NS_LOG_FUNCTION(app);
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
DroneScenarioHelper::UseUdpEchoApplications()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");
  NS_COMPMAN_ENSURE_UNIQUE();

  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  UdpEchoServerHelper echoServer (9);

  // Installing the server application only on the first remote.
  ApplicationContainer serverApps = echoServer.Install (m_remoteNodes.Get(0));
  serverApps.Start (Seconds (m_configurator->GetRemoteApplicationStartTime (0)));
  serverApps.Stop (Seconds (m_configurator->GetRemoteApplicationStopTime (0)));


  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);

  // using the IP of the first remote only as target.
  UdpEchoClientHelper echoClient (GetIpv4Address(m_remoteNodes.Get(0)), 9);
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