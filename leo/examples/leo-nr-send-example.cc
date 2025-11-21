#include "ns3/antenna-module.h"
#include "ns3/applications-module.h"
#include "ns3/buildings-helper.h"
#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/leo-ground-node-helper.h"
#include "ns3/leo-module.h"
#include "ns3/log.h"
#include "ns3/mobility-module.h"
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

NS_LOG_COMPONENT_DEFINE("LeoNrSendExample");

static std::ofstream traceFileOutputStream;
static bool logging = true; // whether to enable logging from the simulation, another option is by
                            // exporting the NS_LOG environment variable

// Map to store transmission times for delay calculation
static std::map<uint32_t, Time> packetTxTimeMap;

// Maps to track gNB-UE connections
static std::map<uint32_t, uint32_t> ueToGnbMap;        // UE node ID -> gNB node ID
static std::map<uint32_t, uint32_t> gnbToSatelliteMap; // gNB net device index -> satellite node ID

// Map to track IP addresses to car node IDs
static std::map<Ipv4Address, uint32_t> ipToCarNodeMap; // IP address (as uint32_t) -> car node ID
static std::map<uint32_t, pair<uint32_t, bool>>
    packetIdCarNodeMap; // car packet ID -> car node ID, isUplink

// Maps to track transfer times for datarate calculation
// Key format: "direction_carIndex" (e.g., "dl_0", "ul_0"), the id is always relative to the car
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
    uint32_t carNodeId = 0;
    bool carIdFound = false;
    bool isUplink = false;
    std::string deviceType = "";

    if (context.find("NrGnbNetDevice") != std::string::npos)
    {
        deviceType = "{ Satellite gNB } ";
    }
    else if (context.find("NrUeNetDevice") != std::string::npos)
    {
        deviceType = "{ Car UE } ";
    }

    // Packet trace detection:

    // 1. Old data on packet_id map
    auto it = packetIdCarNodeMap.find(packetId);
    if (it != packetIdCarNodeMap.end())
    {
        carNodeId = it->second.first;
        isUplink = it->second.second;
        carIdFound = true;
    }

    // 2. IPv4 Header
    if (!carIdFound)
    {
        // Removing possible initial headers
        PppHeader pppHeader;
        packet->RemoveHeader(pppHeader);

        // Use PeekHeader() to try and extract the header
        if (packet->PeekHeader(ipv4Header))
        {
            auto dest = ipv4Header.GetDestination();
            isUplink = (dest == remoteHostIp);

            auto association = ipToCarNodeMap.find(isUplink ? ipv4Header.GetSource()
                                                            : ipv4Header.GetDestination());
            if (association != ipToCarNodeMap.end())
            {
                carNodeId = association->second;
                carIdFound = true;
            }
        }
    }
    // 3. Context-based extraction
    if (!carIdFound)
    {
        // Parse node ID directly from context string using sscanf
        if (sscanf(context.c_str(), "/NodeList/%u/", &carNodeId) == 1)
        {
            if (ueToGnbMap.find(carNodeId) != ueToGnbMap.end())
            {
                carIdFound = true;
                isUplink = !isRx;
            }
        }
    }

    // Fallback on tracking packet
    if (!carIdFound)
    {
        std::cout << "[" << currentTime.GetSeconds() << "s] " << proto << " "
                  << (isRx ? "RX" : "TX") << ": "
                  << "Unknown Car Node, " << deviceType << "PacketID=" << packetId
                  << ", Context=" << context << std::endl;
        return;
    }

    packetIdCarNodeMap[packetId] = make_pair(carNodeId, isUplink);
    std::string transferId = (isUplink ? "ul_" : "dl_") + std::to_string(carNodeId);

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
              << " " << deviceType << "(" << (isUplink ? "From" : "To") << " Car Node "
              << std::to_string(carNodeId) << "): "
              << "Size=" << packetSize << " bytes"
              << (isRx ? (std::string(", Delay=") + delayStr) : "") << ", PacketID=" << packetId
              << std::endl;
}

