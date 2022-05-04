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
#include <ns3/pointer.h>
#include <ns3/mobility-model.h>
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
   */
  Irs ();

  /**
   * \brief Sets the number of IRS rows.
   *
   * \param r Rows of the IRS.
   */
  void SetRows (int r);

  /**
   * \brief Returns the number of IRS rows .
   *
   * \returns Rows.
   */
  int GetRows () const;

  /**
   * \brief Sets the number of IRS columns.
   *
   * \param c Columns of the IRS.
   */
  void SetColumns (int c);

  /**
   * \brief Returns the number of IRS columns .
   *
   * \returns Columns.
   */
  int GetColumns () const;

   /**
   * \brief Sets the X-side dimension of a single Passive Reflective Unit.
   *
   * \param d X-side dimension of the PRU.
   */
  void SetPruX (double d);

  /**
   * \brief Returns the X-side dimension of a single Passive Reflective Unit.
   *
   * \returns X-side dimension of the PRU.
   */
  double GetPruX () const;

     /**
   * \brief Sets the Y-side dimension of a single Passive Reflective Unit.
   *
   * \param d Y-side dimension of the PRU.
   */
  void SetPruY (double d);

  /**
   * \brief Returns the Y-side dimension of a single Passive Reflective Unit.
   *
   * \returns Y-side dimension of the PRU.
   */
  double GetPruY () const;
    /**
   * \brief TODO
   *
   * \param a TODO
   */
  void SetRotoAxis (std::vector<char> a);

  /**
   * \brief TODO
   *
   * \returns TODO
   */
  std::vector<char> GetRotoAxis () const;

    /**
   * \brief TODO
   *
   * \param a TODO
   */
  void SetRotoAnglesRadians (std::vector<double> a);

    /**
   * \brief TODO
   *
   * \param a TODO
   */
  void SetRotoAnglesDegrees (std::vector<double> a);

  /**
   * \brief TODO
   *
   * \returns TODO
   */
  std::vector<double> GetRotoAngles () const;

    /**
   * \brief TODO
   *
   * \returns TODO
   *
   * \param m TODO
   * \param n TODO
   * \param MM TODO
   */
  Vector CalcPruPosition (int m, int n, Ptr<MobilityModel> MM) const;

protected:
  virtual void DoDispose (void);
  virtual void DoInitialize (void);

private:
  int m_rows; //N, the number of rows
  int m_columns; //M, the number of columns
  double m_pruX; //X-side dimension of the Passive Reflective Unit (in meter)
  double m_pruY; //Y-side dimension of the Passive Reflective Unit (in meter)
  std::vector<char> m_rotoAxis; //Well-ordered list of axis (i.e. 'x','y' or 'z') used to set IRS rotation
  std::vector<double> m_rotoAngles; //List of angles (in radians) used to rotate the IRS, the n-th angles refers to n-th axis defined in m_rotoAxis

};

} //namespace ns3

#endif /* IRS_H */
