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

#include <chrono>

#include <ns3/log.h>
#include <ns3/node-container.h>
#include <ns3/net-device-container.h>
#include <ns3/simulator.h>
#include <ns3/mobility-helper.h>
#include <ns3/object-factory.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/ipv4-interface-container.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/yans-wifi-helper.h>
#include <ns3/ssid.h>
#include <ns3/config.h>
#include <ns3/double.h>
#include <ns3/string.h>

#include <ns3/scenario-configuration-helper.h>
#include <ns3/drone-list.h>
#include <ns3/zsp-list.h>
#include <ns3/drone-client.h>
#include <ns3/drone-server.h>
#include <ns3/flight-plan.h>
#include <ns3/proto-point.h>
#include <ns3/speed-coefficients.h>

#include <ns3/report.h>

NS_LOG_COMPONENT_DEFINE("Scenario");

namespace ns3
{

class Scenario
{
public:
  using ms = std::chrono::duration<float, std::milli>;

  Scenario(int argc, char **argv);
  virtual ~Scenario();

  void Run();

private:
  NodeContainer m_drones;
  NodeContainer m_zsps;
  NetDeviceContainer m_netDevices;
  WifiHelper m_wifi;
  YansWifiPhyHelper m_wifiPhy;
  Ipv4InterfaceContainer m_ifaceIps;
  const char *m_ifaceNetMask;

  void ConfigurePhy();
  void ConfigureMac();
  void ConfigureMobility();
  void ConfigureMobilityDrones();
  void ConfigureMobilityZsps();
  void ConfigureNetwork();
  void ConfigureApplication();
  void ConfigureApplicationDrones();
  void ConfigureApplicationZsps();
  void ConfigureSimulator();
};


Scenario::Scenario(int argc, char **argv)
{
  m_ifaceNetMask = "255.0.0.0";
  CONFIGURATOR->Initialize(argc, argv, "LTE-Easley");

  m_drones.Create(CONFIGURATOR->GetDronesN());
  m_zsps.Create(CONFIGURATOR->GetZspsN());

  for (auto zsp = m_zsps.Begin(); zsp != m_zsps.End(); zsp++)
    ZspList::Add(*zsp);

  for (auto drone = m_drones.Begin(); drone != m_drones.End(); drone++)
    DroneList::Add(*drone);

  ConfigurePhy();
  ConfigureMac();
  ConfigureMobility();
  ConfigureApplication();
  ConfigureSimulator();
}

Scenario::~Scenario() {}

void Scenario::Run()
{
  //NS_LOG_FUNCTION_NOARGS();

  std::chrono::high_resolution_clock timer;
  auto start = timer.now();

  //Simulator::Stop(simulation_time);
  Simulator::Run();

  Report::Get()->Save();

  Simulator::Destroy();

  auto stop = timer.now();
  auto duration = std::chrono::duration_cast<ms>(stop - start).count();
  NS_LOG_INFO("Simulation completed in " << duration << "ms.");
}


void Scenario::ConfigurePhy()
{
  //NS_LOG_FUNCTION_NOARGS();

  // WiFi Specific configuration
  YansWifiChannelHelper wifiChannel;
  AsciiTraceHelper ascii;
  m_wifiPhy = YansWifiPhyHelper::Default();

  Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(CONFIGURATOR->GetPhyMode()));

  m_wifi.SetStandard(WIFI_PHY_STANDARD_80211n_2_4GHZ);
  m_wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

  const std::string propagationLossModel = CONFIGURATOR->GetPhyPropagationLossModel();
  NS_ASSERT_MSG(propagationLossModel == "ns3::ThreeLogDistancePropagationLossModel",
      propagationLossModel << " decoder is not implemented!");

  const auto propLossAttr = CONFIGURATOR->GetThreeLogDistancePropagationLossModelAttributes();

  wifiChannel.AddPropagationLoss(CONFIGURATOR->GetPhyPropagationLossModel(),
      propLossAttr[0].first, DoubleValue(propLossAttr[0].second),
      propLossAttr[1].first, DoubleValue(propLossAttr[1].second),
      propLossAttr[2].first, DoubleValue(propLossAttr[2].second),
      propLossAttr[3].first, DoubleValue(propLossAttr[3].second),
      propLossAttr[4].first, DoubleValue(propLossAttr[4].second),
      propLossAttr[5].first, DoubleValue(propLossAttr[5].second),
      propLossAttr[6].first, DoubleValue(propLossAttr[6].second),
      propLossAttr[7].first, DoubleValue(propLossAttr[7].second));

  m_wifiPhy.SetChannel(wifiChannel.Create());
}

void Scenario::ConfigureMac()
{
  //NS_LOG_FUNCTION_NOARGS();

  const std::string phyMode = CONFIGURATOR->GetPhyMode();

  WifiMacHelper wifiMac;
  Ssid ssid = Ssid("wifi-default");

  m_wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
      "DataMode", StringValue(phyMode),
      "ControlMode", StringValue(phyMode));

  wifiMac.SetType("ns3::StaWifiMac",
      "Ssid", SsidValue(ssid));

  NetDeviceContainer dronesDevices = m_wifi.Install(m_wifiPhy, wifiMac, m_drones);
  m_netDevices.Add(dronesDevices);

  wifiMac.SetType("ns3::ApWifiMac",
      "Ssid", SsidValue(ssid));

  NetDeviceContainer zspsDevices = m_wifi.Install(m_wifiPhy, wifiMac, m_zsps);
  m_netDevices.Add(zspsDevices);
}


