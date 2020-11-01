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

#include <ns3/core-module.h>
#include <ns3/scenario-configuration-helper.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("LteScenarioArcturus");

class Scenario
{
public:
  Scenario();
  ~Scenario();
  Scenario& Initialize(int argc, char **argv, std::string name);
  Scenario& SetDronesNumber(int num);
  Scenario& SetDronesPosition(Ptr<PositionAllocator> pos);
  Scenario& SetAntennasNumber(int num);
  Scenario& SetAntennasPosition(Ptr<PositionAllocator> pos);
  Scenario& SetRemotesNumber(int num);

};

} // namespace ns3

using namespace ns3;

int main (int argc, char **argv)
{
  Scenario scenario;

  Ptr<ListPositionAllocator> dronesPositions = CreateObject<ListPositionAllocator>();
  CONFIGURATOR->GetDronesPosition(dronesPositions);

  Ptr<ListPositionAllocator> antennasPositions = CreateObject<ListPositionAllocator>();
  CONFIGURATOR->GetAntennasPosition(antennasPositions);

  scenario.Initialize(argc, argv, "LteScenarioArcturus")
          .SetDronesNumber(CONFIGURATOR->GetDronesN())
          .SetDronesPosition(dronesPositions)
          .SetAntennasNumber(CONFIGURATOR->GetAntennasN())
          .SetAntennasPosition(antennasPositions)
          .SetRemotesNumber(CONFIGURATOR->GetRemotesN())


  return 0;
}