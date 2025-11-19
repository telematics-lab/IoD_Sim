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
#include "leo-orbit.h"
#include "leo-gnd-constant-velocity-mobility-model.h"
#include "ns3/angles.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("GndConstantVelocityMobilityModel");

NS_OBJECT_ENSURE_REGISTERED (GndConstantVelocityMobilityModel);

TypeId
GndConstantVelocityMobilityModel::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::GndConstantVelocityMobilityModel")
    .SetParent<GeocentricConstantPositionMobilityModel> ()
    .SetGroupName ("Leo")
    .AddConstructor<GndConstantVelocityMobilityModel> ()
    .AddAttribute ("Precision",
                "The time precision with which to compute position updates. 0 means arbitrary precision",
                TimeValue (Seconds (1)),
                MakeTimeAccessor (&GndConstantVelocityMobilityModel::SetPrecision),
                MakeTimeChecker ())
    .AddAttribute ("Altitude",
                     "A height from the earth's surface in meters",
                        DoubleValue (0.0),
                        MakeDoubleAccessor (&GndConstantVelocityMobilityModel::m_altitude),
                        MakeDoubleChecker<double> ())
    .AddAttribute ("Speed",
                        "The velocity of the node in m/s",
                        DoubleValue (0),
                        MakeDoubleAccessor (&GndConstantVelocityMobilityModel::m_speed),
                        MakeDoubleChecker<double> (0.0))
    .AddAttribute ("Azimuth",
                     "The azimuth of the velocity vector in degrees",
                        DoubleValue (0.0),
                        MakeDoubleAccessor (&GndConstantVelocityMobilityModel::m_azimuth),
                        MakeDoubleChecker<double> (0.0, 360.0))
    .AddAttribute ("InitialLatitude",
                     "Initial latitude position in degrees",
                        DoubleValue (0.0),
                        MakeDoubleAccessor (&GndConstantVelocityMobilityModel::m_initialLatitude),
                        MakeDoubleChecker<double> (-90.0, 90.0))
    .AddAttribute ("InitialLongitude",
                     "Initial longitude position in degrees",
                        DoubleValue (0.0),
                        MakeDoubleAccessor (&GndConstantVelocityMobilityModel::m_initialLongitude),
                        MakeDoubleChecker<double> (-180.0, 180.0));
    TypeId::AttributeInformation notToUse;
    tid.LookupAttributeByName ("PositionLatLongAlt", &notToUse, true);
    notToUse.supportLevel = TypeId::SupportLevel::OBSOLETE;
  return tid;
}

GndConstantVelocityMobilityModel::GndConstantVelocityMobilityModel() : GeocentricConstantPositionMobilityModel ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

double
GndConstantVelocityMobilityModel::GetSpeed () const
{
  return m_speed;
}

void
GndConstantVelocityMobilityModel::SetSpeed (double velocity)
{
  m_speed = velocity;
  Update();
}

double
GndConstantVelocityMobilityModel::GetAzimuth () const
{
  return m_azimuth;
}

void
GndConstantVelocityMobilityModel::SetAzimuth (double azimuth)
{
  m_azimuth = azimuth;
  Update();
}

Vector
GndConstantVelocityMobilityModel::DoGetGeocentricVelocity () const
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
GndConstantVelocityMobilityModel::DoGetVelocity () const
{
  return CartesianToTopocentric (DoGetGeocentricVelocity(), GetCoordinateTranslationReferencePoint(), GeographicPositions::SPHERE);
}

Vector
GndConstantVelocityMobilityModel::GetVelocity () const
{
  return DoGetVelocity ();
}

Vector
GndConstantVelocityMobilityModel::GetGeocentricVelocity () const
{
  return DoGetGeocentricVelocity ();
}


