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
#include "string-vector.h"

#include <ns3/integer.h>
#include <ns3/names.h>
#include <ns3/object-factory.h>

namespace ns3 {

ATTRIBUTE_HELPER_CPP (StringVector);

StringVector::StringVector ()
{
}

StringVector::StringVector (std::vector<std::string> strs)
{
  for (auto s : strs)
    Add (s);
}

StringVector::StringVector (const StringVector &a)
{
  for (auto c = a.Begin (); c != a.End (); c++)
    Add (*c);
}

StringVector::Iterator
StringVector::Begin () const
{
  return m_stringVector.begin ();
}

StringVector::Iterator
StringVector::begin () const
{
  return m_stringVector.begin ();
}

StringVector::Iterator
StringVector::End () const
{
  return m_stringVector.end ();
}

StringVector::Iterator
StringVector::end () const
{
  return m_stringVector.end ();
}

uint32_t
StringVector::GetN () const
{
  return m_stringVector.size ();
}

std::vector<std::string>
StringVector::Get () const
{
  return m_stringVector;
}

std::string
StringVector::Get (const uint32_t i) const
{
  return m_stringVector[i];
}

std::string
StringVector::GetFront () const
{
  return m_stringVector.front ();
}

std::string
StringVector::GetBack () const
{
  return m_stringVector.back ();
}

void
StringVector::Add (std::string s)
{
  m_stringVector.push_back (s);
}

std::ostream &
operator<< (std::ostream &os, const StringVector &v)
{
  os << v.GetN () << ";";

  for (auto s = v.Begin ();
       s != v.End ();
       s++)
    {
      os << (*s) << ";";
    }

  return os;
}

std::istream &
operator>> (std::istream &is, StringVector &v)
{
  char separator = '\0';
  uint32_t n;
  std::string s;

  is >> n >> separator;
  if (separator != ';')
    is.setstate (std::ios::failbit);

  for (uint32_t i = 0; i < n; i++)
    {
      is >> s >> separator;

      if (separator != ';')
        {
          is.setstate (std::ios::failbit);
          break;
        }

      v.Add (s);
    }

  return is;
}

} // namespace ns3
