/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Tim Schubert <ns-3-leo@timschubert.net>
 */

#include "geo-leo-orbit-mobility.h"

#include "../sgp4/libsgp4/DateTime.h"
#include "../sgp4/libsgp4/SGP4.h"
#include "../sgp4/libsgp4/Tle.h"
#include "leo-orbit.h"
#include "math.h"

#include "ns3/angles.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/geo-leo-orbit-mobility.h"
#include "ns3/geographic-positions.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("GeoLeoOrbitMobility");

NS_OBJECT_ENSURE_REGISTERED(GeoLeoOrbitMobility);

TypeId
GeoLeoOrbitMobility::GetTypeId()
{
    return TypeId("ns3::GeoLeoOrbitMobility")
        .SetParent<GeocentricMobilityModel>()
        .SetGroupName("Leo")
        .AddConstructor<GeoLeoOrbitMobility>()
        .AddAttribute("Altitude",
                      "A height from the earth's surface in kilometers",
                      DoubleValue(1000.0),
                      MakeDoubleAccessor(&GeoLeoOrbitMobility::SetAltitude,
                                         &GeoLeoOrbitMobility::GetAltitude),
                      MakeDoubleChecker<double>())
        .AddAttribute("Inclination",
                      "The inclination of the orbital plane in degrees",
                      DoubleValue(10.0),
                      MakeDoubleAccessor(&GeoLeoOrbitMobility::SetInclination,
                                         &GeoLeoOrbitMobility::GetInclination),
                      MakeDoubleChecker<double>(0.0, 180.0))
        .AddAttribute("Precision",
                      "The time precision with which to compute position updates. 0 means "
                      "arbitrary precision",
                      TimeValue(Seconds(1)),
                      MakeTimeAccessor(&GeoLeoOrbitMobility::SetPrecision),
                      MakeTimeChecker())
        .AddAttribute("Longitude",
                      "The longitude offset of the satellite in degrees",
                      DoubleValue(0.0),
                      MakeDoubleAccessor(&GeoLeoOrbitMobility::SetLongitudeOffset,
                                         &GeoLeoOrbitMobility::GetLongitudeOffset),
                      MakeDoubleChecker<double>(-180.0, 180.0))
        .AddAttribute(
            "RetrogradeOrbit",
            "If true, the satellite moves in the opposite direction of the Earth's rotation",
            BooleanValue(false),
            MakeBooleanAccessor(&GeoLeoOrbitMobility::m_retrogradeOrbit),
            MakeBooleanChecker())
        .AddAttribute(
            "Offset",
            "The initial offset of the satellite in degrees",
            DoubleValue(0.0),
            MakeDoubleAccessor(&GeoLeoOrbitMobility::SetOffset, &GeoLeoOrbitMobility::GetOffset),
            MakeDoubleChecker<double>(0, 360.0))
        .AddAttribute("TleLine1",
                      "TLE Line 1",
                      StringValue(""),
                      MakeStringAccessor(&GeoLeoOrbitMobility::SetTleLine1,
                                         &GeoLeoOrbitMobility::GetTleLine1),
                      MakeStringChecker())
        .AddAttribute("TleLine2",
                      "TLE Line 2",
                      StringValue(""),
                      MakeStringAccessor(&GeoLeoOrbitMobility::SetTleLine2,
                                         &GeoLeoOrbitMobility::GetTleLine2),
                      MakeStringChecker())
        .AddAttribute("TleStartTime",
                      "The start time for TLE propagation (format: ISO 8601)",
                      StringValue(""),
                      MakeStringAccessor(&GeoLeoOrbitMobility::SetTleStartTime,
                                         &GeoLeoOrbitMobility::GetTleStartTime),
                      MakeStringChecker());
}

GeoLeoOrbitMobility::GeoLeoOrbitMobility()
    : GeocentricMobilityModel()
{
    NS_LOG_FUNCTION_NOARGS();
}

GeoLeoOrbitMobility::~GeoLeoOrbitMobility()
{
}

void
GeoLeoOrbitMobility::SetLongitudeOffset(double longitude)
{
    NS_LOG_FUNCTION(this << longitude);
    m_longitude = DegreesToRadians(longitude);
    Update();
}

double
GeoLeoOrbitMobility::GetLongitudeOffset() const
{
    NS_LOG_FUNCTION_NOARGS();
    return RadiansToDegrees(m_longitude);
}

void
GeoLeoOrbitMobility::SetOffset(double offset)
{
    NS_LOG_FUNCTION(this << offset);
    m_offset = DegreesToRadians(offset);
    Update();
}

double
GeoLeoOrbitMobility::GetOffset() const
{
    NS_LOG_FUNCTION_NOARGS();
    return RadiansToDegrees(m_offset);
}

void
GeoLeoOrbitMobility::SetTleLine1(std::string tle1)
{
    NS_LOG_FUNCTION(this << tle1);
    m_tleLine1 = tle1;
    NS_LOG_INFO("TLE Line 1 set to: '" << m_tleLine1 << "'");
    Update();
}

