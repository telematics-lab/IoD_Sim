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
#include "model-configuration-vector.h"

namespace ns3
{

ATTRIBUTE_HELPER_CPP(ModelConfigurationVector);

ModelConfigurationVector::ModelConfigurationVector(ModelConfiguration conf)
    : m_confs{conf}
{
}

ModelConfigurationVector::ModelConfigurationVector(const std::vector<ModelConfiguration>& confs)
    : m_confs{confs}
{
}

ModelConfigurationVector::ModelConfigurationVector(const ModelConfigurationVector& v1,
                                                   const ModelConfigurationVector& v2)
{
    Add(v1);
    Add(v2);
}

ModelConfigurationVector::ModelConfigurationVector(const ModelConfigurationVector& v1,
                                                   const ModelConfigurationVector& v2,
                                                   const ModelConfigurationVector& v3)
{
    Add(v1);
    Add(v2);
    Add(v3);
}

ModelConfigurationVector::ModelConfigurationVector(const ModelConfigurationVector& v1,
                                                   const ModelConfigurationVector& v2,
                                                   const ModelConfigurationVector& v3,
                                                   const ModelConfigurationVector& v4)
{
    Add(v1);
    Add(v2);
    Add(v3);
    Add(v4);
}

ModelConfigurationVector::ModelConfigurationVector(const ModelConfigurationVector& v1,
                                                   const ModelConfigurationVector& v2,
                                                   const ModelConfigurationVector& v3,
                                                   const ModelConfigurationVector& v4,
                                                   const ModelConfigurationVector& v5)
{
    Add(v1);
    Add(v2);
    Add(v3);
    Add(v4);
    Add(v5);
}

ModelConfigurationVector::Iterator
ModelConfigurationVector::Begin() const
{
    return m_confs.begin();
}

ModelConfigurationVector::Iterator
ModelConfigurationVector::begin() const
{
    return Begin();
}

ModelConfigurationVector::MutableIterator
ModelConfigurationVector::MutableBegin()
{
    return m_confs.begin();
}

ModelConfigurationVector::Iterator
ModelConfigurationVector::End() const
{
    return m_confs.end();
}

ModelConfigurationVector::Iterator
ModelConfigurationVector::end() const
{
    return End();
}

ModelConfigurationVector::MutableIterator
ModelConfigurationVector::MutableEnd()
{
    return m_confs.end();
}

uint32_t
ModelConfigurationVector::GetN() const
{
    return m_confs.size();
}

ModelConfiguration
ModelConfigurationVector::operator[](uint32_t i) const
{
    return m_confs[i];
}

ModelConfiguration
ModelConfigurationVector::Get(uint32_t i) const
{
    return m_confs[i];
}

ModelConfiguration
ModelConfigurationVector::GetFront() const
{
    return m_confs.front();
}

ModelConfiguration
ModelConfigurationVector::GetBack() const
{
    return m_confs.back();
}

void
ModelConfigurationVector::Add(const ModelConfigurationVector& v)
{
    for (auto i = v.Begin(); i != v.End(); ++i)
        Add(*i);
}

void
ModelConfigurationVector::Add(ModelConfiguration conf)
{
    m_confs.push_back(conf);
}

std::ostream&
operator<<(std::ostream& os, const ModelConfigurationVector& vector)
{
    os << vector.GetN() << ";";

    for (auto i = vector.Begin(); i != vector.End(); ++i)
        os << *i << ";";

    return os;
}

std::istream&
operator>>(std::istream& is, ModelConfigurationVector& vector)
{
    uint32_t n;
    is >> n;
    is.ignore(1);

    for (uint32_t i = 0; i < n; ++i)
    {
        ModelConfiguration conf;
        is >> conf;
        vector.Add(conf);

        is.ignore(1);
    }

    return is;
}

} // namespace ns3
