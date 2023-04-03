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
#include "str-vec.h"

#include <ns3/integer.h>
#include <ns3/names.h>
#include <ns3/object-factory.h>

namespace ns3
{

ATTRIBUTE_HELPER_CPP(StrVec);

StrVec::StrVec()
{
}

StrVec::StrVec(std::vector<std::string> strs)
{
    for (auto s : strs)
        Add(s);
}

StrVec::StrVec(const StrVec& a)
{
    for (auto c = a.Begin(); c != a.End(); c++)
        Add(*c);
}

StrVec::Iterator
StrVec::Begin() const
{
    return m_stringVector.begin();
}

StrVec::Iterator
StrVec::begin() const
{
    return m_stringVector.begin();
}

StrVec::Iterator
StrVec::End() const
{
    return m_stringVector.end();
}

StrVec::Iterator
StrVec::end() const
{
    return m_stringVector.end();
}

uint32_t
StrVec::GetN() const
{
    return m_stringVector.size();
}

std::vector<std::string>
StrVec::Get() const
{
    return m_stringVector;
}

std::string
StrVec::Get(const uint32_t i) const
{
    return m_stringVector[i];
}

std::string
StrVec::GetFront() const
{
    return m_stringVector.front();
}

std::string
StrVec::GetBack() const
{
    return m_stringVector.back();
}

void
StrVec::Add(std::string s)
{
    m_stringVector.push_back(s);
}

std::ostream&
operator<<(std::ostream& os, const StrVec& v)
{
    os << v.GetN() << ";";

    for (auto s = v.Begin(); s != v.End(); s++)
    {
        os << (*s) << ";";
    }

    return os;
}

std::istream&
operator>>(std::istream& is, StrVec& v)
{
    char separator = '\0';
    uint32_t n;
    std::string s;

    is >> n >> separator;
    if (separator != ';')
        is.setstate(std::ios::failbit);

    for (uint32_t i = 0; i < n; i++)
    {
        is >> s >> separator;

        if (separator != ';')
        {
            is.setstate(std::ios::failbit);
            break;
        }

        v.Add(s);
    }

    return is;
}

} // namespace ns3
