/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2019 The IoD_Sim Authors.
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
#include "curve-point.h"

#include <cmath>

#include <ns3/log.h>
#include <ns3/vector.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CurvePoint");

CurvePoint::CurvePoint (const Vector position,
                        float t,
                        double relativeDistance,
                        double absoluteDistance) :
    m_position {position},
    m_t {t},
    m_relativeDistance {relativeDistance},
    m_absoluteDistance {absoluteDistance}
{}

CurvePoint::CurvePoint () :
    m_position {0.0, 0.0, 0.0},
    m_t {0.0},
    m_relativeDistance {0.0},
    m_absoluteDistance {0.0}
{}

CurvePoint::~CurvePoint ()
{
}

const Vector
CurvePoint::GetRelativeDistanceVector (const Vector &point) const
{
  return m_position - point;
}

const Vector
CurvePoint::GetRelativeDistanceVector (const CurvePoint &point) const
{
  return m_position - point.GetPosition ();
}

const double
CurvePoint::GetRelativeDistance (const Vector &point) const
{
  const Vector diff = GetRelativeDistanceVector (point);
  return sqrt (pow (diff.x, 2) + pow (diff.y, 2) + pow (diff.z, 2));
}

const double
CurvePoint::GetRelativeDistance (const CurvePoint &point) const
{
  return GetRelativeDistance (point.GetPosition ());
}

const double
CurvePoint::GetAbsoluteDistance () const
{
  return m_absoluteDistance;
}

const Vector
CurvePoint::GetPosition () const
{
  return m_position;
}

} // namespace ns3
