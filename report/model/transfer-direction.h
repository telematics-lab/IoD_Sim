/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2022 The IoD_Sim Authors.
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
 *
 * Authors: Giovanni Grieco <giovanni.grieco@poliba.it>
 */
#ifndef TRANSFER_DIRECTION_H
#define TRANSFER_DIRECTION_H

#include <ns3/attribute-helper.h>

namespace ns3 {

class TransferDirection
{
public:
  enum Value : uint8_t
  {
    Received,
    Transmitted
  };

  constexpr static const uint32_t numValues = 2;

  const char *stringValue[numValues] = {
    "Rx",
    "Tx"
  };

  TransferDirection () = default;
  constexpr TransferDirection (Value aDirection) : m_value {aDirection} {}
  /**
   * From C String
   */
  TransferDirection (const char *a);
  /**
   * From C++ String
   */
  TransferDirection (const std::string a);

  operator Value () const { return m_value; }
  explicit operator bool () = delete;

  constexpr bool operator == (Value a) { return m_value == a; }
  constexpr bool operator != (Value a) { return m_value == a; }

  constexpr const char * ToString () const { return stringValue[m_value]; }

private:
  Value m_value;
};

ATTRIBUTE_VALUE_DEFINE (TransferDirection);
ATTRIBUTE_ACCESSOR_DEFINE (TransferDirection);
ATTRIBUTE_CHECKER_DEFINE (TransferDirection);

std::istream & operator >> (std::istream &is, TransferDirection &td);

} // namespace ns3

#endif /* TRANSFER_DIRECTION_H */
