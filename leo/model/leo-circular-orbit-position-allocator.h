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

#ifndef LEO_CIRCULAR_ORBIT_POSITION_ALLOCATOR_H
#define LEO_CIRCULAR_ORBIT_POSITION_ALLOCATOR_H

#include "ns3/position-allocator.h"

/**
 * \file
 * \ingroup leo
 *
 * Declaration of LeoCircularOrbitAllocator
 */

namespace ns3 {

/**
 * \ingroup leo
 * \brief Allocate pairs of longitude and latitude (offset within orbit) for
 * use in LeoCircularOrbitMobilityModel
 */
class LeoCircularOrbitAllocator : public PositionAllocator
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /// constructor
  LeoCircularOrbitAllocator ();
  /// destructor
  virtual ~LeoCircularOrbitAllocator ();

  /**
   * \brief Gets the next position on the latitude longitude grid
   *
   * If all positions have been returned once, the first position is returned
   * again and so on.
   *
   * \return the next latitude, longitude pair
   */
  virtual Vector GetNext (void) const;
  virtual int64_t AssignStreams (int64_t stream);

private:
  /// Number of orbits two distribute the satellites on
  uint64_t m_numOrbits;
  /// Number of satellites per orbit
  uint64_t m_numSatellites;

  /// The last orit that has been assigned
  mutable uint64_t m_lastOrbit;
  /// The last position inside the orbit that has been assigned
  mutable uint64_t m_lastSatellite;
};

};

#endif
