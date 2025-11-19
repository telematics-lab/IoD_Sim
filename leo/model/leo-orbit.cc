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

#include "leo-orbit.h"
#include "ns3/vector.h"

namespace ns3 {

Vector3D
CrossProduct (const Vector3D &l, const Vector3D &r)
{
  return Vector3D (l.y * r.z - l.z * r.y,
		   l.z * r.x - l.x * r.z,
		   l.x * r.y - l.y * r.x);
}

Vector3D
Product (const double &l, const Vector3D &r)
{
  return Vector3D (l * r.x,
		   l * r.y,
		   l * r.z);
}

double
DotProduct (const Vector3D &l, const Vector3D &r)
{
  return (l.x* r.x) + (l.y*r.y) + (l.z*r.z);
}

Vector
CartesianToTopocentric (const Vector &v, const Vector &referencePoint, GeographicPositions::EarthSpheroidType sphereType)
{
  // Convert Cartesian coordinates to topocentric coordinates
  Vector3D geo = GeographicPositions::CartesianToGeographicCoordinates(v, sphereType);
  return GeographicPositions::GeographicToTopocentricCoordinates(geo, referencePoint, sphereType);
}

std::ostream &operator << (std::ostream &os, const LeoOrbit &orbit)
{
  os << orbit.alt << ":" << orbit.inc << ":" << orbit.planes << ":" << orbit.sats;
  return os;
}

std::istream &operator >> (std::istream &is, LeoOrbit &orbit)
{
  char c1, c2, c3;
  is >> orbit.alt >> c1 >> orbit.inc >> c2 >> orbit.planes >> c3 >> orbit.sats;
  if (c1 != ':' ||
      c2 != ':' ||
      c3 != ':')
    {
      is.setstate (std::ios_base::failbit);
    }
  return is;
}

LeoOrbit::LeoOrbit () : alt (0), inc (0), planes (0), sats (0) {}
LeoOrbit::~LeoOrbit () {}

};
