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
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/mobility-helper.h>
#include <ns3/lte-module.h>
#include <ns3/config-store.h>
#include <ns3/buildings-helper.h>

#include <ns3/yans-wifi-helper.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/constant-velocity-mobility-model.h>
#include <ns3/simulator.h>
#include <ns3/rectangle.h>
#include <ns3/box.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/ipv4-interface-container.h>
#include <ns3/net-device-container.h>
#include <ns3/ptr.h>
#include <ns3/ssid.h>
#include <ns3/vector.h>
#include <ns3/yans-wifi-helper.h>
#include <ns3/report.h>

#include <chrono>
#include <iomanip>
#include <string>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <vector>
#include <ns3/assert.h>
#include <ns3/command-line.h>
#include <ns3/config.h>
#include <ns3/log.h>
#include "ns3/flow-monitor-module.h"
#include <ns3/flow-monitor-helper.h>

#include <ns3/drone-client-application.h>
#include <ns3/drone-server-application.h>
#include <ns3/zsp-list.h>
#include <ns3/drone-list.h>
#include <ns3/flight-plan.h>
#include <ns3/proto-point.h>
#include <ns3/scenario-configuration-helper.h>

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
  void ConfigureMobilityEnbs ();
  void ConfigureNetwork ();
  void ConfigureLte ();
  void ConfigureApplication ();
  void ConfigureApplicationDrones ();
  void ConfigureApplicationZsps ();
  void ConfigureSimulator ();

  NodeContainer m_drones;
  NodeContainer m_zsps;
  NodeContainer m_enbs;
  NetDeviceContainer m_netDevices;

  Ptr<LteHelper> m_lteHelper = CreateObject<LteHelper> ();

  WifiHelper m_wifi;
  YansWifiPhyHelper m_wifiPhy;
  Ipv4InterfaceContainer m_ifaceIps;
  const char *m_ifaceNetMask;

  std::string phyMode;
  int32_t rss;
  Time interPacketInterval;
  uint32_t numDrones;
};

NS_LOG_COMPONENT_DEFINE ("Scenario Babbage");

Scenario::Scenario (int argc, char **argv) : m_ifaceNetMask{"255.0.0.0"}
{

  phyMode = "DsssRate1Mbps";
  rss = -80;
  interPacketInterval = Seconds (1.0);
  numDrones = 2;

  LogComponentEnable ("Scenario Babbage", LOG_LEVEL_ALL);
  LogComponentEnable ("DroneServerApplication", LOG_LEVEL_ALL);
  LogComponentEnable ("DroneClientApplication", LOG_LEVEL_ALL);

  std::ofstream out ("../results/dronesim-allen.log");
  std::clog.rdbuf (out.rdbuf ());
  out.close ();

  NS_LOG_INFO ("###################");
  NS_LOG_INFO ("# Dronesim        #");
  NS_LOG_INFO ("# Scenario: Babbage #");
  NS_LOG_INFO ("###################");

  m_drones.Create (numDrones);
  m_zsps.Create (1);
  m_enbs.Create (1);

  for (auto drone = m_drones.Begin (); drone != m_drones.End (); drone++)
    DroneList::Add (*drone);

  for (auto zsp = m_zsps.Begin (); zsp != m_zsps.End (); zsp++)
    ZspList::Add (*zsp);

  ConfigurePhy ();
  ConfigureMac ();
  ConfigureMobility ();
  ConfigureNetwork ();
  ConfigureLte ();
  ConfigureApplication ();
  ConfigureSimulator ();
}

Scenario::~Scenario ()
{
}

void
Scenario::ConfigurePhy ()
{

  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();

  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold",
                      StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

  m_wifi.SetStandard (WIFI_STANDARD_80211n_2_4GHZ);
  m_wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (phyMode),
                                  "ControlMode", StringValue (phyMode));

  m_wifiPhy.Set ("RxGain", DoubleValue (0));
  m_wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency",
                                  DoubleValue (2.4e9));
  m_wifiPhy.SetChannel (wifiChannel.Create ());
}

void
Scenario::ConfigureMac ()
{

  WifiMacHelper wifiMac;

  Ssid ssid = Ssid ("wifi-default");

  wifiMac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));
  NetDeviceContainer staDevice = m_wifi.Install (m_wifiPhy, wifiMac, m_drones);
  m_netDevices.Add (staDevice);

  wifiMac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
  NetDeviceContainer apDevices = m_wifi.Install (m_wifiPhy, wifiMac, m_zsps);
  m_netDevices.Add (apDevices);
}

void
Scenario::ConfigureMobility ()
{

  ConfigureMobilityDrones ();
  ConfigureMobilityZsps ();
  ConfigureMobilityEnbs ();
}

