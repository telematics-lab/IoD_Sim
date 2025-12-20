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

#include "math.h"

#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/geographic-positions.h"
#include "ns3/leo-orbit.h"
#include "ns3/geo-constant-velocity-mobility.h"
#include "ns3/angles.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("GeoConstantVelocityMobility");
NS_OBJECT_ENSURE_REGISTERED (GeoConstantVelocityMobility);

TypeId
GeoConstantVelocityMobility::GetTypeId ()
{
  return TypeId ("ns3::GeoConstantVelocityMobility")
    .SetParent<GeocentricMobilityModel> ()
    .SetGroupName ("Leo")
    .AddConstructor<GeoConstantVelocityMobility> ()
    .AddAttribute ("Precision",
        "The time precision with which to compute position updates. 0 means arbitrary precision",
        TimeValue (Seconds (1)),
        MakeTimeAccessor (&GeoConstantVelocityMobility::SetPrecision),
        MakeTimeChecker ())
    .AddAttribute ("Altitude",
        "A height from the earth's surface in meters",
        DoubleValue (0.0),
        MakeDoubleAccessor (&GeoConstantVelocityMobility::m_altitude),
        MakeDoubleChecker<double> ())
    .AddAttribute ("Speed",
        "The velocity of the node in m/s",
        DoubleValue (0),
        MakeDoubleAccessor (&GeoConstantVelocityMobility::m_speed),
        MakeDoubleChecker<double> (0.0))
    .AddAttribute ("Azimuth",
        "The azimuth of the velocity vector in degrees",
        DoubleValue (0.0),
        MakeDoubleAccessor (&GeoConstantVelocityMobility::m_azimuth),
        MakeDoubleChecker<double> (0.0, 360.0))
    .AddAttribute ("InitialLatitude",
        "Initial latitude position in degrees",
        DoubleValue (0.0),
        MakeDoubleAccessor (&GeoConstantVelocityMobility::SetInitialLatitude,
                            &GeoConstantVelocityMobility::GetInitialLatitude),
        MakeDoubleChecker<double> ())
    .AddAttribute ("InitialLongitude",
        "Initial longitude position in degrees",
        DoubleValue (0.0),
        MakeDoubleAccessor (&GeoConstantVelocityMobility::SetInitialLongitude,
                            &GeoConstantVelocityMobility::GetInitialLongitude),
        MakeDoubleChecker<double> ());
}

GeoConstantVelocityMobility::GeoConstantVelocityMobility() : GeocentricMobilityModel ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

double
GeoConstantVelocityMobility::GetSpeed () const
{
  return m_speed;
}

void
GeoConstantVelocityMobility::SetSpeed (double velocity)
{
  m_speed = velocity;
  Update();
}

double
GeoConstantVelocityMobility::GetAzimuth () const
{
  return m_azimuth;
}

void
GeoConstantVelocityMobility::SetAzimuth (double azimuth)
{
  m_azimuth = azimuth;
  Update();
}

Vector
GeoConstantVelocityMobility::DoGetGeocentricVelocity () const
{
  // Convert spherical velocity to Cartesian coordinates
  // For ground movement, we need to compute velocity in local frame
  double latRad = DegreesToRadians(m_initialLatitude);
  double azimuthRad = DegreesToRadians(m_azimuth);

  // Velocity components in local East-North-Up frame
  double vEast = m_speed * sin(azimuthRad);
  double vNorth = m_speed * cos(azimuthRad);
  double vUp = 0.0; // Ground movement

  // Convert to ECEF coordinates (simplified approximation)
  Vector velocity;
  velocity.x = -vEast * sin(latRad) + vNorth * cos(latRad);
  velocity.y = vEast * cos(latRad) + vNorth * sin(latRad);
  velocity.z = vUp;

  return velocity;
}

