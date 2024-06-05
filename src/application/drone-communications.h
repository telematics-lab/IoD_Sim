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
#ifndef DRONE_COMMUNICATIONS_H
#define DRONE_COMMUNICATIONS_H

#include <ns3/attribute-helper.h>

#include <algorithm>
#include <iterator>

namespace ns3
{

class PacketType
{
  public:
    enum Value : uint8_t
    {
        HELLO,
        HELLO_ACK,
        UPDATE,
        UPDATE_ACK,
        UNKNOWN
    };

    constexpr static const uint8_t numValues = 5;

    const char* stringValue[numValues] = {"HELLO", "HELLO_ACK", "UPDATE", "UPDATE_ACK", "UNKNOWN"};

    PacketType() = default;

    constexpr PacketType(Value aType)
        : m_value{aType}
    {
    }

    /**
     * From C String
     */
    PacketType(const char* a);
    /**
     * From C++ String
     */
    PacketType(const std::string a);
    /**
     * From corresponding index
     */
    PacketType(uint8_t i);

    operator Value() const
    {
        return m_value;
    }

    explicit operator bool() = delete;

    constexpr bool operator==(Value a)
    {
        return m_value == a;
    }

    constexpr bool operator!=(Value a)
    {
        return m_value == a;
    }

    constexpr const char* ToString() const
    {
        return stringValue[m_value];
    }

  private:
    Value m_value;
};

ATTRIBUTE_VALUE_DEFINE(PacketType);
ATTRIBUTE_ACCESSOR_DEFINE(PacketType);
ATTRIBUTE_CHECKER_DEFINE(PacketType);

std::istream& operator>>(std::istream& is, PacketType& packetType);

} // namespace ns3

#endif /* DRONE_COMMUNICATIONS_H */
