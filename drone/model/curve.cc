/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2021 The IoD_Sim Authors.
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
#include "curve.h"

#include <cmath>
#include <vector>

#include <ns3/integer.h>
#include <ns3/log.h>
#include <ns3/object-factory.h>
#include <ns3/vector.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Curve");

class CurvePriv
{
public:
  /**
   * Get virtual knots according to real knots' interest level.
   */
  static const FlightPlan GetVirtualKnots (const FlightPlan knots)
  {
    FlightPlan virtualKnots {};

    for (auto k = knots.Begin (); k != knots.End(); k++)
      {
        auto interestLevel = (*k)->GetInterest ();

        if (interestLevel <= 1)
          {
            virtualKnots.Add (*k);
          }
        else
          {
            for (uint32_t i = 0; i < interestLevel; i++)
              {
                virtualKnots.Add (*k);
              }
          }
      }

    return virtualKnots;
  }
};

Curve::Curve (const FlightPlan knots,
              const float step) :
  m_knots {CurvePriv::GetVirtualKnots (knots)},
  m_knotsN {knots.GetN ()},
  m_step {step}
{
  NS_LOG_FUNCTION (knots.GetN () << step);
}

Curve::Curve (const FlightPlan knots) :
  m_knots {CurvePriv::GetVirtualKnots (knots)},
  m_knotsN {knots.GetN ()}
{
  NS_LOG_FUNCTION (knots.GetN ());
}

Curve::Curve () :
  m_step {0.1}
{
  NS_LOG_FUNCTION_NOARGS ();
}

Curve::~Curve ()
{
}

const double
Curve::Generate () const
{
  NS_LOG_FUNCTION_NOARGS ();

  double absoluteDistance = 0.0;

  NS_LOG_LOGIC ("Constructing Bézier curve with " << m_knotsN << " knots.");
  for (float t = 0.0; t < 1.0; t += m_step) {
    const Vector point = GetPoint (t);
    double relativeDistance = 0.0;

    if (!m_curve.empty ())
      {
        relativeDistance = m_curve.back ().GetRelativeDistance (point);
        absoluteDistance += relativeDistance;
      }

    //NS_LOG_LOGIC("  NP:  " << point << " | t: " << t << " | rD: " << relativeDistance << " | aD: " << absoluteDistance);
    m_curve.push_back ({point, t, relativeDistance, absoluteDistance});
  }

  return absoluteDistance;
}

const Vector
Curve::GetPoint (const float &t) const
{
  const uint32_t n = m_knots.GetN ();
  const uint32_t r = n - 1;

  Vector p {0.0, 0.0, 0.0};

  const double fac_r = Factorial (r);

  for (uint32_t i = 0; i < n; i++)
    {
      const double fact = fac_r / (Factorial (r - i) * Factorial (i));
      const double pdiff = pow (1 - t, r - i);
      const double power = pow (t, i);
      const auto k = m_knots.Get (i)->GetPosition ();

      p.x += fact * pdiff * power * k.x;
      p.y += fact * pdiff * power * k.y;
      p.z += fact * pdiff * power * k.z;
    }

  // NS_LOG_LOGIC ("Curve at t " << t << ": " << p);
  return p;
}

const double
Curve::Factorial (const double x) const
{
  return (x == 0) || (x == 1) ? 1 : x * Factorial(x - 1);
}

} // namespace ns3
