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
#include "model-configuration-matrix.h"

namespace ns3
{

ATTRIBUTE_HELPER_CPP(ModelConfigurationMatrix);

ModelConfigurationMatrix::ModelConfigurationMatrix()
{
}

ModelConfigurationMatrix::ModelConfigurationMatrix(ModelConfigurationVector conf)
    : m_confs{conf}
{
}

ModelConfigurationMatrix::ModelConfigurationMatrix(
    const std::vector<ModelConfigurationVector>& confs)
    : m_confs{confs}
{
}

ModelConfigurationMatrix::ModelConfigurationMatrix(const ModelConfigurationMatrix& v1,
                                                   const ModelConfigurationMatrix& v2)
{
    Add(v1);
    Add(v2);
}

ModelConfigurationMatrix::ModelConfigurationMatrix(const ModelConfigurationMatrix& v1,
                                                   const ModelConfigurationMatrix& v2,
                                                   const ModelConfigurationMatrix& v3)
{
    Add(v1);
    Add(v2);
    Add(v3);
}

ModelConfigurationMatrix::ModelConfigurationMatrix(const ModelConfigurationMatrix& v1,
                                                   const ModelConfigurationMatrix& v2,
                                                   const ModelConfigurationMatrix& v3,
                                                   const ModelConfigurationMatrix& v4)
{
    Add(v1);
    Add(v2);
    Add(v3);
    Add(v4);
}

ModelConfigurationMatrix::ModelConfigurationMatrix(const ModelConfigurationMatrix& v1,
                                                   const ModelConfigurationMatrix& v2,
                                                   const ModelConfigurationMatrix& v3,
                                                   const ModelConfigurationMatrix& v4,
                                                   const ModelConfigurationMatrix& v5)
{
    Add(v1);
    Add(v2);
    Add(v3);
    Add(v4);
    Add(v5);
}

ModelConfigurationMatrix::Iterator
ModelConfigurationMatrix::Begin() const
{
    return m_confs.begin();
}

ModelConfigurationMatrix::Iterator
ModelConfigurationMatrix::begin() const
{
    return Begin();
}

ModelConfigurationMatrix::MutableIterator
ModelConfigurationMatrix::MutableBegin()
{
    return m_confs.begin();
}

ModelConfigurationMatrix::Iterator
ModelConfigurationMatrix::End() const
{
    return m_confs.end();
}

ModelConfigurationMatrix::Iterator
ModelConfigurationMatrix::end() const
{
    return End();
}

ModelConfigurationMatrix::MutableIterator
ModelConfigurationMatrix::MutableEnd()
{
    return m_confs.end();
}

uint32_t
ModelConfigurationMatrix::GetN() const
{
    return m_confs.size();
}

ModelConfigurationVector
ModelConfigurationMatrix::operator[](uint32_t i) const
{
    return m_confs[i];
}

ModelConfigurationVector
ModelConfigurationMatrix::Get(uint32_t i) const
{
    return m_confs[i];
}

ModelConfigurationVector
ModelConfigurationMatrix::GetFront() const
{
    return m_confs.front();
}

ModelConfigurationVector
ModelConfigurationMatrix::GetBack() const
{
    return m_confs.back();
}

void
ModelConfigurationMatrix::Add(const ModelConfigurationMatrix& v)
{
    for (auto i = v.Begin(); i != v.End(); ++i)
        Add(*i);
}

void
ModelConfigurationMatrix::Add(ModelConfigurationVector conf)
{
    m_confs.push_back(conf);
}

std::ostream&
operator<<(std::ostream& os, const ModelConfigurationMatrix& vector)
{
    os << vector.GetN() << ";";

    for (auto i = vector.Begin(); i != vector.End(); ++i)
        os << *i << ";";

    return os;
}

std::istream&
operator>>(std::istream& is, ModelConfigurationMatrix& vector)
{
    uint32_t n;
    is >> n;
    is.ignore(1);

    for (uint32_t i = 0; i < n; ++i)
    {
        ModelConfigurationVector conf;
        is >> conf;
        vector.Add(conf);

        is.ignore(1);
    }

    return is;
}

} // namespace ns3
