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
#include <algorithm>

// NS3 Core Components
#include <ns3/buildings-helper.h>
#include <ns3/config.h>
#include <ns3/csma-module.h>
#include <ns3/internet-module.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/ipv4-routing-helper.h>
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
#include <ns3/drone-client-application.h>
#include <ns3/drone-container.h>
#include <ns3/drone-energy-model-helper.h>
#include <ns3/drone-list.h>
#include <ns3/drone-peripheral.h>
#include <ns3/drone-peripheral-container.h>
#include <ns3/drone-server-application.h>
#include <ns3/energy-source.h>
#include <ns3/input-peripheral.h>
#include <ns3/ipv4-network-layer-configuration.h>
#include <ns3/ipv4-simulation-helper.h>
#include <ns3/lte-netdevice-configuration.h>
#include <ns3/lte-phy-layer-configuration.h>
#include <ns3/lte-phy-simulation-helper.h>
#include <ns3/lte-setup-helper.h>
#include <ns3/mobility-factory-helper.h>
#include <ns3/nat-application.h>
#include <ns3/scenario-configuration-helper.h>
#include <ns3/wifi-netdevice-configuration.h>
#include <ns3/wifi-mac-factory-helper.h>
#include <ns3/wifi-mac-layer-configuration.h>
#include <ns3/wifi-mac-simulation-helper.h>
#include <ns3/wifi-phy-factory-helper.h>
#include <ns3/wifi-phy-layer-configuration.h>
#include <ns3/wifi-phy-simulation-helper.h>
#include <ns3/zsp-list.h>
#include <ns3/remote-list.h>
#include <ns3/node-list.h>
#include <ns3/traced-value.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/interest-region-container.h>
#include <ns3/debug-helper.h>

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
  void ConfigureWorld ();
  void ConfigurePhy ();
  void ConfigureMac ();
  void ConfigureNetwork ();
  void EnableIpv4RoutingTableReporting ();
  void ConfigureEntities (const std::string& entityKey, NodeContainer& nodes);
  void ConfigureEntityMobility (const std::string& entityKey,
                                Ptr<EntityConfiguration> entityConf,
                                const uint32_t entityId);
  NetDeviceContainer ConfigureEntityWifiStack (std::string entityKey,
                                               Ptr<NetdeviceConfiguration> entityNetDev,
                                               Ptr<Node> entityNode,
                                               const uint32_t entityId,
                                               const uint32_t deviceId,
                                               const uint32_t netId);
  void ConfigureLteEnb (Ptr<Node> entityNode, const uint32_t netId);
  void ConfigureLteUe (Ptr<Node> entityNode, const std::vector<LteBearerConfiguration> bearers, const uint32_t netId);
  void InstallEntityIpv4 (Ptr<Node> entityNode, NetDeviceContainer netDevices, const uint32_t netId);
  void InstallEntityIpv4 (Ptr<Node> entityNode, Ptr<NetDevice> netDevice, const uint32_t netId);
  void ConfigureEntityIpv4 (Ptr<Node> entityNode,
                            NetDeviceContainer devContainer,
                            const uint32_t deviceId,
                            const uint32_t netId);
  void ConfigureEntityApplications (const std::string& entityKey,
                                    const Ptr<EntityConfiguration>& conf,
                                    const uint32_t& entityId);
  void ConfigureEntityMechanics (const std::string& entityKey,
                                 Ptr<EntityConfiguration> entityConf,
                                 const uint32_t entityId);
  Ptr<EnergySource> ConfigureEntityBattery (const std::string& entityKey,
                                            Ptr<EntityConfiguration> entityConf,
                                            const uint32_t entityId);
  void ConfigureEntityPeripherals (const std::string& entityKey,
                                   const Ptr<EntityConfiguration>& conf,
                                   const uint32_t& entityId);
  void ConfigureInternetRemotes ();
  void ConfigureInternetBackbone ();
  void EnablePhyLteTraces ();
  void ConfigureRegionsOfInterest ();
  void CourseChange (std::string context, Ptr<const MobilityModel> model);
  void ConfigureSimulator ();

  DroneContainer m_drones;
  NodeContainer m_zsps;
  NodeContainer m_remoteNodes;
  NodeContainer m_backbone;

  std::array<std::vector<Ptr<Object>>, N_LAYERS> m_protocolStacks;
};

NS_LOG_COMPONENT_DEFINE ("Scenario");

