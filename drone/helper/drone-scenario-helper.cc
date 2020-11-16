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


#include <drone-scenario-helper.h>
#include <scenario-configuration-helper.h>
#include <ns3/component-manager.h>

//#define CONFIGURATOR ScenarioConfigurationHelper::Get()


namespace ns3
{

NS_LOG_COMPONENT_DEFINE_MASK ("DroneScenarioHelper", LOG_PREFIX_ALL);

DroneScenarioHelper&
DroneScenarioHelper::Create(int argc, char **argv, const std::string name)
{
  NS_LOG_FUNCTION(argc << " " << argv << " " << name);
  Initialize(argc, argv, name);

  return *this;
}

void
DroneScenarioHelper::Initialize(int argc, char **argv, const std::string name)
{
  NS_LOG_FUNCTION(argc << " " << argv << " " << name);
  m_configurator = ScenarioConfigurationHelper::Get();
  m_configurator->Initialize(argc, argv, name);

  m_components = std::vector<std::string>();

  NS_OBJECT_REGISTER_COMPONENT();
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
  NS_OBJECT_REQUIRE_COMPONENT("Initialize");
  return ScenarioConfigurationHelper::Get();
}


DroneScenarioHelper&
DroneScenarioHelper::SetDronesNumber(int num)
{
  NS_LOG_FUNCTION(num);
  NS_OBJECT_REQUIRE_COMPONENT("Initialize");
  // function code here
  NS_OBJECT_REGISTER_COMPONENT();
  return *this;
}

DSH&
DSH::SetDronesPosition(Ptr<PositionAllocator> pos)
{
  NS_LOG_FUNCTION("PositionAllocator pointer: " << pos);
  NS_OBJECT_REQUIRE_COMPONENT("SetDronesNumber");
  // function code here
  NS_OBJECT_REGISTER_COMPONENT();
  return *this;
}

DSH&
DSH::SetDronesApplication(Ptr<ApplicationContainer> apps)
{
  NS_LOG_FUNCTION("ApplicationContainer pointer: " << apps);
  NS_OBJECT_REQUIRE_COMPONENT("SetDronesNumber");
  // function code here
  NS_OBJECT_REGISTER_COMPONENT();
  return *this;
}

DSH&
DSH::SetAntennasNumber(int num)
{
  NS_LOG_FUNCTION(num);
  NS_OBJECT_REQUIRE_COMPONENT("Initialize");
  // function code here
  NS_OBJECT_REGISTER_COMPONENT();
  return *this;
}

DSH&
DSH::SetAntennasPosition(Ptr<PositionAllocator> pos)
{
  NS_LOG_FUNCTION("PositionAllocator pointer: " << pos);
  NS_OBJECT_REQUIRE_COMPONENT("SetAntennasNumber");
  // function code here
  NS_OBJECT_REGISTER_COMPONENT();
  return *this;
}

DSH&
DSH::SetRemotesNumber(int num)
{
  NS_LOG_FUNCTION(num);
  NS_OBJECT_REQUIRE_COMPONENT("Initialize");
  // function code here
  NS_OBJECT_REGISTER_COMPONENT();
  return *this;
}

DSH&
DSH::SetDronesApplication(Ptr<ApplicationContainer> apps)
{
  NS_LOG_FUNCTION("ApplicationContainer pointer: " << apps);
  NS_OBJECT_REQUIRE_COMPONENT("SetRemotesNumber");
  // function code here
  NS_OBJECT_REGISTER_COMPONENT();
  return *this;
}





} // namespace ns3