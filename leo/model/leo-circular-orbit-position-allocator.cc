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

#include "ns3/integer.h"
#include "leo-circular-orbit-position-allocator.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (LeoCircularOrbitAllocator);

LeoCircularOrbitAllocator::LeoCircularOrbitAllocator ()
  : m_lastOrbit (0), m_lastSatellite (0)
{}

LeoCircularOrbitAllocator::~LeoCircularOrbitAllocator ()
{}

TypeId
LeoCircularOrbitAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LeoCircularOrbitPostionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Leo")
    .AddConstructor<LeoCircularOrbitAllocator> ()
    .AddAttribute ("NumOrbits",
                   "The number of orbits",
                   IntegerValue (1),
                   MakeIntegerAccessor (&LeoCircularOrbitAllocator::m_numOrbits),
                   MakeIntegerChecker<uint16_t> ())
    .AddAttribute ("NumSatellites",
                   "The number of satellites per orbit",
                   IntegerValue (1),
                   MakeIntegerAccessor (&LeoCircularOrbitAllocator::m_numSatellites),
                   MakeIntegerChecker<uint16_t> ())
  ;
  return tid;
}

int64_t
LeoCircularOrbitAllocator::AssignStreams (int64_t stream)
{
  return -1;
}

Vector
LeoCircularOrbitAllocator::GetNext () const
{
  Vector next = Vector (2 * M_PI * (m_lastOrbit / (double) m_numOrbits),
	  	 2 * M_PI * (m_lastSatellite / (double) m_numSatellites),
	  	 0);

  if (m_lastSatellite + 1 == m_numSatellites)
    {
      m_lastOrbit = (m_lastOrbit + 1) % m_numOrbits;
    }
  m_lastSatellite = (m_lastSatellite + 1) % m_numSatellites;

  return next;
}

};