Scenario::Scenario (int argc, char **argv)
{
  CONFIGURATOR->Initialize (argc, argv);
  m_drones.Create (CONFIGURATOR->GetDronesN ());
  m_zsps.Create (CONFIGURATOR->GetZspsN ());
  m_remoteNodes.Create(CONFIGURATOR->GetRemotesN ());
  m_backbone.Add (m_remoteNodes);

  // Register created entities in their lists
  for (auto drone = m_drones.Begin (); drone != m_drones.End (); drone++)
    DroneList::Add (*drone);

  for (auto zsp = m_zsps.Begin (); zsp != m_zsps.End (); zsp++)
    ZspList::Add (*zsp);

  for (auto remote = m_remoteNodes.Begin (); remote != m_remoteNodes.End (); remote++)
    RemoteList::Add (*remote);

  ApplyStaticConfig ();
  ConfigureWorld ();
  ConfigurePhy ();
  ConfigureMac ();
  ConfigureNetwork ();
  ConfigureRegionsOfInterest();
  ConfigureEntities ("drones", m_drones);
  ConfigureEntities ("ZSPs", m_zsps);
  ConfigureInternetBackbone ();
  ConfigureInternetRemotes ();
  EnablePhyLteTraces ();

  //DebugHelper::ProbeNodes();
  ConfigureSimulator ();
}

Scenario::~Scenario ()
{
}

void
Scenario::operator() ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (CONFIGURATOR->IsDryRun ())
    return;

  std::stringstream progressLogFilePath;
  progressLogFilePath << CONFIGURATOR->GetResultsPath () << "progress.log";
  auto progressLogSink = Create<OutputStreamWrapper> (progressLogFilePath.str (), std::ios::out);

  ShowProgress progressLog {Seconds (PROGRESS_REFRESH_INTERVAL_SECONDS), (*progressLogSink->GetStream ())};
  ShowProgress progressStdout {Seconds (PROGRESS_REFRESH_INTERVAL_SECONDS), std::cout};

  Simulator::Run ();
  // Report Module needs the simulator context alive to introspect it
  Report::Get ()->Save ();
  Simulator::Destroy ();
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
Scenario::ConfigureWorld ()
{
  NS_LOG_FUNCTION_NOARGS ();

  CONFIGURATOR->GetBuildings (); // buildings created here are automatically added to BuildingsList
}

