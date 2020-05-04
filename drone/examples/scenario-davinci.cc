/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2019 The IoD_Sim Authors.
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
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <vector>

#include <ns3/assert.h>
#include <ns3/command-line.h>
#include <ns3/config.h>
#include <ns3/double.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/ipv4-interface-container.h>
#include <ns3/log.h>
#include <ns3/mobility-helper.h>
#include <ns3/object-factory.h>
#include <ns3/object-vector.h>
#include <ns3/net-device-container.h>
#include <ns3/position-allocator.h>
#include <ns3/ptr.h>
#include <ns3/ssid.h>
#include <ns3/string.h>
#include <ns3/vector.h>
#include <ns3/wifi-helper.h>
#include <ns3/wifi-mac-helper.h>
#include <ns3/yans-wifi-helper.h>

#include <ns3/report.h>

#include <ns3/drone-client.h>
#include <ns3/drone-list.h>
#include <ns3/drone-server.h>
#include <ns3/flight-plan.h>
#include <ns3/proto-point.h>
#include <ns3/scenario-configuration-helper.h>
#include <ns3/zsp-list.h>

namespace ns3 {

class Scenario
{
public:
  using ms = std::chrono::duration<float, std::milli>;

  Scenario (int argc, char **argv);
  virtual ~Scenario ();

  void operator() ();

private:
  void ConfigurePhy ();
  void ConfigureMac ();
  void ConfigureMobility ();
  void ConfigureMobilityDrones ();
  void ConfigureMobilityZsps ();
  void ConfigureNetwork ();
  void ConfigureApplication ();
  void ConfigureApplicationDrones ();
  void ConfigureApplicationZsps ();
  void ConfigureSimulator ();

  NodeContainer m_drones;
  NodeContainer m_zsps;
  NetDeviceContainer m_netDevices;

  WifiHelper m_wifi;
  YansWifiPhyHelper m_wifiPhy;
  Ipv4InterfaceContainer m_ifaceIps;
  const char *m_ifaceNetMask;
};

NS_LOG_COMPONENT_DEFINE ("Scenario");

Scenario::Scenario (int argc, char **argv) :
  m_ifaceNetMask {"255.0.0.0"}
{
  CONFIGURATOR->Initialize (argc, argv, "davinci");

  m_drones.Create (CONFIGURATOR->GetDronesN ());
  m_zsps.Create (CONFIGURATOR->GetZspsN ());

  // Register created Drones and ZSPs in /DroneList/ and /ZspList/ respectively
  for (auto drone = m_drones.Begin (); drone != m_drones.End (); drone++)
    DroneList::Add (*drone);

  for (auto zsp = m_zsps.Begin (); zsp != m_zsps.End (); zsp++)
    ZspList::Add (*zsp);

  ConfigurePhy ();
  ConfigureMac ();
  ConfigureMobility ();
  ConfigureNetwork ();
  ConfigureApplication ();
  ConfigureSimulator ();
}

Scenario::~Scenario ()
{
}

void
Scenario::ConfigurePhy ()
{
  NS_LOG_FUNCTION_NOARGS ();

  const std::string phyMode = CONFIGURATOR->GetPhyMode ();
  YansWifiChannelHelper wifiChannel;
  AsciiTraceHelper ascii;

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold",
                      StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold",
                      StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  //Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
  //                    StringValue (phyMode));

  // The helpers used below will help us putting together the wifi NICs we want
  // wifi.EnableLogComponents();  // Turn on all Wifi logging

  m_wifi.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);

  m_wifiPhy = YansWifiPhyHelper::Default ();
  // This is one parameter that matters when using FixedRssLossModel
  // set it to zero; otherwise, gain will be added
  m_wifiPhy.Set ("RxGain", DoubleValue (0));
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  m_wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss  ("ns3::FriisPropagationLossModel",
                                   "Frequency", DoubleValue (2.4e9)); // carrier
  m_wifiPhy.SetChannel (wifiChannel.Create());
}

void
Scenario::ConfigureMac ()
{
  NS_LOG_FUNCTION_NOARGS ();

  const std::string phyMode = CONFIGURATOR->GetPhyMode ();

  WifiMacHelper wifiMac;
  Ssid ssid = Ssid ("wifi-default");  // const ?

  m_wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode", StringValue (phyMode),
                                  "ControlMode", StringValue (phyMode));

  // setup sta => drones
  wifiMac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid));

  NetDeviceContainer dronesDevices = m_wifi.Install (m_wifiPhy, wifiMac, m_drones);
  m_netDevices.Add (dronesDevices);

  // setup ap => ZSPs
  wifiMac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid));

  NetDeviceContainer zspsDevices = m_wifi.Install (m_wifiPhy, wifiMac, m_zsps);
  m_netDevices.Add (zspsDevices);
}

void
Scenario::ConfigureMobility ()
{
  NS_LOG_FUNCTION_NOARGS ();

  ConfigureMobilityDrones ();
  ConfigureMobilityZsps ();
}

