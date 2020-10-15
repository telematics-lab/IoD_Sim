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
 *
 * Source: lte-lena-simple-epc.cc
 * <https://www.nsnam.org/doxygen/lena-simple-epc_8cc_source.html>
 * Copyright (c) 2011-2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Authors: Jaume Nin <jaume.nin@cttc.cat>
 *          Manuel Requena <manuel.requena@cttc.es>
 * Edited to use drones and a host as a zsp in a Scenario singleton
 * Author: Michele Abruzzese <michele.abruzzese93@gmail.com>
 */

#include <chrono>

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/lte-module.h"
//#include "ns3/gtk-config-store.h"

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
  NodeContainer _drones, _antennas, _hosts;
  NetDeviceContainer _ueDevs, _enbDevs, _remoteDevs;
  Ptr<LteHelper> lteHelper;
  Ptr<PointToPointEpcHelper> epcHelper;
  Ipv4InterfaceContainer _hostIpInterfaces, _ueIpInterfaces;
  Ipv4Mask _ipv4Mask;

  void ConfigureSimulator();
  void ConfigureMobility();
  void ConfigureMobilityDrones();
  void ConfigureMobilityZsps();
  void ConfigureProtocol();
  void ConfigurePhy();
  void ConfigureMac();
  void ConfigureRlc();
  void ConfigureNetwork();
  void ConfigureApplication();
  void ConfigureApplicationDrones();
  void ConfigureApplicationHosts();
};


Scenario::~Scenario() {}

Scenario::Scenario(int argc, char **argv)
{
  CONFIGURATOR->Initialize(argc, argv, "LteScenarioBasic");

  _hosts.Create(1);
  _drones.Create(CONFIGURATOR->GetDronesN());
  _antennas.Create(CONFIGURATOR->GetZspsN());

  for (auto zsp = _antennas.Begin(); zsp != _antennas.End(); zsp++)
    ZspList::Add(*zsp);

  for (auto drone = _drones.Begin(); drone != _drones.End(); drone++)
    DroneList::Add(*drone);

  ConfigureMobility();
  ConfigureProtocol();
  ConfigureApplication();
  ConfigureSimulator();
}

void Scenario::ConfigureSimulator()
{
  NS_LOG_FUNCTION_NOARGS();

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  //std::stringstream phyTraceLog, pcapLog;
  //AsciiTraceHelper ascii;

  //phyTraceLog << CONFIGURATOR->GetResultsPath() << "-phy.log";
  //pcapLog << CONFIGURATOR->GetResultsPath() << "-host";

  Report::Get()->Initialize("LTE-Easley", CONFIGURATOR->GetCurrentDateTime(), CONFIGURATOR->GetResultsPath());

  Simulator::Stop(Seconds(CONFIGURATOR->GetDuration()));
}

void Scenario::Run()
{
  NS_LOG_FUNCTION_NOARGS();

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


void Scenario::ConfigureMobility()
{
  NS_LOG_FUNCTION_NOARGS();
  ConfigureMobilityZsps();
  ConfigureMobilityDrones();
}

void Scenario::ConfigureMobilityDrones()
{
  NS_LOG_FUNCTION_NOARGS();

  MobilityHelper mobilityDrones;

  for (uint32_t i = 0; i < _drones.GetN(); i++)
  {
    mobilityDrones.SetMobilityModel("ns3::ParametricSpeedDroneMobilityModel",
        "SpeedCoefficients", SpeedCoefficientsValue(CONFIGURATOR->GetDroneSpeedCoefficients(i)),
        "FlightPlan", FlightPlanValue(CONFIGURATOR->GetDroneFlightPlan(i)),
        "CurveStep", DoubleValue(CONFIGURATOR->GetCurveStep()));
    mobilityDrones.Install(_drones.Get(i));
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
  mobilityZsps.Install(_antennas);
}


void Scenario::ConfigureProtocol()
{
  NS_LOG_FUNCTION_NOARGS();

  Config::SetDefault("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue(true));
  Config::SetDefault("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue(true));
  Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(true));
  Config::SetDefault("ns3::LteHelper::UsePdschForCqiGeneration", BooleanValue(true));


  lteHelper = CreateObject<LteHelper>();

  //ConfigurePhy();
  // Include options for Carrier Aggregation (ns3::LteHelper::UseCa)
  //lteHelper->SetAttribute("PathlossModel", StringValue("ns3::Cost231PropagationLossModel"));

  //ConfigureMac();
  epcHelper = CreateObject<PointToPointEpcHelper>();
  lteHelper->SetEpcHelper(epcHelper);

  InternetStackHelper internet;
  internet.Install(_hosts);
  internet.Install(_drones);

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
  p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
  p2ph.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10)));

  Ptr<Node> remoteHost = _hosts.Get(0);
  _remoteDevs = p2ph.Install(epcHelper->GetPgwNode(), remoteHost);

  //lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");
  //lteHelper->SetSchedulerAttribute("HarqEnabled", BooleanValue(true));
  //lteHelper->SetSchedulerAttribute("CqiTimerThreshold", UintegerValue(1000));

  _ueDevs = lteHelper->InstallUeDevice(_drones);
  _enbDevs = lteHelper->InstallEnbDevice(_antennas);

  //ConfigureNetwork();
  _ipv4Mask = Ipv4Mask("255.0.0.0");

  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase("1.0.0.0", _ipv4Mask);
  _hostIpInterfaces = ipv4h.Assign(_remoteDevs);

  // check if necessary
  //Ptr<Node> remoteHost = _hosts.Get(0);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
  remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), _ipv4Mask, 1);



  NS_LOG_INFO("> Assigning IP Addresses:");
  _ueIpInterfaces = epcHelper->AssignUeIpv4Address(NetDeviceContainer(_ueDevs));

  // assigning the default gateway to each ue
  for (uint32_t i = 0; i < _drones.GetN(); i++)
  {
    Ptr<Node> ueNode = _drones.Get(i);

    Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting(ueNode->GetObject<Ipv4>());
    ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);

    auto netDev_id = ueNode->GetId();
    auto address = _ueIpInterfaces.GetAddress(i);
    NS_LOG_INFO("[Node " << netDev_id << "] assigned address: " << address);
  }

  lteHelper->Attach(_ueDevs);

  // 0 is localhost, hence 1 is the only device added (the remote host)
  //Ipv4Address remoteHostAddress = _hostIpInterfaces.GetAddress(1);

  //ConfigureRlc();
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
  lteHelper->ActivateDataRadioBearer(_ueDevs, bearer);

  lteHelper->EnableTraces();
}

