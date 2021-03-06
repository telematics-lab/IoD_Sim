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
#include <ns3/core-module.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/ipv4-interface-container.h>
#include <ns3/log.h>
#include <ns3/mobility-helper.h>
#include <ns3/net-device-container.h>
#include <ns3/ptr.h>
#include <ns3/ssid.h>
#include <ns3/vector.h>
#include <ns3/yans-wifi-helper.h>
#include <ns3/mobility-model.h>

#include <chrono>
#include <iomanip>

#include <ns3/report.h>

#include <ns3/drone-list.h>
#include <ns3/drone-server.h>
#include <ns3/drone-client.h>
#include <ns3/zsp-list.h>

using namespace ns3;
using ms = std::chrono::duration<float, std::milli>;

NS_LOG_COMPONENT_DEFINE("Scenario");

int main(int argc, char** argv) {
    /*
     * Constant properties
     */
    const std::string phyMode = "DsssRate1Mbps";
    const Time interPacketInterval = Seconds(1.0);
    uint32_t numDrones = 2;
    double simulationDuration = 120.0;

    CommandLine cmd;
    cmd.AddValue("numDrones", "Number of drones to simulate.", numDrones);
    cmd.AddValue("duration", "Duration of the simulation.", simulationDuration);
    cmd.Parse(argc, argv);



    /*
     * Logging configuration
     */
    LogComponentEnable("Scenario", LOG_LEVEL_ALL);
    LogComponentEnable("DroneServer", LOG_LEVEL_ALL);
    LogComponentEnable("DroneClient", LOG_LEVEL_ALL);

    // redirect stdout to log file
    std::ofstream out("../results/dronesim-babbage.log");
    std::clog.rdbuf(out.rdbuf());

    NS_LOG_INFO("#####################");
    NS_LOG_INFO("# Dronesim          #");
    NS_LOG_INFO("# Scenario: Babbage #");
    NS_LOG_INFO("#####################");



    /*
     * Static configuration of the scenario.
     */
    // disable fragmentation for frames below 2200 bytes
    Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
    // turn off RTS/CTS for frames below 2200 bytes
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
    // Fix non-unicast data rate to be the same as that of unicast
    Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

    NS_LOG_DEBUG("> Static configuration OK.");



    /*
     * Nodes configuration.
     */
    NodeContainer nodes;
    NodeContainer ap;
    NetDeviceContainer devices;

    nodes.Create(numDrones);
    ap.Create(1);

    // Register created Drones and ZSPs in /DroneList/ and /ZspList/ respectively
    for (auto drone = nodes.Begin (); drone != nodes.End (); drone++)
        DroneList::Add (*drone);

    for (auto zsp = ap.Begin (); zsp != ap.End (); zsp++)
        ZspList::Add (*zsp);

    NS_LOG_INFO("> Simulating " << numDrones << " drones.");



    /*
     * PHY Layer
     */
    WifiHelper wifi;
    YansWifiPhyHelper wifiPhy;
    YansWifiChannelHelper wifiChannel;

    // The below set of helpers will help us to put together the wifi NICs we want
    // wifi.EnableLogComponents();  // Turn on all Wifi logging

    wifi.SetStandard(WIFI_STANDARD_80211n_2_4GHZ);

    // This is one parameter that matters when using FixedRssLossModel
    // set it to zero; otherwise, gain will be added
    wifiPhy.Set("RxGain", DoubleValue(0));
    // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
    wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel",
                                   "Frequency", DoubleValue(2.4e9) // portante
                                  );
    wifiPhy.SetChannel(wifiChannel.Create());

    NS_LOG_DEBUG("> WiFi PHY OK.");



    /*
     * MAC Layer Configuration.
     */
    // Add a mac and disable rate control
    WifiMacHelper wifiMac;

    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode", StringValue(phyMode),
                                 "ControlMode", StringValue(phyMode));

    // Setup the rest of the mac
    Ssid ssid = Ssid("wifi-default");
    // setup sta.
    wifiMac.SetType("ns3::StaWifiMac",
                    "Ssid", SsidValue(ssid));
    NetDeviceContainer staDevice = wifi.Install(wifiPhy, wifiMac, nodes);
    devices = staDevice;
    // setup ap.
    wifiMac.SetType("ns3::ApWifiMac",
                    "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevices = wifi.Install(wifiPhy, wifiMac, ap);
    devices.Add(apDevices);

    NS_LOG_DEBUG("> WiFi MAC OK.");



    /*
     * Mobility Configuration for Drones.
     */
    MobilityHelper mobilityDrones;
    ObjectFactory position;
    int64_t streamIndex = 0; // used to get consistent random numbers across scenarios

    // Note that with FixedRssLossModel, the positions below are not
    // used for received signal strength.
    position.SetTypeId("ns3::RandomBoxPositionAllocator");
    position.Set("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"));
    position.Set("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"));
    position.Set("Z", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"));

    Ptr<PositionAllocator> positionAllocatorDrones = position.Create()->GetObject<PositionAllocator>();
    streamIndex += positionAllocatorDrones->AssignStreams(streamIndex);

    std::string speed("ns3::ConstantRandomVariable[Constant=10.0]");
    std::string pause("ns3::ConstantRandomVariable[Constant=1.0]");

    mobilityDrones.SetMobilityModel("ns3::RandomWaypointMobilityModel",
                                   "Speed", StringValue(speed),
                                   "Pause", StringValue(pause),
                                   "PositionAllocator", PointerValue(positionAllocatorDrones));

    mobilityDrones.Install(nodes);

    NS_LOG_DEBUG("> Mobility Drones OK.");



    /*
     * Mobility Configuration for APs.
     */
    MobilityHelper mobilityAp;
    Ptr<ListPositionAllocator> positionAllocatorAp = CreateObject<ListPositionAllocator>();

    positionAllocatorAp = CreateObject<ListPositionAllocator>();
    positionAllocatorAp->Add(Vector(0.0, 0.0, 0.0));
    mobilityAp.SetPositionAllocator(positionAllocatorAp);
    mobilityAp.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityAp.Install(ap);

    NS_LOG_DEBUG("> Mobility AP OK.");



    /*
     * Network Layer configuration.
     */
    InternetStackHelper internet;
    Ipv4AddressHelper ipv4;
    const char *subnetMask = "255.0.0.0";

    internet.Install(nodes);
    internet.Install(ap);

    NS_LOG_INFO("> Assigning IP Addresses.");
    ipv4.SetBase("10.0.0.0", subnetMask);
    Ipv4InterfaceContainer i = ipv4.Assign(devices);

    for (uint j = 0; j < devices.GetN(); j++) {
        NS_LOG_INFO("[Node " << devices.Get(j)->GetNode()->GetId()
                    << "] assigned address " << i.GetAddress(j, 0));
    }

    NS_LOG_DEBUG("> Network OK.");



    /*
     * Transport Layer Configuration.
     */
    NS_LOG_INFO("> Creating applications for nodes.");
    for (auto node = nodes.Begin(); node != nodes.End(); node++) {
        Ptr<DroneClient> client = CreateObjectWithAttributes<DroneClient>(
            "Ipv4Address", Ipv4AddressValue(i.GetAddress((*node)->GetId())),
            "Ipv4SubnetMask", Ipv4MaskValue(subnetMask),
            "Duration", DoubleValue(simulationDuration));

        (*node)->AddApplication(client);
        client->SetStartTime(Seconds(1));
    }

    Ptr<DroneServer> server = CreateObjectWithAttributes<DroneServer>(
        "Ipv4Address", Ipv4AddressValue(i.GetAddress(i.GetN() - 1)),
        "Ipv4SubnetMask", Ipv4MaskValue(subnetMask));
    ap.Get(0)->AddApplication(server);

    server->SetStartTime(Seconds(0));

    NS_LOG_DEBUG("> Application Set Up OK.");



    /*
     * Simulation Configuration.
     */
    // Tracing
    wifiPhy.EnablePcap("../results/dronesim", devices);

    AsciiTraceHelper ascii;
    wifiPhy.EnableAsciiAll(ascii.CreateFileStream("../results/PHY_comms.log"));

    NS_LOG_INFO("> Starting Simulation.");
    std::chrono::high_resolution_clock timer;
    auto start = timer.now();
    auto in_time_t = std::chrono::system_clock::to_time_t(start);
    std::ostringstream dateTime;

    dateTime << std::put_time (std::localtime (&in_time_t), "%Y-%m-%d.%H-%M-%S");

    std::ostringstream resultsPath;
    resultsPath << "../results/babbage-" << dateTime.str ();

    // Enable Report
    Report::Get ()->Initialize ("babbage",
                                dateTime.str (),
                                resultsPath.str ());

    Simulator::Stop(Seconds(simulationDuration));

    Simulator::Run();

    // Report Module needs the simulator context alive to introspect it
    Report::Get ()->Save ();

    Simulator::Destroy();

    auto stop = timer.now();
    auto deltaTime = std::chrono::duration_cast<ms>(stop - start).count();

    NS_LOG_INFO("> Simulation terminated. Took " << deltaTime << "ms.");

    std::clog.rdbuf(std::cout.rdbuf());
    out.close();
}