void
Scenario::ConfigurePhy ()
{
  NS_LOG_FUNCTION_NOARGS ();

  const auto phyLayerConfs = CONFIGURATOR->GetPhyLayers ();

  size_t phyId = 0;
  for (auto& phyLayerConf : phyLayerConfs)
    {
      if (phyLayerConf->GetType ().compare("wifi") == 0)
        {
          YansWifiChannelHelper wifiChannel;
          const auto wifiConf = StaticCast<WifiPhyLayerConfiguration, PhyLayerConfiguration> (phyLayerConf);
          const auto wifiSim = CreateObject<WifiPhySimulationHelper> ();
          YansWifiPhyHelper* wifiPhy = wifiSim->GetWifiPhyHelper ();

          wifiSim->GetWifiHelper ()->SetStandard (wifiConf->GetStandard ());

          wifiPhy->Set ("RxGain", DoubleValue (wifiConf->GetRxGain ()));
          // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
          wifiPhy->SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

          WifiPhyFactoryHelper::SetPropagationDelay (wifiChannel, wifiConf->GetChannelPropagationDelayModel ());
          WifiPhyFactoryHelper::AddPropagationLoss (wifiChannel, wifiConf->GetChannelPropagationLossModel ());

          wifiPhy->SetChannel (wifiChannel.Create ());

          m_protocolStacks[PHY_LAYER].push_back (wifiSim);
        }
      else if (phyLayerConf->GetType ().compare("lte") == 0)
        {
          auto lteSim = CreateObject<LtePhySimulationHelper> (phyId);
          auto lteConf = StaticCast<LtePhyLayerConfiguration, PhyLayerConfiguration> (phyLayerConf);
          auto lteHelper = lteSim->GetLteHelper();

          lteHelper->Initialize ();

          auto pathlossConf = lteConf->GetChannelPropagationLossModel ();
          lteHelper->SetAttribute ("PathlossModel", StringValue (pathlossConf.GetName ()));
          for (auto& attr : pathlossConf.GetAttributes ())
            lteHelper->SetPathlossModelAttribute (attr.first, *(attr.second));

          auto spectrumConf = lteConf->GetChannelSpectrumModel ();
          lteHelper->SetSpectrumChannelType (spectrumConf.GetName ());
          for (auto& attr : spectrumConf.GetAttributes ())
            lteHelper->SetSpectrumChannelAttribute (attr.first, *(attr.second));

          m_backbone.Add (lteSim->GetEpcHelper ()->GetPgwNode ());

          m_protocolStacks[PHY_LAYER].push_back (lteSim);
        }
      else
        {
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
  for (auto& macLayerConf : macLayerConfs)
    {
      if (macLayerConf->GetType ().compare ("wifi") == 0)
        {
          const auto wifiPhy = StaticCast<WifiPhySimulationHelper, Object> (m_protocolStacks[PHY_LAYER][i]);
          const auto wifiMac = CreateObject<WifiMacSimulationHelper> ();
          const auto wifiConf = StaticCast<WifiMacLayerConfiguration, MacLayerConfiguration> (macLayerConf);
          Ssid ssid = Ssid (wifiConf->GetSsid ());

          WifiMacFactoryHelper::SetRemoteStationManager (*(wifiPhy->GetWifiHelper ()),
                                                         wifiConf->GetRemoteStationManagerConfiguration ());

          m_protocolStacks[MAC_LAYER].push_back (wifiMac);
        }
      else if (macLayerConf->GetType ().compare ("lte") == 0)
        {
          // NO OPERATION NEEDED HERE
          m_protocolStacks[MAC_LAYER].push_back (nullptr);
        }
      else
        {
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
  for (auto& layerConf : layerConfs)
    {
      if (layerConf->GetType ().compare("ipv4") == 0)
        {
          const auto ipv4Conf = StaticCast<Ipv4NetworkLayerConfiguration, NetworkLayerConfiguration> (layerConf);
          const auto ipv4Sim = CreateObject<Ipv4SimulationHelper> (ipv4Conf->GetMask (), ipv4Conf->GetGatewayAddress ());

          ipv4Sim->GetIpv4Helper ().SetBase (Ipv4Address (ipv4Conf->GetAddress ().c_str ()),
                                             Ipv4Mask (ipv4Conf->GetMask ().c_str ()));

          EnableIpv4RoutingTableReporting ();

          m_protocolStacks[NET_LAYER].push_back (ipv4Sim);
        }
      else
        {
          NS_FATAL_ERROR ("Unsupported Network Layer Type: " << layerConf->GetType ());
        }
    }
}

void
Scenario::EnableIpv4RoutingTableReporting ()
{
  // this method should be called once
  static bool hasBeenCalled = false;
  if (hasBeenCalled)
    return;
  else
    hasBeenCalled = true;

  NS_LOG_FUNCTION_NOARGS ();

  std::stringstream routingTableFilePath;
  routingTableFilePath << CONFIGURATOR->GetResultsPath () << "ipv4-routing-tables.txt";
  auto routingTableSink = Create<OutputStreamWrapper> (routingTableFilePath.str (), std::ios::out);
  Ipv4RoutingHelper::PrintRoutingTableAllAt (Seconds (1.0), routingTableSink);
}

void
Scenario::ConfigureEntities (const std::string& entityKey, NodeContainer& nodes)
{
  NS_LOG_FUNCTION (entityKey);

  const auto entityConfs = CONFIGURATOR->GetEntitiesConfiguration (entityKey);
  size_t entityId = 0;

  for (auto& entityConf : entityConfs) {
    size_t deviceId = 0;

    auto entityNode = nodes.Get (entityId);
    ConfigureEntityMobility (entityKey, entityConf, entityId);

    for (auto& entityNetDev : entityConf->GetNetDevices ())
      {
        const auto netId = entityNetDev->GetNetworkLayerId ();

        if (entityNetDev->GetType ().compare("wifi") == 0)
          {
            auto devContainer = ConfigureEntityWifiStack (entityKey, entityNetDev, entityNode, entityId, deviceId, netId);
            InstallEntityIpv4 (entityNode, devContainer, netId);
            ConfigureEntityIpv4 (entityNode, devContainer, deviceId, netId);
          }
        else if (entityNetDev->GetType ().compare("lte") == 0)
          {
            const auto entityLteDevConf = StaticCast<LteNetdeviceConfiguration, NetdeviceConfiguration> (entityNetDev);
            const auto role = entityLteDevConf->GetRole ();

            switch (role)
              {
              case eNB:
                ConfigureLteEnb (entityNode, netId);
                break;
              case UE:
                ConfigureLteUe (entityNode, entityLteDevConf->GetBearers (), netId);
                break;
              default:
                NS_FATAL_ERROR ("Unrecognized LTE role for entity ID " << entityId);
              }
          }
        else
          {
            NS_FATAL_ERROR ("Unsupported Drone Network Device Type: " << entityNetDev->GetType ());
          }


        deviceId++;
      }

      ConfigureEntityApplications (entityKey, entityConf, entityId);

      if (entityKey.compare("drones") == 0)
        {
          DroneEnergyModelHelper energyModel;
          Ptr<EnergySource> energySource;

          ConfigureEntityMechanics (entityKey, entityConf, entityId);
          energySource = ConfigureEntityBattery (entityKey, entityConf, entityId);
          /// Installing Energy Model on Drone
          energyModel.Install (StaticCast<Drone, Node> (entityNode), energySource);
          ConfigureEntityPeripherals (entityKey, entityConf, entityId);
        }

    entityId++;
  }

  BuildingsHelper::Install (nodes);
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
  {
    mobility.Install (m_drones.Get (entityId));
    std::ostringstream oss;
    oss <<
      "/NodeList/" << m_drones.Get (entityId)->GetId () <<
      "/$ns3::MobilityModel/CourseChange";
    Config::Connect (oss.str (), MakeCallback (&Scenario::CourseChange,this));
  }
  else if (entityKey.compare ("ZSPs") == 0)
    mobility.Install (m_zsps.Get (entityId));
}

NetDeviceContainer
Scenario::ConfigureEntityWifiStack (const std::string entityKey,
                                    Ptr<NetdeviceConfiguration> entityNetDev,
                                    Ptr<Node> entityNode,
                                    const uint32_t entityId,
                                    const uint32_t deviceId,
                                    const uint32_t netId)
{
  NS_LOG_FUNCTION (entityNetDev << entityNode << entityId << deviceId << netId);

  auto wifiPhy = StaticCast<WifiPhySimulationHelper, Object> (m_protocolStacks[PHY_LAYER][netId]);
  auto wifiMac = StaticCast<WifiMacSimulationHelper, Object> (m_protocolStacks[MAC_LAYER][netId]);
  auto wifiNetDev = StaticCast<WifiNetdeviceConfiguration, NetdeviceConfiguration> (entityNetDev);
  auto wifiMacAttrs = wifiNetDev->GetMacLayer ().GetAttributes ();

  if (wifiMacAttrs.size () == 0)
    wifiMac->GetMacHelper ().SetType (wifiNetDev->GetMacLayer ().GetName ());
  else if (wifiMacAttrs.size () == 1)
    wifiMac->GetMacHelper ().SetType (wifiNetDev->GetMacLayer ().GetName (),
                                      wifiMacAttrs[0].first,
                                      *wifiMacAttrs[0].second);

  NetDeviceContainer devContainer = wifiPhy->GetWifiHelper ()->Install (*wifiPhy->GetWifiPhyHelper (),
                                                                        wifiMac->GetMacHelper (),
                                                                        entityNode);

  // Configure WiFi PHY Logging
  std::stringstream phyTraceLog;
  std::stringstream pcapLog;
  AsciiTraceHelper  ascii;

  // Configure WiFi TXT PHY Logging
  phyTraceLog << CONFIGURATOR->GetResultsPath ()
              << "wifi-phy-" << netId
              << "-" << entityKey
              << "-host-" << entityId
              << "-" << deviceId
              << ".log";
  wifiPhy->GetWifiPhyHelper ()->EnableAscii (ascii.CreateFileStream (phyTraceLog.str ()),
                                             entityNode->GetId(),
                                             devContainer.Get(0)->GetIfIndex());

  // Configure WiFi PCAP Logging
  pcapLog << CONFIGURATOR->GetResultsPath () << "wifi-phy-" << netId << "-" << entityKey <<  "-host";
  wifiPhy->GetWifiPhyHelper ()->EnablePcap (pcapLog.str (), entityNode->GetId(), devContainer.Get(0)->GetIfIndex());

  return devContainer;
}

void
Scenario::ConfigureLteEnb (Ptr<Node> entityNode, const uint32_t netId)
{
  // NOTICE: no checks are made for backbone/netid combination that do not represent an LTE backbone!
  static std::vector<NodeContainer> backbonePerStack (m_protocolStacks[PHY_LAYER].size ());
  auto ltePhy = StaticCast<LtePhySimulationHelper, Object> (m_protocolStacks[PHY_LAYER][netId]);

  LteSetupHelper::InstallSingleEnbDevice(ltePhy->GetLteHelper (), entityNode);
  for (NodeContainer::Iterator eNB = backbonePerStack[netId].Begin (); eNB != backbonePerStack[netId].End (); eNB++)
    ltePhy->GetLteHelper ()->AddX2Interface (entityNode, *eNB);
  backbonePerStack[netId].Add (entityNode);
}

void
Scenario::ConfigureLteUe (Ptr<Node> entityNode, const std::vector<LteBearerConfiguration> bearers, const uint32_t netId)
{
  // NOTICE: no checks are made for ue/netid combination that do not represent an LTE backbone!
  static std::vector<NodeContainer> uePerStack (m_protocolStacks[PHY_LAYER].size ());
  auto ltePhy = StaticCast<LtePhySimulationHelper, Object> (m_protocolStacks[PHY_LAYER][netId]);
  Ipv4StaticRoutingHelper routingHelper;

  auto dev = LteSetupHelper::InstallSingleUeDevice (ltePhy->GetLteHelper (), entityNode);

  // Install network layer in order to proceed with IPv4 LTE configuration
  InstallEntityIpv4 (entityNode, dev, netId);
  // Register UEs into network 7.0.0.0/8
  // unfortunately this is hardwired into EpcHelper implementation
  ltePhy->GetEpcHelper ()->AssignUeIpv4Address (NetDeviceContainer (dev));
  // create a static route for each UE to the SGW/PGW in order to communicate
  // with the internet
  auto nodeIpv4 = entityNode->GetObject<Ipv4> ();
  Ptr<Ipv4StaticRouting> ueStaticRoute = routingHelper.GetStaticRouting (nodeIpv4);
  ueStaticRoute->SetDefaultRoute (ltePhy->GetEpcHelper ()->GetUeDefaultGatewayAddress (),
                                  nodeIpv4->GetInterfaceForDevice(dev));
  // auto attach each drone UE to the eNB with the strongest signal
  ltePhy->GetLteHelper ()->Attach (dev);
  // init bearers on UE
  for (auto& bearerConf : bearers)
    {
      EpsBearer bearer (bearerConf.GetType (), bearerConf.GetQos ());
      ltePhy->GetLteHelper ()->ActivateDedicatedEpsBearer (dev, bearer, EpcTft::Default ());
    }
}

void
Scenario::InstallEntityIpv4 (Ptr<Node> entityNode,
                             NetDeviceContainer netDevices,
                             const uint32_t netId)
{
  NS_LOG_FUNCTION (entityNode << netId);

  for (NetDeviceContainer::Iterator i = netDevices.Begin (); i != netDevices.End (); i++)
    InstallEntityIpv4 (entityNode, *i, netId);
}

void
Scenario::InstallEntityIpv4 (Ptr<Node> entityNode,
                             Ptr<NetDevice> netDevice,
                             const uint32_t netId)
{
  NS_LOG_FUNCTION (entityNode << netId);

  auto ipv4Obj = entityNode->GetObject<Ipv4> ();

  if (ipv4Obj == nullptr)
    {
      auto netLayer = StaticCast<Ipv4SimulationHelper, Object> (m_protocolStacks[NET_LAYER][netId]);
      netLayer->GetInternetHelper ().Install (entityNode);
    }
  else
    {
      ipv4Obj->AddInterface (netDevice);
    }
}

void
Scenario::ConfigureEntityIpv4 (Ptr<Node> entityNode,
                               NetDeviceContainer devContainer,
                               const uint32_t deviceId,
                               const uint32_t netId)
{
  NS_LOG_FUNCTION (entityNode << deviceId << netId);
  NS_ASSERT_MSG (1 == devContainer.GetN (),
                 "ConfigureEntityIpv4 assumes to receive a NetDeviceContainer with only one NetDevice, but there are "
                 << devContainer.GetN ());

  auto netLayer = StaticCast<Ipv4SimulationHelper, Object> (m_protocolStacks[NET_LAYER][netId]);
  auto assignedIPs = netLayer->GetIpv4Helper ().Assign (devContainer);

  Ipv4StaticRoutingHelper routingHelper;
  auto deviceIP = assignedIPs.Get(0);
  Ptr<Ipv4StaticRouting> ueStaticRoute = routingHelper.GetStaticRouting (deviceIP.first);

  const Ipv4Address nodeAddr = assignedIPs.GetAddress(0, 0);
  if (nodeAddr != netLayer->GetGatewayAddress ())
    ueStaticRoute->SetDefaultRoute (netLayer->GetGatewayAddress (), deviceIP.second);
}

void
Scenario::ConfigureEntityApplications (const std::string& entityKey,
                                       const Ptr<EntityConfiguration>& conf,
                                       const uint32_t& entityId)
{
  NS_LOG_FUNCTION (entityKey << conf << entityId);

  for (auto appConf : conf->GetApplications ())
    {
      ObjectFactory f {appConf.GetName ()};

      for (auto attr : appConf.GetAttributes ())
        f.Set (attr.first, *attr.second);

      auto app = StaticCast<Application, Object> (f.Create ());

      if (entityKey == "drones")
        m_drones.Get (entityId)->AddApplication (app);
      else if (entityKey == "ZSPs")
        m_zsps.Get (entityId)->AddApplication (app);
      else
        NS_FATAL_ERROR ("Unsupported Entity Type " << entityKey);
    }
}

void
Scenario::ConfigureEntityMechanics(const std::string& entityKey,
                                   Ptr<EntityConfiguration> entityConf,
                                   const uint32_t entityId)
{
  NS_LOG_FUNCTION_NOARGS();
  const auto mechanics = entityConf->GetMechanics();
  for (auto attr : mechanics.GetAttributes ())
      m_drones.Get(entityId)->SetAttribute (attr.first, *attr.second);
}

Ptr<EnergySource>
Scenario::ConfigureEntityBattery(const std::string& entityKey,
                                 Ptr<EntityConfiguration> entityConf,
                                 const uint32_t entityId)
{
  NS_LOG_FUNCTION_NOARGS();
  const auto battery = entityConf->GetBattery();
  ObjectFactory batteryFactory;
  batteryFactory.SetTypeId(entityConf->GetBattery().GetName());

  for (auto attr : battery.GetAttributes ())
      batteryFactory.Set(attr.first, *attr.second);
  auto mountedBattery = batteryFactory.Create<EnergySource>();

  mountedBattery->SetNode(m_drones.Get(entityId));
  m_drones.Get(entityId)->AggregateObject(mountedBattery);
  return mountedBattery;
}

void
Scenario::ConfigureEntityPeripherals (const std::string& entityKey,
                                      const Ptr<EntityConfiguration>& conf,
                                      const uint32_t& entityId)
{
  NS_LOG_FUNCTION (entityKey << entityId << conf);
  auto dronePeripheralsContainer = m_drones.Get(entityId)->getPeripherals();
  if (conf->GetPeripherals().size() == 0) return;
  for (auto perConf : conf->GetPeripherals ())
    {
      dronePeripheralsContainer->Add(perConf.GetName());
      for (auto attr : perConf.GetAttributes ())
          dronePeripheralsContainer->Set(attr.first, *attr.second);
      auto peripheral = dronePeripheralsContainer->Create();
      for (uint32_t i = 0; i < (uint32_t) peripheral->GetNRoI (); i++)
      {
        auto reg = irc->GetRoI (i);
        if (!irc->GetRoI (i))
          NS_FATAL_ERROR("Region of Interest #" << i << " does not exist.");
      }
    }
  dronePeripheralsContainer->InstallAll(m_drones.Get(entityId));
}

void
Scenario::ConfigureInternetRemotes ()
{
  NS_LOG_FUNCTION_NOARGS ();

  const auto remoteConfs = CONFIGURATOR->GetRemotesConfiguration ();
  size_t remoteId = 0;

  for (auto& conf : remoteConfs)
    {
      const auto appConfs = conf->GetApplications ();

      for (auto& appConf : appConfs)
        {
          TypeId appTid;
          NS_ASSERT_MSG (TypeId::LookupByNameFailSafe (appConf.GetName (), &appTid),
                         "Failed to initialize application " << appConf.GetName () << ". It does not exist!");

          ObjectFactory factory (appTid.GetName ());

          for (auto& appAttr : appConf.GetAttributes ())
            {
              factory.Set (appAttr.first, *appAttr.second);
            }

          auto app = StaticCast<Application, Object> (factory.Create ());
          m_remoteNodes.Get (remoteId)->AddApplication (app);
        }

      remoteId++;
    }
}

void
Scenario::ConfigureInternetBackbone ()
{
  NS_LOG_FUNCTION_NOARGS ();

  InternetStackHelper internetHelper;
  Ipv4StaticRoutingHelper routingHelper;

  internetHelper.Install (m_remoteNodes);

  // setup a CSMA LAN between all the remotes and network gateways in the backbone
  CsmaHelper csma;
  NetDeviceContainer backboneDevs = csma.Install (m_backbone);

  // set a new address base for the backbone
  Ipv4AddressHelper ipv4H;
  ipv4H.SetBase (Ipv4Address ("200.0.0.0"), Ipv4Mask ("255.0.0.0"));
  ipv4H.Assign (backboneDevs);

  //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // create static routes between each remote node to each network gateway
  internetHelper.SetRoutingHelper (routingHelper);

  for (uint32_t i = 0; i < m_remoteNodes.GetN (); i++)
    {
      Ptr<Node> remoteNode = m_backbone.Get (i);
      for (uint32_t j = m_remoteNodes.GetN (); j < m_backbone.GetN (); j++)
        {
          Ptr<Node> netGwNode = m_backbone.Get (j);
          uint32_t netGwBackboneDevIndex = netGwNode->GetNDevices () - 1;

          // since network gateway have at least 2 ipv4 addresses (one in the network and the other for the backbone)
          // we should create a route to the internal ip using the backbone ip as next hop
          Ipv4Address netGwBackboneIpv4 = netGwNode->GetObject<Ipv4> ()
                                            ->GetAddress (netGwBackboneDevIndex, 0).GetLocal ();
          Ipv4Address netGwInternalIpv4 = netGwNode->GetObject<Ipv4> ()
                                            ->GetAddress (1, 0).GetLocal ();

          NS_LOG_LOGIC ("Setting-up static route: REMOTE " << i << " "
                        << "(" << remoteNode->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () << ") "
                        << "-> NETWORK " << j - m_remoteNodes.GetN () << " "
                        << "(" << netGwBackboneIpv4 << " -> " << netGwInternalIpv4 << ")");

          Ptr<Ipv4StaticRouting> staticRouteRem = routingHelper.GetStaticRouting (remoteNode->GetObject<Ipv4> ());
          staticRouteRem->AddNetworkRouteTo (netGwInternalIpv4, Ipv4Mask ("255.0.0.0"), netGwBackboneIpv4, 1);
        }
    }

  std::stringstream logFilePathBuilder;
  logFilePathBuilder << CONFIGURATOR->GetResultsPath () << "internet";
  const auto logFilePath = logFilePathBuilder.str ();

  csma.EnablePcapAll (logFilePath, true);
  csma.EnableAsciiAll (logFilePath);
}

void
Scenario::EnablePhyLteTraces ()
{
  NS_LOG_FUNCTION_NOARGS ();

  for (size_t phyId = 0; phyId < m_protocolStacks[PHY_LAYER].size(); phyId++)
    {
      auto obj = m_protocolStacks[PHY_LAYER][phyId];

      if (typeid(*obj) == typeid(LtePhySimulationHelper))
        {
          /* LteHelperQuirk:
           *  This class is an hack to allow access to private members of LteHelper class,
           *  in particular to the StatsCalculators in order to set their output path.
           *  A structurally identical class is defined with all attributes set to public,
           *  then with a reinterpret_cast we interpret the LteHelper as this new class
           *  so the compiler won't complain about accessing its private members.
           */
          class LteHelperQuirk : public Object
          {
          public:
            Ptr<SpectrumChannel> dlC, ulC;
            Ptr<Object>  dlPlM, ulPlM;
            ObjectFactory a, b, c, d, e, f, g, h, i, j, k;
            std::string fMT;
            ObjectFactory fMF;
            Ptr<SpectrumPropagationLossModel> fM;
            bool fSA;
            Ptr<PhyStatsCalculator> phyStat;
            Ptr<PhyTxStatsCalculator> phyTxStat;
            Ptr<PhyRxStatsCalculator> phyRxStat;
            Ptr<MacStatsCalculator> macStat;
            Ptr<RadioBearerStatsCalculator> rlcStat;
            Ptr<RadioBearerStatsCalculator> pdcpStat;
            RadioBearerStatsConnector radioBearerStatsConnector;
            Ptr<EpcHelper> m_epcHelper;
            uint64_t m_imsiCounter;
            uint16_t m_cellIdCounter;
            bool l, m, n, o;
            std::map< uint8_t, ComponentCarrier > m_componentCarrierPhyParams;
            uint16_t m_noOfCcs;
          };

          auto phy = StaticCast<LtePhySimulationHelper, Object> (obj);
          auto lteHelper = phy->GetLteHelper ();
          std::stringstream basePath;

          basePath << CONFIGURATOR->GetResultsPath () << "lte-" << phyId << "-";

          lteHelper->EnableTraces ();

          auto rlcStat = lteHelper->GetRlcStats ();
          rlcStat->SetDlOutputFilename (basePath.str () + "RlcDlStats.txt");
          rlcStat->SetUlOutputFilename (basePath.str () + "RlcUlStats.txt");

          auto pdcpStat = lteHelper->GetPdcpStats ();
          pdcpStat->SetDlPdcpOutputFilename (basePath.str () + "PdcpDlStats.txt");
          pdcpStat->SetUlPdcpOutputFilename (basePath.str () + "PdcpUlStats.txt");

          auto lteHelperQ = reinterpret_cast<LteHelperQuirk*> (&(*lteHelper));

          lteHelperQ->phyStat->SetUeSinrFilename (basePath.str () + "PhySinrUlStats.txt");
          lteHelperQ->phyStat->SetInterferenceFilename (basePath.str () + "PhyInterferenceUlStats.txt");
          lteHelperQ->phyStat->SetCurrentCellRsrpSinrFilename (basePath.str () + "PhyRsrpSinrDlStats.txt");

          lteHelperQ->phyRxStat->SetDlRxOutputFilename (basePath.str () + "PhyRxDlStats.txt");
          lteHelperQ->phyRxStat->SetUlRxOutputFilename (basePath.str () + "PhyRxUlStats.txt");

          lteHelperQ->phyTxStat->SetDlTxOutputFilename (basePath.str () + "PhyTxDlStats.txt");
          lteHelperQ->phyTxStat->SetUlTxOutputFilename (basePath.str () + "PhyTxUlStats.txt");

          lteHelperQ->macStat->SetDlOutputFilename (basePath.str () + "MacDlStats.txt");
          lteHelperQ->macStat->SetUlOutputFilename (basePath.str () + "MacUlStats.txt");
        }
    }
}

void
Scenario::ConfigureRegionsOfInterest ()
{
  const auto regions = CONFIGURATOR->GetRegionsOfInterest ();
  Ptr<InterestRegion> reg;
  for (auto region : regions)
  {
    reg = irc->Create(region);
  }
}

void
Scenario::CourseChange (std::string context, Ptr<const MobilityModel> model)
{
  Vector position = model->GetPosition ();
  std::string start = "/NodeList/";
  std::string end = "/$ns3::MobilityModel/CourseChange";
  std::string id = context.substr (context.find (start) + start.length (), context.length () - end.length () - start.length ());
  auto dronePeripheralsContainer = m_drones.Get (std::stoi (id))->getPeripherals ();
  Ptr<DronePeripheral> peripheral;
  std::vector<int> regionindex;
  for (DronePeripheralContainer::Iterator i = dronePeripheralsContainer->Begin (); i != dronePeripheralsContainer->End (); i++)
  {
    peripheral = *i;
    regionindex = peripheral->GetRegionsOfInterest ();
    int status = irc->IsInRegions (regionindex, position);
    if (regionindex.empty ()) continue;
    if (status >= 0 || status == -2)
    {
      if (peripheral->GetState () != DronePeripheral::PeripheralState::ON)
        peripheral->SetState (DronePeripheral::PeripheralState::ON);
    } else {
      if (peripheral->GetState () == DronePeripheral::PeripheralState::ON)
        peripheral->SetState (DronePeripheral::PeripheralState::IDLE);
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

} // namespace ns3

int
main (int argc, char **argv)
{
  ns3::Scenario s (argc, argv);
  s ();  // run the scenario as soon as it is ready

  return 0;
}
