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

#define DSH DroneScenarioHelper::Get()
#define CONFIG DSH->GetConfigurator()

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Scenario");

int main (int argc, char **argv)
{
  DSH->Initialize(argc, argv, "Drone Scenario");
  DSH->UseTestUdpEchoApplications();

  NS_LOG_DEBUG("Drone1 IP: " << DSH->GetDroneIpv4Address(0));
  NS_LOG_DEBUG("Drone2 IP: " << DSH->GetDroneIpv4Address(1));
  NS_LOG_DEBUG("Remote1 IP: " << DSH->GetRemoteIpv4Address(0));
/*
  for (uint32_t i = 0; i < CONFIG->GetDronesN(); ++i)
  {
    Ptr<Application> clientApp = CreateObjectWithAttributes<DroneClient>(
        "Ipv4Address", Ipv4AddressValue(DSH->GetDroneIpv4Address(i)),
        "Ipv4SubnetMask", Ipv4MaskValue("255.0.0.0"),
        "Duration", DoubleValue(CONFIG->GetDuration()),
        "DestinationIpv4Address", Ipv4AddressValue(DSH->GetRemoteIpv4Address(0)));
    clientApp->SetStartTime(Seconds(CONFIG->GetDroneApplicationStartTime(i)));
    clientApp->SetStopTime(Seconds(CONFIG->GetDroneApplicationStopTime(i)));
    DSH->SetDroneApplication(i, clientApp);
  }

  Ptr<Application> serverApp = CreateObjectWithAttributes<DroneServer>(
      "Ipv4Address", Ipv4AddressValue(DSH->GetRemoteIpv4Address(0)),
      "Ipv4SubnetMask", Ipv4MaskValue("255.0.0.0"));
  serverApp->SetStartTime(Seconds(CONFIG->GetRemoteApplicationStartTime(0)));
  serverApp->SetStopTime(Seconds(CONFIG->GetRemoteApplicationStopTime(0)));
  DSH->SetRemoteApplication(0, serverApp);
*/

  Ptr<RadioEnvironmentMapHelper> remHelper = CreateObject<RadioEnvironmentMapHelper> ();
  remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/3"));
  remHelper->SetAttribute ("OutputFile", StringValue ("../results/rem.out"));
  remHelper->SetAttribute ("XMin", DoubleValue (-100.0));
  remHelper->SetAttribute ("XMax", DoubleValue (100.0));
  remHelper->SetAttribute ("XRes", UintegerValue (100));
  remHelper->SetAttribute ("YMin", DoubleValue (-100.0));
  remHelper->SetAttribute ("YMax", DoubleValue (100.0));
  remHelper->SetAttribute ("YRes", UintegerValue (75));
  remHelper->SetAttribute ("Z", DoubleValue (10));
  remHelper->SetAttribute ("UseDataChannel", BooleanValue (true));
  //remHelper->SetAttribute ("RbId", IntegerValue (10));
  remHelper->Install ();

  DSH->Run();

  return 0;
}