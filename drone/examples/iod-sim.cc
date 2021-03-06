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
// Standard Library
#include <vector>
// NS3 Core Components
#include <ns3/config.h>
#include <ns3/log.h>
#include <ns3/mobility-helper.h>
#include <ns3/object-factory.h>
#include <ns3/net-device-container.h>
#include <ns3/ptr.h>
#include <ns3/show-progress.h>
#include <ns3/ssid.h>
#include <ns3/string.h>
// IoD Sim Report Module
#include <ns3/report.h>
// IoD Sim Components
#include <ns3/drone-client.h>
#include <ns3/drone-list.h>
#include <ns3/drone-server.h>
#include <ns3/ipv4-network-layer-configuration.h>
#include <ns3/ipv4-simulation-helper.h>
#include <ns3/mobility-factory-helper.h>
#include <ns3/scenario-configuration-helper.h>
#include <ns3/wifi-mac-factory-helper.h>
#include <ns3/wifi-mac-layer-configuration.h>
#include <ns3/wifi-mac-simulation-helper.h>
#include <ns3/wifi-phy-factory-helper.h>
#include <ns3/wifi-phy-layer-configuration.h>
#include <ns3/wifi-phy-simulation-helper.h>
#include <ns3/zsp-list.h>

namespace ns3 {

constexpr int N_LAYERS = 4;
constexpr int PHY_LAYER = 0;
constexpr int MAC_LAYER = 1;
constexpr int NET_LAYER = 2;
constexpr int APP_LAYER = 3;
constexpr int PROGRESS_REFRESH_INTERVAL_SECONDS = 1;

class Scenario
{
public:
  Scenario (int argc, char **argv);
  virtual ~Scenario ();

  void operator() ();

private:
  void ApplyStaticConfig ();
  void ConfigurePhy ();
  void ConfigureMac ();
  void ConfigureNetwork ();
  void ConfigureEntities (const std::string& entityKey, NodeContainer& nodes);
  void ConfigureEntityMobility (const std::string& entityKey,
                                Ptr<EntityConfiguration> entityConf,
                                const uint32_t entityId);
  NetDeviceContainer ConfigureEntityWifiStack (Ptr<NetdeviceConfiguration> entityNetDev,
                                               Ptr<Node> entityNode,
                                               const uint32_t entityId,
                                               const uint32_t deviceId,
                                               const uint32_t netId);
  Ipv4Address ConfigureEntityIpv4Network (Ptr<Node> entityNode,
                                          NetDeviceContainer devContainer,
                                          const uint32_t deviceId,
                                          const uint32_t netId);
  void ConfigureEntityApplications (const std::string& entityKey,
                                    const uint32_t& entityId,
                                    const uint32_t& deviceId,
                                    const uint32_t& netId,
                                    const Ipv4Address& deviceAddress,
                                    const Ptr<EntityConfiguration>& conf);
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
  ConfigureSimulator ();
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
  NS_LOG_FUNCTION (entityKey);

  const auto entityConfs = CONFIGURATOR->GetEntitiesConfiguration (entityKey);
  size_t entityId = 0;

