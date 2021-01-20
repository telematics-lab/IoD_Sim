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

NS_LOG_COMPONENT_DEFINE ("Scenario");

int main (int argc, char **argv)
{
  auto scenario = DroneScenarioHelper::Get();
  scenario->Create(argc, argv, "LTE Scenario - Arcturus");
  auto CFG = scenario->GetConfigurator();

  scenario->SetNodesNumberFromConfig()
          ->SetDronesMobilityFromConfig()
          ->SetAntennasPositionFromConfig()
          ->InstallInternetStack()
          ->CreateLteEpc()
          ->CreateRemotesToEpcNetwork()
          ->CreateDronesToAntennasNetwork()
          ->CreateIpv4Routing();

  NS_LOG_DEBUG("Drone1 IP: " << scenario->GetDroneIpv4Address(0));
  NS_LOG_DEBUG("Drone2 IP: " << scenario->GetDroneIpv4Address(1));
  NS_LOG_DEBUG("Remote1 IP: " << scenario->GetRemoteIpv4Address(0));


  for (uint32_t i = 0; i < CFG->GetDronesN(); ++i)
  {
    Ptr<Application> clientApp = CreateObjectWithAttributes<DroneClient>(
        "Ipv4Address", Ipv4AddressValue(scenario->GetDroneIpv4Address(i)),
        "Ipv4SubnetMask", Ipv4MaskValue("255.0.0.0"),
        "Duration", DoubleValue(CFG->GetDuration()),
        "DestinationIpv4Address", Ipv4AddressValue(scenario->GetRemoteIpv4Address(0)));
    clientApp->SetStartTime(Seconds(CFG->GetDroneApplicationStartTime(i)));
    clientApp->SetStopTime(Seconds(CFG->GetDroneApplicationStopTime(i)));
    scenario->SetDroneApplication(i, clientApp);
  }

  Ptr<Application> serverApp = CreateObjectWithAttributes<DroneServer>(
      "Ipv4Address", Ipv4AddressValue(scenario->GetRemoteIpv4Address(0)),
      "Ipv4SubnetMask", Ipv4MaskValue("255.0.0.0"));
  serverApp->SetStartTime(Seconds(CFG->GetRemoteApplicationStartTime(0)));
  serverApp->SetStopTime(Seconds(CFG->GetRemoteApplicationStopTime(0)));
  scenario->SetRemoteApplication(0, serverApp);


  //scenario->UseTestUdpEchoApplications();

  scenario->SetSimulationParameters(Seconds(10.0))
          ->Run();


  return 0;
}