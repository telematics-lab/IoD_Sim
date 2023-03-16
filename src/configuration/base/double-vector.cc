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
#include "double-vector.h"

#include <ns3/integer.h>
#include <ns3/names.h>
#include <ns3/object-factory.h>

namespace ns3 {

ATTRIBUTE_HELPER_CPP (DoubleVector);

DoubleVector::DoubleVector ()
{
}

DoubleVector::DoubleVector (std::vector<double> coeffs)
{
  for (auto c : coeffs)
    {
      Add (c);
    }
}

DoubleVector::DoubleVector (const DoubleVector &a)
{
  for (auto c = a.Begin (); c != a.End (); c++)
    {
      Add (*c);
    }
}

DoubleVector::Iterator
DoubleVector::Begin () const
{
  return m_DoubleVector.begin ();
}

DoubleVector::Iterator
DoubleVector::End () const
{
  return m_DoubleVector.end ();
}

uint32_t
DoubleVector::GetN () const
{
  return m_DoubleVector.size ();
}

std::vector<double>
DoubleVector::Get () const
{
  std::vector<double> v;

  for (auto c : m_DoubleVector)
    {
      v.push_back (c);
    }

  return v;
}

double
DoubleVector::Get (const uint32_t i) const
{
  return m_DoubleVector[i];
}

double
DoubleVector::GetFront () const
{
  return m_DoubleVector.front ();
}

double
DoubleVector::GetBack () const
{
  return m_DoubleVector.back ();
}

void
DoubleVector::Add (double coeff)
{
  m_DoubleVector.push_back (coeff);
}

std::ostream &
operator<< (std::ostream &os, const DoubleVector &coeffs)
{
  os << coeffs.GetN () << ";";

  for (auto coeff = coeffs.Begin ();
       coeff != coeffs.End ();
       coeff++)
    {
      os << (*coeff) << ";";
    }

  return os;
}

std::istream &
operator>> (std::istream &is, DoubleVector &coeffs)
{
  char separator = '\0';
  uint32_t n;
  double coeff;

  is >> n >> separator;
  if (separator != ';')
    is.setstate (std::ios::failbit);

  for (uint32_t i = 0; i < n; i++)
    {
      is >> coeff >> separator;

      if (separator != ';')
        {
          is.setstate (std::ios::failbit);
          break;
        }

      coeffs.Add (coeff);
    }

  return is;
}

} // namespace ns3
