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
#ifndef REPORT_ZSP_H
#define REPORT_ZSP_H

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
 * Retrieve and store data about a ZSP, like its
 *  - position (presumably fixed)
 *  - network stack
 *  - traffic (Rx and Tx)
 *  - and eventual cumulative statistics that can be derived
 */
class ReportZsp : public ReportEntity
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
  ReportZsp ();
  /**
   * Default destructor
   */
  ~ReportZsp ();
private:
  /**
   * Write Zsp report data to a XML file with a given handler
   *
   * \param handle the XML handler to write data on
   */
  void DoWrite (xmlTextWriterPtr handle);

  /**
   * Get ZSP position
   */
  void DoInitializeTrajectoryMonitor ();

  /**
   * Explore a given zsp object and build an abstraction of its network stack
   */
  void DoInitializeNetworkStacks ();

  /**
   * Placeholder, monitor nothing because the ZSP in fixed in space
   *
   * \params mobility the mobility model of the entity to inspect
   */
  void DoMonitorTrajectory (const Ptr<const MobilityModel> mobility);

  /// ZSP position
  ReportLocation m_position;
  /// abstract representation of the network stack
  ReportProtocolStack m_stack;
};

} // namespace ns3

#endif /* REPORT_ZSP_H */
