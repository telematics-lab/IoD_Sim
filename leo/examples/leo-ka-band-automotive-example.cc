#include "ns3/antenna-module.h"
#include "ns3/applications-module.h"
#include "ns3/buildings-helper.h"
#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/geo-constant-velocity-mobility.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/leo-module.h"
#include "ns3/log.h"
#include "ns3/nr-helper.h"
#include "ns3/nr-mac-scheduler-tdma-rr.h"
#include "ns3/nr-module.h"
#include "ns3/nr-pdcp-header.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/packet.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/ppp-header.h"

#include <cmath>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LeoKaBandAutomotiveExample");

static std::ofstream traceFileOutputStream;
static bool logging = true; // whether to enable logging from the simulation

// Map to store transmission times for delay calculation
static std::map<uint32_t, Time> packetTxTimeMap;

// Maps to track gNB-UE connections
static std::map<uint32_t, uint32_t> ueToGnbMap;        // UE node ID -> gNB node ID
static std::map<uint32_t, uint32_t> gnbToSatelliteMap; // gNB net device index -> satellite node ID

// Map to track IP addresses to vehicle node IDs
static std::map<Ipv4Address, uint32_t> ipToVehicleNodeMap; // IP address -> vehicle node ID
static std::map<uint32_t, pair<uint32_t, bool>>
    packetIdVehicleNodeMap; // packet ID -> vehicle node ID, isUplink

// Maps to track transfer times for datarate calculation
static std::map<std::string, Time>
    transferStartTime;                              // Transfer ID -> first packet transmission time
static std::map<std::string, Time> transferEndTime; // Transfer ID -> last packet reception time

static Ipv4Address remoteHostIp;
static uint32_t remoteHostNodeId;

void
ConnectionLogAndTrace(std::string context, Ptr<const Packet> pkt, std::string proto, bool isRx)
{
    Ptr<Packet> packet = pkt->Copy();
    Ipv4Header ipv4Header;
    uint32_t packetId = packet->GetUid();
    uint32_t packetSize = packet->GetSize();
    Time currentTime = Simulator::Now();
    uint32_t vehicleNodeId = 0;
    bool vehicleIdFound = false;
    bool isUplink = false;
    std::string deviceType = "";

    if (context.find("NrGnbNetDevice") != std::string::npos)
    {
        deviceType = "{ Satellite gNB } ";
    }
    else if (context.find("NrUeNetDevice") != std::string::npos)
    {
        deviceType = "{ Vehicle UE } ";
    }

    // Packet trace detection:

    // 1. Old data on packet_id map
    auto it = packetIdVehicleNodeMap.find(packetId);
    if (it != packetIdVehicleNodeMap.end())
    {
        vehicleNodeId = it->second.first;
        isUplink = it->second.second;
        vehicleIdFound = true;
    }

    // 2. IPv4 Header
    if (!vehicleIdFound)
    {
        // Removing possible initial headers
        PppHeader pppHeader;
        packet->RemoveHeader(pppHeader);

        // Use PeekHeader() to try and extract the header
        if (packet->PeekHeader(ipv4Header))
        {
            auto dest = ipv4Header.GetDestination();
            isUplink = (dest == remoteHostIp);

            auto association = ipToVehicleNodeMap.find(isUplink ? ipv4Header.GetSource()
                                                                : ipv4Header.GetDestination());
            if (association != ipToVehicleNodeMap.end())
            {
                vehicleNodeId = association->second;
                vehicleIdFound = true;
            }
        }
    }
    // 3. Context-based extraction
    if (!vehicleIdFound)
    {
        // Parse node ID directly from context string using sscanf
        if (sscanf(context.c_str(), "/NodeList/%u/", &vehicleNodeId) == 1)
        {
            if (ueToGnbMap.find(vehicleNodeId) != ueToGnbMap.end())
            {
                vehicleIdFound = true;
                isUplink = !isRx;
            }
        }
    }

    // Fallback on tracking packet
    if (!vehicleIdFound)
    {
        std::cout << "[" << currentTime.GetSeconds() << "s] " << proto << " "
                  << (isRx ? "RX" : "TX") << ": "
                  << "Unknown Vehicle Node, " << deviceType << "PacketID=" << packetId
                  << ", Context=" << context << std::endl;
        return;
    }

    packetIdVehicleNodeMap[packetId] = make_pair(vehicleNodeId, isUplink);
    std::string transferId = (isUplink ? "ul_" : "dl_") + std::to_string(vehicleNodeId);

    if (isRx)
    {
        // Take the latest time
        transferEndTime[transferId] = currentTime;
    }
    else
    {
        // Take only the first time
        if (transferStartTime.find(transferId) == transferStartTime.end())
        {
            transferStartTime[transferId] = currentTime;
        }
    }
    // Calculate delay if we have the transmission time
    std::string delayStr = "N/A";
    if (isRx)
    {
        auto txTimeMapElement = packetTxTimeMap.find(packetId);
        if (txTimeMapElement != packetTxTimeMap.end())
        {
            Time delay = currentTime - txTimeMapElement->second;
            delayStr = std::to_string(delay.GetMilliSeconds()) + "ms";
        }
    }
    else
    {
        if (packetTxTimeMap.find(packetId) == packetTxTimeMap.end())
        {
            packetTxTimeMap[packetId] = currentTime;
        }
    }
    std::cout << "[" << currentTime.GetSeconds() << "s] " << proto << " " << (isRx ? "RX" : "TX")
              << " " << deviceType << "(" << (isUplink ? "From" : "To") << " Vehicle Node "
              << std::to_string(vehicleNodeId) << "): "
              << "Size=" << packetSize << " bytes"
              << (isRx ? (std::string(", Delay=") + delayStr) : "") << ", PacketID=" << packetId
              << std::endl;
}

