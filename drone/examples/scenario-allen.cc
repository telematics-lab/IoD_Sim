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
/*
 * Dronesim - Scenario "Allen"
 * ===========================
 *
 * === Trivia ===
 * Frances E. Allen
 * became the first female IBM Fellow in 1989. In 2006, she became the first
 * female recipient of the ACM's Turing Award.
 *
 * More Info: https://en.wikipedia.org/wiki/Frances_E._Allen
 *
 *
 * === About this Scenario ===
 * Goals:
 *   - Simple Rendezvous process to know which is the Authority (AP) in a
 *     network.
 *   - Transmit / Receive packets.
 *   - Scale up to N independent drones.
 *
 *
 * == Communication ==
 * Communication is done via IEEE 802.11n @ 2.4 GHz. Each packet is a
 * JSON over UDP, to ensure simple decode/encode operation and fast data
 * exchange without the burden of a connection-oriented communication.
 * Each JSON object MUST have a "cmd" field (as in "command"). Possible
 * list of commands are available in "model/drone-communications.h"
 * as PacketType enum. This communication also has a ACK mechanism to
 * ensure packet reached destination.
 *
 *
 * == Rendezvous ==
 * A rendezvous process is made to get the IP Address of the Authority
 * (i.e. an Access Point) in a network of drones.
 * Each drone signals a HELLO packet and expects a HELLO_ACK with the IP of
 * the Authority. In this way, the drone can easily recognize between a
 * packet coming from an Authority and a packet coming from a common drone.
 *
 *
 * == Server states ==
 * To ease the debug process, a server will have a state associated to it.
 * The state can be CLOSED or LISTEN, as defined in ServerState enum in
 * model/dronesim-server.h.
 *
 *
 * == Drone states ==
 * To ease the debug process and the rendezvous mechanism, a drone can be in
 * CLOSED, HELLO_SENT or CONNECTED state, as defined in ClientState enum in
 * model/dronesim-client.h.
 *
 *
 * === Problems ===
 *  - High-intensive simulation requires time
 *      Suggestions: multithreading
 *  - Spontaneous crash of WifiPhy
 *  - No scalability for AP (it was not a goal)
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

void MonitorRxCallback (Ptr< const Packet > packet,
                        uint16_t channelFreqMhz,
                        WifiTxVector txVector,
                        MpduInfo aMpdu,
                        SignalNoiseDbm signalNoise)
{
    std::cout << "Packet " << packet->GetUid() << " received with RSSI: " << signalNoise.signal << " dBm" << std::endl;
}

void DroneClientTxCallback  (Ptr<const Packet> p)
{
    std::cout << "DroneClient: Packet sent!" << std::endl;
}

/*
 * Organizzazione dei primi 4 layer con scambio messaggi in broadcast.
 */

