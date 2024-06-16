/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2024 The IoD_Sim Authors.
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
#include "int-vector.h"

#include <ns3/integer.h>
#include <ns3/names.h>
#include <ns3/object-factory.h>

namespace ns3
{

ATTRIBUTE_HELPER_CPP(IntVector);

IntVector::IntVector(std::vector<int> coeffs)
{
    for (auto c : coeffs)
    {
        Add(c);
    }
}

IntVector::IntVector(const IntVector& a)
{
    for (auto c = a.Begin(); c != a.End(); c++)
    {
        Add(*c);
    }
}

IntVector::Iterator
IntVector::Begin() const
{
    return m_IntVector.begin();
}

IntVector::Iterator
IntVector::End() const
{
    return m_IntVector.end();
}

uint32_t
IntVector::GetN() const
{
    return m_IntVector.size();
}

std::vector<int>
IntVector::Get() const
{
    std::vector<int> v;

    for (auto c : m_IntVector)
    {
        v.push_back(c);
    }

    return v;
}

int
IntVector::Get(const uint32_t i) const
{
    return m_IntVector[i];
}

int
IntVector::GetFront() const
{
    return m_IntVector.front();
}

int
IntVector::GetBack() const
{
    return m_IntVector.back();
}

void
IntVector::Add(int coeff)
{
    m_IntVector.push_back(coeff);
}

std::ostream&
operator<<(std::ostream& os, const IntVector& coeffs)
{
    os << coeffs.GetN() << ";";

    for (auto coeff = coeffs.Begin(); coeff != coeffs.End(); coeff++)
    {
        os << (*coeff) << ";";
    }

    return os;
}

std::istream&
operator>>(std::istream& is, IntVector& coeffs)
{
    char separator = '\0';
    uint32_t n;
    int coeff;

    is >> n >> separator;
    if (separator != ';')
        is.setstate(std::ios::failbit);

    for (uint32_t i = 0; i < n; i++)
    {
        is >> coeff >> separator;

        if (separator != ';')
        {
            is.setstate(std::ios::failbit);
            break;
        }

        coeffs.Add(coeff);
    }

    return is;
}

} // namespace ns3
