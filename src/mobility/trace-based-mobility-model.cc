#include "trace-based-mobility-model.h"

#include "trace-reader.h"

#include "ns3/geographic-positions.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TraceBasedMobilityModel");

NS_OBJECT_ENSURE_REGISTERED(TraceBasedMobilityModel);

TypeId
TraceBasedMobilityModel::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::TraceBasedMobilityModel")
            .SetParent<GeocentricMobilityModel>()
            .SetGroupName("Mobility")
            .AddConstructor<TraceBasedMobilityModel>()
            .AddAttribute("TraceFile",
                          "Path to the trace file (tar.gz)",
                          StringValue(""),
                          MakeStringAccessor(&TraceBasedMobilityModel::SetTraceFile),
                          MakeStringChecker())
            .AddAttribute("DeviceId",
                          "Device ID in the trace file",
                          StringValue(""),
                          MakeStringAccessor(&TraceBasedMobilityModel::SetDeviceId),
                          MakeStringChecker())
            .AddAttribute("Precision",
                          "The time precision with which to compute position updates. 0 means "
                          "arbitrary precision",
                          TimeValue(Seconds(1)),
                          MakeTimeAccessor(&TraceBasedMobilityModel::SetPrecision),
                          MakeTimeChecker());
    return tid;
}

TraceBasedMobilityModel::TraceBasedMobilityModel()
    : m_precision(Seconds(1)),
      m_lastTime(Seconds(0)),
      m_firstUpdate(true)
{
}

TraceBasedMobilityModel::~TraceBasedMobilityModel()
{
}

void
TraceBasedMobilityModel::SetTraceFile(std::string file)
{
    m_traceFile = file;
    if (!m_deviceId.empty())
    {
        Vector initialPosition;
        if (TraceReader::Get()->Register(m_traceFile, m_deviceId, initialPosition))
        {
            m_position = initialPosition;
            NS_LOG_INFO("Set initial position for device "
                        << m_deviceId << " to (" << initialPosition.x << ", " << initialPosition.y
                        << ", " << initialPosition.z << ")");
        }
        else
        {
            NS_FATAL_ERROR("Failed to register device " << m_deviceId << " in trace file "
                                                        << m_traceFile);
        }
    }
}

void
TraceBasedMobilityModel::SetDeviceId(std::string deviceId)
{
    m_deviceId = deviceId;
    if (!m_traceFile.empty())
    {
        Vector initialPosition;
        if (TraceReader::Get()->Register(m_traceFile, m_deviceId, initialPosition))
        {
            m_position = initialPosition;
            NS_LOG_INFO("Set initial position for device "
                        << m_deviceId << " to (" << initialPosition.x << ", " << initialPosition.y
                        << ", " << initialPosition.z << ")");
        }
        else
        {
            NS_FATAL_ERROR("Failed to register device " << m_deviceId << " in trace file "
                                                        << m_traceFile);
        }
    }
}

void
TraceBasedMobilityModel::SetPrecision(Time precision)
{
    m_precision = precision;
    Update();
}

Time
TraceBasedMobilityModel::GetPrecision() const
{
    return m_precision;
}

void
TraceBasedMobilityModel::Update()
{
    Vector newGeoPos = CalcPosition(Simulator::Now());
    Time now = Simulator::Now();

    if (m_firstUpdate)
    {
        m_velocity = Vector(0, 0, 0);
        m_firstUpdate = false;
    }
    else if (now > m_lastTime)
    {
        double dt = (now - m_lastTime).GetSeconds();
        if (dt > 0)
        {
            // Convert both to Cartesian for velocity calculation
            Vector newCartesianPos =
                GeographicPositions::GeographicToCartesianCoordinates(newGeoPos.x,
                                                                      newGeoPos.y,
                                                                      newGeoPos.z,
                                                                      GetEarthSpheroidType());
            Vector oldCartesianPos =
                GeographicPositions::GeographicToCartesianCoordinates(m_position.x,
                                                                      m_position.y,
                                                                      m_position.z,
                                                                      GetEarthSpheroidType());

            // Calculate velocity taking earth curvature into account
            Vector v_secant = (newCartesianPos - oldCartesianPos) * (1.0 / dt);

            // Decompose into radial (vertical) and tangential (horizontal) components
            Vector normal = oldCartesianPos;
            double radius = normal.GetLength();
            if (radius > 1e-6)
            {
                normal = normal * (1.0 / radius); // Normalize
            }
            else
            {
                normal = Vector(0, 0, 1); // Fallback
            }

            Vector v_rad =
                normal * (v_secant.x * normal.x + v_secant.y * normal.y + v_secant.z * normal.z);
            Vector v_tan_chord = v_secant - v_rad;

            double chord_speed = v_tan_chord.GetLength();

            // Correct horizontal speed using arc length
            double chord_dist = chord_speed * dt;
            if (chord_dist > 1e-9 && radius > 1e-6)
            {
                double theta = 2.0 * std::asin(chord_dist / (2.0 * radius));
                double arc_dist = radius * theta;
                double arc_speed = arc_dist / dt;

                // Scale tangential velocity
                if (chord_speed > 1e-9)
                {
                    v_tan_chord = v_tan_chord * (arc_speed / chord_speed);
                }
            }

            m_velocity = v_tan_chord + v_rad;
        }
    }

    m_position = newGeoPos;
    m_lastTime = now;

    NotifyCourseChange();
    if (m_updateEvent.IsPending())
    {
        Simulator::Cancel(m_updateEvent);
        m_updateEvent = EventId();
    }
    if (m_precision > Seconds(0))
    {
        m_updateEvent = Simulator::Schedule(m_precision, &TraceBasedMobilityModel::Update, this);
    }
}

Vector
TraceBasedMobilityModel::CalcPosition(Time t) const
{
    Vector geoPos; // lat, lon, alt
    if (TraceReader::Get()->GetNextPoint(m_traceFile, m_deviceId, t, geoPos))
    {
        return geoPos;
    }
    return m_position;
}

Vector
TraceBasedMobilityModel::DoGetPosition(PositionType type) const
{
    Vector geoPos;
    if (m_precision == Time(0))
    {
        geoPos = CalcPosition(Simulator::Now());
    }
    else
    {
        geoPos = m_position;
    }

    switch (type)
    {
    case PositionType::GEOCENTRIC:
        return GeographicPositions::GeographicToCartesianCoordinates(geoPos.x,
                                                                     geoPos.y,
                                                                     geoPos.z,
                                                                     GetEarthSpheroidType());
    case PositionType::GEOGRAPHIC:
        return geoPos;
    case PositionType::TOPOCENTRIC:
        return GeographicPositions::GeographicToTopocentricCoordinates(
            geoPos,
            GetGeographicReferencePoint(),
            GetEarthSpheroidType());
    case PositionType::PROJECTED:
        return GeographicPositions::GeographicToProjectedCoordinates(geoPos,
                                                                     GetEarthSpheroidType());
    default:
        NS_FATAL_ERROR("Unknown/unsupported position type");
        return Vector();
    }
}

void
TraceBasedMobilityModel::DoSetPosition(const Vector& position, PositionType type)
{
    NS_LOG_WARN("TraceBasedMobilityModel::DoSetPosition ignored");
}

Vector
TraceBasedMobilityModel::DoGetVelocity() const
{
    return m_velocity;
}

int64_t
TraceBasedMobilityModel::DoAssignStreams(int64_t)
{
    return 0;
}

} // namespace ns3
