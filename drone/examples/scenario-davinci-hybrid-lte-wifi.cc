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
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/mobility-helper.h>
#include <ns3/lte-module.h>
#include <ns3/config-store.h>
#include <ns3/buildings-helper.h>

#include <ns3/yans-wifi-helper.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/constant-velocity-mobility-model.h>
#include <ns3/simulator.h>
#include <ns3/rectangle.h>
#include <ns3/box.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/udp-client-server-helper.h>
#include <ns3/packet-sink-helper.h>
#include <ns3/ipv4-interface-container.h>
#include <ns3/ipv4-static-routing.h>
#include <ns3/ipv4-static-routing-helper.h>
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

#include <ns3/drone-client.h>
#include <ns3/drone-server.h>
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
  void ConfigureApplicationDronesWifi ();
  void ConfigureApplicationDronesLte ();
  void ConfigureApplicationZsps ();
  void ConfigureApplicationEnbs ();
  void ConfigureSimulator ();
  void CourseChange ();

  NodeContainer m_drones;
  NodeContainer m_zsps;
  NodeContainer m_enbs;
  NetDeviceContainer m_wifiDevices;
  NetDeviceContainer m_lteDevices;

  Ptr<LteHelper> m_lteHelper;
  Ptr<PointToPointEpcHelper> m_epcHelper;

  WifiHelper m_wifi;
  YansWifiPhyHelper m_wifiPhy;
  Ipv4InterfaceContainer m_ifaceIps;
  Ipv4InterfaceContainer m_lteInterface;
  const char *m_ifaceNetMask;
};

NS_LOG_COMPONENT_DEFINE ("Scenario DaVinci");

Scenario::Scenario (int argc, char **argv) : m_ifaceNetMask{"255.0.0.0"}
{

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  CONFIGURATOR->Initialize (argc, argv, "davinci");

  m_drones.Create (CONFIGURATOR->GetDronesN ());
  m_zsps.Create (CONFIGURATOR->GetZspsN ());
  m_enbs.Create (CONFIGURATOR->GetEnbsN ());

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
  NS_LOG_FUNCTION_NOARGS ();

  const std::string phyMode = CONFIGURATOR->GetPhyMode ();
  //YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  YansWifiChannelHelper wifiChannel;

  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

  m_wifi.SetStandard (WIFI_STANDARD_80211n_2_4GHZ);

  m_wifiPhy = YansWifiPhyHelper::Default ();

  m_wifiPhy.Set ("RxGain", DoubleValue (0));
  m_wifiPhy.Set ("TxGain", DoubleValue (0));
  m_wifiPhy.Set ("Antennas", UintegerValue (1));
  //m_wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue (-80));
  m_wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel",
                                  "Frequency", DoubleValue (2.4e9));
  //wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", DoubleValue(250));
  m_wifiPhy.SetChannel (wifiChannel.Create ());
}

void
Scenario::ConfigureMac ()
{
  NS_LOG_FUNCTION_NOARGS ();

  const std::string phyMode = CONFIGURATOR->GetPhyMode ();

  WifiMacHelper wifiMac;
  Ssid ssid = Ssid ("wifi");

  m_wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode", StringValue (phyMode),
                                  "ControlMode", StringValue (phyMode));

  wifiMac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid));
  NetDeviceContainer staDevices = m_wifi.Install (m_wifiPhy, wifiMac, m_drones);
  m_wifiDevices.Add (staDevices);

  wifiMac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));
  NetDeviceContainer apDevices = m_wifi.Install (m_wifiPhy, wifiMac, m_zsps);
  m_wifiDevices.Add (apDevices);
}

void
Scenario::ConfigureMobility ()
{
  NS_LOG_FUNCTION_NOARGS ();

  ConfigureMobilityDrones ();
  ConfigureMobilityZsps ();
  ConfigureMobilityEnbs ();
  //BuildingsHelper::MakeMobilityModelConsistent ();
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
      //BuildingsHelper::Install (m_drones.Get (i));
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
  //BuildingsHelper::Install (m_zsps);
}

void
Scenario::ConfigureMobilityEnbs ()
{
  NS_LOG_FUNCTION_NOARGS ();

  MobilityHelper mobilityEnbs;
  auto positionAllocatorEnbs = CreateObject<ListPositionAllocator> ();

  CONFIGURATOR->GetEnbsPosition (positionAllocatorEnbs);

  mobilityEnbs.SetPositionAllocator (positionAllocatorEnbs);
  mobilityEnbs.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityEnbs.Install (m_enbs);
  //BuildingsHelper::Install (m_enbs);
}

void
Scenario::ConfigureNetwork ()
{
  NS_LOG_FUNCTION_NOARGS ();

  InternetStackHelper internet;
  Ipv4AddressHelper ipv4;

  internet.Install (m_drones);
  internet.Install (m_zsps);

  ipv4.SetBase ("10.0.0.0", m_ifaceNetMask);
  m_ifaceIps = ipv4.Assign (m_wifiDevices);

  for (uint i = 0; i < m_wifiDevices.GetN (); i++)
    {
      NS_LOG_INFO ("[Node " << m_wifiDevices.Get (i)->GetNode ()->GetId () << "] assigned address "
                            << m_ifaceIps.GetAddress (i, 0));
    }
}

