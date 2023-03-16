/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2023 The IoD_Sim Authors.
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
#include "report-data-stats.h"

#include <ns3/integer.h>
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ReportDataStats");

TypeId
ReportDataStats::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ReportDataStats")
    .AddConstructor<ReportDataStats> ()
    .SetParent<Object> ()
    .AddAttribute ("kind", "Type of data transfer",
                   PacketTypeValue (),
                   MakePacketTypeAccessor (&ReportDataStats::m_kind),
                   MakePacketTypeChecker ())
    .AddAttribute ("amount", "The amount of bytes transferred",
                   IntegerValue (0),
                   MakeIntegerAccessor (&ReportDataStats::m_bytes),
                   MakeIntegerChecker<uint32_t> ())
    ;

  return tid;
}

ReportDataStats::ReportDataStats () :
  m_kind {PacketType::UNKNOWN},
  m_bytes {0}
{
  NS_LOG_FUNCTION (this);
}

ReportDataStats::~ReportDataStats ()
{
  NS_LOG_FUNCTION (this);
}

void
ReportDataStats::Write (xmlTextWriterPtr h) const
{
  NS_LOG_FUNCTION (h);
  if (h == nullptr)
    {
      NS_LOG_WARN ("Passed handler is not valid: " << h << ". "
                   "Data will be discarded.");
      return;
    }

  std::ostringstream bBytes;
  int rc;

  bBytes << m_bytes;
  rc = xmlTextWriterWriteElement (h,
                                  BAD_CAST m_kind.ToString (),
                                  BAD_CAST bBytes.str ().c_str ());
  NS_ASSERT (rc >= 0);
}

} // namespace ns3
