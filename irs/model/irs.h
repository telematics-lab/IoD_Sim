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
#ifndef IRS_H
#define IRS_H

#include <ns3/vector.h>
#include <ns3/object.h>
#include <vector>

namespace ns3 {


/**
 * \brief Base class describing an Intelligent Reflective Surface aggregable to a node.
 */
class Irs : public Object
{
public:

  /**
   * \brief Get the type ID.
   *
   * \returns the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief constructor used to instantiate an IRS.
   *
   * \param p1               first point used to define a plane and his normal.
   * \param p2               second point used to define a plane and his normal.
   * \param p3               third point used to define a plane and his normal.
   *
   */
  Irs (Vector p1, Vector p2, Vector p3 );

  /**
   * \brief Sets the number of IRS rows.
   *
   * \param r Rows of the IRS.
   */
  void setRows (int r);

  /**
   * \brief Returns the number of IRS rows .
   *
   * \returns Rows.
   */
  double getRows () const;

protected:
  virtual void DoDispose (void);
  virtual void DoInitialize (void);

private:
  int m_rows; //N, the number of rows
  int m_columns; //M, the number of columns
  double m_pruSide; //Side dimension of the Passive Reflective Unit (in meter)
  std::vector<double> m_directorCoeff; //Director coefficient of the plane passing from p1, p2 and p3
};

} //namespace ns3

#endif /* IRS_H */
