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
#include "ipv4-layer.h"

#include <ns3/log.h>
#include <ns3/string.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Ipv4Layer");
NS_OBJECT_ENSURE_REGISTERED (Ipv4Layer);

TypeId
Ipv4Layer::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv4Layer")
    .AddConstructor<Ipv4Layer> ()
    .SetParent<ProtocolLayer> ()
    .AddAttribute ("Ipv4Address", "The IPv4 Address",
                   StringValue (),
                   MakeStringAccessor (&Ipv4Layer::m_hostAddr),
                   MakeStringChecker ())
    .AddAttribute ("BroadcastAddress", "Broadcast Address",
                   StringValue (),
                   MakeStringAccessor (&Ipv4Layer::m_broadcastAddr),
                   MakeStringChecker ())
    ;

  return tid;
}

Ipv4Layer::Ipv4Layer ()
{
  NS_LOG_FUNCTION (this);
}

Ipv4Layer::~Ipv4Layer ()
{
  NS_LOG_FUNCTION (this);
}

void
Ipv4Layer::Write (xmlTextWriterPtr h)
{
  NS_LOG_FUNCTION (h);
  if (h == nullptr)
    {
      NS_LOG_WARN ("Passed handler is not valid: " << h << ". "
                   "Data will be discarded.");
      return;
    }

  int rc;

  rc = xmlTextWriterStartElement(h, BAD_CAST "ipv4");
  NS_ASSERT (rc >= 0);

  /* Nested Elements */
  rc = xmlTextWriterWriteElement(h,
                                 BAD_CAST "address",
                                 BAD_CAST m_hostAddr.c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteElement(h,
                                 BAD_CAST "broadcast",
                                 BAD_CAST m_broadcastAddr.c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterEndElement(h);
  NS_ASSERT (rc >= 0);
}

} // namespace ns3
