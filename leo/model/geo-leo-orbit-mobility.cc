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

#include "leo-orbit.h"
#include "math.h"

#include "ns3/angles.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/geographic-positions.h"
#include "ns3/geo-leo-orbit-mobility.h"
#include "ns3/simulator.h"

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
            MakeDoubleChecker<double>(0, 360.0));
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

Vector
GeoLeoOrbitMobility::Update()
{
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

    return m_position;
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

void
GeoLeoOrbitMobility::SetRetrogradeOrbit(bool retrograde)
{
    NS_LOG_FUNCTION(this << retrograde);
    m_retrogradeOrbit = retrograde;
    Update();
}

bool
GeoLeoOrbitMobility::GetRetrogradeOrbit() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_retrogradeOrbit;
}

// GeocentricMobilityModel interface implementation
Vector
GeoLeoOrbitMobility::DoGetPosition(PositionType type) const
{
    Vector geocentricPos;
    if (m_precision == Time(0))
    {
        // Notice: NotifyCourseChange () will not be called
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

void
GeoLeoOrbitMobility::DoSetPosition(const Vector& position, PositionType type)
{
    // Setting position in a classical way has no sense in this model
    NS_LOG_WARN("Setting position in a classical way has no sense in GeoLeoOrbitMobility");
}

}; // namespace ns3
