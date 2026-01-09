/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2026 The IoD_Sim Authors.
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

ATTRIBUTE_HELPER_CPP(VectorType<T>);

template <typename T>
VectorType()
{
}

template <typename T>
VectorType::VectorType(std::vector<int> coeffs)
{
    for (auto c : coeffs)
    {
        Add(c);
    }
}

VectorType::VectorType(const VectorType& a)
{
    for (auto c = a.Begin(); c != a.End(); c++)
    {
        Add(*c);
    }
}

VectorType::Iterator
VectorType::Begin() const
{
    return m_VectorType.begin();
}

VectorType::Iterator
VectorType::End() const
{
    return m_VectorType.end();
}

uint32_t
VectorType::GetN() const
{
    return m_VectorType.size();
}

std::vector<int>
VectorType::Get() const
{
    std::vector<int> v;

    for (auto c : m_VectorType)
    {
        v.push_back(c);
    }

    return v;
}

int
VectorType::Get(const uint32_t i) const
{
    return m_VectorType[i];
}

int
VectorType::GetFront() const
{
    return m_VectorType.front();
}

int
VectorType::GetBack() const
{
    return m_VectorType.back();
}

void
VectorType::Add(int coeff)
{
    m_VectorType.push_back(coeff);
}

std::ostream&
operator<<(std::ostream& os, const VectorType& coeffs)
{
    os << coeffs.GetN() << ";";

    for (auto coeff = coeffs.Begin(); coeff != coeffs.End(); coeff++)
    {
        os << (*coeff) << ";";
    }

    return os;
}

std::istream&
operator>>(std::istream& is, VectorType& coeffs)
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
