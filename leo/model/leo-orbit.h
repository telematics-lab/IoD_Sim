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

#ifndef LEO_ORBIT_H
#define LEO_ORBIT_H

#include "ns3/uinteger.h"
#include "ns3/vector.h"
#include "ns3/geographic-positions.h"

/**
 * \file
 * \ingroup leo
 */

namespace ns3
{

// Type alias for 3D vectors
typedef Vector Vector3D;

/**
 * \brief Calculate cross product of two 3D vectors
 * \param l left vector
 * \param r right vector
 * \return cross product vector
 */
Vector3D CrossProduct (const Vector3D &l, const Vector3D &r);

/**
 * \brief Multiply a scalar with a 3D vector
 * \param l scalar value
 * \param r vector
 * \return scaled vector
 */
Vector3D Product (const double &l, const Vector3D &r);

/**
 * \brief Calculate dot product of two 3D vectors
 * \param l left vector
 * \param r right vector
 * \return dot product scalar value
 */
double DotProduct (const Vector3D &l, const Vector3D &r);

Vector CartesianToTopocentric (const Vector &v, const Vector &referencePoint, GeographicPositions::EarthSpheroidType sphereType = GeographicPositions::SPHERE);

class LeoOrbit;

/**
 * \brief Serialize an orbit
 * \return the stream
 * \param os the stream
 * \param [in] orbit is an Orbit
 */
std::ostream &operator << (std::ostream &os, const LeoOrbit &orbit);

/**
 * \brief Deserialize an orbit
 * \return the stream
 * \param is the stream
 * \param [out] orbit is the Orbit
 */
std::istream &operator >> (std::istream &is, LeoOrbit &orbit);

/**
 * \ingroup leo
 * \brief Orbit definition
 */
class LeoOrbit {
public:
  /// constructor
  LeoOrbit ();
  /**
   * \brief Constructor
   * \param a altitude
   * \param i inclination
   * \param p number planes
   * \param s number of satellites in plane
   */
  LeoOrbit (double a, double i, double p, double s) : alt (a), inc (i), planes (p), sats (s) {}
  /// destructor
  virtual ~LeoOrbit ();
  /// Altitude of orbit
  double alt;
  /// Inclination of orbit
  double inc;
  /// Number of planes with that altitude and inclination
  uint16_t planes;
  /// Number of satellites in those planes
  uint16_t sats;
};

};

#endif
