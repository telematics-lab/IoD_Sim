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
 *
 * Authors: Giovanni Grieco <giovanni.grieco@poliba.it>
 */
#include "transfer-direction.h"

#include <cstring>

namespace ns3 {

ATTRIBUTE_VALUE_IMPLEMENT (TransferDirection);
ATTRIBUTE_CHECKER_IMPLEMENT (TransferDirection);

TransferDirection::TransferDirection (const char *a)
{
  for (uint32_t i = 0; i < numValues; i++)
    {
      if (std::strcmp (a, stringValue[i]) == 0)
        {
          m_value = Value (i);
          return;
        }
    }

  NS_FATAL_ERROR ("Cannot decode transfer direction correctly: " << a);
}

TransferDirection::TransferDirection (const std::string a) :
  TransferDirection (a.c_str ())
{

}

std::istream &
operator >> (std::istream &is, TransferDirection &td)
{
  std::string tdString;

  is >> tdString;
  td = TransferDirection (tdString);

  return is;
}

} // namespace ns3