Vector
GeoConstantVelocityMobility::DoGetVelocity () const
{
  // Convert ECEF velocity vector to Topocentric (ENU) velocity vector
  // We cannot use CartesianToTopocentric because that function transforms a POSITION (implies translation),
  // whereas we need to rotate a VECTOR (velocity).
  // TODO verify this code!
  Vector ecefVelocity = DoGetGeocentricVelocity();

  // Optimization: if velocity is zero, return zero immediately
  if (ecefVelocity.x == 0.0 && ecefVelocity.y == 0.0 && ecefVelocity.z == 0.0)
    {
      return Vector(0.0, 0.0, 0.0);
    }

  Vector refPoint = GetGeographicReferencePoint(); // (Lat, Lon, Alt)

  double phi = DegreesToRadians(refPoint.x);
  double lambda = DegreesToRadians(refPoint.y);

  double sinPhi = std::sin(phi);
  double cosPhi = std::cos(phi);
  double sinLam = std::sin(lambda);
  double cosLam = std::cos(lambda);

  // ECEF to ENU rotation matrix
  // x_enu = -sinLam * x + cosLam * y
  // y_enu = -sinPhi*cosLam * x - sinPhi*sinLam * y + cosPhi * z
  // z_enu = cosPhi*cosLam * x + cosPhi*sinLam * y + sinPhi * z

  Vector topocentricVelocity;
  topocentricVelocity.x = -sinLam * ecefVelocity.x + cosLam * ecefVelocity.y;
  topocentricVelocity.y = -sinPhi * cosLam * ecefVelocity.x - sinPhi * sinLam * ecefVelocity.y + cosPhi * ecefVelocity.z;
  topocentricVelocity.z = cosPhi * cosLam * ecefVelocity.x + cosPhi * sinLam * ecefVelocity.y + sinPhi * ecefVelocity.z;

  return topocentricVelocity;
}

Vector
GeoConstantVelocityMobility::GetVelocity () const
{
  return DoGetVelocity ();
}

Vector
GeoConstantVelocityMobility::GetGeocentricVelocity () const
{
  return DoGetGeocentricVelocity ();
}


Vector
GeoConstantVelocityMobility::CalcPosition (Time t) const
{
  // Calculate current position based on initial position, velocity, azimuth and time
  double timeSeconds = t.GetSeconds();

  // Distance traveled
  double distance = m_speed * timeSeconds;

  // Convert to angular displacement on Earth's surface
  double earthRadius = GeographicPositions::EARTH_SPHERE_RADIUS + m_altitude; // in meters
  double angularDistance = distance / earthRadius; // in radians

  // Convert initial position to radians
  double lat1 = DegreesToRadians(m_initialLatitude);
  double lon1 = DegreesToRadians(m_initialLongitude);
  double bearing = DegreesToRadians(m_azimuth);

  // Calculate new position using great circle navigation
  double lat2 = asin(sin(lat1) * cos(angularDistance) +
                     cos(lat1) * sin(angularDistance) * cos(bearing));
  double lon2 = lon1 + atan2(sin(bearing) * sin(angularDistance) * cos(lat1),
                             cos(angularDistance) - sin(lat1) * sin(lat2));

  // Convert to Cartesian ECEF coordinates
  double cosLat = cos(lat2);
  double sinLat = sin(lat2);
  double cosLon = cos(lon2);
  double sinLon = sin(lon2);

  Vector position;
  position.x = earthRadius * cosLat * cosLon;
  position.y = earthRadius * cosLat * sinLon;
  position.z = earthRadius * sinLat;

  return position;
}

Vector
GeoConstantVelocityMobility::Update ()
{
  m_position = CalcPosition (Simulator::Now ());
  NotifyCourseChange ();
  if (m_updateEvent.IsPending()) {
    Simulator::Cancel(m_updateEvent);
    m_updateEvent = EventId(); // Reset the event
  }
  if (m_precision > Seconds (0))
    {
      m_updateEvent = Simulator::Schedule (m_precision, &GeoConstantVelocityMobility::Update, this);
    }

  return m_position;
}

