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

#ifndef LEO_POLAR_POSITION_ALLOCATOR_H
#define LEO_POLAR_POSITION_ALLOCATOR_H

#include "ns3/position-allocator.h"

/**
 * \file
 * \ingroup leo
 *
 * Declaration of LeoPolarPositionAllocator
 */

namespace ns3 {

/**
 * \ingroup leo
 * \brief Allocator for pairs of latitude and longitude.
 */
class LeoPolarPositionAllocator : public PositionAllocator
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /// constructor
  LeoPolarPositionAllocator ();
  /// desctructor
  virtual ~LeoPolarPositionAllocator ();

  virtual Vector GetNext (void) const;
  virtual int64_t AssignStreams (int64_t stream);

private:
  /// Number of latitudial positions
  uint32_t m_latNum;
  /// Number of longitudinal positions
  uint32_t m_lonNum;

  /// Current latitudinal position
  mutable uint32_t m_lat;
  /// Current longitudinal position
  mutable uint32_t m_lon;
};

};

#endif