void
Scenario::ConfigureMobilityDrones ()
{

  MobilityHelper mobilityDrones;
  ObjectFactory position;
  int64_t streamIndex = 0;

  position.SetTypeId ("ns3::RandomBoxPositionAllocator");
  position.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"));
  position.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"));
  position.Set ("Z", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"));

  Ptr<PositionAllocator> positionAllocatorDrones =
      position.Create ()->GetObject<PositionAllocator> ();
  streamIndex += positionAllocatorDrones->AssignStreams (streamIndex);

  std::string speed ("ns3::ConstantRandomVariable[Constant=10.0]");
  std::string pause ("ns3::ConstantRandomVariable[Constant=1.0]");

  mobilityDrones.SetMobilityModel ("ns3::RandomWaypointMobilityModel", "Speed", StringValue (speed),
                                   "Pause", StringValue (pause), "PositionAllocator",
                                   PointerValue (positionAllocatorDrones));

  mobilityDrones.Install (m_drones);
}

void
Scenario::ConfigureMobilityZsps ()
{

  MobilityHelper mobility;

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (5.0, 0.0, 0.0));
  positionAlloc->Add (Vector (-5.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (m_zsps);
}

void
Scenario::ConfigureMobilityEnbs ()
{

  MobilityHelper mobility;

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (5.0, 0.0, 0.0));
  positionAlloc->Add (Vector (-5.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (m_enbs);
}

void
Scenario::ConfigureNetwork ()
{

  InternetStackHelper internet;
  Ipv4AddressHelper ipv4;

  internet.Install (m_drones);
  internet.Install (m_zsps);

  NS_LOG_INFO ("> Assigning IP Addresses.");
  ipv4.SetBase ("10.0.0.0", "255.0.0.0");
  m_ifaceIps = ipv4.Assign (m_netDevices);

  for (uint i = 0; i < m_netDevices.GetN (); i++)
    NS_LOG_INFO ("[Node " << m_netDevices.Get (i)->GetNode ()->GetId () << "] assigned address "
                          << m_ifaceIps.GetAddress (i, 0));
}

void
Scenario::ConfigureLte ()
{

  m_lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::Cost231PropagationLossModel"));

  m_lteHelper->SetSchedulerType ("ns3::PfFfMacScheduler");
  m_lteHelper->SetSchedulerAttribute ("HarqEnabled", BooleanValue (true));

  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  enbDevs = m_lteHelper->InstallEnbDevice (m_enbs);
  ueDevs = m_lteHelper->InstallUeDevice (m_drones);

  m_lteHelper->Attach (ueDevs, enbDevs.Get (0));

  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VIDEO;
  GbrQosInformation qos;
  qos.gbrDl = 20000000;
  qos.gbrUl = 5000000;
  qos.mbrDl = 20000000;
  qos.mbrUl = 5000000;
  EpsBearer bearer (q, qos);
  m_lteHelper->ActivateDataRadioBearer (ueDevs, bearer);

  Ptr<LteEnbNetDevice> lteEnbDev = enbDevs.Get (0)->GetObject<LteEnbNetDevice> ();
  Ptr<LteEnbPhy> enbPhy = lteEnbDev->GetPhy ();
  enbPhy->SetAttribute ("TxPower", DoubleValue (46.0));
}

void
Scenario::ConfigureApplication ()
{

  ConfigureApplicationDrones ();
  ConfigureApplicationZsps ();
}

void
Scenario::ConfigureApplicationDrones ()
{

  for (auto drone = m_drones.Begin (); drone != m_drones.End (); drone++)
    {
      auto client = CreateObjectWithAttributes<DroneClientApplication> (
          "Ipv4Address", Ipv4AddressValue (m_ifaceIps.GetAddress ((*drone)->GetId ())),
          "Ipv4SubnetMask", Ipv4MaskValue (m_ifaceNetMask));
      (*drone)->AddApplication (client);

      client->SetStartTime (Seconds (1));
    }
}

void
Scenario::ConfigureApplicationZsps ()
{

  Ptr<DroneServerApplication> server = CreateObjectWithAttributes<DroneServerApplication> (
      "Ipv4Address", Ipv4AddressValue (m_ifaceIps.GetAddress (m_ifaceIps.GetN () - 1)),
      "Ipv4SubnetMask", Ipv4MaskValue (m_ifaceNetMask));
  m_zsps.Get (0)->AddApplication (server);

  server->SetStartTime (Seconds (0));
}

void
Scenario::ConfigureSimulator ()
{

  m_wifiPhy.EnablePcap ("../results/dronesim", m_netDevices);

  AsciiTraceHelper ascii;
  m_wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("../results/PHY_comms.log"));

  std::chrono::high_resolution_clock timer;
  auto start = timer.now ();
  auto in_time_t = std::chrono::system_clock::to_time_t (start);
  std::ostringstream dateTime;

  dateTime << std::put_time (std::localtime (&in_time_t), "%Y-%m-%d.%H-%M-%S");

  std::ostringstream resultsPath;
  resultsPath << "../results/babbage-" << dateTime.str ();

  Report::Get ()->Initialize ("babbage", dateTime.str (), resultsPath.str ());

  Simulator::Stop (Seconds (120.0));

  std::cout << "> Starting simulation..." << std::endl;

  Simulator::Run ();

  Report::Get ()->Save ();

  Simulator::Destroy ();

  auto stop = timer.now ();
  auto deltaTime = std::chrono::duration_cast<ms> (stop - start).count ();

  std::cout << "Simulation terminated. Took " << deltaTime << "ms." << std::endl;

  std::clog.rdbuf (std::cout.rdbuf ());
}

void
Scenario::operator() ()
{
}
} // namespace ns3

int
main (int argc, char **argv)
{

  ns3::Scenario s (argc, argv);
  s ();

  return 0;
}
