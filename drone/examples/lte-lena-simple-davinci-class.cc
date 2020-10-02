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
//#include <ns3/ipv4-address-helper.h>
//#include <ns3/ipv4-interface-container.h>
//#include <ns3/internet-stack-helper.h>
//#include <ns3/yans-wifi-helper.h>
//#include <ns3/ssid.h>
#include <ns3/config.h>
#include <ns3/double.h>
#include <ns3/string.h>

#include <ns3/lte-helper.h>
#include <ns3/config-store.h>
#include <ns3/buildings-helper.h>

#include <ns3/scenario-configuration-helper.h>
#include <ns3/drone-list.h>
#include <ns3/zsp-list.h>
//#include <ns3/drone-client.h>
//#include <ns3/drone-server.h>
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
  Ptr<LteHelper> lteHelper;

  void ConfigureSimulator();
  void ConfigureMobility();
  void ConfigureMobilityDrones();
  void ConfigureMobilityZsps();
  void ConfigureProtocol();
  void ConfigurePhy();
  void ConfigureMac();
  void ConfigureRlc();
  //void ConfigureNetwork();
  //void ConfigureApplication();
  //void ConfigureApplicationDrones();
  //void ConfigureApplicationZsps();
};


Scenario::~Scenario() {}

Scenario::Scenario(int argc, char **argv)
{
  //LogComponentEnable("Scenario", LOG_LEVEL_ALL);

  CONFIGURATOR->Initialize(argc, argv, "lte-simple-davinci-class");

  m_zsps.Create(1);
  m_drones.Create(CONFIGURATOR->GetDronesN());

  //for (auto zsp = m_zsps.Begin(); zsp != m_zsps.End(); zsp++)
  //  ZspList::Add(*zsp);

  //for (auto drone = m_drones.Begin(); drone != m_drones.End(); drone++)
  //  DroneList::Add(*drone);

  ConfigureMobility();
  ConfigureProtocol();
  //ConfigureApplication();
  ConfigureSimulator();
}

void Scenario::ConfigureSimulator()
{
  NS_LOG_FUNCTION_NOARGS();

  std::stringstream phyTraceLog, pcapLog;
  //AsciiTraceHelper ascii;

  phyTraceLog << CONFIGURATOR->GetResultsPath() << "-phy.log";
  pcapLog << CONFIGURATOR->GetResultsPath() << "-host";

  Report::Get()->Initialize("lte-lena-simple-davinci-class", CONFIGURATOR->GetCurrentDateTime(), CONFIGURATOR->GetResultsPath());

  Simulator::Stop(Seconds(CONFIGURATOR->GetDuration() + 3));
}

void Scenario::Run()
{
  NS_LOG_FUNCTION_NOARGS();

  std::chrono::high_resolution_clock timer;
  auto start = timer.now();

  NS_LOG_INFO("> Starting simulation...");

  Simulator::Run();

  Report::Get()->Save();

  Simulator::Destroy();

  auto stop = timer.now();
  auto duration = std::chrono::duration_cast<ms>(stop - start).count();
  NS_LOG_INFO("> Simulation terminated. Took: " << duration/1000 << " s.");
}


void Scenario::ConfigureProtocol()
{
  NS_LOG_FUNCTION_NOARGS();
/*
  Config::SetDefault("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue(true));
  Config::SetDefault("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue(true));
  Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(true));
  Config::SetDefault("ns3::LteHelper::UsePdschForCqiGeneration", BooleanValue(true));
*/

  ConfigStore config;
  config.ConfigureDefaults();

  lteHelper = CreateObject<LteHelper>();

  ConfigurePhy();
  ConfigureMac();
  ConfigureRlc();
  //ConfigureNetwork();
}

void Scenario::ConfigurePhy()
{
  NS_LOG_FUNCTION_NOARGS();

  lteHelper->SetAttribute("PathlossModel", StringValue("ns3::Cost231PropagationLossModel"));

  lteHelper->EnablePhyTraces();
}

