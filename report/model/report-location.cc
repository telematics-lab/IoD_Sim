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
#include "report-location.h"

#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ReportLocation");

ReportLocation::ReportLocation (Vector position, Time instant) :
  m_position {position},
  m_instant {instant}
{
  NS_LOG_FUNCTION (this << position << instant);
}

ReportLocation::ReportLocation ()
{
  NS_LOG_FUNCTION (this);
}

ReportLocation::~ReportLocation ()
{
  NS_LOG_FUNCTION (this);
}

void
ReportLocation::Write (xmlTextWriterPtr h)
{
  NS_LOG_FUNCTION (h);
  if (h == nullptr)
    {
      NS_LOG_WARN ("Passed handler is not valid: " << h << ". "
                   "Data will be discarded.");
      return;
    }

  int rc;

  rc = xmlTextWriterStartElement(h, BAD_CAST "position");
  NS_ASSERT (rc >= 0);

  /* Nested Elements */
  std::stringstream bX, bY, bZ, bT;
  bX << m_position.x;
  bY << m_position.y;
  bZ << m_position.z;
  bT << m_instant.GetNanoSeconds ();

  rc = xmlTextWriterWriteElement (h, BAD_CAST "x", BAD_CAST bX.str ().c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteElement (h, BAD_CAST "y", BAD_CAST bY.str ().c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteElement (h, BAD_CAST "z", BAD_CAST bZ.str ().c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteElement (h, BAD_CAST "t", BAD_CAST bT.str ().c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterEndElement(h);
  NS_ASSERT (rc >= 0);
}

} // namespace ns3
