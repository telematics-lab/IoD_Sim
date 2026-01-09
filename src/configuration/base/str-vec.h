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
#ifndef STRING_VECTOR_H
#define STRING_VECTOR_H

#include <ns3/attribute-helper.h>
#include <ns3/attribute.h>

#include <istream>
#include <ostream>
#include <string>
#include <vector>

namespace ns3
{

/**
 * \brief Definition of a new attribute type, in the form vector<std::string>.
 */
class StrVec
{
  public:
    typedef std::vector<std::string>::const_iterator Iterator;

    StrVec() = default;
    StrVec(std::vector<std::string> coefficients);
    StrVec(const StrVec& a);

    Iterator Begin() const;
    Iterator begin() const;
    Iterator End() const;
    Iterator end() const;

    uint32_t GetN() const;

    std::vector<std::string> Get() const;
    std::string Get(const uint32_t i) const;

    std::string GetFront() const;
    std::string GetBack() const;

    void Add(std::string str);

  private:
    std::vector<std::string> m_stringVector;
};

ATTRIBUTE_HELPER_HEADER(StrVec);

std::ostream& operator<<(std::ostream& os, const StrVec& sv);
std::istream& operator>>(std::istream& is, StrVec& sv);

} // namespace ns3

#endif /* STRING_VECTOR_H */