void
CourseChange(std::string context, Ptr<const MobilityModel> position)
{
    auto mobility = DynamicCast<const GeocentricConstantPositionMobilityModel>(position);
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
                     NodeContainer cars,
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

    // Populate IP to Car Node mapping
    for (uint32_t i = 0; i < cars.GetN(); ++i)
    {
        Ipv4Address carIp = ueIpIface.GetAddress(i);
        uint32_t carNodeId = cars.Get(i)->GetId();
        ipToCarNodeMap[carIp] = carNodeId;
        std::cout << "Car Node " << carNodeId << " assigned IP: " << carIp << std::endl;
    }

    // Find which gNB each UE is attached to by checking signal strength/distance
    for (uint32_t u = 0; u < cars.GetN(); ++u)
    {
        Ptr<Node> ueNode = cars.Get(u);
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
    std::string scenario = "NTN-Suburban"; // scenario
    double frequency = 28e9;               // central frequency
    double bandwidth = 400e6;              // bandwidth

    double carSpeed = 30.0; // m/s
    double carLatitude = 19.5;
    double carLongitude = 1.5;
    double satTxPower = 40; // txPower
    double ueTxPower = 23;  // txPower
    // Satellite parameters
    double satAntennaGainDb = 58.5; // dB
    // UE Parameters
    double ueAntennaGainDb = 39.7; // dB
    int numCars = 1;               // number of cars in the simulation
    std::string duration = "420ms";
    std::string traceFile = "";
    std::string antennaModel = "IsotropicAntennaModel"; // Antenna model
    std::string satMode = "single";                     // Satellite mode, either single or multiple
    Time mobilityPrecision = MilliSeconds(1000);        // Precision for mobility updates
    bool enableGnb = true;                              // Whether to enable gNB transmission
    std::string appType = "UDP"; // Application type: UDP, TCP, or TCP-Unlimited
    uint32_t packetSize = 1024;  // Packet size in bytes
    uint32_t maxPackets = 10;    // Maximum number of packets (not used for TCP-Unlimited)
    Time packetInterval = MilliSeconds(10); // Interval between packets (not used for TCP-Unlimited)
    auto rnd_seed = time(nullptr);

    CommandLine cmd(__FILE__);
    cmd.AddValue("scenario",
                 "The scenario for the simulation. Valid options are: "
                 "NTN-DenseUrban, NTN-Urban, NTN-Suburban, and NTN-Rural",
                 scenario);
    cmd.AddValue("frequency", "The central carrier frequency in Hz.", frequency);
    cmd.AddValue("bandwidth", "The total bandwidth in Hz.", bandwidth);
    cmd.AddValue("carSpeed", "Speed of the car in m/s", carSpeed);
    cmd.AddValue("carLatitude", "Initial latitude of the car (only with 1 car)", carLatitude);
    cmd.AddValue("carLongitude", "Initial longitude of the car (only with 1 car)", carLongitude);
    cmd.AddValue("numCars", "Number of cars in the simulation", numCars);
    cmd.AddValue("antennaModel",
                 "The antenna model to use for the simulation. "
                 "Valid options are: IsotropicAntennaModel, CircularApertureAntennaModel, "
                 "ParabolicAntennaModel, "
                 "ThreeGppAntennaModel, and CosineAntennaModel",
                 antennaModel);
    cmd.AddValue("satMode",
                 "The satellite mode to use for the simulation. 'single', 'multiple', 'dislocated",
                 satMode);
    cmd.AddValue("satAntennaGainDb", "The satellite antenna gain in dB", satAntennaGainDb);
    cmd.AddValue("ueAntennaGainDb", "The UE antenna gain in dB", ueAntennaGainDb);
    cmd.AddValue("traceFile", "CSV file to store mobility trace in", traceFile);
    cmd.AddValue("logging", "If set to 0, log components will be disabled.", logging);
    cmd.AddValue("duration", "Duration of the simulation in seconds", duration);
    cmd.AddValue("traceFile", "CSV file to store mobility trace in", traceFile);
    cmd.AddValue("satTxPower", "Transmission power in dBm", satTxPower);
    cmd.AddValue("carTxPower", "Transmission power in dBm", ueTxPower);
    cmd.AddValue("seed", "Random seed for the simulation (default: current time)", rnd_seed);
    cmd.AddValue("mobilityPrecision",
                 "Precision for mobility updates (e.g., 50ms, 100ms, etc.)",
                 mobilityPrecision);
    cmd.AddValue("enableGnb", "Enable gNB transmission (1) or disable (0)", enableGnb);
    cmd.AddValue("appType", "Application type: UDP, TCP, or TCP-Unlimited", appType);
    cmd.AddValue("packetSize", "Packet size in bytes", packetSize);
    cmd.AddValue("maxPackets",
                 "Maximum number of packets to send (not used for TCP-Unlimited)",
                 maxPackets);
    cmd.AddValue("packetInterval",
                 "Interval between packets (e.g., 10ms, 100ms) - not used for TCP-Unlimited",
                 packetInterval);
    cmd.Parse(argc, argv);

    srand(rnd_seed);

    Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod",
                       TimeValue(MilliSeconds(100))); // update the channel at every 10 ms
    Config::SetDefault("ns3::ThreeGppChannelConditionModel::UpdatePeriod",
                       TimeValue(MilliSeconds(0))); // do not update the channel condition

    LeoOrbitNodeHelper orbit;

    orbit.SetPrecision(mobilityPrecision); // Set precision for position updates
    // Create and configure satellites using LEO orbit helper
    NodeContainer satellites;
    if (satMode == "multiple")
    {
        satellites = orbit.Install(LeoOrbit(300, 20, 10, 10));
    }
    else if (satMode == "dislocated")
    {
        satellites = orbit.Install(300, 20, 90, 180);
    }
    else if (satMode == "single")
    {
        satellites = orbit.Install(300, 20, 0, 0);
    }
    else
    {
        NS_FATAL_ERROR("Invalid satellite mode: " << satMode);
    }
    // Create ground nodes (cars)
    NodeContainer cars;
    if (numCars > 1)
    {
        cars.Create(numCars); // Create multiple cars
        for (uint32_t i = 0; i < cars.GetN(); ++i)
        {
            auto car = cars.Get(i);
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
                                      DoubleValue(carSpeed),
                                      "Azimuth",
                                      DoubleValue(rndAzimuth),
                                      "Precision",
                                      TimeValue(mobilityPrecision));
            mobility.SetPositionAllocator(nullptr);
            mobility.Install(car);
        }
    }
    else
    {
        cars.Create(1); // Create a single car
        MobilityHelper mobility;
        mobility.SetMobilityModel("ns3::GeoConstantVelocityMobility",
                                  "InitialLatitude",
                                  DoubleValue(carLatitude),
                                  "InitialLongitude",
                                  DoubleValue(carLongitude),
                                  "Altitude",
                                  DoubleValue(2),
                                  "Speed",
                                  DoubleValue(carSpeed),
                                  "Azimuth",
                                  DoubleValue(0),
                                  "Precision",
                                  TimeValue(mobilityPrecision));
        mobility.SetPositionAllocator(nullptr);
        mobility.Install(cars);
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
     * Default values for the simulation. We are progressively removing all
     * the instances of SetDefault, but we need it for legacy code (LTE)
     */
    Config::SetDefault("ns3::NrRlcUm::MaxTxBufferSize", UintegerValue(999999999));

    // Configure NR MAC scheduler parameters
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
     * Spectrum configuration. We create a single operational band and configure the scenario.
     */

    BandwidthPartInfoPtrVector allBwps;
    CcBwpCreator ccBwpCreator;
    const uint8_t numCcPerBand = 1; // in this example we have a single band, and that band is
                                    // composed of a single component carrier

    /* Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
     * a single BWP per CC and a single BWP in CC.
     *
     * Hence, the configured spectrum is:
     *
     * |---------------Band---------------|
     * |---------------CC-----------------|
     * |---------------BWP----------------|
     */
    CcBwpCreator::SimpleOperationBandConf bandConf(frequency, bandwidth, numCcPerBand);
    OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc(bandConf);
    // Create the channel helper
    Ptr<NrChannelHelper> channelHelper = CreateObject<NrChannelHelper>();
    // Set and configure the channel to the current band
    channelHelper->ConfigureFactories(scenario, "Default", "ThreeGpp");
    channelHelper->AssignChannelsToBands({band});
    allBwps = CcBwpCreator::GetAllBwps({band});

    // Configure ideal beamforming method
    idealBeamformingHelper->SetAttribute("BeamformingMethod",
                                         TypeIdValue(DirectPathBeamforming::GetTypeId()));

    // Configure scheduler
    nrHelper->SetSchedulerTypeId(NrMacSchedulerTdmaRR::GetTypeId());

    // Setup antennas arrays
    nrHelper->SetUeAntennaAttribute("NumRows", UintegerValue(2));
    nrHelper->SetUeAntennaAttribute("NumColumns", UintegerValue(4));
    nrHelper->SetGnbAntennaAttribute("NumRows", UintegerValue(8));
    nrHelper->SetGnbAntennaAttribute("NumColumns", UintegerValue(8));

    if (antennaModel == "IsotropicAntennaModel")
    {
        /*
        No Attributes are defined for this type.
        */
        nrHelper->SetUeAntennaAttribute(
            "AntennaElement",
            PointerValue(
                CreateObjectWithAttributes<IsotropicAntennaModel>("Gain",
                                                                  DoubleValue(ueAntennaGainDb))));
        nrHelper->SetGnbAntennaAttribute(
            "AntennaElement",
            PointerValue(
                CreateObjectWithAttributes<IsotropicAntennaModel>("Gain",
                                                                  DoubleValue(satAntennaGainDb))));
    }
    else if (antennaModel == "CircularApertureAntennaModel")
    {
        /*
        AntennaCircularApertureRadius: The radius of the aperture of the antenna, in meters
            Set with class: ns3::DoubleValue
            Underlying type: double 0:1.79769e+308
            Initial value: 0.5
            Flags: constructwrite
            Support level: SUPPORTED
        AntennaMaxGainDb: The maximum gain value in dB of the antenna
            Set with class: ns3::DoubleValue
            Underlying type: double 0:1.79769e+308
            Initial value: 1
            Flags: constructwrite
            Support level: SUPPORTED
        AntennaMinGainDb: The minimum gain value in dB of the antenna
            Set with class: ns3::DoubleValue
            Underlying type: double -1.79769e+308:1.79769e+308
            Initial value: -100
            Flags: constructwrite
            Support level: SUPPORTED
        ForceGainBounds: Force GetGainDb to [AntennaMinGainDb, AntennaMaxGainDb] range
            Set with class: ns3::BooleanValue
            Underlying type: bool
            Initial value: true
            Flags: constructwriteread
            Support level: SUPPORTED
        OperatingFrequency: The operating frequency in Hz of the antenna
            Set with class: ns3::DoubleValue
            Underlying type: double 0:1.79769e+308
            Initial value: 2e+09
            Flags: constructwrite
            Support level: SUPPORTED
        */
        nrHelper->SetUeAntennaAttribute(
            "AntennaElement",
            PointerValue(CreateObjectWithAttributes<CircularApertureAntennaModel>(
                "AntennaMaxGainDb",
                DoubleValue(ueAntennaGainDb)
                //,"OperatingFrequency", DoubleValue(1e6)
                /*
                    Diminuire questa frequenza fa aumentare il datarate
                    e l'effetto della distanza rimane comunque percepito
                */
                )));
        nrHelper->SetGnbAntennaAttribute(
            "AntennaElement",
            PointerValue(CreateObjectWithAttributes<CircularApertureAntennaModel>(
                "AntennaMaxGainDb",
                DoubleValue(satAntennaGainDb)
                //,"OperatingFrequency", DoubleValue(1e6)
                )));
    }
    else if (antennaModel == "ParabolicAntennaModel")
    {
        /*
        Beamwidth: The 3dB beamwidth (degrees)
            Set with class: ns3::DoubleValue
            Underlying type: double 0:180
            Initial value: 60
            Flags: construct write read
        Orientation: The angle (degrees) that expresses the orientation of the antenna on the x-y
        plane relative to the x axis Set with class: ns3::DoubleValue Underlying type: double
        -360:360 Initial value: 0 Flags: construct write read MaxAttenuation: The maximum
        attenuation (dB) of the antenna radiation pattern. Set with class: ns3::DoubleValue
            Underlying type: double -1.79769e+308:1.79769e+308
            Initial value: 20
            Flags: construct write read
        */
        nrHelper->SetUeAntennaAttribute(
            "AntennaElement",
            PointerValue(CreateObjectWithAttributes<ParabolicAntennaModel>(
                //"Orientation", DoubleValue(85)
                )));
        nrHelper->SetGnbAntennaAttribute(
            "AntennaElement",
            PointerValue(CreateObjectWithAttributes<ParabolicAntennaModel>(
                //"Orientation", DoubleValue(85)
                )));
    }
    else if (antennaModel == "ThreeGppAntennaModel")
    {
        /*
        No Attributes are defined for this type.
        */
        nrHelper->SetUeAntennaAttribute(
            "AntennaElement",
            PointerValue(CreateObjectWithAttributes<ThreeGppAntennaModel>()));
        nrHelper->SetGnbAntennaAttribute(
            "AntennaElement",
            PointerValue(CreateObjectWithAttributes<ThreeGppAntennaModel>()));
    }
    else if (antennaModel == "CosineAntennaModel")
    {
        /*
        Beamwidth: The 3dB beamwidth (degrees)
            Set with class: ns3::DoubleValue
            Underlying type: double 0:180
            Initial value: 60
            Flags: construct write read
        Orientation: The angle (degrees) that expresses the orientation of the antenna on the x-y
        plane relative to the x axis Set with class: ns3::DoubleValue Underlying type: double
        -360:360 Initial value: 0 Flags: construct write read MaxGain: The gain (dB) at the antenna
        boresight (the direction of maximum gain) Set with class: ns3::DoubleValue Underlying type:
        double -1.79769e+308:1.79769e+308 Initial value: 0 Flags: construct write read
        */
        nrHelper->SetUeAntennaAttribute("AntennaElement",
                                        PointerValue(CreateObjectWithAttributes<CosineAntennaModel>(
                                            "MaxGain",
                                            DoubleValue(ueAntennaGainDb))));
        nrHelper->SetGnbAntennaAttribute(
            "AntennaElement",
            PointerValue(
                CreateObjectWithAttributes<CosineAntennaModel>("MaxGain",
                                                               DoubleValue(satAntennaGainDb))));
    }
    else
    {
        NS_FATAL_ERROR("Invalid antenna model specified: " << antennaModel);
    }

    // install nr net devices
    NetDeviceContainer gnbNetDev = nrHelper->InstallGnbDevice(satellites, allBwps);
    NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice(cars, allBwps);

    int64_t randomStream = 1;
    randomStream += nrHelper->AssignStreams(gnbNetDev, randomStream);
    randomStream += nrHelper->AssignStreams(ueNetDev, randomStream);

    for (uint32_t i = 0; i < gnbNetDev.GetN(); ++i)
    {
        // Set gNB transmission power based on enableGnb parameter
        if (enableGnb)
        {
            nrHelper->GetGnbPhy(gnbNetDev.Get(i), 0)->SetTxPower(satTxPower);
        }
        else
        {
            nrHelper->GetGnbPhy(gnbNetDev.Get(i), 0)
                ->SetTxPower(-1000); // Effectively disable transmission
        }
    }

    for (uint32_t i = 0; i < ueNetDev.GetN(); ++i)
    {
        nrHelper->GetUePhy(ueNetDev.Get(i), 0)->SetTxPower(ueTxPower);
    }

    InternetStackHelper internet;
    internet.Install(cars);
    internet.Install(satellites);

    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = nrEpcHelper->AssignUeIpv4Address(NetDeviceContainer(ueNetDev));

    // Install applications on remote host (satellite) connected to PGW (Packet Data Network
    // Gateway)
    Ptr<Node> pgw = nrEpcHelper->GetPgwNode();

    // Create the remote host
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create(1);
    Ptr<Node> remoteHost = remoteHostContainer.Get(0);
    internet.Install(remoteHostContainer);

    // Create the Internet
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
        std::cout << "Installing UDP applications" << std::endl;

        for (uint32_t u = 0; u < cars.GetN(); ++u)
        {
            uint16_t dlPort = dlPortBase + u; // Different port for each car
            uint16_t ulPort = ulPortBase + u; // Different port for each car

            // Downlink: Remote host -> Car
            UdpServerHelper dlPacketSinkHelper(dlPort);
            serverApps.Add(dlPacketSinkHelper.Install(cars.Get(u)));

            UdpClientHelper dlClient(ueIpIface.GetAddress(u), dlPort);
            dlClient.SetAttribute("Interval", TimeValue(packetInterval));
            dlClient.SetAttribute("MaxPackets", UintegerValue(maxPackets));
            dlClient.SetAttribute("PacketSize", UintegerValue(packetSize));
            clientApps.Add(dlClient.Install(remoteHost)); // Remote host sends to car

            // Uplink: Car -> Remote host
            UdpServerHelper ulPacketSinkHelper(ulPort);
            ulServerApps.Add(ulPacketSinkHelper.Install(remoteHost));

            UdpClientHelper ulClient(remoteHostIp, ulPort); // Remote host IP
            ulClient.SetAttribute("Interval", TimeValue(packetInterval));
            ulClient.SetAttribute("MaxPackets", UintegerValue(maxPackets));
            ulClient.SetAttribute("PacketSize", UintegerValue(packetSize));
            ulClientApps.Add(ulClient.Install(cars.Get(u))); // Car sends to remote host

            std::cout << "Car " << u << " - DL Port: " << dlPort << ", UL Port: " << ulPort
                      << std::endl;
        }
    }
    else if (appType == "TCP")
    {
        std::cout << "Installing TCP applications" << std::endl;

        for (uint32_t u = 0; u < cars.GetN(); ++u)
        {
            uint16_t dlPort = dlPortBase + u; // Different port for each car
            uint16_t ulPort = ulPortBase + u; // Different port for each car

            // Downlink: Remote host -> Car
            PacketSinkHelper dlSinkHelper("ns3::TcpSocketFactory",
                                          InetSocketAddress(Ipv4Address::GetAny(), dlPort));
            serverApps.Add(dlSinkHelper.Install(cars.Get(u)));

            BulkSendHelper dlBulkSendHelper("ns3::TcpSocketFactory",
                                            InetSocketAddress(ueIpIface.GetAddress(u), dlPort));
            dlBulkSendHelper.SetAttribute("MaxBytes", UintegerValue(packetSize * maxPackets));
            dlBulkSendHelper.SetAttribute("SendSize", UintegerValue(packetSize));
            clientApps.Add(dlBulkSendHelper.Install(remoteHost)); // Remote host sends to car

            // Uplink: Car -> Remote host
            PacketSinkHelper ulSinkHelper("ns3::TcpSocketFactory",
                                          InetSocketAddress(Ipv4Address::GetAny(), ulPort));
            ulServerApps.Add(ulSinkHelper.Install(remoteHost));

            BulkSendHelper ulBulkSendHelper("ns3::TcpSocketFactory",
                                            InetSocketAddress(remoteHostIp, ulPort));
            ulBulkSendHelper.SetAttribute("MaxBytes", UintegerValue(packetSize * maxPackets));
            ulBulkSendHelper.SetAttribute("SendSize", UintegerValue(packetSize));
            ulClientApps.Add(ulBulkSendHelper.Install(cars.Get(u))); // Car sends to remote host

            std::cout << "Car " << u << " - DL Port: " << dlPort << ", UL Port: " << ulPort
                      << std::endl;
        }
    }
    else if (appType == "TCP-Unlimited")
    {
        std::cout << "Installing TCP Unlimited applications (continuous data transfer)"
                  << std::endl;

        for (uint32_t u = 0; u < cars.GetN(); ++u)
        {
            uint16_t dlPort = dlPortBase + u; // Different port for each car
            uint16_t ulPort = ulPortBase + u; // Different port for each car

            // Downlink: Remote host -> Car
            PacketSinkHelper dlSinkHelper("ns3::TcpSocketFactory",
                                          InetSocketAddress(Ipv4Address::GetAny(), dlPort));
            serverApps.Add(dlSinkHelper.Install(cars.Get(u)));

            BulkSendHelper dlBulkSendHelper("ns3::TcpSocketFactory",
                                            InetSocketAddress(ueIpIface.GetAddress(u), dlPort));
            dlBulkSendHelper.SetAttribute("MaxBytes", UintegerValue(0)); // 0 = unlimited
            dlBulkSendHelper.SetAttribute("SendSize", UintegerValue(packetSize));
            clientApps.Add(dlBulkSendHelper.Install(remoteHost)); // Remote host sends to car

            // Uplink: Car -> Remote host
            PacketSinkHelper ulSinkHelper("ns3::TcpSocketFactory",
                                          InetSocketAddress(Ipv4Address::GetAny(), ulPort));
            ulServerApps.Add(ulSinkHelper.Install(remoteHost));

            BulkSendHelper ulBulkSendHelper("ns3::TcpSocketFactory",
                                            InetSocketAddress(remoteHostIp, ulPort));
            ulBulkSendHelper.SetAttribute("MaxBytes", UintegerValue(0)); // 0 = unlimited
            ulBulkSendHelper.SetAttribute("SendSize", UintegerValue(packetSize));
            ulClientApps.Add(ulBulkSendHelper.Install(cars.Get(u))); // Car sends to remote host

            std::cout << "Car " << u << " - DL Port: " << dlPort << ", UL Port: " << ulPort
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
    PopulateUeGnbMapping(ueNetDev, gnbNetDev, cars, satellites, ueIpIface);

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
        // Similarly for the gNB side or reception:
        Config::Connect("/NodeList/*/DeviceList/*/$ns3::NrGnbNetDevice/Tx",
                        MakeCallback(&TxDataTrace));
        Config::Connect("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/Rx",
                        MakeCallback(&RxDataTrace));
        // Similarly for the gNB side or reception:
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
        std::cout << "============= Starting " << appType << " applications " << std::endl;
    });

    Simulator::Stop(Time(duration));
    std::cout << "============= Starting simulation for " << duration
              << " =============" << std::endl;

    Simulator::Run();

    // Print statistics
    std::cout << "\n============= TRAFFIC STATISTICS =============" << std::endl;
    std::cout << "Application Type: " << appType << std::endl;
    std::cout << "Antenna Type: " << antennaModel << std::endl;
    std::cout << "Sat-TxPower: " << satTxPower << " dBm" << std::endl;
    std::cout << "Car-TxPower: " << ueTxPower << " dBm" << std::endl;
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

    std::cout << "\n--- DOWNLINK STATISTICS (Remote Host -> Car) ---" << std::endl;

    if (appType == "UDP")
    {
        for (uint32_t u = 0; u < serverApps.GetN(); ++u)
        {
            Ptr<UdpServer> serverApp = serverApps.Get(u)->GetObject<UdpServer>();
            auto receivedPackets = serverApp->GetReceived();
            auto totalBytes = receivedPackets * packetSize;
            uint32_t carNodeId = serverApp->GetNode()->GetId();
            uint32_t gnbNodeId = ueToGnbMap[carNodeId];

            std::cout << "Car Node " << carNodeId << " (via gNB/Satellite Node " << gnbNodeId << ")"
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
            uint32_t carNodeId = sinkApp->GetNode()->GetId();
            uint32_t gnbNodeId = ueToGnbMap[carNodeId];

            // Calculate datarate based on actual transfer time
            double dataRateMbps = 0.0;
            std::string transferTimeInfo = "N/A";

            std::string transferId = "dl_" + std::to_string(carNodeId);
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

            std::cout << "Car Node " << carNodeId << " (via gNB/Satellite Node " << gnbNodeId << ")"
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

    std::cout << "\n--- UPLINK STATISTICS (Car -> Remote Host) ---" << std::endl;

    if (appType == "UDP")
    {
        for (uint32_t u = 0; u < ulServerApps.GetN(); ++u)
        {
            Ptr<UdpServer> ulServerApp = ulServerApps.Get(u)->GetObject<UdpServer>();
            auto receivedPackets = ulServerApp->GetReceived();
            auto totalBytes = receivedPackets * packetSize;

            // Find which car sent to this uplink server (car index u)
            uint32_t carNodeId = cars.Get(u)->GetId();
            uint32_t gnbNodeId = ueToGnbMap[carNodeId];

            std::cout << "Remote Host - Packets from Car Node " << carNodeId
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

            // Find which car sent to this uplink server (car index u)
            uint32_t carNodeId = cars.Get(u)->GetId();
            uint32_t gnbNodeId = ueToGnbMap[carNodeId];

            // Calculate datarate based on actual transfer time
            double dataRateMbps = 0.0;
            std::string transferTimeInfo = "N/A";

            std::string transferId = "ul_" + std::to_string(carNodeId);
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

            std::cout << "Remote Host - Bytes from Car Node " << carNodeId
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

    std::cout << "\n--- TRANSFER TIMING DETAILS ---" << std::endl;
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

    std::cout << "IP to Car Node Mapping:" << std::endl;
    for (const auto& pair : ipToCarNodeMap)
    {
        Ipv4Address ip(pair.first);
        std::cout << "  IP " << ip << " -> Car Node " << pair.second << std::endl;
    }

    Simulator::Destroy();

    if (traceFileOutputStream.is_open())
    {
        traceFileOutputStream.close();
    }
}