std::string
GeoLeoOrbitMobility::GetTleLine1() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_tleLine1;
}

void
GeoLeoOrbitMobility::SetTleLine2(std::string tle2)
{
    NS_LOG_FUNCTION(this << tle2);
    m_tleLine2 = tle2;
    NS_LOG_INFO("TLE Line 2 set to: '" << m_tleLine2 << "'");

    // If both TLE lines are set, initialize orbital parameters from TLE
    if (!m_tleLine1.empty() && !m_tleLine2.empty())
    {
        InitializeFromTLE();
    }
    Update();
}

std::string
GeoLeoOrbitMobility::GetTleLine2() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_tleLine2;
}

void
GeoLeoOrbitMobility::InitializeFromTLE()
{
    NS_LOG_FUNCTION(this);

    try
    {
        NS_LOG_INFO("Initializing orbital parameters from TLE...");
        libsgp4::Tle tle(m_tleLine1, m_tleLine2);
        libsgp4::SGP4 sgp4(tle);

        libsgp4::DateTime tleEpoch = tle.Epoch();
        libsgp4::Eci eci = sgp4.FindPosition(tleEpoch);
        libsgp4::CoordGeodetic geo = eci.ToGeodetic();

        double altitude = geo.altitude;
        m_orbitHeight = GeographicPositions::EARTH_SPHERE_RADIUS / 1000.0 + altitude;
        m_inclination = tle.Inclination(false);

        double speed = sqrt(LEO_EARTH_GM_KM_E10 / m_orbitHeight) * 1e5;
        double timeOffset = m_tleStartTime.GetSeconds();
        double angularDistance = (speed * timeOffset) / GeographicPositions::EARTH_SPHERE_RADIUS;
        m_offset = angularDistance;
        m_longitude = DegreesToRadians(geo.longitude);
    }
    catch (const std::exception& e)
    {
        NS_LOG_ERROR("Failed to initialize from TLE: " << e.what());
        NS_LOG_ERROR("  TLE Line 1: '" << m_tleLine1 << "'");
        NS_LOG_ERROR("  TLE Line 2: '" << m_tleLine2 << "'");
    }
}

void
GeoLeoOrbitMobility::SetTleStartTime(std::string startTime)
{
    NS_LOG_FUNCTION(this << startTime);
    if (startTime.empty())
    {
        m_tleStartTime = Seconds(0);
        m_tleStartTimeString = "";
        return;
    }

    // Parse ISO 8601 format: YYYY-MM-DD HH:MM:SS or YYYY-MM-DDTHH:MM:SS or YYYY-MM-DDTHH:MM:SSZ
    int year, month, day, hour, minute, second;
    int parsed = sscanf(startTime.c_str(),
                        "%d-%d-%d %d:%d:%d",
                        &year,
                        &month,
                        &day,
                        &hour,
                        &minute,
                        &second);

    if (parsed != 6)
    {
        // Try with 'T' separator
        parsed = sscanf(startTime.c_str(),
                        "%d-%d-%dT%d:%d:%d",
                        &year,
                        &month,
                        &day,
                        &hour,
                        &minute,
                        &second);
    }

    if (parsed == 6)
    {
        // Create DateTime for the start time
        libsgp4::DateTime startDt(year, month, day, hour, minute, second);

        // Calculate offset from TLE epoch (we need TLE to be loaded first)
        if (m_tle)
        {
            libsgp4::DateTime tleEpoch = m_tle->Epoch();
            double offsetSeconds = (startDt - tleEpoch).TotalSeconds();
            m_tleStartTime = Seconds(offsetSeconds);
            m_tleStartTimeString = startTime;
            NS_LOG_INFO("TLE start time set: " << offsetSeconds << "s from TLE epoch");
        }
        else
        {
            // TLE not loaded yet, just save the string and calculate offset later
            m_tleStartTimeString = startTime;
            NS_LOG_WARN("TLE not loaded yet, will calculate offset when TLE is set");
        }
    }
    else
    {
        NS_LOG_WARN("Invalid TleStartTime format: '"
                    << startTime << "'. Expected ISO 8601 (YYYY-MM-DD HH:MM:SS). Using TLE epoch.");
        m_tleStartTime = Seconds(0);
        m_tleStartTimeString = "";
    }
    Update();
}

std::string
GeoLeoOrbitMobility::GetTleStartTime() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_tleStartTimeString;
}

double
GeoLeoOrbitMobility::GetSpeed() const
{
    NS_LOG_FUNCTION_NOARGS();
    return sqrt(LEO_EARTH_GM_KM_E10 / m_orbitHeight) * 1e5;
}

Vector
GeoLeoOrbitMobility::DoGetGeocentricVelocity() const
{
    Vector3D pos = DoGetPosition(PositionType::GEOCENTRIC);
    pos = Vector3D(pos.x / pos.GetLength(), pos.y / pos.GetLength(), pos.z / pos.GetLength());
    Vector3D heading = CrossProduct(PlaneNorm(), pos);
    return Product(GetSpeed(), heading);
}