void Scenario::ConfigurePhy()
{
  NS_LOG_FUNCTION_NOARGS();

  // Include options for Carrier Aggregation (ns3::LteHelper::UseCa)

  lteHelper->SetAttribute("PathlossModel", StringValue("ns3::Cost231PropagationLossModel"));
}

void Scenario::ConfigureMac()
{
  NS_LOG_FUNCTION_NOARGS();

  epcHelper = CreateObject<PointToPointEpcHelper>();
  lteHelper->SetEpcHelper(epcHelper);

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
  p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
  p2ph.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10)));

  Ptr<Node> remoteHost = _hosts.Get(0);
  _remoteDevs = p2ph.Install(epcHelper->GetPgwNode(), remoteHost);

  lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");
  lteHelper->SetSchedulerAttribute("HarqEnabled", BooleanValue(true));
  //lteHelper->SetSchedulerAttribute("CqiTimerThreshold", UintegerValue(1000));

  _ueDevs = lteHelper->InstallUeDevice(_drones);
  _enbDevs = lteHelper->InstallEnbDevice(_antennas);

  lteHelper->Attach(_ueDevs);
}

void Scenario::ConfigureRlc()
{
  NS_LOG_FUNCTION_NOARGS();

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
  lteHelper->ActivateDataRadioBearer(_ueDevs, bearer);
}

void Scenario::ConfigureNetwork()
{
  NS_LOG_FUNCTION_NOARGS();

  _ipv4Mask = Ipv4Mask("255.0.0.0");

  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase("1.0.0.0", _ipv4Mask);
  _hostIpInterfaces = ipv4h.Assign(_remoteDevs);

  // 0 is localhost, hence 1 is the only device added (the remote host)
  //Ipv4Address remoteHostAddress = _hostIpInterfaces.GetAddress(1);

  // check if necessary
  Ptr<Node> remoteHost = _hosts.Get(0);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
  remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), _ipv4Mask, 1);

  InternetStackHelper internet;
  internet.Install(_hosts);
  internet.Install(_drones);

  NS_LOG_INFO("> Assigning IP Addresses:");
  _ueIpInterfaces = epcHelper->AssignUeIpv4Address(NetDeviceContainer(_ueDevs));

  // assigning the default gateway to each ue
  for (uint32_t i = 0; i < _drones.GetN(); i++)
  {
    Ptr<Node> ueNode = _drones.Get(i);

    Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting(ueNode->GetObject<Ipv4>());
    ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);

    auto netDev_id = ueNode->GetId();
    auto address = _ueIpInterfaces.GetAddress(i);
    NS_LOG_INFO("[Node " << netDev_id << "] assigned address: " << address);
  }
}


void Scenario::ConfigureApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  ConfigureApplicationDrones();
  ConfigureApplicationHosts();
}

void Scenario::ConfigureApplicationDrones()
{
  NS_LOG_FUNCTION_NOARGS();

  for (uint32_t i = 0; i < _drones.GetN(); i++)
  {
    auto client = CreateObjectWithAttributes<DroneClient>(
        "Ipv4Address", Ipv4AddressValue(_ueIpInterfaces.GetAddress(i)),
        "Ipv4SubnetMask", Ipv4MaskValue(_ipv4Mask),
        "Duration", DoubleValue(CONFIGURATOR->GetDuration()));

    double droneAppStartTime = CONFIGURATOR->GetDroneApplicationStartTime(i);
    client->SetStartTime(Seconds(droneAppStartTime));
    double droneAppStopTime = CONFIGURATOR->GetDroneApplicationStopTime(i);
    client->SetStopTime(Seconds(droneAppStopTime));

    _drones.Get(i)->AddApplication(client);

    NS_LOG_LOGIC("[Node " << i << "] active from " << droneAppStartTime << "s to " << droneAppStopTime << "s.");
  }
}

void Scenario::ConfigureApplicationHosts()
{
  NS_LOG_FUNCTION_NOARGS();

  auto server = CreateObjectWithAttributes<DroneServer>(
        "Ipv4Address", Ipv4AddressValue(_hostIpInterfaces.GetAddress(1)),
        "Ipv4SubnetMask", Ipv4MaskValue(_ipv4Mask));

  double zspAppStartTime = CONFIGURATOR->GetZspApplicationStartTime(0);
  server->SetStartTime(Seconds(zspAppStartTime));
  double zspAppStopTime = CONFIGURATOR->GetZspApplicationStopTime(0);
  server->SetStopTime(Seconds(zspAppStopTime));

  _hosts.Get(0)->AddApplication(server);

  NS_LOG_LOGIC("[Server 0] active from " << zspAppStartTime << "s to " << zspAppStopTime << "s.");
}

} // namespace ns3


int main(int argc, char **argv)
{
  ns3::LogComponentEnable("Scenario", ns3::LOG_LEVEL_ALL);

  ns3::Scenario scenario(argc, argv);

  scenario.Run();

  return 0;
}