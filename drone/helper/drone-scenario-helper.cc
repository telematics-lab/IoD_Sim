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


#include "drone-scenario-helper.h"

//#define CONFIGURATOR ScenarioConfigurationHelper::Get()


namespace ns3
{

NS_LOG_COMPONENT_DEFINE_MASK ("DroneScenarioHelper", LOG_PREFIX_ALL);

void
DroneScenarioHelper::Initialize (uint32_t argc, char **argv, const std::string& name /*="DroneScenario"*/)
{
  NS_LOG_FUNCTION (argc << argv << name);

  m_configurator = ScenarioConfigurationHelper::Get ();
  m_configurator->Initialize (argc, argv, name);

  this->LoadGlobalSettings ();
  this->CreateNodes ();
  this->SetMobilityModels ();
  this->SetupNetworks ();
  this->LoadIndividualSettings ();
}

void
DroneScenarioHelper::EnableTraces (uint32_t net_id) const
{
  if (m_configurator->RadioMap ())
    {
      return;
    }

  NS_LOG_FUNCTION (net_id);

  m_networks.Get (net_id)->EnableTraces ();
}

void
DroneScenarioHelper::EnableTraces (const std::string& net_name) const
{
  if (m_configurator->RadioMap ())
    {
      return;
    }

  NS_LOG_FUNCTION (net_name);

  m_networks.Get (net_name)->EnableTraces ();
}

void
DroneScenarioHelper::EnableTracesAll () const
{
  if (m_configurator->RadioMap ())
    {
      return;
    }

  NS_LOG_FUNCTION_NOARGS ();

  for (auto net = m_networks.Begin (); net != m_networks.End (); net++)
    {
      (*net)->EnableTraces ();
    }
}

ScenarioConfigurationHelper*
DroneScenarioHelper::GetConfigurator () const
{
  return ScenarioConfigurationHelper::Get ();
}

void
DroneScenarioHelper::Run () const
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_configurator->RadioMap ())
    {
      bool anyLte = false;
      for (auto i = m_networks.Begin (); i != m_networks.End (); i++)
        {
          anyLte = (*i)->GetProtocol () == "lte";
          if (anyLte)
            {
              break;
            }
        }
      NS_ASSERT_MSG (anyLte,
                     "Environment Radio Map can be generated only if an LTE network is present. Aborting simulation.");
      NS_LOG_INFO ("Generating Environment Radio Map, simulation will not run.");
      this->GenerateRadioMap ();
    }
  else
    {
      Simulator::Stop (Seconds (m_configurator->GetDuration ()));
    }

  Simulator::Run ();
  Simulator::Destroy ();

  if (m_configurator->RadioMap ())
    {
      system (("python3 ../tools/makeplot-rem.py " + m_configurator->GetResultsPath () + m_configurator->GetName () + ".rem").c_str ());
    }
}

uint32_t
DroneScenarioHelper::GetRemoteId (uint32_t num) const
{
  return m_remoteNodes.Get (num)->GetId ();
}

uint32_t
DroneScenarioHelper::GetAntennaId (uint32_t num) const
{
  NS_ASSERT_MSG (num < m_antennaNodes.GetN (), "Antenna index out of bound");
  return m_antennaNodes.Get (num)->GetId ();
}



void
DroneScenarioHelper::CreateNodes ()
{
  NS_LOG_FUNCTION_NOARGS ();

  m_droneNodes.Create (m_configurator->GetDronesN ());
  m_antennaNodes.Create (m_configurator->GetAntennasN ());
  m_remoteNodes.Create (m_configurator->GetRemotesN ());
}

void
DroneScenarioHelper::SetMobilityModels () const
{
  NS_LOG_FUNCTION_NOARGS ();

  this->SetDronesMobility ();
  this->SetAntennasPosition ();

  /*
  install the building helper to each antenna and drone of the scenario.
  Once the building have been created in GetBuildings() there
  is no need to store them in a container since the BuildingHelper has a global
  reference to all of them already
  */
  m_configurator->GetBuildings ();

  BuildingsHelper::Install (m_antennaNodes);
  BuildingsHelper::Install (m_droneNodes);

  //BuildingsHelper::MakeMobilityModelConsistent ();
}