Vector
GeoLeoOrbitMobility::DoGetVelocity() const
{
    return CartesianToTopocentric(DoGetGeocentricVelocity(),
                                  GetGeographicReferencePoint(),
                                  GetEarthSpheroidType());
}

Vector
GeoLeoOrbitMobility::GetVelocity() const
{
    NS_LOG_FUNCTION_NOARGS();
    return DoGetVelocity();
}

Vector
GeoLeoOrbitMobility::GetGeocentricVelocity() const
{
    NS_LOG_FUNCTION_NOARGS();
    return DoGetGeocentricVelocity();
}

Vector3D
GeoLeoOrbitMobility::PlaneNorm() const
{
    double lat = CalcLatitude();
    return Vector3D(sin(-m_inclination) * cos(lat),
                    sin(-m_inclination) * sin(lat),
                    cos(m_inclination));
}

double
GeoLeoOrbitMobility::GetProgress(Time t) const
{
    NS_LOG_FUNCTION(this << t);
    // ensure correct gradient (not against earth rotation)
    int sign = (m_inclination > M_PI / 2) ? -1 : 1;

    if (m_retrogradeOrbit)
    {
        sign *= -1;
    }

    // 2pi * (distance travelled / circumference of earth) + offset
    return sign * (((GetSpeed() * t.GetSeconds()) / GeographicPositions::EARTH_SPHERE_RADIUS)) +
           m_offset;
}

Vector3D
GeoLeoOrbitMobility::RotatePlane(double a, const Vector3D& x) const
{
    Vector3D n = PlaneNorm();

    return Product(DotProduct(n, x), n) + Product(cos(a), CrossProduct(CrossProduct(n, x), n)) +
           Product(sin(a), CrossProduct(n, x));
}

double
GeoLeoOrbitMobility::CalcLatitude() const
{
    return m_longitude + ((Simulator::Now().GetDouble() / Hours(24).GetDouble()) * 2 * M_PI);
}

Vector
GeoLeoOrbitMobility::CalcPosition(Time t) const
{
    double lat = CalcLatitude();
    // account for orbit latitude and earth rotation offset
    Vector3D x = Product(
        m_orbitHeight * 1000,
        Vector3D(cos(m_inclination) * cos(lat), cos(m_inclination) * sin(lat), sin(m_inclination)));

    return RotatePlane(GetProgress(t), x);
}

void
GeoLeoOrbitMobility::DoSetPosition(const Vector& position, PositionType type)
{
    // Setting position in a classical way has no sense in this model
    NS_LOG_WARN("Setting position in a classical way has no sense in GeoLeoOrbitMobility");
}

void
GeoLeoOrbitMobility::Update()
{
    // Use classical orbital model
    m_position = CalcPosition(Simulator::Now());

    NotifyCourseChange();
    if (m_updateEvent.IsPending())
    {
        Simulator::Cancel(m_updateEvent);
        m_updateEvent = EventId(); // Reset the event
    }
    if (m_precision > Seconds(0))
    {
        m_updateEvent = Simulator::Schedule(m_precision, &GeoLeoOrbitMobility::Update, this);
    }
}

void
GeoLeoOrbitMobility::SetPrecision(Time precision)
{
    m_precision = precision;
    Update();
}

double
GeoLeoOrbitMobility::GetAltitude() const
{
    return m_orbitHeight - GeographicPositions::EARTH_SPHERE_RADIUS / 1000.0;
}

void
GeoLeoOrbitMobility::SetAltitude(double h)
{
    m_orbitHeight = GeographicPositions::EARTH_SPHERE_RADIUS / 1000.0 + h;
    Update();
}

double
GeoLeoOrbitMobility::GetInclination() const
{
    return RadiansToDegrees(m_inclination);
}

void
GeoLeoOrbitMobility::SetInclination(double incl)
{
    NS_ASSERT_MSG(incl != 0.0, "Plane must not be orthogonal to axis");
    m_inclination = DegreesToRadians(incl);
    Update();
}

Vector
GeoLeoOrbitMobility::DoGetPosition(PositionType type) const
{
    Vector geocentricPos;
    if (m_precision == Time(0))
    {
        // Calculate position on-demand using classical model
        geocentricPos = CalcPosition(Simulator::Now());
    }
    else
    {
        geocentricPos = m_position;
    }

    switch (type)
    {
    case PositionType::GEOCENTRIC:
        return geocentricPos;
    case PositionType::GEOGRAPHIC:
        return GeographicPositions::CartesianToGeographicCoordinates(geocentricPos,
                                                                     GetEarthSpheroidType());
    case PositionType::TOPOCENTRIC:
        return CartesianToTopocentric(geocentricPos,
                                      GetGeographicReferencePoint(),
                                      GetEarthSpheroidType());
    case PositionType::PROJECTED:
        return GeographicPositions::GeographicToProjectedCoordinates(
            GeographicPositions::CartesianToGeographicCoordinates(geocentricPos,
                                                                  GetEarthSpheroidType()),
            GetEarthSpheroidType());

    default:
        NS_FATAL_ERROR("Unknown/unsupported position type");
        return Vector();
    }
}
}; // namespace ns3
