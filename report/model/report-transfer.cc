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
#include "report-transfer.h"

#include <sstream>

#include <ns3/integer.h>
#include <ns3/log.h>
#include <ns3/string.h>

#include "transfer-direction.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ReportTransfer");

TypeId
ReportTransfer::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ReportTransfer")
    .AddConstructor<ReportTransfer> ()
    .SetParent<Object> ()
    .AddAttribute ("EntityID", "The id of entity",
                   IntegerValue (),
                   MakeIntegerAccessor (&ReportTransfer::m_entityid),
                   MakeIntegerChecker<std::uint32_t> ())
    .AddAttribute ("Interface", "The ipv4 interface number",
                   IntegerValue (),
                   MakeIntegerAccessor (&ReportTransfer::m_iface),
                   MakeIntegerChecker<std::int32_t> ())
    .AddAttribute ("PacketType", "The Type of the Packet",
                   PacketTypeValue (),
                   MakePacketTypeAccessor (&ReportTransfer::m_type),
                   MakePacketTypeChecker ())
    .AddAttribute ("TransferDirection", "The direction of the transferred data",
                   TransferDirectionValue (),
                   MakeTransferDirectionAccessor (&ReportTransfer::m_direction),
                   MakeTransferDirectionChecker ())
    .AddAttribute ("Time", "The Simulation Time",
                   TimeValue (),
                   MakeTimeAccessor (&ReportTransfer::m_time),
                   MakeTimeChecker ())
    .AddAttribute ("SourceAddress", "The IPv4 Address of the sender",
                   StringValue (),
                   MakeStringAccessor (&ReportTransfer::m_sourceAddress),
                   MakeStringChecker ())
    .AddAttribute ("DestinationAddress", "The IPv4 Address of the target host",
                   StringValue (),
                   MakeStringAccessor (&ReportTransfer::m_destinationAddress),
                   MakeStringChecker ())
    .AddAttribute ("PacketLength", "The length of the payload",
                   IntegerValue (),
                   MakeIntegerAccessor (&ReportTransfer::m_length),
                   MakeIntegerChecker<std::uint32_t> ())
    .AddAttribute ("SequenceNumber", "The sequence number of the DCL packet",
                   IntegerValue (),
                   MakeIntegerAccessor (&ReportTransfer::m_sequenceNumber),
                   MakeIntegerChecker<std::uint32_t> ())
    .AddAttribute ("Payload", "The payload content",
                   StringValue (),
                   MakeStringAccessor (&ReportTransfer::m_payload),
                   MakeStringChecker ())
    ;

    return tid;
}

ReportTransfer::ReportTransfer ()
{
  NS_LOG_FUNCTION (this);
}

ReportTransfer::~ReportTransfer ()
{
  NS_LOG_FUNCTION (this);
}

int32_t
ReportTransfer::GetIface ()
{
  return m_iface;
}
void
ReportTransfer::Write (xmlTextWriterPtr h)
{
  NS_LOG_FUNCTION (h);
  if (h == nullptr)
    {
      NS_LOG_WARN ("Passed handler is not valid: " << h << ". "
                   "Data will be discarded.");
      return;
    }

  std::ostringstream bLength,
                     bTime,
                     bSequenceNumber;
  int rc;

  rc = xmlTextWriterStartElement (h, BAD_CAST "transfer");
  NS_ASSERT (rc >= 0);

  /* Attributes */
  rc = xmlTextWriterWriteAttribute (h,
                                    BAD_CAST "type",
                                    BAD_CAST m_type.ToString ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteAttribute (h,
                                    BAD_CAST "entityid",
                                    BAD_CAST std::to_string(m_entityid).c_str());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteAttribute (h,
                                    BAD_CAST "iface",
                                    BAD_CAST std::to_string(m_iface).c_str());
  NS_ASSERT (rc >= 0);

  /* Nested Elements */
  rc = xmlTextWriterWriteElement (h,
                                  BAD_CAST "direction",
                                  BAD_CAST m_direction.ToString ());
  NS_ASSERT (rc >= 0);

  bLength << m_length;
  rc = xmlTextWriterWriteElement (h,
                                  BAD_CAST "length",
                                  BAD_CAST bLength.str ().c_str ());
  NS_ASSERT (rc >= 0);

  bTime << m_time.GetNanoSeconds ();
  rc = xmlTextWriterWriteElement (h,
                                  BAD_CAST "time",
                                  BAD_CAST bTime.str ().c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteElement (h,
                                  BAD_CAST "sourceAddress",
                                  BAD_CAST m_sourceAddress.c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteElement (h,
                                  BAD_CAST "destinationAddress",
                                  BAD_CAST m_destinationAddress.c_str ());
  NS_ASSERT (rc >= 0);

  bSequenceNumber << m_sequenceNumber;
  rc = xmlTextWriterWriteElement (h,
                                  BAD_CAST "sequenceNumber",
                                  BAD_CAST bSequenceNumber.str ().c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteElement (h,
                                  BAD_CAST "payload",
                                  BAD_CAST m_payload.c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterEndElement (h);
  NS_ASSERT (rc >= 0);
}

} // namespace ns3
