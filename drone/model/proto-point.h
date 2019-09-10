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
#ifndef PROTO_POINT_H
#define PROTO_POINT_H

#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/vector.h>

namespace ns3 {

/**
 * \ingroup mobility
 * \brief A point that describes part of a trajectory prototype.
 *
 * This point should be used as part of a ProtoTrajectory in order to construct
 * the ideal trajectory that the drone should run on.
 * It is useful to define the ProtoPoint with a spatial position and a grade
 * of interest. The latter is used to determine the order of the curve for that
 * particular point. If the interest is 0, it will be used as a destination for
 * the drone.
 * If the point is a destination, you can set the time, in seconds, for which
 * the drone will rest on.
 */
class ProtoPoint : public Object
{
public:
  /**
   * Register the type using ns-3 TypeId System.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /**
   * The default constructor. Initialize a point at (0, 0, 0) with interest 0
   * and rest time of 0 seconds.
   */
  ProtoPoint ();
  /**
   * Destruct the object.
   */
  virtual ~ProtoPoint ();

  /**
   * Set the position of the point of interest.
   *
   * \param position The new position.
   */
  void SetPosition (const Vector &position);
  /**
   * \return The position of the point of interest.
   */
  const Vector GetPosition () const;

  /**
   * Set the interest level of the point of interest.
   *
   * \param value The new level of interest.
   */
  void SetInterest (const uint32_t &interest);
  /**
   * \return The current interest level for the point of interest.
   */
  const uint32_t GetInterest () const;

  /**
   * Set the time to rest at this destination in seconds.
   * Note that if the point is not a destination, this property has no effect.
   *
   * \param seconds The time to rest.
   */
  void SetRestTime (const Time &seconds);
  /**
   * \return The current time to rest applied to this point of interest.
   */
  const Time GetRestTime () const;

private:
  /**
   * \brief The position of the point of interest.
   */
  Vector   m_position;
  /**
   * \brief The interest level of the point of interest. If this level is 0, then this
   * point will be used as a destination.
   */
  uint32_t m_interest;
  /**
   * \brief The time, in seconds, that the drone will rest at this point of
   * destination.
   */
  Time     m_restTime;
};

} // namespace ns3

#endif /* PROTO_POINT_H */
