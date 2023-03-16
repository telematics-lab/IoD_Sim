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
#include "wifi-mac-layer.h"

#include <ns3/log.h>
#include <ns3/string.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WifiMacLayer");
NS_OBJECT_ENSURE_REGISTERED (WifiMacLayer);

TypeId
WifiMacLayer::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::WifiMacLayer")
    .AddConstructor<WifiMacLayer> ()
    .SetParent<ProtocolLayer> ()
    .AddAttribute ("Ssid", "The SSID of the BSS",
                   StringValue (),
                   MakeStringAccessor (&WifiMacLayer::m_ssid),
                   MakeStringChecker ())
    .AddAttribute ("Mode", "The mode of the Wifi",
                   StringValue (),
                   MakeStringAccessor (&WifiMacLayer::m_mode),
                   MakeStringChecker ())
    ;

  return tid;
}

WifiMacLayer::WifiMacLayer ()
{
  NS_LOG_FUNCTION (this);
}

WifiMacLayer::~WifiMacLayer ()
{
  NS_LOG_FUNCTION (this);
}

void
WifiMacLayer::Write (xmlTextWriterPtr h)
{
  NS_LOG_FUNCTION (h);
  if (h == nullptr)
    {
      NS_LOG_WARN ("Passed handler is not valid: " << h << ". "
                   "Data will be discarded.");
      return;
    }

  int rc;

  rc = xmlTextWriterStartElement(h, BAD_CAST "mac");
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteElement(h, BAD_CAST "ssid", BAD_CAST m_ssid.c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteElement(h, BAD_CAST "mode", BAD_CAST m_mode.c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterEndElement(h);
  NS_ASSERT (rc >= 0);
}

} // namespace ns3
