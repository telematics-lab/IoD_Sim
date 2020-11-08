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
#include <ns3/applications-module.h>
#include <ns3/drone-server.h>
#include <ns3/drone-client.h>
#include <ns3/drone-scenario-helper.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LteScenarioArcturus");

int main (int argc, char **argv)
{
  auto scenario = DroneScenarioHelper::Create(argc, argv, "LTE Scenario - Arcturus");
  auto CFG = scenario.GetConfigurator();

  Ptr<ListPositionAllocator> dronesPositions = CreateObject<ListPositionAllocator>();
  CFG->GetDronesPosition(dronesPositions);

  Ptr<ListPositionAllocator> antennasPositions = CreateObject<ListPositionAllocator>();
  CFG->GetAntennasPosition(antennasPositions);

  ApplicationContainer clientApps;
  for (uint32_t i = 0; i < CFG->GetDronesN(); ++i)
  {
    Ptr<Application> clientApp = CreateObject<DroneClient>();
    clientApp->SetStartTime(Seconds(CFG->GetDroneApplicationStartTime(i)));
    clientApp->SetStopTime(Seconds(CFG->GetDroneApplicationStopTime(i)));
    clientApps.Add(clientApp);
  }

  ApplicationContainer serverApps;
  Ptr<Application> serverApp = CreateObject<DroneServer>();
  serverApp->SetStartTime(Seconds(CFG->GetRemoteApplicationStartTime(0)));
  serverApp->SetStopTime(Seconds(CFG->GetRemoteApplicationStopTime(0)));
  serverApps.Add(serverApp);


  scenario.SetDronesNumber(CFG->GetDronesN())
          .SetDronesPosition(dronesPositions)
          .SetAntennasNumber(CFG->GetAntennasN())
          .SetAntennasPosition(antennasPositions)
          .SetRemotesNumber(CFG->GetRemotesN())
          .SetDronesApplication(clientApps)
          .SetRemotesApplication(serverApps);

  scenario.Run();


  return 0;
}