  for (auto& entityConf : entityConfs) {
    size_t deviceId = 0;

    for (auto& entityNetDev : entityConf->GetNetDevices ()) {
      if (entityNetDev->GetType ().compare("wifi") == 0) {
        const auto netId = entityNetDev->GetNetworkLayerId ();
        auto entityNode = nodes.Get (entityId);
        auto devContainer = ConfigureEntityWifiStack (entityNetDev, entityNode, entityId, deviceId, netId);
        auto assignedIpv4 = ConfigureEntityIpv4Network (entityNode, devContainer, deviceId, netId);
        ConfigureEntityMobility (entityKey, entityConf, entityId);
        ConfigureEntityApplications (entityKey, entityId, deviceId, netId, assignedIpv4, entityConf);

      } else {
        NS_FATAL_ERROR ("Unsupported Drone Network Device Type: " << entityNetDev->GetType ());
      }

      deviceId++;
    }

    entityId++;
  }
}

NetDeviceContainer
Scenario::ConfigureEntityWifiStack (Ptr<NetdeviceConfiguration> entityNetDev,
                                    Ptr<Node> entityNode,
                                    const uint32_t entityId,
                                    const uint32_t deviceId,
                                    const uint32_t netId)
{
  NS_LOG_FUNCTION (entityNetDev << entityNode << entityId << deviceId << netId);

  auto wifiPhy = StaticCast<WifiPhySimulationHelper, Object> (m_protocolStacks[PHY_LAYER][netId]);
  auto wifiMac = StaticCast<WifiMacSimulationHelper, Object> (m_protocolStacks[MAC_LAYER][netId]);

  wifiMac->GetMacHelper ().SetType (entityNetDev->GetMacLayer ().GetName (),
                                    entityNetDev->GetMacLayer ().GetAttributes ()[0].first,
                                    *entityNetDev->GetMacLayer ().GetAttributes ()[0].second);

  NetDeviceContainer devContainer = wifiPhy->GetWifiHelper ()->Install (*wifiPhy->GetWifiPhyHelper (),
                                                                        wifiMac->GetMacHelper (),
                                                                        entityNode);

  // Configure WiFi PHY Logging
  std::stringstream phyTraceLog;
  std::stringstream pcapLog;
  AsciiTraceHelper  ascii;

  // Configure WiFi TXT PHY Logging
  phyTraceLog << CONFIGURATOR->GetResultsPath () << "-phy-" << netId << "-host-" << entityId << "-" << deviceId << ".log";
  wifiPhy->GetWifiPhyHelper ()->EnableAscii (ascii.CreateFileStream (phyTraceLog.str ()), entityId, deviceId);

  // Configure WiFi PCAP Logging
  pcapLog << CONFIGURATOR->GetResultsPath () << "-phy-" << netId << "-host";
  wifiPhy->GetWifiPhyHelper ()->EnablePcap (pcapLog.str (), entityId, deviceId);

  return devContainer;
}

Ipv4Address
Scenario::ConfigureEntityIpv4Network (Ptr<Node> entityNode,
                                      NetDeviceContainer devContainer,
                                      const uint32_t deviceId,
                                      const uint32_t netId)
{
  NS_LOG_FUNCTION (entityNode << deviceId << netId);

  auto netLayer = StaticCast<Ipv4SimulationHelper, Object> (m_protocolStacks[NET_LAYER][netId]);

  netLayer->GetInternetHelper ().Install (entityNode);
  auto assignedIPs = netLayer->GetIpv4Helper ().Assign (devContainer);

  return assignedIPs.GetAddress (deviceId, 0);
}

void
Scenario::ConfigureEntityMobility (const std::string& entityKey,
                                   Ptr<EntityConfiguration> entityConf,
                                   const uint32_t entityId)
{
  NS_LOG_FUNCTION (entityKey << entityConf << entityId);

  MobilityHelper mobility;
  const auto mobilityType = entityConf->GetMobilityModel ().GetName (); // Configure Entity Mobility
  MobilityFactoryHelper::SetMobilityModel (mobility, entityConf->GetMobilityModel ());

  if (entityKey.compare ("drones") == 0)
    mobility.Install (m_drones.Get (entityId));
  else if (entityKey.compare ("ZSPs") == 0)
    mobility.Install (m_zsps.Get (entityId));
}

void
Scenario::ConfigureEntityApplications (const std::string& entityKey,
                                       const uint32_t& entityId,
                                       const uint32_t& deviceId,
                                       const uint32_t& netId,
                                       const Ipv4Address& deviceAddress,
                                       const Ptr<EntityConfiguration>& conf)
{
  NS_LOG_FUNCTION (entityKey << entityId << deviceId << netId << deviceAddress << conf);

  for (auto& appConf : conf->GetApplications ()) {
    const auto appType = appConf->GetType ();
    if (appType.compare("ns3::DroneClient") == 0) {
      auto ipv4Conf = StaticCast<Ipv4SimulationHelper, Object> (m_protocolStacks[NET_LAYER][netId]);
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
      auto ipv4Conf = StaticCast<Ipv4SimulationHelper, Object> (m_protocolStacks[NET_LAYER][netId]);
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

  ShowProgress progress {Seconds (PROGRESS_REFRESH_INTERVAL_SECONDS), std::cout};

  Simulator::Run ();
  // Report Module needs the simulator context alive to introspect it
  Report::Get ()->Save ();
  Simulator::Destroy ();
}

} // namespace ns3

int
main (int argc, char **argv)
{
  ns3::Scenario s (argc, argv);
  s ();  // run the scenario as soon as it is ready

  return 0;
}