void
Scenario::ConfigureMobilityDrones ()
{
  NS_LOG_FUNCTION_NOARGS ();

  MobilityHelper mobility;

  for (uint32_t i = 0; i < m_drones.GetN (); i++)
    {
      mobility.SetMobilityModel ("ns3::ConstantAccelerationDroneMobilityModel",
                                 "Acceleration", DoubleValue (CONFIGURATOR->GetDroneAcceleration (i)),
                                 "MaxSpeed", DoubleValue (CONFIGURATOR->GetDroneMaxSpeed (i)),
                                 "FlightPlan", FlightPlanValue (CONFIGURATOR->GetDroneFlightPlan (i)),
                                 "CurveStep", DoubleValue (CONFIGURATOR->GetCurveStep ()));

      mobility.Install (m_drones.Get (i));
    }
}

void
Scenario::ConfigureMobilityZsps ()
{
  NS_LOG_FUNCTION_NOARGS ();

  MobilityHelper mobilityZsps;
  auto positionAllocatorZsps = CreateObject<ListPositionAllocator> ();

  CONFIGURATOR->GetZspsPosition (positionAllocatorZsps);

  mobilityZsps.SetPositionAllocator (positionAllocatorZsps);
  mobilityZsps.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityZsps.Install (m_zsps);
}

void
Scenario::ConfigureNetwork ()
{
  NS_LOG_FUNCTION_NOARGS ();

  InternetStackHelper internet;
  Ipv4AddressHelper ipv4;

  internet.Install (m_drones);
  internet.Install (m_zsps);

  NS_LOG_INFO ("> Assigning IP Addresses.");
  ipv4.SetBase ("10.0.0.0", m_ifaceNetMask);
  m_ifaceIps = ipv4.Assign (m_netDevices);

  for (uint i = 0; i < m_netDevices.GetN(); i++)
    NS_LOG_INFO("[Node " << m_netDevices.Get (i)->GetNode ()->GetId ()
                << "] assigned address " << m_ifaceIps.GetAddress (i, 0));
}

void
Scenario::ConfigureApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  ConfigureApplicationDrones ();
  ConfigureApplicationZsps ();
}

void
Scenario::ConfigureApplicationDrones ()
{
  NS_LOG_FUNCTION_NOARGS ();

  for (uint32_t i = 0; i < m_drones.GetN (); i++)
    {
      auto client = CreateObjectWithAttributes<DroneClient>
        ("Ipv4Address", Ipv4AddressValue (m_ifaceIps.GetAddress(i)),
         "Ipv4SubnetMask", Ipv4MaskValue (m_ifaceNetMask),
         "Duration", DoubleValue (CONFIGURATOR->GetDuration ()));

      double droneAppStartTime = CONFIGURATOR->GetDroneApplicationStartTime (i);
      double droneAppStopTime = CONFIGURATOR->GetDroneApplicationStopTime (i);

      NS_LOG_LOGIC ("[Node " << i << "] will operate in time interval from "
                    << droneAppStartTime << " to "
                    << droneAppStopTime << "s.");

      m_drones.Get (i)->AddApplication (client);
      client->SetStartTime (Seconds (droneAppStartTime));
      client->SetStopTime (Seconds (droneAppStopTime));
    }
}

void
Scenario::ConfigureApplicationZsps ()
{
  NS_LOG_FUNCTION_NOARGS ();

  auto server = CreateObjectWithAttributes<DroneServer>
    ("Ipv4Address", Ipv4AddressValue (m_ifaceIps.GetAddress (m_ifaceIps.GetN () - 1)),
     "Ipv4SubnetMask", Ipv4MaskValue (m_ifaceNetMask));

  m_zsps.Get (0)->AddApplication (server);

  server->SetStartTime (Seconds (CONFIGURATOR->GetZspApplicationStartTime (0)));
  server->SetStopTime (Seconds (CONFIGURATOR->GetZspApplicationStopTime (0)));
}

void
Scenario::ConfigureSimulator ()
{
  NS_LOG_FUNCTION_NOARGS ();
  std::stringstream phyTraceLog,
                    pcapLog;
  AsciiTraceHelper  ascii;

  phyTraceLog << CONFIGURATOR->GetResultsPath () << "-phy.log";
  pcapLog << CONFIGURATOR->GetResultsPath () << "-host";

  // Enable Tracing
  m_wifiPhy.EnableAsciiAll (ascii.CreateFileStream (phyTraceLog.str ()));
  m_wifiPhy.EnablePcap (pcapLog.str (), m_netDevices);

  // Enable Report
  Report::Get ()->Initialize ("davinci",
                              CONFIGURATOR->GetCurrentDateTime (),
                              CONFIGURATOR->GetResultsPath ());

  Simulator::Stop (Seconds (CONFIGURATOR->GetDuration ()));
}

void
Scenario::operator() ()
{
  NS_LOG_FUNCTION_NOARGS ();

  std::chrono::high_resolution_clock timer;
  auto start = timer.now ();

  Simulator::Run ();

  // Report Module needs the simulator context alive to introspect it
  Report::Get ()->Save ();

  Simulator::Destroy ();

  auto stop = timer.now ();
  auto deltaTime = std::chrono::duration_cast<ms> (stop - start).count ();
  NS_LOG_INFO ("Simulation terminated. Took " << deltaTime << "ms.");
}

} // namespace ns3

int
main (int argc, char **argv)
{
  ns3::Scenario s (argc, argv);
  s ();  // run the scenario as soon as it is ready

  return 0;
}
