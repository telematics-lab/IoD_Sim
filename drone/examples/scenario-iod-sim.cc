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
// Standard Library
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <vector>
// NS3 Core Components
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
// IoD Sim Report Module
#include <ns3/report.h>
// IoD Sim Components
#include <ns3/drone-client.h>
#include <ns3/drone-list.h>
#include <ns3/drone-server.h>
#include <ns3/flight-plan.h>
#include <ns3/ipv4-simulation-helper.h>
#include <ns3/mobility-factory-helper.h>
#include <ns3/scenario-configuration-helper.h>
#include <ns3/speed-coefficients.h>
#include <ns3/proto-point.h>
#include <ns3/wifi-mac-factory-helper.h>
#include <ns3/wifi-mac-layer-configuration.h>
#include <ns3/wifi-mac-simulation-helper.h>
#include <ns3/wifi-phy-factory-helper.h>
#include <ns3/wifi-phy-layer-configuration.h>
#include <ns3/wifi-phy-simulation-helper.h>
#include <ns3/ipv4-network-layer-configuration.h>
#include <ns3/zsp-list.h>

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

  void ConfigureEntities (const std::string& entityKey, NodeContainer& nodes);
  void ConfigureEntityApplications (const std::string& entityKey,
                                    const uint32_t& entityId,
                                    const uint32_t& deviceId,
                                    const Ipv4Address& deviceAddress,
                                    const Ptr<EntityConfiguration>& conf);

  void ConfigureMobility ();
  void ConfigureMobilityDrones ();
  void ConfigureMobilityZsps ();
  void ConfigureApplication ();
  void ConfigureApplicationDrones ();
  void ConfigureApplicationZsps ();
  void ConfigureSimulator ();

  NodeContainer m_drones;
  NodeContainer m_zsps;

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
  ConfigureEntities ("drones", m_drones);
  ConfigureEntities ("ZSPs", m_zsps);
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

  size_t phyId = 0;
  for (auto& phyLayerConf : phyLayerConfs) {
    if (phyLayerConf->GetType ().compare("wifi") == 0) {
      YansWifiChannelHelper wifiChannel;
      const auto wifiConf = StaticCast<WifiPhyLayerConfiguration, PhyLayerConfiguration> (phyLayerConf);
      const auto wifiSim = CreateObject<WifiPhySimulationHelper> ();
      YansWifiPhyHelper* wifiPhy = wifiSim->GetWifiPhyHelper ();

      wifiSim->GetWifiHelper ()->SetStandard (wifiConf->GetStandard ());

      // This is one parameter that matters when using FixedRssLossModel
      // set it to zero; otherwise, gain will be added
      wifiPhy->Set ("RxGain", DoubleValue (wifiConf->GetRxGain ()));
      // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
      wifiPhy->SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

      WifiPhyFactoryHelper::SetPropagationDelay (wifiChannel, wifiConf->GetChannelPropagationDelayModel ());
      WifiPhyFactoryHelper::AddPropagationLoss (wifiChannel, wifiConf->GetChannelPropagationLossModel ());

      wifiPhy->SetChannel (wifiChannel.Create ());

      // Configure Results Export
      std::stringstream phyTraceLog,
                        pcapLog;
      AsciiTraceHelper  ascii;

      phyTraceLog << CONFIGURATOR->GetResultsPath () << "-phy-" << phyId << ".log";
      pcapLog << CONFIGURATOR->GetResultsPath () << "-phy- " << phyId << " -host";

      // Enable Tracing
      wifiPhy->EnableAsciiAll (ascii.CreateFileStream (phyTraceLog.str ()));
      //wifiPhy->EnablePcap (pcapLog.str (), m_netDevices);

      m_protocolStacks[PHY_LAYER].push_back (wifiSim);
    } else {
      NS_FATAL_ERROR ("Unsupported PHY Layer Type: " << phyLayerConf->GetType ());
    }

    phyId++;
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

      WifiMacFactoryHelper::SetRemoteStationManager (*(wifiPhy->GetWifiHelper ()),
                                                     wifiConf->GetRemoteStationManagerConfiguration ());

      m_protocolStacks[MAC_LAYER].push_back (wifiMac);
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
      const auto ipv4Sim = CreateObject<Ipv4SimulationHelper> (ipv4Conf->GetMask ());

      ipv4Sim->GetIpv4Helper ().SetBase (Ipv4Address (ipv4Conf->GetAddress ().c_str ()),
                                         Ipv4Mask (ipv4Conf->GetMask ().c_str ()));

      m_protocolStacks[NET_LAYER].push_back (ipv4Sim);
    } else {
      NS_FATAL_ERROR ("Unsupported Network Layer Type: " << layerConf->GetType ());
    }
  }
}