int main(int argc, char** argv) {
    /*
     * Constant properties
     */
    const bool verboseMode = false;
    const std::string phyMode = "DsssRate1Mbps";
    const int32_t rss = -80;  // dBm
    const Time interPacketInterval = Seconds(1.0);
    uint32_t numDrones = 2;


    CommandLine cmd;
    cmd.AddValue("numDrones", "Number of drones to simulate.", numDrones);
    cmd.Parse(argc, argv);



    /*
     * Logging configuration
     */
    LogComponentEnable("Scenario", LOG_LEVEL_ALL);
    LogComponentEnable("DroneServer", LOG_LEVEL_ALL);
    LogComponentEnable("DroneClient", LOG_LEVEL_ALL);

    // redirect stdout to log file
    std::ofstream out("../results/dronesim-allen.log");
    std::clog.rdbuf(out.rdbuf());

    NS_LOG_INFO("###################");
    NS_LOG_INFO("# Dronesim        #");
    NS_LOG_INFO("# Scenario: Allen #");
    NS_LOG_INFO("###################");



    /*
     * Static configuration of the scenario.
     */
    // disable fragmentation for frames below 2200 bytes
    Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
    // turn off RTS/CTS for frames below 2200 bytes
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
    // Fix non-unicast data rate to be the same as that of unicast
    Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));



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
    if (verboseMode)
        wifi.EnableLogComponents();  // Turn on all Wifi logging

    wifi.SetStandard(WIFI_STANDARD_80211n_2_4GHZ);

    wifiPhy = YansWifiPhyHelper::Default();
    // This is one parameter that matters when using FixedRssLossModel
    // set it to zero; otherwise, gain will be added
    wifiPhy.Set("RxGain", DoubleValue(0));
    // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
    wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    // The below FixedRssLossModel will cause the rss to be fixed regardless
    // of the distance between the two stations, and the transmit power
    wifiChannel.AddPropagationLoss("ns3::FixedRssLossModel", "Rss", DoubleValue(rss));
    wifiPhy.SetChannel(wifiChannel.Create());



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



    /*
     * WiFi Mobility Configuration.
     */
    MobilityHelper mobility;

    // Note that with FixedRssLossModel, the positions below are not
    // used for received signal strength.
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(1.0, 0.0, 0.0));
    positionAlloc->Add(Vector(-1.0, 0.0, 0.0));
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install((NodeContainer) nodes);

    positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(5.0, 0.0, 0.0));
    positionAlloc->Add(Vector(-5.0, 0.0, 0.0));
    mobility.SetPositionAllocator(positionAlloc);
    mobility.Install(ap);



    /*
     * Network Layer configuration.
     */
    InternetStackHelper internet;
    Ipv4AddressHelper ipv4;
    const char *subnetMask = "255.0.0.0";

    internet.Install(nodes);
    internet.Install(ap);

    NS_LOG_INFO("> Assigning IP Addresses.");
    ipv4.SetBase("10.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer i = ipv4.Assign(devices);

    for (uint j = 0; j < devices.GetN(); j++) {
        NS_LOG_INFO("[Node " << devices.Get(j)->GetNode()->GetId()
                    << "] assigned address " << i.GetAddress(j, 0));
    }



    /*
     * Transport Layer Configuration.
     */
    NS_LOG_INFO("> Creating applications for nodes.");
    for (auto node = nodes.Begin(); node != nodes.End(); node++) {
        auto client = CreateObjectWithAttributes<DroneClient>(
            "Ipv4Address", Ipv4AddressValue(i.GetAddress((*node)->GetId ())),
            "Ipv4SubnetMask", Ipv4MaskValue(subnetMask));
        (*node)->AddApplication(client);

        client->SetStartTime(Seconds(1));
    }

    Ptr<DroneServer> server = CreateObjectWithAttributes<DroneServer>(
        "Ipv4Address", Ipv4AddressValue(i.GetAddress(i.GetN() - 1)),
        "Ipv4SubnetMask", Ipv4MaskValue(subnetMask));
    ap.Get(0)->AddApplication(server);

    server->SetStartTime(Seconds(0));



    /*
     * Simulation Configuration.
     */
    //Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx",
    //                              MakeCallback(&MonitorRxCallback));
    Config::ConnectWithoutContext("/NodeList/*/ApplicationList/*/$ns3::DroneClient/Tx",
                                  MakeCallback(&DroneClientTxCallback));

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
    resultsPath << "../results/allen-" << dateTime.str ();

    // Enable Report
    Report::Get ()->Initialize ("allen",
                                dateTime.str (),
                                resultsPath.str ());

    Simulator::Stop(Seconds(120.0));

    Simulator::Run();

    // Report Module needs the simulator context alive to introspect it
    Report::Get ()->Save ();

    Simulator::Destroy();

    auto stop = timer.now();
    auto deltaTime = std::chrono::duration_cast<ms>(stop - start).count();

    server.~Ptr(); // destruction of this object happens very late. Let's force it.
    NS_LOG_INFO("> Simulation terminated. Took " << deltaTime << "ms.");

    std::clog.rdbuf(std::cout.rdbuf());
    out.close();
}