// GeocentricMobilityModel interface implementation
Vector
GeoConstantVelocityMobility::DoGetPosition(PositionType type) const
{
    Vector geocentricPos;
    if (m_precision == Time(0))
    {
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
        return GeographicPositions::CartesianToGeographicCoordinates(geocentricPos, GetEarthSpheroidType());
    case PositionType::TOPOCENTRIC:
        return CartesianToTopocentric(geocentricPos, GetGeographicReferencePoint(), GetEarthSpheroidType());
    case PositionType::PROJECTED:
        return GeographicPositions::GeographicToProjectedCoordinates(
          GeographicPositions::CartesianToGeographicCoordinates(geocentricPos, GetEarthSpheroidType()),
          GetEarthSpheroidType()
        );
    default:
        NS_FATAL_ERROR("Unknown/unsupported position type");
        return Vector();
    }
}

void
GeoConstantVelocityMobility::DoSetPosition(const Vector& position, PositionType type)
{
    Vector geographicPos;

    switch (type)
    {
    case PositionType::GEOCENTRIC:
        geographicPos = GeographicPositions::CartesianToGeographicCoordinates(position,
                                                                              GeographicPositions::SPHERE);
        break;
    case PositionType::GEOGRAPHIC:
        geographicPos = position;
        break;
    case PositionType::TOPOCENTRIC:
        {
            // For now, not implemented properly - would need reference point conversion
            NS_FATAL_ERROR("Topocentric coordinate setting not yet implemented");
        }
        break;
    case PositionType::PROJECTED:
        geographicPos = GeographicPositions::ProjectedToGeographicCoordinates(position, GeographicPositions::SPHERE);
        break;
    default:
        NS_FATAL_ERROR("Unknown/unsupported position type");
        return;
    }

    NS_ASSERT_MSG((geographicPos.x >= -90) && (geographicPos.x <= 90),
                  "Latitude must be between -90 deg and +90 deg");
    NS_ASSERT_MSG(geographicPos.z >= 0, "Altitude must be higher or equal than 0 meters");

    m_initialLatitude = geographicPos.x;
    m_initialLongitude = geographicPos.y;
    m_altitude = geographicPos.z;
    Update();
}

void GeoConstantVelocityMobility::SetPrecision (Time precision) {
  m_precision = precision;
  Update();
}

void
GeoConstantVelocityMobility::SetInitialLatitude(double latitude)
{
    // wrap latitude if > 90 or < -90
    // e.g. 95 -> 85 and flip longitude

    // Normalize to [-180, 180) first (handling very large values)
    // but latitude is a bit different, it oscillates

    // Standard normalization:
    // 1. Bring into [-180, 180] like longitude, but for latitude 90 is pole.

    // Easier approach: normalize to [0, 360) first

    while (latitude > 90.0)
    {
        latitude = 180.0 - latitude;
        m_initialLongitude += 180.0;
    }
    while (latitude < -90.0)
    {
        latitude = -180.0 - latitude;
        m_initialLongitude += 180.0;
    }

    // After flipping longitude, we need to re-normalize longitude
    SetInitialLongitude(m_initialLongitude);

    m_initialLatitude = latitude;
    Update();
}

double
GeoConstantVelocityMobility::GetInitialLatitude() const
{
    return m_initialLatitude;
}

void
GeoConstantVelocityMobility::SetInitialLongitude(double longitude)
{
    // Normalize longitude to [-180, 180)

    while (longitude <= -180.0)
    {
        longitude += 360.0;
    }
    while (longitude >= 180.0)
    {
        longitude -= 360.0;
    }

    m_initialLongitude = longitude;
    Update();
}

double
GeoConstantVelocityMobility::GetInitialLongitude() const
{
    return m_initialLongitude;
}

} // namespace ns3
