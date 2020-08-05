/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2020 The IoD_Sim Authors.
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
#ifndef CURVE_POINT_H
#define CURVE_POINT_H

#include <ns3/vector.h>

namespace ns3 {

/**
 * \brief A point constructed during curve generation with the position
 *        relative to the curve length.
 */
class CurvePoint
{
public:
  /**
   * \brief constructor used by the curve generator.
   *
   * \param position         position in space.
   * \param t                parameter used by the curve for the generation
   *                         of the point. t is a float number between 0 and 1.
   * \param relativeDistance the euclidean distance relative to the point
   *                         preceding this one on the curve.
   * \param absoluteDistance the total length of the curve from the origin to
   *                         this point.
   */
  CurvePoint (const Vector position,
              float t,
              double relativeDistance,
              double absoluteDistance);
  /**
   * \brief default constructor
   */
  CurvePoint ();
  /**
   * \brief default destructor
   */
  virtual ~CurvePoint ();


  /**
   * \brief get the distance vector relative to a given point
   *
   * \param point the second point to calculate the distance vector
   * \return the distance vector
   */
  const Vector GetRelativeDistanceVector (const Vector &point) const;
  /**
   * \brief get the distance vector relative to a given point of the curve
   *
   * \param point the second point to calculate the distance vector
   * \return the distance vector
   */
  const Vector GetRelativeDistanceVector (const CurvePoint &point) const;
  /**
   * \brief get the distance relative to a given point
   *
   * \param point the second point to calculate the distance
   * \return the scalar value of the euclidean distance
   */
  const double GetRelativeDistance (const Vector &point) const;
  /**
   * \brief get the distance relative to a given point point of the curve
   *
   * \param point the second point to calculate the distance
   * \return the scalar value of the euclidean distance
   */
  const double GetRelativeDistance (const CurvePoint &point) const;

  /**
   * \brief get the length of the curve up to the point of the curve
   *
   * \return the length of the curve up to this point of the curve
   */
  const double GetAbsoluteDistance () const;

  /**
   * \return the location of this point in space
   */
  const Vector GetPosition () const;

private:
  Vector m_position;          /// Location of this point in space.
  float  m_t;                 /// Parameter used by the curve for the
                              /// generation of the point. t is a float
                              /// Number between 0 and 1.
  double m_relativeDistance;  /// The euclidean distance relative to the point
                              /// preceding this one on the curve.
  double m_absoluteDistance;  /// The total length of the curve from the origin
                              /// to this point.
};

} // namespace ns3

#endif /* CURVE_POINT_H */
