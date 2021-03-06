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
#ifndef SPEED_COEFFICIENTS_H
#define SPEED_COEFFICIENTS_H

#include <istream>
#include <ostream>
#include <vector>

#include <ns3/attribute.h>
#include <ns3/attribute-helper.h>
#include <ns3/double.h>

namespace ns3 {

/**
 * \brief keep track of a set of coefficients.
 *
 * This is useful if we want to operate on more than one SpeedCoefficients
 * at a time or just keep them organized in a data structure and pass it to a
 * MobilityModel compatible with the IoD proposed strucutres.
 */
class SpeedCoefficients
{
public:
  typedef std::vector<double>::const_iterator Iterator;

  SpeedCoefficients ();
  SpeedCoefficients (std::vector<double> coefficients);
  SpeedCoefficients (const SpeedCoefficients &a);

  Iterator Begin () const;
  Iterator End () const;

  uint32_t GetN () const;

  std::vector<double> Get () const;
  double Get (const uint32_t i) const;

  double GetFront () const;
  double GetBack () const;

  void Add (double coefficient);

private:
  std::vector<double> m_speedCoefficients;
};

ATTRIBUTE_HELPER_HEADER (SpeedCoefficients);

std::ostream & operator<< (std::ostream &os,
                           const SpeedCoefficients &SpeedCoefficients);
std::istream & operator>> (std::istream &is,
                           SpeedCoefficients &SpeedCoefficients);

} // namespace ns3

#endif /* SPEED_COEFFICIENTS_H */