void
CourseChange(std::string context, Ptr<const MobilityModel> position)
{
    auto mobility = DynamicCast<const GeoConstantVelocityMobility>(position);
    if (mobility)
    {
        auto pos = mobility->GetPosition(ns3::PositionType::GEOCENTRIC);
        auto geo = mobility->GetPosition(ns3::PositionType::GEOGRAPHIC);
        Ptr<const Node> node = position->GetObject<Node>();
        traceFileOutputStream << Simulator::Now() << "," << node->GetId() << "," << pos.x << ","
                              << pos.y << "," << pos.z << "," << mobility->GetVelocity() << ","
                              << geo.x << "," << geo.y << "," << geo.z << std::endl;
    }
}

void
PacketSinkRxTrace(std::string context, Ptr<const Packet> packet)
{
    ConnectionLogAndTrace(context, packet, "UDP", true);
}

void
UdpClientTxTrace(std::string context, Ptr<const Packet> packet)
{
    ConnectionLogAndTrace(context, packet, "UDP", false);
}

void
PointToPointTxTrace(std::string context, Ptr<const Packet> packet)
{
    ConnectionLogAndTrace(context, packet, "P2P", false);
}

void
PointToPointRxTrace(std::string context, Ptr<const Packet> packet)
{
    ConnectionLogAndTrace(context, packet, "P2P", true);
}

void
TcpSinkRxTrace(std::string context, Ptr<const Packet> packet, const Address& from)
{
    ConnectionLogAndTrace(context, packet, "TCP", true);
}

void
BulkSendTxTrace(std::string context, Ptr<const Packet> packet)
{
    ConnectionLogAndTrace(context, packet, "TCP", false);
}

void
TxDataTrace(std::string context, Ptr<const Packet> p, const ns3::Address& addr)
{
    ConnectionLogAndTrace(context, p, "5G", false);
}

void
RxDataTrace(std::string context, Ptr<const Packet> p)
{
    ConnectionLogAndTrace(context, p, "5G", true);
}

// Function to populate UE-gNB mapping
void
PopulateUeGnbMapping(NetDeviceContainer ueNetDev,
                     NetDeviceContainer gnbNetDev,
                     NodeContainer vehicles,
                     NodeContainer satellites,
                     Ipv4InterfaceContainer ueIpIface)
{
    // Clear existing mappings
    ueToGnbMap.clear();
    gnbToSatelliteMap.clear();

    // Also clear transfer time maps for a fresh start
    transferStartTime.clear();
    transferEndTime.clear();

    // Map gNB net device index to satellite node ID
    for (uint32_t i = 0; i < gnbNetDev.GetN(); ++i)
    {
        Ptr<Node> satellite = gnbNetDev.Get(i)->GetNode();
        gnbToSatelliteMap[i] = satellite->GetId();
    }

    // Populate IP to Vehicle Node mapping
    for (uint32_t i = 0; i < vehicles.GetN(); ++i)
    {
        Ipv4Address vehicleIp = ueIpIface.GetAddress(i);
        uint32_t vehicleNodeId = vehicles.Get(i)->GetId();
        ipToVehicleNodeMap[vehicleIp] = vehicleNodeId;
        std::cout << "Vehicle Node " << vehicleNodeId << " assigned IP: " << vehicleIp << std::endl;
    }

    // Find which gNB each UE is attached to by checking signal strength/distance
    for (uint32_t u = 0; u < vehicles.GetN(); ++u)
    {
        Ptr<Node> ueNode = vehicles.Get(u);
        Ptr<MobilityModel> ueMobility = ueNode->GetObject<MobilityModel>();

        double minDistance = std::numeric_limits<double>::max();
        uint32_t closestGnbIdx = 0;

        for (uint32_t g = 0; g < satellites.GetN(); ++g)
        {
            Ptr<Node> gnbNode = satellites.Get(g);
            Ptr<MobilityModel> gnbMobility = gnbNode->GetObject<MobilityModel>();

            double distance = ueMobility->GetDistanceFrom(gnbMobility);
            if (distance < minDistance)
            {
                minDistance = distance;
                closestGnbIdx = g;
            }
        }

        ueToGnbMap[ueNode->GetId()] = gnbToSatelliteMap[closestGnbIdx];
        std::cout << "UE Node " << ueNode->GetId() << " connected to gNB/Satellite Node "
                  << gnbToSatelliteMap[closestGnbIdx] << " (distance: " << minDistance / 1000.0
                  << " km)" << std::endl;
    }
}

