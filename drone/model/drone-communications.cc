/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2021 The IoD_Sim Authors.
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
#include "drone-communications.h"

#include <cstring>

namespace ns3 {

ATTRIBUTE_VALUE_IMPLEMENT (PacketType);
ATTRIBUTE_CHECKER_IMPLEMENT (PacketType);

PacketType::PacketType (const char *a)
{
  for (uint8_t i = 0; i < numValues; i++)
    {
      if (std::strcmp (a, stringValue[i]) == 0)
        {
          m_value = Value (i);
          return;
        }
    }

  NS_FATAL_ERROR ("Cannot decode packet type correctly: " << a);
}

PacketType::PacketType (const std::string a)
{
  const char *aC = a.c_str ();

  for (uint8_t i = 0; i < numValues; i++)
    {
      if (std::strcmp (aC, stringValue[i]) == 0)
        {
          m_value = Value (i);
          return;
        }
    }

  NS_FATAL_ERROR ("Cannot decode packet type correctly: " << aC);
}

PacketType::PacketType (uint8_t i)
{
  if (i < numValues)
    m_value = Value (i);
  else
    NS_FATAL_ERROR ("PacketType Index is out of range: " << i);
}

std::istream &
operator >> (std::istream &is, PacketType &packetType)
{
  std::string packetString;

  is >> packetString;
  packetType = PacketType (packetString);

  return is;
}

} // namespace ns3
