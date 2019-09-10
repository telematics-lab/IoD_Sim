/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 The IoD_Sim Authors.
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
#include "wifi-phy-layer.h"

#include <ns3/integer.h>
#include <ns3/log.h>
#include <ns3/string.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("WifiPhyLayer");
NS_OBJECT_ENSURE_REGISTERED (WifiPhyLayer);

TypeId
WifiPhyLayer::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::WifiPhyLayer")
    .AddConstructor<WifiPhyLayer> ()
    .SetParent<ProtocolLayer> ()
    .AddAttribute ("Frequency", "Wifi Carrier Frequency in use, in MHz",
                   IntegerValue (0),
                   MakeIntegerAccessor (&WifiPhyLayer::m_frequency),
                   MakeIntegerChecker<uint16_t> ())
    .AddAttribute ("Standard", "Wifi Standard in use",
                   StringValue (),
                   MakeStringAccessor (&WifiPhyLayer::m_standard),
                   MakeStringChecker ())
    .AddAttribute ("PropagationDelayModel", "The propagation delay model used",
                   StringValue (),
                   MakeStringAccessor (&WifiPhyLayer::m_propagationDelayModel),
                   MakeStringChecker ())
    .AddAttribute ("PropagationLossModel", "The propagation loss model used",
                   StringValue (),
                   MakeStringAccessor (&WifiPhyLayer::m_propagationLossModel),
                   MakeStringChecker ())
    ;

  return tid;
}

WifiPhyLayer::WifiPhyLayer ()
{
  NS_LOG_FUNCTION (this);
}

WifiPhyLayer::~WifiPhyLayer ()
{
  NS_LOG_FUNCTION (this);
}

void
WifiPhyLayer::Write (xmlTextWriterPtr h)
{
  NS_LOG_FUNCTION (h);
  if (h == nullptr)
    {
      NS_LOG_WARN ("Passed handler is not valid: " << h << ". "
                   "Data will be discarded.");
      return;
    }

  int rc;

  rc = xmlTextWriterStartElement(h, BAD_CAST "phy");
  NS_ASSERT (rc >= 0);

  /* Nested Elements */
  rc = xmlTextWriterWriteElement(h,
                                 BAD_CAST "standard",
                                 BAD_CAST m_standard.c_str ());
  NS_ASSERT (rc >= 0);

  std::stringstream bFrequency;
  bFrequency << m_frequency;
  rc = xmlTextWriterWriteElement (h,
                                  BAD_CAST "frequency",
                                  BAD_CAST bFrequency.str ().c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteElement (h,
                                  BAD_CAST "propagationDelayModel",
                                  BAD_CAST m_propagationDelayModel.c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteElement (h,
                                  BAD_CAST "propagationLossModel",
                                  BAD_CAST m_propagationLossModel.c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterEndElement(h);
  NS_ASSERT (rc >= 0);
}

} // namespace ns3
