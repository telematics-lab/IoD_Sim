/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2024 The IoD_Sim Authors.
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
#ifndef CURVE_H
#define CURVE_H

#include "curve-point.h"
#include "flight-plan.h"

#include <ns3/vector.h>

#include <deque>
#include <vector>

namespace ns3
{

/**
 * Generate a discrete curve given a flight plan and a step using Bézier
 * general equation.
 */
class Curve
{
  public:
    typedef std::vector<CurvePoint>::iterator Iterator;

    /**
     * \brief the constructor requesting all the needed parameters.
     */
    Curve(const FlightPlan knots, const float step);
    /**
     * \brief the constructor requesting only the flight plan. The step will be
     *        1/100.
     */
    Curve(const FlightPlan knots);
    /**
     * \brief default constructor with no flight plan, hence no curve to
     *        generate.
     */
    Curve();
    /**
     * \brief generate the curve using the given flight plan and step during
     *        object initialization.
     *
     * \return the length of the newly generated curve.
     */
    const double Generate() const;

  protected:
    /**
     * \brief Calculate the point of a curve given using Bézier generator.
     *
     * \param t the parameter needed by Bézier general equation, a float between
     *          0 and 1.
     * \return the point in the curve.
     */
    const Vector GetPoint(const float& t) const;

    /**
     * \brief Helper function to calculate the factorial of a number using
     *        standard facilities.
     *
     * \param x the integer for which is needed it's factorial
     * \reutrn x!
     */
    const double Factorial(const double x) const;

    mutable std::vector<CurvePoint> m_curve; /// The ordered set of points
                                             /// representing the curve.
    FlightPlan m_knots; /// Flight plan (as in virtual knots) used to generate the curve.
    size_t m_knotsN;    /// Number of real knots being used to generate the curve.
    float m_step;       /// Step of the curve.
};

} // namespace ns3

#endif /* CURVE_H */
