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
 * Authors: Giovanni Grieco <giovanni.grieco@poliba.it>, Giovanni Iacovelli <giovanni.iacovelli@poliba.it>
 */
#ifndef REPORT_DRONE_H
#define REPORT_DRONE_H

#include <vector>

#include <ns3/ipv4.h>
#include <ns3/mobility-model.h>
#include <ns3/node.h>

#include <libxml/xmlwriter.h>

#include "report-data-stats.h"
#include "report-entity.h"
#include "report-location.h"
#include "report-protocol-stack.h"
#include "report-transfer.h"
#include "report-peripheral.h"

namespace ns3 {

/**
 * Retrieve and store data about a drone, like its
 *  - trajectory
 *  - network stacks
 *  - traffic (Rx and Tx)
 *  - and eventual cumulative statistics that can be derived
 */
class ReportDrone : public ReportEntity
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
  ReportDrone ();
  /**
   * Default destructor
   */
  ~ReportDrone ();

protected:
  void DoInitialize();

private:
  /**
   * Write internal interface
   *
   * \param handle the XML handler to write data on
   */
  void DoWrite (xmlTextWriterPtr h);

  /**
   * Initialize probe to get trajectory data
   */
  void DoInitializeTrajectoryMonitor ();

  /**
   * Callback to monitor drone trajectory
   *
   * \params mobility the mobility model of the drone to inspect
   */
  void DoMonitorTrajectory (const Ptr<const MobilityModel> mobility);

  /**
   * Initialize Peripherals
   */
  void DoInitializePeripherals ();


  /// drone trajectory
  std::vector<ReportLocation> m_trajectory;
  std::vector<int> m_roi;
  std::vector<ReportPeripheral> m_peripherals;
};

} // namespace ns3

#endif /* REPORT_DRONE_H */
