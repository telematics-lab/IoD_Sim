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
#include <ns3/speed-coefficients.h>
#include <ns3/wifi-mac-factory-helper.h>
#include <ns3/wifi-mac-layer-configuration.h>
#include <ns3/wifi-phy-factory-helper.h>
#include <ns3/wifi-phy-layer-configuration.h>
#include <ns3/ipv4-network-layer-configuration.h>
#include <ns3/zsp-list.h>
#include <ns3/wifi-phy-simulation-helper.h>
#include <ns3/wifi-mac-simulation-helper.h>
#include <ns3/ipv4-simulation-helper.h>

namespace ns3 {

constexpr int N_LAYERS = 4;
constexpr int PHY_LAYER = 0;
constexpr int MAC_LAYER = 1;
constexpr int NET_LAYER = 2;
constexpr int APP_LAYER = 3;

class Scenario
{
public:
  using ms = std::chrono::duration<float, std::milli>;

  Scenario (int argc, char **argv);
  virtual ~Scenario ();

  void operator() ();

private:
  void ApplyStaticConfig ();
  void ConfigurePhy ();
  void ConfigureMac ();
  void ConfigureNetwork ();

  void ConfigureDrones ();

  void ConfigureMobility ();
  void ConfigureMobilityDrones ();
  void ConfigureMobilityZsps ();
  void ConfigureApplication ();
  void ConfigureApplicationDrones ();
  void ConfigureApplicationZsps ();
  void ConfigureSimulator ();

  NodeContainer m_drones;
  NodeContainer m_zsps;
  NetDeviceContainer m_netDevices;

  Ipv4InterfaceContainer m_ifaceIps;

