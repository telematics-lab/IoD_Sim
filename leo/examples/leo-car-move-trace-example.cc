#include "ns3/core-module.h"
#include "ns3/geocentric-mobility-model.h"
#include "ns3/leo-ground-node-helper.h"
#include "ns3/leo-module.h"
#include "ns3/mobility-module.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LeoCarMoveTraceExample");

static std::ofstream traceFileOutputStream;
static std::ostream* traceStream;

void
CourseChange(std::string context, Ptr<const MobilityModel> position)
{
    auto mobility = DynamicCast<const GeocentricMobilityModel>(position);
    if (mobility)
    {
        auto pos = mobility->GetPosition(ns3::PositionType::GEOCENTRIC);
        auto geo = mobility->GetPosition(ns3::PositionType::GEOGRAPHIC);
        Ptr<const Node> node = position->GetObject<Node>();
        *traceStream << Simulator::Now() << "," << node->GetId() << "," << pos.x << "," << pos.y
                     << "," << pos.z << "," << mobility->GetVelocity() << "," << geo.x << ","
                     << geo.y << "," << geo.z << std::endl;
    }
}

int
main(int argc, char* argv[])
{
    CommandLine cmd;
    std::string orbitFile;
    std::string traceFile = "";
    std::string duration = "60s";
    double carSpeed = 30.0; // m/s
    double carLatitude = 0.0;
    double carLongitude = 0.0;
    double carAzimuth = 45.0; // Nordest

    cmd.AddValue("traceFile", "CSV file to store mobility trace in", traceFile);
    cmd.AddValue("precision", "ns3::GeoLeoOrbitMobility::Precision");
    cmd.AddValue("duration", "Duration of the simulation in seconds", duration);
    cmd.AddValue("carSpeed", "Speed of the car in m/s", carSpeed);
    cmd.AddValue("carLatitude", "Initial latitude of the car", carLatitude);
    cmd.AddValue("carLongitude", "Initial longitude of the car", carLongitude);
    cmd.AddValue("carAzimuth", "Initial azimuth of the car in degrees", carAzimuth);
    cmd.Parse(argc, argv);

    LeoOrbitNodeHelper orbit;
    NodeContainer satellites = orbit.Install(LeoOrbit(400, 20, 1, 10));

    // Create ground nodes (cars)
    NodeContainer cars;
    cars.Create(1); // Create 1 car

    // Install GeoConstantVelocityMobility on cars
    MobilityHelper mobility;
    mobility.SetMobilityModel(
        "ns3::GeoConstantVelocityMobility",
        "InitialLatitude",
        DoubleValue(carLatitude),
        "InitialLongitude",
        DoubleValue(carLongitude),
        "Altitude",
        DoubleValue(0.0),
        "Speed",
        DoubleValue(carSpeed),
        "Azimuth",
        DoubleValue(carAzimuth),
        "Precision",
        TimeValue(Seconds(1.0)),
        "EarthSpheroidType",
        EnumValue(GeographicPositions::EarthSpheroidType::SPHERE)); // Update every second
    mobility.SetPositionAllocator(nullptr);
    mobility.Install(cars);

    Config::Connect("/NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback(&CourseChange));

    // redirect cout if traceFile is specified
    if (!traceFile.empty())
    {
        traceFileOutputStream.open(traceFile);
        if (!traceFileOutputStream.is_open())
        {
            NS_FATAL_ERROR("Could not open trace file " << traceFile);
            return 1;
        }
        traceStream = &traceFileOutputStream;
    }
    else
    {
        // Use cout when no file is specified
        traceStream = &std::cout;
    }
    *traceStream << "Time,Node,X,Y,Z,Speed,Latitude,Longitude,Altitude" << std::endl;

    Simulator::Stop(Time(duration));
    Simulator::Run();
    Simulator::Destroy();

    if (!traceFile.empty())
    {
        traceFileOutputStream.close();
    }
}