uint32_t
DroneScenarioHelper::MobilityToEnum (const std::string& mobilityModel) const
{
  NS_LOG_FUNCTION (mobilityModel);

  /*
  this function uses the _mobilityModels array to check if a mobilityModel
  ha been implemented and convert it and returns the index of where it
  is defined in the array. These indexes will be checked agaist values in the
  _mobilityModelName enum to call the appropriate procedure of the switch case
  when calling SetDronesMobility()
  */
  uint32_t mobilityModelValue = 0;
  while (mobilityModelValue < _MobilityModelName::ENUM_SIZE)
    {
      if (mobilityModel == _mobilityModels[mobilityModelValue])
        {
          break;
        }
      ++mobilityModelValue;
    }

  NS_ASSERT_MSG (mobilityModelValue < _MobilityModelName::ENUM_SIZE,
                 "No mobility model exists with name '" << mobilityModel << "'. Please check configuration file.");

  return mobilityModelValue;
}

void
DroneScenarioHelper::SetDronesMobility () const
{
  NS_LOG_FUNCTION_NOARGS ();

  std::string globalMobilityModel = m_configurator->GetDronesMobilityModel ();

  for (uint32_t i = 0; i < m_droneNodes.GetN (); i++)
    {
      MobilityHelper mobility;

      std::string mobilityModel = globalMobilityModel;
      if (globalMobilityModel == "mixed")
        {
          mobilityModel = m_configurator->GetDroneMobilityModel (i);
        }

      switch (this->MobilityToEnum (mobilityModel))
        {
          case CONSTANT_POSITION:
            {
              mobility.SetMobilityModel (mobilityModel);
              mobility.Install (m_droneNodes.Get (i));
              m_droneNodes.Get (i)->GetObject<MobilityModel>()->SetPosition (m_configurator->GetDronePosition (i));
            } break;

          case WAYPOINTS:
            {
              mobility.SetMobilityModel (mobilityModel);
              mobility.Install (m_droneNodes.Get (i));

              auto wpMobilityModel = m_droneNodes.Get (i)->GetObject<WaypointMobilityModel>();
              for (Waypoint wp : m_configurator->GetDroneWaypoints (i))
                {
                  wpMobilityModel->AddWaypoint (wp);
                }
            } break;

          case PARAMETRIC_SPEED:
            {
              mobility.SetMobilityModel (mobilityModel,
                                         "SpeedCoefficients", SpeedCoefficientsValue (m_configurator->GetDroneSpeedCoefficients (i)),
                                         "FlightPlan", FlightPlanValue (m_configurator->GetDroneFlightPlan (i)),
                                         "CurveStep", DoubleValue (m_configurator->GetCurveStep ()));
              mobility.Install (m_droneNodes.Get (i));
            } break;

          case CONSTANT_ACCELERATION:
            {
              for (uint32_t i = 0; i < m_droneNodes.GetN (); i++)
                {
                  mobility.SetMobilityModel (mobilityModel,
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
DroneScenarioHelper::SetAntennasPosition () const
{
  NS_LOG_FUNCTION_NOARGS ();

  Ptr<ListPositionAllocator> position = CreateObject<ListPositionAllocator>();
  m_configurator->GetAntennasPosition (position);
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (position);
  mobility.Install (m_antennaNodes);
}


void
DroneScenarioHelper::LoadGlobalSettings () const
{
  NS_LOG_FUNCTION_NOARGS ();
  auto settings = m_configurator->GetGlobalSettings ();
  for (auto s : settings)
    {
      Config::SetDefault (s.first, StringValue (s.second));
    }

  ConfigStore config;
  config.ConfigureDefaults ();
}

void
DroneScenarioHelper::LoadIndividualSettings () const
{
  NS_LOG_FUNCTION_NOARGS ();
  auto settings = m_configurator->GetIndividualSettings ();
  for (auto s : settings)
    {
      Config::Set (s.first, StringValue (s.second));
    }
}

void
DroneScenarioHelper::SetupNetworks ()
{
  NS_LOG_FUNCTION_NOARGS ();

  m_networks = m_configurator->GetNetworks ();

  /*
  set the base IP address for the first network
  all the network using this Ipv4AddressHelper will use a new base
  address incrementing by one the base address used by the previous net
  to avoid address collisions
  */
  Ipv4AddressHelper ipv4H;
  ipv4H.SetBase ("100.0.0.0", "255.0.0.0");

  // install the internet helper on all the internet nodes in the scenario
  m_internetHelper.Install (m_remoteNodes);
  m_internetHelper.Install (m_droneNodes);

  // add the remotes to the backbone
  m_backbone.Add (m_remoteNodes);

  // generate all the networks in the scenario
  for (uint32_t i = 0; i < m_networks.GetN (); i++)
    {
      Ptr<DroneNetwork> droneNetwork = m_networks.Get (i);
      std::string netName = droneNetwork->GetName ();
      // get all the drones in this network
      for (uint32_t id : m_configurator->GetDronesInNetwork (netName))
        {
          droneNetwork->AttachDrone (m_droneNodes.Get (id));
        }
      // get all the antennas in this network
      for (uint32_t id : m_configurator->GetAntennasInNetwork (netName))
        {
          droneNetwork->AttachAntenna (m_antennaNodes.Get (id));
        }

      // pass the Ipv4AddressHelper to the network
      droneNetwork->SetIpv4AddressHelper (ipv4H);

      // generate the network and add its gateway to the backbone
      droneNetwork->Generate ();
      m_backbone.Add (droneNetwork->GetGateway ());
    }

  // setup a CSMA LAN between all the remotes and network gateways in the backbone
  CsmaHelper csma;
  //csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  //csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  NetDeviceContainer backboneDevs = csma.Install (m_backbone);

  // set a new address base for the backbone
  ipv4H.SetBase (Ipv4Address ("200.0.0.0"), Ipv4Mask ("255.0.0.0"));
  ipv4H.Assign (backboneDevs);

  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // create static routes between each remote node to each network gateway
  Ipv4StaticRoutingHelper routingHelper;
  m_internetHelper.SetRoutingHelper (routingHelper);
  for (uint32_t i = 0; i < m_remoteNodes.GetN (); i++)
    {
      Ptr<Node> remoteNode = m_backbone.Get (i);
      for (uint32_t j = m_remoteNodes.GetN (); j < m_backbone.GetN (); j++)
        {
          Ptr<Node> netGwNode = m_backbone.Get (j);
          uint32_t netGwBackboneDevIndex = netGwNode->GetNDevices () - 1;

          // since network gateway have at least 2 ipv4 addresses (one in the network and the other for the backbone)
          // we should create a route to the internal ip using the backbone ip as next hop
          Ipv4Address netGwBackboneIpv4 = netGwNode->GetObject<Ipv4>()->GetAddress (netGwBackboneDevIndex, 0).GetLocal ();
          Ipv4Address netGwInternalIpv4 = netGwNode->GetObject<Ipv4>()->GetAddress (1, 0).GetLocal ();

          NS_LOG_LOGIC ("Setting-up static route: REMOTE " << i << " (" << remoteNode->GetObject<Ipv4>()->GetAddress (1, 0).GetLocal () <<
                        ") -> NETWORK " << j - m_remoteNodes.GetN () << " (" << netGwBackboneIpv4 << " -> " << netGwInternalIpv4 << ")");

          Ptr<Ipv4StaticRouting> staticRouteRem = routingHelper.GetStaticRouting (remoteNode->GetObject<Ipv4>());
          staticRouteRem->AddNetworkRouteTo (netGwInternalIpv4, Ipv4Mask ("255.0.0.0"), netGwBackboneIpv4, 1);
        }
    }
}


void
DroneScenarioHelper::GenerateRadioMap () const
{
  // making it static in order for it to be alive when simulation run
  static Ptr<RadioEnvironmentMapHelper> m_remHelper = CreateObject<RadioEnvironmentMapHelper> ();
  m_remHelper->SetAttribute ("OutputFile", StringValue (m_configurator->GetResultsPath () + m_configurator->GetName () + ".rem"));

  // setting default values range (-50:50) for both x and y, 100x100 points at height 10 mt
  m_remHelper->SetAttribute ("XMin", DoubleValue (-50.0));
  m_remHelper->SetAttribute ("XMax", DoubleValue (50.0));
  m_remHelper->SetAttribute ("XRes", UintegerValue (100));
  m_remHelper->SetAttribute ("YMin", DoubleValue (-50.0));
  m_remHelper->SetAttribute ("YMax", DoubleValue (50.0));
  m_remHelper->SetAttribute ("YRes", UintegerValue (100));
  m_remHelper->SetAttribute ("Z", DoubleValue (10.0));

  auto parameters = m_configurator->GetRadioMapParameters ();
  for (auto par : parameters)
    {
      m_remHelper->SetAttribute (par.first, StringValue (par.second));
    }

  m_remHelper->Install ();

  NS_LOG_DEBUG ("Buildings present: " << BuildingList::GetNBuildings ());
}


Ipv4Address
DroneScenarioHelper::GetIpv4Address (Ptr<Node> node, uint32_t index) const
{
  return node->GetObject<Ipv4>()->GetAddress (index, 0).GetLocal ();
}

Ipv4Address
DroneScenarioHelper::GetDroneIpv4Address (uint32_t id, uint32_t index) const
{
  return this->GetIpv4Address (m_droneNodes.Get (id), index);
}

Ipv4Address
DroneScenarioHelper::GetRemoteIpv4Address (uint32_t id, uint32_t index) const
{
  // add 1 since index 0 is the PGW (for LTE with EPC network)
  return this->GetIpv4Address (m_remoteNodes.Get (id), index);
}

// private
void
DroneScenarioHelper::SetApplications (NodeContainer nodes, ApplicationContainer apps) const
{
  for (uint32_t i = 0; i < apps.GetN (); ++i)
    {
      nodes.Get (i)->AddApplication (apps.Get (i));
    }
}

// private
void
DroneScenarioHelper::SetApplication (NodeContainer nodes, uint32_t id, Ptr<Application> app) const
{
  nodes.Get (id)->AddApplication (app);
}

void
DroneScenarioHelper::SetDroneApplication (uint32_t id, Ptr<Application> app) const
{
  NS_LOG_FUNCTION (id << app);

  this->SetApplication (m_droneNodes, id, app);
}

void
DroneScenarioHelper::SetDronesApplication (ApplicationContainer apps) const
{
  NS_LOG_FUNCTION (&apps);
  this->SetApplications (m_droneNodes, apps);
}

void
DroneScenarioHelper::SetRemoteApplication (uint32_t id, Ptr<Application> app) const
{
  NS_LOG_FUNCTION (app);

  this->SetApplication (m_remoteNodes, id, app);
}

void
DroneScenarioHelper::UseUdpEchoApplications () const
{
  NS_LOG_FUNCTION_NOARGS ();

  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  UdpEchoServerHelper echoServer (9);

  // Installing the server application only on the first remote.
  ApplicationContainer serverApps = echoServer.Install (m_remoteNodes.Get (0));
  serverApps.Start (Seconds (m_configurator->GetRemoteApplicationStartTime (0)));
  serverApps.Stop (Seconds (m_configurator->GetRemoteApplicationStopTime (0)));


  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);

  // using the IP of the first remote only as target.
  UdpEchoClientHelper echoClient (GetIpv4Address (m_remoteNodes.Get (0), 1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
  ApplicationContainer clientApps = echoClient.Install (m_droneNodes);
  for (uint32_t i = 0; i < m_droneNodes.GetN (); ++i)
    {
      clientApps.Get (i)->SetStartTime (Seconds (m_configurator->GetDroneApplicationStartTime (i)));
      clientApps.Get (i)->SetStopTime (Seconds (m_configurator->GetDroneApplicationStopTime (i)));
    }
}


} // namespace ns3