int
main(int argc, char* argv[])
{
    std::string scenario = "NTN-Urban"; // scenario for Ka-band automotive

    // Ka-band frequencies: 30 GHz UL, 20 GHz DL (using 20 GHz as reference)
    double dlFrequency = 20e9; // 20 GHz downlink
    double ulFrequency = 30e9; // 30 GHz uplink
    double bandwidth = 80e6;   // 80 MHz bandwidth for Ka-band

    double vehicleSpeed = 30.0; // m/s
    double vehicleLatitude = 19.5;
    double vehicleLongitude = 1.5;

    // Ka-band specific power settings
    double satTxPower = 33.0; // 33 dBm (2W) as specified
    double ueTxPower = 33.0;  // 33 dBm (2W) as specified

    // Ka-band antenna parameters https://hscc.csie.ncu.edu.tw/38811.pdf
    double satAntennaGainDb = 43.2; // TBD - typical for Ka-band satellite
    double ueAntennaGainDb = 43.2;  // TBD - typical for automotive Ka-band terminal

    double hpbwDegrees = 65.0; // Half Power Beam Width as specified

    // RF parameters
    double noiseFigureDb = 1.2;       // 1.2 dB as specified
    double antennaTemperatureK = 150; // 150 K as specified

    double apertureRadius = 0.3; // Approximate radius for Ka-band automotive
                                 // https://hscc.csie.ncu.edu.tw/38811.pdf page 20

    int numVehicles = 1; // number of vehicles in the simulation
    std::string duration = "420ms";
    std::string traceFile = "";
    std::string satMode = "single";              // Satellite mode
    Time mobilityPrecision = MilliSeconds(1000); // Precision for mobility updates
    bool enableGnb = true;                       // Whether to enable gNB transmission
    std::string appType = "UDP";                 // Application type: UDP, TCP, or TCP-Unlimited
    uint32_t packetSize = 1024;                  // Packet size in bytes
    uint32_t maxPackets = 10;                    // Maximum number of packets
    Time packetInterval = MilliSeconds(10);      // Interval between packets
    auto rnd_seed = time(nullptr);

    CommandLine cmd(__FILE__);
    cmd.AddValue("scenario",
                 "The scenario for the Ka-band automotive simulation. Valid options are: "
                 "NTN-DenseUrban, NTN-Urban, NTN-Suburban, and NTN-Rural",
                 scenario);
    cmd.AddValue("dlFrequency", "The downlink carrier frequency in Hz (Ka-band).", dlFrequency);
    cmd.AddValue("ulFrequency", "The uplink carrier frequency in Hz (Ka-band).", ulFrequency);
    cmd.AddValue("bandwidth", "The total bandwidth in Hz.", bandwidth);
    cmd.AddValue("vehicleSpeed", "Speed of the vehicle in m/s", vehicleSpeed);
    cmd.AddValue("vehicleLatitude", "Initial latitude of the vehicle", vehicleLatitude);
    cmd.AddValue("vehicleLongitude", "Initial longitude of the vehicle", vehicleLongitude);
    cmd.AddValue("numVehicles", "Number of vehicles in the simulation", numVehicles);
    cmd.AddValue("satAntennaGainDb", "The satellite antenna gain in dB", satAntennaGainDb);
    cmd.AddValue("ueAntennaGainDb", "The UE antenna gain in dB", ueAntennaGainDb);
    cmd.AddValue("hpbwDegrees", "Half Power Beam Width in degrees", hpbwDegrees);
    cmd.AddValue("noiseFigureDb", "Noise figure in dB", noiseFigureDb);
    cmd.AddValue("antennaTemperatureK", "Antenna temperature in Kelvin", antennaTemperatureK);
    cmd.AddValue("traceFile", "CSV file to store mobility trace in", traceFile);
    cmd.AddValue("logging", "If set to 0, log components will be disabled.", logging);
    cmd.AddValue("duration", "Duration of the simulation", duration);
    cmd.AddValue("satTxPower", "Satellite transmission power in dBm", satTxPower);
    cmd.AddValue("ueTxPower", "Vehicle UE transmission power in dBm", ueTxPower);
    cmd.AddValue("seed", "Random seed for the simulation", rnd_seed);
    cmd.AddValue("mobilityPrecision", "Precision for mobility updates", mobilityPrecision);
    cmd.AddValue("enableGnb", "Enable gNB transmission (1) or disable (0)", enableGnb);
    cmd.AddValue("appType", "Application type: UDP, TCP, or TCP-Unlimited", appType);
    cmd.AddValue("packetSize", "Packet size in bytes", packetSize);
    cmd.AddValue("maxPackets", "Maximum number of packets to send", maxPackets);
    cmd.AddValue("packetInterval", "Interval between packets", packetInterval);
    cmd.Parse(argc, argv);

    srand(rnd_seed);

    // Ka-band specific channel configuration
    Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod",
                       TimeValue(MilliSeconds(100))); // update the channel at every 100 ms
    Config::SetDefault("ns3::ThreeGppChannelConditionModel::UpdatePeriod",
                       TimeValue(MilliSeconds(0))); // do not update the channel condition

    LeoOrbitNodeHelper orbit;
    orbit.SetPrecision(mobilityPrecision); // Set precision for position updates

    // Create and configure satellites using LEO orbit helper
    NodeContainer satellites;
    if (satMode == "multiple")
    {
        satellites = orbit.Install(LeoOrbit(550, 20, 10, 10)); // Higher altitude for Ka-band
    }
    else if (satMode == "dislocated")
    {
        satellites = orbit.Install(400, 20, 90, 180);
    }
    else if (satMode == "single")
    {
        satellites = orbit.Install(400, 20, 0, 0); // Single satellite at 400 km
    }
    else
    {
        NS_FATAL_ERROR("Invalid satellite mode: " << satMode);
    }

    // Create ground nodes (vehicles) - automotive environment
    NodeContainer vehicles;
    if (numVehicles > 1)
    {
        vehicles.Create(numVehicles); // Create multiple vehicles
        for (uint32_t i = 0; i < vehicles.GetN(); ++i)
        {
            auto vehicle = vehicles.Get(i);
            auto rndLat = (rand() % 180000) / 1000.0 - 90.0;  // Random latitude
            auto rndLon = (rand() % 360000) / 1000.0 - 180.0; // Random longitude
            auto rndAzimuth = (rand() % 360000) / 1000.0;     // Random azimuth

            MobilityHelper mobility;
            mobility.SetMobilityModel("ns3::GeoConstantVelocityMobility",
                                      "InitialLatitude",
                                      DoubleValue(rndLat),
                                      "InitialLongitude",
                                      DoubleValue(rndLon),
                                      "Altitude",
                                      DoubleValue(2),
                                      "Speed",
                                      DoubleValue(vehicleSpeed),
                                      "Azimuth",
                                      DoubleValue(rndAzimuth),
                                      "Precision",
                                      TimeValue(mobilityPrecision));
            mobility.SetPositionAllocator(nullptr);
            mobility.Install(vehicle);
        }
    }
    else
    {
        vehicles.Create(1); // Create a single vehicle
        MobilityHelper mobility;
        mobility.SetMobilityModel("ns3::GeoConstantVelocityMobility",
                                  "InitialLatitude",
                                  DoubleValue(vehicleLatitude),
                                  "InitialLongitude",
                                  DoubleValue(vehicleLongitude),
                                  "Altitude",
                                  DoubleValue(2),
                                  "Speed",
                                  DoubleValue(vehicleSpeed),
                                  "Azimuth",
                                  DoubleValue(0),
                                  "Precision",
                                  TimeValue(mobilityPrecision));
        mobility.SetPositionAllocator(nullptr);
        mobility.Install(vehicles);
    }

    if (traceFile != "")
    {
        traceFileOutputStream.open(traceFile);
        if (!traceFileOutputStream.is_open())
        {
            NS_FATAL_ERROR("Could not open trace file " << traceFile);
        }
        Config::Connect("/NodeList/*/$ns3::MobilityModel/CourseChange",
                        MakeCallback(&CourseChange));
        traceFileOutputStream << "Time,Node,X,Y,Z,Speed,Latitude,Longitude,Altitude" << std::endl;
    }

    /*
     * Default values for the Ka-band simulation
     */
    Config::SetDefault("ns3::NrRlcUm::MaxTxBufferSize", UintegerValue(999999999));

    // Configure NR MAC scheduler parameters for Ka-band
    Config::SetDefault("ns3::NrMacSchedulerNs3::DlCtrlSymbols", UintegerValue(1));
    Config::SetDefault("ns3::NrMacSchedulerNs3::UlCtrlSymbols", UintegerValue(1));

    /*
     * Create NR simulation helpers
     */
    Ptr<NrPointToPointEpcHelper> nrEpcHelper = CreateObject<NrPointToPointEpcHelper>();
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
    nrHelper->SetBeamformingHelper(idealBeamformingHelper);
    nrHelper->SetEpcHelper(nrEpcHelper);

    /*
     * Spectrum configuration for Ka-band
     */
    BandwidthPartInfoPtrVector allBwps;
    CcBwpCreator ccBwpCreator;

    /* Create the configuration for Ka-band
     * Using downlink frequency as reference
     */
    CcBwpCreator::SimpleOperationBandConf bandConfDl(dlFrequency, bandwidth, 1);
    OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc(bandConfDl);

    CcBwpCreator::SimpleOperationBandConf bandConfUl(ulFrequency, bandwidth, 1);
    OperationBandInfo band2 = ccBwpCreator.CreateOperationBandContiguousCc(bandConfUl);

    // Create the channel helper for Ka-band
    Ptr<NrChannelHelper> channelHelper = CreateObject<NrChannelHelper>();
    // Set and configure the channel to the current band
    channelHelper->ConfigureFactories(scenario, "Default", "ThreeGpp");
    channelHelper->AssignChannelsToBands({band1, band2});
    allBwps = CcBwpCreator::GetAllBwps({band1, band2});

    // Configure ideal beamforming method
    idealBeamformingHelper->SetAttribute("BeamformingMethod",
                                         TypeIdValue(DirectPathBeamforming::GetTypeId()));

    // Configure scheduler
    nrHelper->SetSchedulerTypeId(NrMacSchedulerTdmaRR::GetTypeId());

    // Setup antennas arrays for Ka-band
    // Vehicle UE: (M,N,P,Mg,Ng) = (TBD,TBD,2,1,1) - using 2x4 as default
    nrHelper->SetUeAntennaAttribute("NumRows", UintegerValue(2));
    nrHelper->SetUeAntennaAttribute("NumColumns", UintegerValue(4));
    nrHelper->SetUeAntennaAttribute("IsDualPolarized", BooleanValue(true));

    // Satellite gNB: Higher gain directional array
    nrHelper->SetGnbAntennaAttribute("NumRows", UintegerValue(8));
    nrHelper->SetGnbAntennaAttribute("NumColumns", UintegerValue(8));
    nrHelper->SetGnbAntennaAttribute("IsDualPolarized", BooleanValue(true));

    // Configure directional antennas with HPBW = 65 degrees
    // Using CircularApertureAntennaModel for Ka-band circular aperture antennas
    // double apertureRadius = DegreesToRadians(hpbwDegrees); // 65 degrees HPBW

    // Vehicle UE antenna (directional for Ka-band automotive)
    nrHelper->SetUeAntennaAttribute(
        "AntennaElement",
        PointerValue(CreateObjectWithAttributes<CircularApertureAntennaModel>(
            "AntennaMaxGainDb",
            DoubleValue(ueAntennaGainDb),
            "AntennaMinGainDb",
            DoubleValue(-20),
            "AntennaCircularApertureRadius",
            DoubleValue(apertureRadius),
            "OperatingFrequency",
            DoubleValue(dlFrequency))));

    // Satellite gNB antenna (high gain directional)
    nrHelper->SetGnbAntennaAttribute(
        "AntennaElement",
        PointerValue(CreateObjectWithAttributes<CircularApertureAntennaModel>(
            "AntennaMaxGainDb",
            DoubleValue(satAntennaGainDb),
            "AntennaMinGainDb",
            DoubleValue(-30),
            "AntennaCircularApertureRadius",
            DoubleValue(apertureRadius),
            "OperatingFrequency",
            DoubleValue(dlFrequency))));

    nrHelper->SetUePhyAttribute("NoiseFigure", DoubleValue(noiseFigureDb));
    nrHelper->SetUePhyAttribute("TxPower", DoubleValue(ueTxPower));

    nrHelper->SetGnbPhyAttribute("NoiseFigure", DoubleValue(noiseFigureDb));
    nrHelper->SetGnbPhyAttribute("TxPower", DoubleValue(satTxPower));
    nrHelper->SetGnbPhyAttribute("Pattern", StringValue("DL|UL"));

    // install nr net devices
    NetDeviceContainer gnbNetDev = nrHelper->InstallGnbDevice(satellites, allBwps);
    NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice(vehicles, allBwps);

    int64_t randomStream = 1;
    randomStream += nrHelper->AssignStreams(gnbNetDev, randomStream);
    randomStream += nrHelper->AssignStreams(ueNetDev, randomStream);

    InternetStackHelper internet;
    internet.Install(vehicles);
    internet.Install(satellites);

    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = nrEpcHelper->AssignUeIpv4Address(NetDeviceContainer(ueNetDev));

    // Install applications on remote host connected to PGW
    Ptr<Node> pgw = nrEpcHelper->GetPgwNode();

    // Create the remote host
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create(1);
    Ptr<Node> remoteHost = remoteHostContainer.Get(0);
    internet.Install(remoteHostContainer);

    // Create the Internet connection with high-speed backhaul for Ka-band
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
    p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
    p2ph.SetChannelAttribute("Delay", TimeValue(MicroSeconds(10)));
    NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);

    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);

    remoteHostIp = internetIpIfaces.GetAddress(1);
    remoteHostNodeId = remoteHost->GetId();

    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
    remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);

    // assign IP address to UEs, and install applications
    uint16_t dlPortBase = 1234;
    uint16_t ulPortBase = 2234;
    ApplicationContainer clientApps;
    ApplicationContainer serverApps;
    ApplicationContainer ulClientApps; // For uplink traffic
    ApplicationContainer ulServerApps; // For uplink traffic

    if (appType == "UDP")
    {
        std::cout << "Installing UDP applications for Ka-band automotive simulation" << std::endl;

        for (uint32_t u = 0; u < vehicles.GetN(); ++u)
        {
            uint16_t dlPort = dlPortBase + u; // Different port for each vehicle
            uint16_t ulPort = ulPortBase + u; // Different port for each vehicle

            // Downlink: Remote host -> Vehicle (20 GHz)
            UdpServerHelper dlPacketSinkHelper(dlPort);
            serverApps.Add(dlPacketSinkHelper.Install(vehicles.Get(u)));

            UdpClientHelper dlClient(ueIpIface.GetAddress(u), dlPort);
            dlClient.SetAttribute("Interval", TimeValue(packetInterval));
            dlClient.SetAttribute("MaxPackets", UintegerValue(maxPackets));
            dlClient.SetAttribute("PacketSize", UintegerValue(packetSize));
            clientApps.Add(dlClient.Install(remoteHost)); // Remote host sends to vehicle

            // Uplink: Vehicle -> Remote host (30 GHz)
            UdpServerHelper ulPacketSinkHelper(ulPort);
            ulServerApps.Add(ulPacketSinkHelper.Install(remoteHost));

            UdpClientHelper ulClient(remoteHostIp, ulPort); // Remote host IP
            ulClient.SetAttribute("Interval", TimeValue(packetInterval));
            ulClient.SetAttribute("MaxPackets", UintegerValue(maxPackets));
            ulClient.SetAttribute("PacketSize", UintegerValue(packetSize));
            ulClientApps.Add(ulClient.Install(vehicles.Get(u))); // Vehicle sends to remote host

            std::cout << "Vehicle " << u << " - DL Port: " << dlPort << ", UL Port: " << ulPort
                      << std::endl;
        }
    }
    else if (appType == "TCP")
    {
        std::cout << "Installing TCP applications for Ka-band automotive simulation" << std::endl;

        for (uint32_t u = 0; u < vehicles.GetN(); ++u)
        {
            uint16_t dlPort = dlPortBase + u;
            uint16_t ulPort = ulPortBase + u;

            // Downlink: Remote host -> Vehicle
            PacketSinkHelper dlSinkHelper("ns3::TcpSocketFactory",
                                          InetSocketAddress(Ipv4Address::GetAny(), dlPort));
            serverApps.Add(dlSinkHelper.Install(vehicles.Get(u)));

            BulkSendHelper dlBulkSendHelper("ns3::TcpSocketFactory",
                                            InetSocketAddress(ueIpIface.GetAddress(u), dlPort));
            dlBulkSendHelper.SetAttribute("MaxBytes", UintegerValue(packetSize * maxPackets));
            dlBulkSendHelper.SetAttribute("SendSize", UintegerValue(packetSize));
            clientApps.Add(dlBulkSendHelper.Install(remoteHost));

            // Uplink: Vehicle -> Remote host
            PacketSinkHelper ulSinkHelper("ns3::TcpSocketFactory",
                                          InetSocketAddress(Ipv4Address::GetAny(), ulPort));
            ulServerApps.Add(ulSinkHelper.Install(remoteHost));

            BulkSendHelper ulBulkSendHelper("ns3::TcpSocketFactory",
                                            InetSocketAddress(remoteHostIp, ulPort));
            ulBulkSendHelper.SetAttribute("MaxBytes", UintegerValue(packetSize * maxPackets));
            ulBulkSendHelper.SetAttribute("SendSize", UintegerValue(packetSize));
            ulClientApps.Add(ulBulkSendHelper.Install(vehicles.Get(u)));

            std::cout << "Vehicle " << u << " - DL Port: " << dlPort << ", UL Port: " << ulPort
                      << std::endl;
        }
    }
    else if (appType == "TCP-Unlimited")
    {
        std::cout << "Installing TCP Unlimited applications for Ka-band automotive simulation"
                  << std::endl;

        for (uint32_t u = 0; u < vehicles.GetN(); ++u)
        {
            uint16_t dlPort = dlPortBase + u;
            uint16_t ulPort = ulPortBase + u;

            // Downlink: Remote host -> Vehicle
            PacketSinkHelper dlSinkHelper("ns3::TcpSocketFactory",
                                          InetSocketAddress(Ipv4Address::GetAny(), dlPort));
            serverApps.Add(dlSinkHelper.Install(vehicles.Get(u)));

            BulkSendHelper dlBulkSendHelper("ns3::TcpSocketFactory",
                                            InetSocketAddress(ueIpIface.GetAddress(u), dlPort));
            dlBulkSendHelper.SetAttribute("MaxBytes", UintegerValue(0)); // 0 = unlimited
            dlBulkSendHelper.SetAttribute("SendSize", UintegerValue(packetSize));
            clientApps.Add(dlBulkSendHelper.Install(remoteHost));

            // Uplink: Vehicle -> Remote host
            PacketSinkHelper ulSinkHelper("ns3::TcpSocketFactory",
                                          InetSocketAddress(Ipv4Address::GetAny(), ulPort));
            ulServerApps.Add(ulSinkHelper.Install(remoteHost));

            BulkSendHelper ulBulkSendHelper("ns3::TcpSocketFactory",
                                            InetSocketAddress(remoteHostIp, ulPort));
            ulBulkSendHelper.SetAttribute("MaxBytes", UintegerValue(0)); // 0 = unlimited
            ulBulkSendHelper.SetAttribute("SendSize", UintegerValue(packetSize));
            ulClientApps.Add(ulBulkSendHelper.Install(vehicles.Get(u)));

            std::cout << "Vehicle " << u << " - DL Port: " << dlPort << ", UL Port: " << ulPort
                      << " (Unlimited TCP transfer)" << std::endl;
        }
    }
    else
    {
        NS_FATAL_ERROR("Invalid application type: " << appType
                                                    << ". Must be UDP, TCP, or TCP-Unlimited.");
    }

    // attach UEs to the closest gNB
    nrHelper->AttachToClosestGnb(ueNetDev, gnbNetDev);

    // Populate UE-gNB mapping for statistics
    PopulateUeGnbMapping(ueNetDev, gnbNetDev, vehicles, satellites, ueIpIface);

    if (logging)
    {
        // Connect trace sources for packet tracking
        if (appType == "UDP")
        {
            // Trace UDP application layer
            Config::Connect("/NodeList/*/ApplicationList/*/$ns3::UdpClient/Tx",
                            MakeCallback(&UdpClientTxTrace));
            Config::Connect("/NodeList/*/ApplicationList/*/$ns3::UdpServer/Rx",
                            MakeCallback(&PacketSinkRxTrace));
        }
        else if (appType == "TCP" || appType == "TCP-Unlimited")
        {
            // Trace TCP application layer
            Config::Connect("/NodeList/*/ApplicationList/*/$ns3::BulkSendApplication/Tx",
                            MakeCallback(&BulkSendTxTrace));
            Config::Connect("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",
                            MakeCallback(&TcpSinkRxTrace));
        }

        // Trace Point-to-Point devices (backhaul)
        Config::Connect("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacTx",
                        MakeCallback(&PointToPointTxTrace));
        Config::Connect("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/MacRx",
                        MakeCallback(&PointToPointRxTrace));

        // Trace NR devices
        Config::Connect("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/Tx",
                        MakeCallback(&TxDataTrace));
        Config::Connect("/NodeList/*/DeviceList/*/$ns3::NrGnbNetDevice/Tx",
                        MakeCallback(&TxDataTrace));
        Config::Connect("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/Rx",
                        MakeCallback(&RxDataTrace));
        Config::Connect("/NodeList/*/DeviceList/*/$ns3::NrGnbNetDevice/Rx",
                        MakeCallback(&RxDataTrace));
    }

    // start server and client apps
    serverApps.Start(Seconds(0.1)); // Earlier start
    clientApps.Start(Seconds(0.2)); // Start transmission quickly after attachment
    serverApps.Stop(Time(duration) - Seconds(0.01));
    clientApps.Stop(Time(duration) - Seconds(0.1)); // Reduced stop margin

    ulServerApps.Start(Seconds(0.1)); // Start uplink servers
    ulClientApps.Start(Seconds(0.3)); // Start uplink clients slightly later
    ulServerApps.Stop(Time(duration) - Seconds(0.01));
    ulClientApps.Stop(Time(duration) - Seconds(0.1));

    // enable the traces provided by the nr module
    nrHelper->EnableTraces();

    Simulator::Schedule(Seconds(0.1), [appType]() {
        std::cout << "============= Starting Ka-band automotive " << appType << " simulation "
                  << std::endl;
    });

    Simulator::Stop(Time(duration));
    std::cout << "============= Starting Ka-band automotive simulation for " << duration
              << " =============" << std::endl;

    Simulator::Run();

    // Print Ka-band specific statistics
    std::cout << "\n============= Ka-BAND AUTOMOTIVE TRAFFIC STATISTICS =============" << std::endl;
    std::cout << "Application Type: " << appType << std::endl;
    std::cout << "Antenna Type: Directional (HPBW=" << hpbwDegrees << "°, Circular Polarization)"
              << std::endl;
    std::cout << "DL Frequency: " << dlFrequency / 1e9 << " GHz" << std::endl;
    std::cout << "UL Frequency: " << ulFrequency / 1e9 << " GHz" << std::endl;
    std::cout << "Bandwidth: " << bandwidth / 1e6 << " MHz" << std::endl;
    std::cout << "Sat-TxPower: " << satTxPower << " dBm (2W)" << std::endl;
    std::cout << "UE-TxPower: " << ueTxPower << " dBm (2W)" << std::endl;
    std::cout << "Noise Figure: " << noiseFigureDb << " dB" << std::endl;
    std::cout << "Antenna Temperature: " << antennaTemperatureK << " K" << std::endl;
    std::cout << "Satellite Distribution: " << satMode << std::endl;
    std::cout << "Packet Size: " << packetSize << " bytes" << std::endl;
    if (appType != "TCP-Unlimited")
    {
        std::cout << "Max Packets: " << maxPackets << std::endl;
        std::cout << "Packet Interval: " << packetInterval.GetMilliSeconds() << "ms" << std::endl;
    }
    else
    {
        std::cout << "Mode: Continuous TCP transfer (unlimited)" << std::endl;
    }

    std::cout << "\n--- DOWNLINK STATISTICS (Remote Host -> Vehicle, " << dlFrequency / 1e9
              << " GHz) ---" << std::endl;

    if (appType == "UDP")
    {
        for (uint32_t u = 0; u < serverApps.GetN(); ++u)
        {
            Ptr<UdpServer> serverApp = serverApps.Get(u)->GetObject<UdpServer>();
            auto receivedPackets = serverApp->GetReceived();
            auto totalBytes = receivedPackets * packetSize;
            uint32_t vehicleNodeId = serverApp->GetNode()->GetId();
            uint32_t gnbNodeId = ueToGnbMap[vehicleNodeId];

            std::cout << "Vehicle Node " << vehicleNodeId << " (via gNB/Satellite Node "
                      << gnbNodeId << ")"
                      << " - " << receivedPackets << "/" << maxPackets << " packets"
                      << " (" << (100.0 * receivedPackets / maxPackets) << "%), "
                      << "Total Bytes: " << totalBytes
                      << (receivedPackets == maxPackets ? " ✅" : " ❌") << std::endl;
        }
    }
    else if (appType == "TCP" || appType == "TCP-Unlimited")
    {
        for (uint32_t u = 0; u < serverApps.GetN(); ++u)
        {
            Ptr<PacketSink> sinkApp = serverApps.Get(u)->GetObject<PacketSink>();
            auto totalBytes = sinkApp->GetTotalRx();
            uint32_t vehicleNodeId = sinkApp->GetNode()->GetId();
            uint32_t gnbNodeId = ueToGnbMap[vehicleNodeId];

            // Calculate datarate based on actual transfer time
            double dataRateMbps = 0.0;
            std::string transferTimeInfo = "N/A";

            std::string transferId = "dl_" + std::to_string(vehicleNodeId);
            if (transferStartTime.find(transferId) != transferStartTime.end() &&
                transferEndTime.find(transferId) != transferEndTime.end())
            {
                Time transferDuration = transferEndTime[transferId] - transferStartTime[transferId];
                if (transferDuration.GetSeconds() > 0)
                {
                    dataRateMbps = (totalBytes * 8.0) / (transferDuration.GetSeconds() * 1e6);
                    transferTimeInfo = std::to_string(transferDuration.GetSeconds()) + "s";
                }
            }

            std::cout << "Vehicle Node " << vehicleNodeId << " (via gNB/Satellite Node "
                      << gnbNodeId << ")"
                      << " - Total Bytes: " << totalBytes << ", Transfer Time: " << transferTimeInfo
                      << ", Data Rate: " << dataRateMbps << " Mbps";

            if (appType == "TCP")
            {
                std::cout << " (" << (100.0 * totalBytes / (packetSize * maxPackets)) << "%) "
                          << (totalBytes >= packetSize * maxPackets ? " ✅" : " ❌");
            }
            std::cout << std::endl;
        }
    }

    std::cout << "\n--- UPLINK STATISTICS (Vehicle -> Remote Host, " << ulFrequency / 1e9
              << " GHz) ---" << std::endl;

    if (appType == "UDP")
    {
        for (uint32_t u = 0; u < ulServerApps.GetN(); ++u)
        {
            Ptr<UdpServer> ulServerApp = ulServerApps.Get(u)->GetObject<UdpServer>();
            auto receivedPackets = ulServerApp->GetReceived();
            auto totalBytes = receivedPackets * packetSize;

            uint32_t vehicleNodeId = vehicles.Get(u)->GetId();
            uint32_t gnbNodeId = ueToGnbMap[vehicleNodeId];

            std::cout << "Remote Host - Packets from Vehicle Node " << vehicleNodeId
                      << " (via gNB/Satellite Node " << gnbNodeId << "): " << receivedPackets << "/"
                      << maxPackets << " packets"
                      << " (" << (100.0 * receivedPackets / maxPackets) << "%), "
                      << "Total Bytes: " << totalBytes
                      << (receivedPackets == maxPackets ? " ✅" : " ❌") << std::endl;
        }
    }
    else if (appType == "TCP" || appType == "TCP-Unlimited")
    {
        for (uint32_t u = 0; u < ulServerApps.GetN(); ++u)
        {
            Ptr<PacketSink> ulSinkApp = ulServerApps.Get(u)->GetObject<PacketSink>();
            auto totalBytes = ulSinkApp->GetTotalRx();

            uint32_t vehicleNodeId = vehicles.Get(u)->GetId();
            uint32_t gnbNodeId = ueToGnbMap[vehicleNodeId];

            // Calculate datarate based on actual transfer time
            double dataRateMbps = 0.0;
            std::string transferTimeInfo = "N/A";

            std::string transferId = "ul_" + std::to_string(vehicleNodeId);
            if (transferStartTime.find(transferId) != transferStartTime.end() &&
                transferEndTime.find(transferId) != transferEndTime.end())
            {
                Time transferDuration = transferEndTime[transferId] - transferStartTime[transferId];
                if (transferDuration.GetSeconds() > 0)
                {
                    dataRateMbps = (totalBytes * 8.0) / (transferDuration.GetSeconds() * 1e6);
                    transferTimeInfo = std::to_string(transferDuration.GetSeconds()) + "s";
                }
            }

            std::cout << "Remote Host - Bytes from Vehicle Node " << vehicleNodeId
                      << " (via gNB/Satellite Node " << gnbNodeId << "): " << totalBytes
                      << ", Transfer Time: " << transferTimeInfo << ", Data Rate: " << dataRateMbps
                      << " Mbps";

            if (appType == "TCP")
            {
                std::cout << " (" << (100.0 * totalBytes / (packetSize * maxPackets)) << "%) "
                          << (totalBytes >= packetSize * maxPackets ? " ✅" : " ❌");
            }
            std::cout << std::endl;
        }
    }

    std::cout << "\n--- Ka-BAND TRANSFER TIMING DETAILS ---" << std::endl;
    std::cout << "Transfer Start Times:" << std::endl;
    for (const auto& pair : transferStartTime)
    {
        std::cout << "  " << pair.first << ": " << pair.second.GetSeconds() << "s" << std::endl;
    }
    std::cout << "Transfer End Times:" << std::endl;
    for (const auto& pair : transferEndTime)
    {
        std::cout << "  " << pair.first << ": " << pair.second.GetSeconds() << "s" << std::endl;
    }

    std::cout << "IP to Vehicle Node Mapping:" << std::endl;
    for (const auto& pair : ipToVehicleNodeMap)
    {
        Ipv4Address ip(pair.first);
        std::cout << "  IP " << ip << " -> Vehicle Node " << pair.second << std::endl;
    }

    Simulator::Destroy();

    if (traceFileOutputStream.is_open())
    {
        traceFileOutputStream.close();
    }

    return 0;
}