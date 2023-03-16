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
#include "drone-control-layer.h"

#include <ns3/log.h>
#include <ns3/string.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DroneControlLayer");
NS_OBJECT_ENSURE_REGISTERED (DroneControlLayer);

TypeId
DroneControlLayer::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::DroneControlLayer")
    .AddConstructor<DroneControlLayer> ()
    .SetParent<ProtocolLayer> ()
    .AddAttribute ("NotImplemented", "This is a placeholder for future developments.",
                   StringValue (),
                   MakeStringAccessor (&DroneControlLayer::m_notImplemented),
                   MakeStringChecker ())
    ;

  return tid;
}

DroneControlLayer::DroneControlLayer ()
{
  NS_LOG_FUNCTION (this);
}

DroneControlLayer::~DroneControlLayer ()
{
  NS_LOG_FUNCTION (this);
}

void
DroneControlLayer::Write (xmlTextWriterPtr h)
{
  NS_LOG_FUNCTION (h);
  if (h == nullptr)
    {
      NS_LOG_WARN ("Passed handler is not valid: " << h << ". "
                   "Data will be discarded.");
      return;
    }

  int rc;

  rc = xmlTextWriterStartElement (h, BAD_CAST "dcl");
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteElement (h, BAD_CAST "notImplemented", BAD_CAST "");
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterEndElement (h);
  NS_ASSERT (rc >= 0);
}

} // namespace ns3
