/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2023 The IoD_Sim Authors.
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
#ifndef INT_VECTOR_H
#define INT_VECTOR_H

#include <istream>
#include <ostream>
#include <vector>

#include <ns3/attribute.h>
#include <ns3/attribute-helper.h>
#include <ns3/integer.h>

namespace ns3 {

/**
 * \brief Definition of a new attribute type, in the form vector<int>.
 */
class IntVector
{
public:
  typedef std::vector<int>::const_iterator Iterator;

  IntVector ();
  IntVector (std::vector<int> coefficients);
  IntVector (const IntVector &a);

  Iterator Begin () const;
  Iterator End () const;

  uint32_t GetN () const;

  std::vector<int> Get () const;
  int Get (const uint32_t i) const;

  int GetFront () const;
  int GetBack () const;

  void Add (int coefficient);

private:
  std::vector<int> m_IntVector;
};

ATTRIBUTE_HELPER_HEADER (IntVector);

std::ostream & operator<< (std::ostream &os,
                           const IntVector &IntVector);
std::istream & operator>> (std::istream &is,
                           IntVector &IntVector);

} // namespace ns3

#endif /* INT_VECTOR_H */