Vector
GndConstantVelocityMobilityModel::CalcPosition (Time t) const
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
GndConstantVelocityMobilityModel::Update ()
{
  m_position = CalcPosition (Simulator::Now ());
  NotifyCourseChange ();
  if (m_updateEvent.IsPending()) {
    Simulator::Cancel(m_updateEvent);
    m_updateEvent = EventId(); // Reset the event
  }
  if (m_precision > Seconds (0))
    {
      m_updateEvent = Simulator::Schedule (m_precision, &GndConstantVelocityMobilityModel::Update, this);
    }

  return m_position;
}

Vector
GndConstantVelocityMobilityModel::DoGetPosition (void) const
{
  return CartesianToTopocentric (DoGetGeocentricPosition(), GetCoordinateTranslationReferencePoint(), GeographicPositions::SPHERE);
}

void
GndConstantVelocityMobilityModel::DoSetPosition (const Vector &position)
{
  // Ignoring the topocentric position, we set the geographic position directly TODO find a better way to manage this
  /* This code is theorically correct but creates conflicts with the already present attributes
  auto geo = GeographicPositions::TopocentricToGeographicCoordinates(position, GetCoordinateTranslationReferencePoint(), GeographicPositions::SPHERE);
  DoSetGeographicPosition(geo);
  */
}

Vector
GndConstantVelocityMobilityModel::DoGetGeographicPosition() const
{
  // Convert from topocentric to geographic coordinates
  return GeographicPositions::CartesianToGeographicCoordinates(
      DoGetGeocentricPosition(),GeographicPositions::SPHERE
  );
}

void
GndConstantVelocityMobilityModel::DoSetGeographicPosition(const Vector& latLonAlt)
{
  NS_ASSERT_MSG((latLonAlt.x >= -90) && (latLonAlt.x <= 90),
                "Latitude must be between -90 deg and +90 deg");
  NS_ASSERT_MSG(latLonAlt.z >= 0, "Altitude must be higher or equal than 0 meters");
  std::cout << "Setting geographic position: (" 
       << latLonAlt.x << ", " << latLonAlt.y << ", " << latLonAlt.z << ")" << std::endl;
  m_initialLatitude = latLonAlt.x;
  m_initialLongitude = latLonAlt.y;
  m_altitude = latLonAlt.z;
  Update();
}

Vector
GndConstantVelocityMobilityModel::DoGetGeocentricPosition() const
{
  if (m_precision == Time (0))
    {
      return CalcPosition (Simulator::Now ());
    }
  return m_position;
}

void GndConstantVelocityMobilityModel::SetPrecision (Time precision) {
  m_precision = precision;
  Update();
}

void
GndConstantVelocityMobilityModel::DoSetGeocentricPosition(const Vector& position)
{
  Vector geographicCoordinates = GeographicPositions::CartesianToGeographicCoordinates(
    position, GeographicPositions::SPHERE);
  DoSetGeographicPosition(geographicCoordinates);
}

Vector
GndConstantVelocityMobilityModel::GetGeographicPosition() const
{
  return DoGetGeographicPosition();
}

void
GndConstantVelocityMobilityModel::SetGeographicPosition(const Vector& latLonAlt)
{
  DoSetGeographicPosition(latLonAlt);
}

Vector
GndConstantVelocityMobilityModel::GetGeocentricPosition() const
{
  return DoGetGeocentricPosition();
}

void
GndConstantVelocityMobilityModel::SetGeocentricPosition(const Vector& position)
{
  DoSetGeocentricPosition(position);
}

void
GndConstantVelocityMobilityModel::SetCoordinateTranslationReferencePoint(const Vector& refPoint)
{
  // Use base class implementation
  GeocentricConstantPositionMobilityModel::SetCoordinateTranslationReferencePoint(refPoint);
}

Vector
GndConstantVelocityMobilityModel::GetCoordinateTranslationReferencePoint() const
{
  // Use base class implementation
  return GeocentricConstantPositionMobilityModel::GetCoordinateTranslationReferencePoint();
}

Vector
GndConstantVelocityMobilityModel::GetPosition() const
{
  return DoGetPosition();
}



void
GndConstantVelocityMobilityModel::SetPosition(const Vector& position)
{
  DoSetPosition(position);
}

} // namespace ns3