void
Scenario::ConfigureLte ()
{
  NS_LOG_FUNCTION_NOARGS ();

  m_lteHelper = CreateObject<LteHelper> (); //Create an LteHelper object
  m_lteHelper->SetAttribute (
      "PathlossModel", StringValue ("ns3::Cost231PropagationLossModel")); //Pathloss model selection

  //using the Proportional Fair (PF) scheduler
  m_lteHelper->SetSchedulerType ("ns3::PfFfMacScheduler");
  m_lteHelper->SetSchedulerAttribute ("HarqEnabled", BooleanValue (true));
  //m_lteHelper->SetSchedulerAttribute ("CqiTimerThreshold", UintegerValue (1000));

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  enbDevs = m_lteHelper->InstallEnbDevice (m_enbs); //Install an LTE protocol stack on the eNB(s).
  ueDevs = m_lteHelper->InstallUeDevice (m_drones); //Install an LTE protocol stack on the UEs.

  // Attach a UE to a eNB
  m_lteHelper->Attach (ueDevs, enbDevs.Get (0));
  //Attach the UEs to an eNB. This will configure each UE according to the eNB configuration,
  //and create an RRC connection between them.

  // Activate a data radio bearer between each UE and the eNB it is attached to.
  //For constant bit rate (CBR) traffic, it is suggested to set MBR to GBR.
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VIDEO; // define Qci type --> Qos class indicator
  GbrQosInformation qos;
  /*qos.gbrDl = 132;  // bit/s, considering IP, UDP, RLC, PDCP header size
  qos.gbrUl = 132;
  qos.mbrDl = qos.gbrDl;
  qos.mbrUl = qos.gbrUl;*/
  qos.gbrDl = 20000000; // Downlink GBR (bit/s) ---> 20 Mbps
  qos.gbrUl = 5000000; // Uplink GBR ---> 5 Mbps
  qos.mbrDl = 20000000; // Downlink MBR
  qos.mbrUl = 5000000; // Uplink MBR,
  EpsBearer bearer (q, qos);
  m_lteHelper->ActivateDataRadioBearer (ueDevs, bearer);

  Ptr<LteEnbNetDevice> lteEnbDev = enbDevs.Get (0)->GetObject<LteEnbNetDevice> ();
  Ptr<LteEnbPhy> enbPhy = lteEnbDev->GetPhy ();
  //enbPhy->SetAttribute ("TxPower", DoubleValue (46.0));
}

void
Scenario::ConfigureApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  ConfigureApplicationDronesWifi ();
  ConfigureApplicationZsps ();
}

void
Scenario::ConfigureApplicationDronesWifi ()
{
  NS_LOG_FUNCTION_NOARGS ();

  for (uint32_t i = 0; i < m_drones.GetN (); i++)
    {
      auto client = CreateObjectWithAttributes<DroneClient> (
          "Ipv4Address", Ipv4AddressValue (m_ifaceIps.GetAddress (i)), "Ipv4SubnetMask",
          Ipv4MaskValue (m_ifaceNetMask), "Duration", DoubleValue (CONFIGURATOR->GetDuration ()));

      double droneAppStartTime = CONFIGURATOR->GetDroneApplicationStartTime (i);
      double droneAppStopTime = CONFIGURATOR->GetDroneApplicationStopTime (i);

      NS_LOG_LOGIC ("[Node " << i << "] will operate in time interval from " << droneAppStartTime
                             << " to " << droneAppStopTime << "s.");

      m_drones.Get (i)->AddApplication (client);
      client->SetStartTime (Seconds (droneAppStartTime));
      client->SetStopTime (Seconds (droneAppStopTime));
    }
}

void
Scenario::ConfigureApplicationZsps ()
{
  NS_LOG_FUNCTION_NOARGS ();

  auto server = CreateObjectWithAttributes<DroneServer> (
      "Ipv4Address", Ipv4AddressValue (m_ifaceIps.GetAddress (m_ifaceIps.GetN () - 1)),
      "Ipv4SubnetMask", Ipv4MaskValue (m_ifaceNetMask));

  m_zsps.Get (0)->AddApplication (server);

  server->SetStartTime (Seconds (CONFIGURATOR->GetZspApplicationStartTime (0)));
  server->SetStopTime (Seconds (CONFIGURATOR->GetZspApplicationStopTime (0)));
}

void
Scenario::ConfigureSimulator ()
{
  NS_LOG_FUNCTION_NOARGS ();

  std::stringstream phyTraceLog, pcapLog;
  AsciiTraceHelper ascii;

  phyTraceLog << CONFIGURATOR->GetResultsPath () << "-phy.log";
  pcapLog << CONFIGURATOR->GetResultsPath () << "-host";

  m_wifiPhy.EnableAsciiAll (ascii.CreateFileStream (phyTraceLog.str ()));
  m_wifiPhy.EnablePcap (pcapLog.str (), m_wifiDevices);

  m_lteHelper->EnablePhyTraces ();
  m_lteHelper->EnableMacTraces ();
  m_lteHelper->EnableRlcTraces ();

  Report::Get ()->Initialize ("davinci", CONFIGURATOR->GetCurrentDateTime (),
                              CONFIGURATOR->GetResultsPath ());

  Simulator::Stop (Seconds (CONFIGURATOR->GetDuration ()));

  Ptr<RadioBearerStatsCalculator> rlcStats = m_lteHelper->GetRlcStats ();
  rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (0.1)));
}

void
Scenario::operator() ()
{
  NS_LOG_FUNCTION_NOARGS ();

  std::chrono::high_resolution_clock timer;
  auto start = timer.now ();

  std::cout << "> Starting simulation..." << std::endl;

  Simulator::Run ();

  Report::Get ()->Save ();

  Simulator::Destroy ();

  auto stop = timer.now ();
  auto deltaTime = std::chrono::duration_cast<ms> (stop - start).count ();

  std::cout << "Simulation terminated. Took " << deltaTime << "ms." << std::endl;
}
} // namespace ns3

int
main (int argc, char **argv)
{

  ns3::Scenario s (argc, argv);
  s ();

  return 0;
}
