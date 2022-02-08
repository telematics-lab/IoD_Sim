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
#ifndef INTEREST_REGION_H
#define INTEREST_REGION_H

#include <ns3/object.h>
#include <ns3/vector.h>
#include <ns3/box.h>
#include "double-vector.h"

namespace ns3 {

/**
 * \brief A 3D region of interest modelled as a box
 */
class InterestRegion: public Object
{
public:
  /**
   * \brief Get the type ID.
   *
   * \returns the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief default constructor
   */
  InterestRegion ();
  /**
   * \brief default destructor
   */
  virtual ~InterestRegion ();
  /**
   * \return the 3D coordinates of the box
   */
  const DoubleVector GetCoordinates () const;
  /**
   * \brief Sets the 3D coordinates of the box
   *
   * \param coords DoubleVector containing 3D coordinates of the box
   */
  void SetCoordinates (const DoubleVector &coords);

  bool IsInside (const Vector &position) const;

protected:
  virtual void DoDispose (void);
  virtual void DoInitialize (void);

private:
  Box m_box;
  DoubleVector m_coordinates;
};

} // namespace ns3

#endif /* CURVE_POINT_H */