  std::array<std::vector<Ptr<Object>>, N_LAYERS> m_protocolStacks;
};

NS_LOG_COMPONENT_DEFINE ("Scenario");

Scenario::Scenario (int argc, char **argv)
{
  CONFIGURATOR->Initialize (argc, argv);

  m_drones.Create (CONFIGURATOR->GetDronesN ());
  m_zsps.Create (CONFIGURATOR->GetZspsN ());

  // Register created Drones and ZSPs in /DroneList/ and /ZspList/ respectively
  for (auto drone = m_drones.Begin (); drone != m_drones.End (); drone++)
    DroneList::Add (*drone);

  for (auto zsp = m_zsps.Begin (); zsp != m_zsps.End (); zsp++)
    ZspList::Add (*zsp);

  ApplyStaticConfig ();
  ConfigurePhy ();
  ConfigureMac ();
  ConfigureNetwork ();
  // ConfigureMobility ();
  // ConfigureApplication ();
  // ConfigureSimulator ();
}

Scenario::~Scenario ()
{
}

void
Scenario::ApplyStaticConfig ()
{
  NS_LOG_FUNCTION_NOARGS ();

  const auto staticConfig = CONFIGURATOR->GetStaticConfig ();
  for (auto& param : staticConfig)
      Config::SetDefault (param.first, StringValue (param.second));
}

void
Scenario::ConfigurePhy ()
{
  NS_LOG_FUNCTION_NOARGS ();

  const auto phyLayerConfs = CONFIGURATOR->GetPhyLayers ();

  for (auto& phyLayerConf : phyLayerConfs) {
    if (phyLayerConf->GetType ().compare("wifi") == 0) {
      YansWifiChannelHelper wifiChannel;
      const auto wifiConf = StaticCast<WifiPhyLayerConfiguration, PhyLayerConfiguration> (phyLayerConf);
      const auto wifiSim = CreateObject<WifiPhySimulationHelper> ();

      wifiSim->GetWifiHelper ().SetStandard (wifiConf->GetStandard ());

      // This is one parameter that matters when using FixedRssLossModel
      // set it to zero; otherwise, gain will be added
      wifiSim->GetWifiPhyHelper ().Set ("RxGain", DoubleValue (wifiConf->GetRxGain ()));
      // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
      wifiSim->GetWifiPhyHelper ().SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

      WifiPhyFactoryHelper::SetPropagationDelay (wifiChannel, wifiConf->GetChannelPropagationDelayModel ());
      WifiPhyFactoryHelper::AddPropagationLoss (wifiChannel, wifiConf->GetChannelPropagationLossModel ());

      wifiSim->GetWifiPhyHelper ().SetChannel (wifiChannel.Create ());

      m_protocolStacks[PHY_LAYER].push_back (wifiSim);
    } else {
      NS_FATAL_ERROR ("Unsupported PHY Layer Type: " << phyLayerConf->GetType ());
    }
  }
}

void
Scenario::ConfigureMac ()
{
  NS_LOG_FUNCTION_NOARGS ();

  const auto macLayerConfs = CONFIGURATOR->GetMacLayers ();

  size_t i = 0;
  for (auto& macLayerConf : macLayerConfs) {
    if (macLayerConf->GetType ().compare ("wifi") == 0) {
      const auto wifiPhy = StaticCast<WifiPhySimulationHelper, Object> (m_protocolStacks[PHY_LAYER][i]);
      const auto wifiMac = CreateObject<WifiMacSimulationHelper> ();
      const auto wifiConf = StaticCast<WifiMacLayerConfiguration, MacLayerConfiguration> (macLayerConf);
      Ssid ssid = Ssid (wifiConf->GetSsid ());

      WifiMacFactoryHelper::SetRemoteStationManager (wifiPhy->GetWifiHelper (),
                                                     wifiConf->GetRemoteStationManagerConfiguration ());

      m_protocolStacks[MAC_LAYER].push_back (wifiMac);

      // TODO L8R
      // setup sta => drones
      // wifiMac.SetType ("ns3::StaWifiMac",
      //                 "Ssid", SsidValue (ssid));

      // NetDeviceContainer dronesDevices = m_wifi.Install (m_wifiPhy, wifiMac, m_drones);
      // m_netDevices.Add (dronesDevices);

      // // setup ap => ZSPs
      // wifiMac.SetType ("ns3::ApWifiMac",
      //                 "Ssid", SsidValue (ssid));

      // NetDeviceContainer zspsDevices = m_wifi.Install (m_wifiPhy, wifiMac, m_zsps);
      // m_netDevices.Add (zspsDevices);
    } else {
      NS_FATAL_ERROR ("Unsupported MAC Layer Type: " << macLayerConf->GetType ());
    }

    i++;
  }
}

void
Scenario::ConfigureNetwork ()
{
  NS_LOG_FUNCTION_NOARGS ();

  const auto layerConfs = CONFIGURATOR->GetNetworkLayers ();
  for (auto& layerConf : layerConfs) {
    if (layerConf->GetType ().compare("ipv4") == 0) {
      const auto ipv4Conf = StaticCast<Ipv4NetworkLayerConfiguration, NetworkLayerConfiguration> (layerConf);
      const auto ipv4Sim = CreateObject<Ipv4SimulationHelper> ();

      ipv4Sim->GetIpv4Helper ().SetBase (Ipv4Address (ipv4Conf->GetAddress ().c_str ()),
                                         Ipv4Mask (ipv4Conf->GetMask ().c_str ()));

      m_protocolStacks[NET_LAYER].push_back (ipv4Sim);
    } else {
      NS_FATAL_ERROR ("Unsupported Network Layer Type: " << layerConf ->GetType ());
    }
  }


  // internet.Install (m_drones);
  // internet.Install (m_zsps);

  // NS_LOG_INFO ("> Assigning IP Addresses.");
  // ipv4.SetBase ("10.0.0.0", m_ifaceNetMask);
  // m_ifaceIps = ipv4.Assign (m_netDevices);

  // for (uint i = 0; i < m_netDevices.GetN(); i++)
  //   NS_LOG_INFO("[Node " << m_netDevices.Get (i)->GetNode ()->GetId ()
  //               << "] assigned address " << m_ifaceIps.GetAddress (i, 0));
}

void
Scenario::ConfigureDrones ()
{
  NS_LOG_FUNCTION_NOARGS ();


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
      mobility.SetMobilityModel ("ns3::ParametricSpeedDroneMobilityModel",
                                 "SpeedCoefficients", SpeedCoefficientsValue(CONFIGURATOR->GetDroneSpeedCoefficients (i)),
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
        ("Ipv4Address", Ipv4AddressValue (m_ifaceIps.GetAddress (i)),
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
  Report::Get ()->Initialize (CONFIGURATOR->GetName (),
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
  //Report::Get ()->Save ();

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