void Scenario::ConfigureMac()
{
  NS_LOG_FUNCTION_NOARGS();

  lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");
  lteHelper->SetSchedulerAttribute("HarqEnabled", BooleanValue(true));
  //lteHelper->SetSchedulerAttribute("CqiTimerThreshold", UintegerValue(1000));

  NetDeviceContainer ueDevices = lteHelper->InstallUeDevice(m_drones);
  m_netDevices.Add(ueDevices);

  NetDeviceContainer enbDevices = lteHelper->InstallEnbDevice(m_zsps);
  m_netDevices.Add(enbDevices);

  lteHelper->Attach(ueDevices, enbDevices.Get(0));

  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VIDEO;
  GbrQosInformation qos;
  //qos.gbrDl = 132;  // bit/s, considering IP, UDP, RLC, PDCP header size
  //qos.gbrUl = 132;
  //qos.mbrDl = qos.gbrDl;
  //qos.mbrUl = qos.gbrUl;
  qos.gbrDl = 20000000; 	   // Downlink GBR (bit/s) ---> 20 Mbps
  qos.gbrUl = 5000000;	 	  // Uplink GBR ---> 5 Mbps
  qos.mbrDl = 20000000;		 // Downlink MBR
  qos.mbrUl = 5000000; 		// Uplink MBR,
  EpsBearer bearer(q, qos);
  lteHelper->ActivateDataRadioBearer (ueDevices, bearer);

  lteHelper->EnableMacTraces();
}

void Scenario::ConfigureRlc()
{
  NS_LOG_FUNCTION_NOARGS();
  // add Radio Bearer related things here
  // Needs ueDevices to be passed from ConfigureMac() procedure

  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats ();
  rlcStats->SetAttribute ("StartTime", TimeValue (Seconds (0.04)));
  rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (1.0)));

  lteHelper->EnableRlcTraces();
}

/*
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
*/


void Scenario::ConfigureMobility()
{
  NS_LOG_FUNCTION_NOARGS();
  ConfigureMobilityDrones();
  ConfigureMobilityZsps();
}

void Scenario::ConfigureMobilityDrones()
{
  NS_LOG_FUNCTION_NOARGS();

  MobilityHelper mobilityDrones;

  for (uint32_t i = 0; i < m_drones.GetN(); i++)
  {
    mobilityDrones.SetMobilityModel ("ns3::ConstantAccelerationDroneMobilityModel",
        "Acceleration", DoubleValue (CONFIGURATOR->GetDroneAcceleration (i)),
        "MaxSpeed", DoubleValue (CONFIGURATOR->GetDroneMaxSpeed (i)),
        "FlightPlan", FlightPlanValue (CONFIGURATOR->GetDroneFlightPlan (i)),
        "CurveStep", DoubleValue (CONFIGURATOR->GetCurveStep ()));
    mobilityDrones.Install(m_drones.Get(i));
    BuildingsHelper::Install(m_drones.Get(i));
  }
}

void Scenario::ConfigureMobilityZsps()
{
  NS_LOG_FUNCTION_NOARGS();
  MobilityHelper mobilityZsps;
  auto positionAllocatorZsps = CreateObject<ListPositionAllocator>();

  CONFIGURATOR->GetZspsPosition(positionAllocatorZsps);

  mobilityZsps.SetPositionAllocator(positionAllocatorZsps);
  mobilityZsps.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobilityZsps.Install(m_zsps);
  BuildingsHelper::Install(m_zsps);
}

/*
void Scenario::ConfigureApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  ConfigureApplicationDrones();
  ConfigureApplicationZsps();
}

void Scenario::ConfigureApplicationDrones()
{
  NS_LOG_FUNCTION_NOARGS();

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
  NS_LOG_FUNCTION_NOARGS();

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
*/

} // namespace ns3


int main(int argc, char **argv)
{
  ns3::Scenario scenario(argc, argv);
  scenario.Run();

  return 0;
}