void Scenario::ConfigureMobility()
{
  //NS_LOG_FUNCTION_NOARGS();
  ConfigureMobilityDrones();
  ConfigureMobilityZsps();
}

void Scenario::ConfigureMobilityDrones()
{
  //NS_LOG_FUNCTION_NOARGS();

  MobilityHelper mobilityDrones;

  for (uint32_t i = 0; i < m_drones.GetN(); i++)
  {
    mobilityDrones.SetMobilityModel("ns3::ParametricSpeedDroneMobilityModel",
        "SpeedCoefficients", SpeedCoefficientsValue(CONFIGURATOR->GetDroneSpeedCoefficients(i)),
        "FlightPlan", FlightPlanValue(CONFIGURATOR->GetDroneFlightPlan(i)),
        "CurveStep", DoubleValue(CONFIGURATOR->GetCurveStep()));
    mobilityDrones.Install(m_drones.Get(i));
  }
}

void Scenario::ConfigureMobilityZsps()
{
  //NS_LOG_FUNCTION_NOARGS();
  MobilityHelper mobilityZsps;
  auto positionAllocatorZsps = CreateObject<ListPositionAllocator>();

  CONFIGURATOR->GetZspsPosition(positionAllocatorZsps);

  mobilityZsps.SetPositionAllocator(positionAllocatorZsps);
  mobilityZsps.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobilityZsps.Install(m_zsps);
}


void Scenario::ConfigureNetwork()
{
  //NS_LOG_FUNCTION_NOARGS();

  InternetStackHelper internet;
  Ipv4AddressHelper ipv4;

  internet.Install(m_drones);
  internet.Install(m_zsps);

  NS_LOG_INFO("> Assigning IP Addresses:");
  ipv4.SetBase("10.0.0.0", m_ifaceNetMask);
  m_ifaceIps = ipv4.Assign(m_netDevices);

  for (uint32_t i = 0; i < m_netDevices.GetN(); i++)
  {
    auto netDev_id = m_netDevices.Get(i)->GetNode()->GetId();
    auto address = m_ifaceIps.GetAddress(i, 0);
    NS_LOG_INFO("[Node " << netDev_id << "] assigned address: " << address);
  }
}


void Scenario::ConfigureApplication()
{
  //NS_LOG_FUNCTION_NOARGS();
  ConfigureApplicationDrones();
  ConfigureApplicationZsps();
}

void Scenario::ConfigureApplicationDrones()
{
  //NS_LOG_FUNCTION_NOARGS();

  for (uint32_t i = 0; i < m_drones.GetN(); i++)
  {
    auto client = CreateObjectWithAttributes<DroneClient>(
        "Ipv4Address", Ipv4AddressValue(m_ifaceIps.GetAddress(i)),
        "Ipv4SubnetMask", Ipv4MaskValue(m_ifaceNetMask),
        "Duration", DoubleValue(CONFIGURATOR->GetDuration()));

    double droneAppStartTime = CONFIGURATOR->GetDroneApplicationStartTime(i);
    client->SetStartTime(Seconds(droneAppStartTime));
    double droneAppStopTime = CONFIGURATOR->GetDroneApplicationStopTime(i);
    client->SetStopTime(Seconds(droneAppStopTime));

    m_drones.Get(i)->AddApplication(client);

    NS_LOG_LOGIC("[Node " << i << "] active from " << droneAppStartTime << "s to " << droneAppStopTime << "s.");
  }
}

void Scenario::ConfigureApplicationZsps()
{
  //NS_LOG_FUNCTION_NOARGS();

  auto server = CreateObjectWithAttributes<DroneServer>(
        "Ipv4Address", Ipv4AddressValue(m_ifaceIps.GetAddress(m_ifaceIps.GetN() - 1)),
        "Ipv4SubnetMask", Ipv4MaskValue(m_ifaceNetMask));

  double zspAppStartTime = CONFIGURATOR->GetZspApplicationStartTime(0);
  server->SetStartTime(Seconds(zspAppStartTime));
  double zspAppStopTime = CONFIGURATOR->GetZspApplicationStopTime(0);
  server->SetStopTime(Seconds(zspAppStopTime));

  m_zsps.Get(0)->AddApplication(server);

  NS_LOG_LOGIC("[Server 0] active from " << zspAppStartTime << "s to " << zspAppStopTime << "s.");
}


void Scenario::ConfigureSimulator()
{
  //NS_LOG_FUNCTION_NOARGS();

  std::stringstream phyTraceLog, pcapLog;
  AsciiTraceHelper ascii;

  phyTraceLog << CONFIGURATOR->GetResultsPath() << "-phy.log";
  pcapLog << CONFIGURATOR->GetResultsPath() << "-host";

  Report::Get()->Initialize("LTE-Easley", CONFIGURATOR->GetCurrentDateTime(), CONFIGURATOR->GetResultsPath());

  Simulator::Stop(Seconds(CONFIGURATOR->GetDuration()));
}

} // namespace ns3


int main(int argc, char **argv)
{
  ns3::Scenario scenario(argc, argv);
  scenario.Run();

  return 0;
}