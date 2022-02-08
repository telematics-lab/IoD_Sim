/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2022 The IoD_Sim Authors.
 *
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
 */
#include "interest-region.h"

#include <cmath>

#include <ns3/log.h>
#include <ns3/vector.h>
#include <ns3/double.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("InterestRegion");
NS_OBJECT_ENSURE_REGISTERED (InterestRegion);

TypeId
InterestRegion::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::InterestRegion")
    .SetParent<Object> ()
    .SetGroupName ("Scenario")
    .AddConstructor<InterestRegion> ()
    .AddAttribute ("Coordinates", "Box 3D coordinates, i.e., xMin|xMax|yMin|yMax|zMin|zMax|.",
                  DoubleVectorValue (),
                  MakeDoubleVectorAccessor (&InterestRegion::SetCoordinates),
                  MakeDoubleVectorChecker ())
  ;
  return tid;
}

InterestRegion::InterestRegion ()
{
}

InterestRegion::~InterestRegion ()
{
}

void
InterestRegion::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void
InterestRegion::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  Object::DoInitialize ();
}

const DoubleVector
InterestRegion::GetCoordinates () const
{
  return m_coordinates;
}

void
InterestRegion::SetCoordinates (const DoubleVector &coords)
{
  m_coordinates = coords;
  m_box = Box(m_coordinates.Get(0),m_coordinates.Get(1),m_coordinates.Get(2),m_coordinates.Get(3),m_coordinates.Get(4),m_coordinates.Get(5));
}

bool
InterestRegion::IsInside (const Vector &position) const
{
  return m_box.IsInside(position);
}

} // namespace ns3