void
Scenario::ConfigureEntities (const std::string& entityKey, NodeContainer& nodes)
{
  NS_LOG_FUNCTION_NOARGS ();

  size_t entityId = 0;
  const auto entityConfs = CONFIGURATOR->GetEntitiesConfiguration (entityKey);

  for (auto& entityConf : entityConfs) {
    size_t deviceId = 0;

    for (auto& entityNetDev : entityConf->GetNetDevices ()) {
      if (entityNetDev->GetType ().compare("wifi") == 0) {
        auto wifiPhy = StaticCast<WifiPhySimulationHelper, Object> (m_protocolStacks[PHY_LAYER][deviceId]);
        auto wifiMac = StaticCast<WifiMacSimulationHelper, Object> (m_protocolStacks[MAC_LAYER][deviceId]);
        auto netLayer = StaticCast<Ipv4SimulationHelper, Object> (m_protocolStacks[NET_LAYER][deviceId]);

        // TODO: helper?
        wifiMac->GetMacHelper ().SetType (entityNetDev->GetMacLayer ().GetName (),
                                          entityNetDev->GetMacLayer ().GetAttributes ()[0].first,
                                          *entityNetDev->GetMacLayer ().GetAttributes ()[0].second);

        NetDeviceContainer devContainer = wifiPhy->GetWifiHelper ()->Install (*wifiPhy->GetWifiPhyHelper (),
                                                                              wifiMac->GetMacHelper (),
                                                                              nodes.Get (entityId));

        //const auto networkLayerId = entityNetDev->GetNetworkLayerId ();
        netLayer->GetInternetHelper ().Install (nodes.Get(entityId));
        auto assignedIPs = netLayer->GetIpv4Helper ().Assign (devContainer);

        // Configure Entity Mobility
        const auto mobilityType = entityConf->GetMobilityModel ().GetName ();
        MobilityHelper mobility;
        MobilityFactoryHelper::SetMobilityModel (mobility, entityConf->GetMobilityModel ());
        if (entityKey.compare ("drones") == 0)
          mobility.Install (m_drones.Get (entityId));
        else
          mobility.Install (m_zsps.Get (entityId));

        ConfigureEntityApplications (entityKey, entityId, deviceId, assignedIPs.GetAddress (deviceId, 0), entityConf);

      } else {
        NS_FATAL_ERROR ("Unsupported Drone Network Device Type: " << entityNetDev->GetType ());
      }

      deviceId++;
    }

    entityId++;
  }
}

void
Scenario::ConfigureEntityApplications (const std::string& entityKey,
                                       const uint32_t& entityId,
                                       const uint32_t& deviceId,
                                       const Ipv4Address& deviceAddress,
                                       const Ptr<EntityConfiguration>& conf)
{
  NS_LOG_FUNCTION_NOARGS ();

  for (auto& appConf : conf->GetApplications ()) {
    const auto appType = appConf->GetType ();
    if (appType.compare("ns3::DroneClient") == 0) {
      auto ipv4Conf = StaticCast<Ipv4SimulationHelper, Object> (m_protocolStacks[NET_LAYER][deviceId]);
      const auto ipMask = ipv4Conf->GetMask ();
      auto app = CreateObjectWithAttributes<DroneClient>("Ipv4Address", Ipv4AddressValue (deviceAddress),
                                                         "Ipv4SubnetMask", Ipv4MaskValue (ipMask.c_str ()),
                                                         "Duration", DoubleValue (CONFIGURATOR->GetDuration ()));

      if (entityKey.compare("drones") == 0)
        m_drones.Get (entityId)->AddApplication (app);
      else if (entityKey.compare("ZSPs") == 0)
        m_zsps.Get (entityId)->AddApplication (app);

      app->SetStartTime (Seconds (appConf->GetStartTime ()));
      app->SetStopTime (Seconds (appConf->GetStopTime ()));

    } else if (appType.compare("ns3::DroneServer") == 0) {
      auto ipv4Conf = StaticCast<Ipv4SimulationHelper, Object> (m_protocolStacks[NET_LAYER][deviceId]);
      const auto ipMask = ipv4Conf->GetMask ();
      auto app = CreateObjectWithAttributes<DroneServer>("Ipv4Address", Ipv4AddressValue (deviceAddress),
                                                         "Ipv4SubnetMask", Ipv4MaskValue (ipMask.c_str ()));

      if (entityKey.compare("drones") == 0)
        m_drones.Get (entityId)->AddApplication (app);
      else if (entityKey.compare("ZSPs") == 0)
        m_zsps.Get (entityId)->AddApplication (app);

      app->SetStartTime (Seconds (appConf->GetStartTime ()));
      app->SetStopTime (Seconds (appConf->GetStopTime ()));

    } else {
      NS_FATAL_ERROR ("Unsupported Application Type: " << appType);
    }
  }
}

void
Scenario::ConfigureSimulator ()
{
  NS_LOG_FUNCTION_NOARGS ();

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
