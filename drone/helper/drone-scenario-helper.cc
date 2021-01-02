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

DroneScenarioHelper&
DroneScenarioHelper::Create(int argc, char **argv, const std::string name)
{
  NS_LOG_FUNCTION(argc << argv << name);
  Initialize(argc, argv, name);

  return *this;
}

void
DroneScenarioHelper::Initialize(int argc, char **argv, const std::string name)
{
  NS_LOG_FUNCTION(argc << argv << name);
  m_configurator = ScenarioConfigurationHelper::Get();
  m_configurator->Initialize(argc, argv, name);

  NS_COMPMAN_REGISTER_COMPONENT();
}

DroneScenarioHelper::~DroneScenarioHelper()
{
  NS_LOG_FUNCTION_NOARGS();
  Simulator::Destroy();
}

ScenarioConfigurationHelper*
DroneScenarioHelper::GetConfigurator()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");
  return ScenarioConfigurationHelper::Get();
}



DroneScenarioHelper&
DroneScenarioHelper::SetDronesNumber(int num)
{
  NS_LOG_FUNCTION(num);
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  m_droneNodes.Create(num);

  NS_COMPMAN_REGISTER_COMPONENT();
  return *this;
}

DroneScenarioHelper&
DroneScenarioHelper::SetAntennasNumber(int num)
{
  NS_LOG_FUNCTION(num);
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  m_antennaNodes.Create(num);

  NS_COMPMAN_REGISTER_COMPONENT();
  return *this;
}

DroneScenarioHelper&
DroneScenarioHelper::SetRemotesNumber(int num)
{
  NS_LOG_FUNCTION(num);
  NS_COMPMAN_REQUIRE_COMPONENT("Initialize");

  m_remoteNodes.Create(num);

  NS_COMPMAN_REGISTER_COMPONENT();
  return *this;
}



DroneScenarioHelper&
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
    Ptr<PositionAllocator> position;
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
  return *this;
}

DroneScenarioHelper&
DroneScenarioHelper::SetAntennasPositionFromConfig()
{
  NS_LOG_FUNCTION_NOARGS();
  NS_COMPMAN_REQUIRE_COMPONENT("SetAntennasNumber");

  Ptr<PositionAllocator> position;
  m_configurator->GetAntennasPosition(position);
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(position);
  mobility.Install(m_antennaNodes);

  NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetAntennasPosition");
  return *this;
}


DroneScenarioHelper&
DroneScenarioHelper::SetDroneApplication(uint32_t id, Ptr<Application> app)
{
  NS_LOG_FUNCTION(app);
  NS_COMPMAN_REQUIRE_COMPONENT("SetDronesNumber");

  m_droneNodes.Get(id)->AddApplication(app);

  NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetDroneApplication" + std::to_string(id));

  if (!NS_COMPMAN_CHECK_COMPONENT("SetDronesApplication") &&
      NS_COMPMAN_CHECK_MULTI_COMPONENT("SetDroneApplication", m_droneNodes.GetN()))
  {
    NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetDronesApplication");
  }

  return *this;
}

DroneScenarioHelper&
DroneScenarioHelper::SetDronesApplication(Ptr<ApplicationContainer> apps)
{
  NS_LOG_FUNCTION(apps);
  NS_COMPMAN_REQUIRE_COMPONENT("SetDronesNumber");

  for (uint32_t i=0; i<apps->GetN(); ++i)
  {
    m_droneNodes.Get(i)->AddApplication(apps->Get(i));
  }

  NS_COMPMAN_REGISTER_COMPONENT();
  return *this;
}


DroneScenarioHelper&
DroneScenarioHelper::SetRemoteApplication(uint32_t id, Ptr<Application> app)
{
  NS_LOG_FUNCTION(app);
  NS_COMPMAN_REQUIRE_COMPONENT("SetRemotesNumber");

  m_remoteNodes.Get(id)->AddApplication(app);

  NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetRemoteApplication" + std::to_string(id));

  if (!NS_COMPMAN_CHECK_COMPONENT("SetRemotesApplication") &&
      NS_COMPMAN_CHECK_MULTI_COMPONENT("SetRemoteApplication", m_remoteNodes.GetN()))
  {
    NS_COMPMAN_REGISTER_COMPONENT_WITH_NAME("SetRemotesApplication");
  }

  return *this;
}

DroneScenarioHelper&
DroneScenarioHelper::SetRemotesApplication(Ptr<ApplicationContainer> apps)
{
  NS_LOG_FUNCTION(apps);
  NS_COMPMAN_REQUIRE_COMPONENT("SetRemotesNumber");

  for (uint32_t i=0; i<apps->GetN(); ++i)
  {
    m_remoteNodes.Get(i)->AddApplication(apps->Get(i));
  }

  NS_COMPMAN_REGISTER_COMPONENT();
  return *this;
}








} // namespace ns3