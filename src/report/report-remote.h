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
 * Authors: Giovanni Iacovelli <giovanni.iacovelli@poliba.it>
 */
#ifndef REPORT_REMOTE_H
#define REPORT_REMOTE_H

#include <ns3/ipv4.h>
#include <ns3/mobility-model.h>
#include <ns3/object.h>
#include <ns3/packet.h>

#include <libxml/xmlwriter.h>

#include "report-data-stats.h"
#include "report-entity.h"
#include "report-location.h"
#include "report-protocol-stack.h"
#include "report-transfer.h"

namespace ns3 {

/**
 * Retrieve and store data about a Remote, like its
 *  - network stack
 *  - traffic (Rx and Tx)
 *  - and eventual cumulative statistics that can be derived
 */
class ReportRemote : public ReportEntity
{
public:
  /**
   * Register the type using ns-3 TypeId System.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /**
   * Default constructor
   */
  ReportRemote ();
  /**
   * Default destructor
   */
  ~ReportRemote ();
private:
  /**
   * Write Remote report data to a XML file with a given handler
   *
   * \param handle the XML handler to write data on
   */
  void DoWrite (xmlTextWriterPtr handle);

  /// abstract representation of the network stack
  ReportProtocolStack m_stack;
};

} // namespace ns3

#endif /* REPORT_REMOTE_H */
