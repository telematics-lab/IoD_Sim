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
#ifndef DOUBLE_VECTOR_H
#define DOUBLE_VECTOR_H

#include <ns3/attribute-helper.h>
#include <ns3/attribute.h>
#include <ns3/double.h>

#include <istream>
#include <ostream>
#include <vector>

namespace ns3
{

/**
 * \brief Definition of a new attribute type, in the form vector<double>.
 */
class DoubleVector
{
  public:
    typedef std::vector<double>::const_iterator Iterator;

    DoubleVector() = default;
    DoubleVector(std::vector<double> coefficients);
    DoubleVector(const DoubleVector& a);

    Iterator Begin() const;
    Iterator End() const;

    uint32_t GetN() const;

    std::vector<double> Get() const;
    double Get(const uint32_t i) const;

    double GetFront() const;
    double GetBack() const;

    void Add(double coefficient);

  private:
    std::vector<double> m_DoubleVector;
};

ATTRIBUTE_HELPER_HEADER(DoubleVector);

std::ostream& operator<<(std::ostream& os, const DoubleVector& DoubleVector);
std::istream& operator>>(std::istream& is, DoubleVector& DoubleVector);

} // namespace ns3

#endif /* DOUBLE_VECTOR_H */
