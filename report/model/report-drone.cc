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
#include "report-drone.h"

#include <ns3/config.h>
#include <ns3/drone-communications.h>
#include <ns3/integer.h>
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/log.h>
#include <ns3/node.h>
#include <ns3/node-list.h>
#include <ns3/simulator.h>
#include <ns3/string.h>
#include <ns3/udp-header.h>
#include <ns3/interest-region-container.h>
#include <ns3/drone.h>
#include <ns3/drone-list.h>
#include <ns3/report-helper.h>
#include <ns3/storage-peripheral.h>
#include <ns3/input-peripheral.h>

#include <rapidjson/document.h>

#include "drone-control-layer.h"
#include "ipv4-layer.h"
#include "wifi-inspector.h"
#include "wifi-mac-layer.h"
#include "wifi-phy-layer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ReportDrone");
NS_OBJECT_ENSURE_REGISTERED (ReportDrone);

TypeId
ReportDrone::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ReportDrone")
    .AddConstructor<ReportDrone> ()
    .SetParent<ReportEntity> ()
    ;

  return tid;
}

ReportDrone::ReportDrone ()
{
  DoInitializePeripherals();
  NS_LOG_FUNCTION (this);
}

ReportDrone::~ReportDrone ()
{
  NS_LOG_FUNCTION (this);
}

void
ReportDrone::DoWrite (xmlTextWriterPtr h)
{
  NS_LOG_FUNCTION (h);
  if (h == nullptr)
    {
      NS_LOG_WARN ("Passed handler is not valid: " << h << ". "
                   "Data will be discarded.");
      return;
    }

  int rc;

  rc = xmlTextWriterStartElement (h, BAD_CAST "Drone");
  NS_ASSERT (rc >= 0);

  /* Nested Elements */
  rc = xmlTextWriterStartElement (h, BAD_CAST "trajectory");
  NS_ASSERT (rc >= 0);

  for (auto& location : m_trajectory)
    location.Write (h);

  rc = xmlTextWriterEndElement (h);
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterStartElement (h, BAD_CAST "Peripherals");
  NS_ASSERT (rc >= 0);

  for (auto p : m_peripherals)
    p.Write (h);

  rc = xmlTextWriterEndElement (h);
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterStartElement (h, BAD_CAST "NetDevices");
  NS_ASSERT (rc >= 0);

  for (int nid = 0; nid < (int) m_networkStacks.size(); nid++)
  {
    rc = xmlTextWriterStartElement(h, BAD_CAST "NetDevice");
    NS_ASSERT (rc >= 0);

    m_networkStacks[nid].Write (h);

    rc = xmlTextWriterStartElement (h, BAD_CAST "dataTx");
    NS_ASSERT (rc >= 0);

    for (auto rid=m_dataTx.begin(); rid !=m_dataTx.end();)
    {
      if ((*rid)->GetIface() == nid)
      {
        (*rid)->Write (h);
        m_dataTx.erase(rid);
      } else {
        rid++;
      }
    }
    rc = xmlTextWriterEndElement (h);
    NS_ASSERT (rc >= 0);

    rc = xmlTextWriterStartElement (h, BAD_CAST "dataRx");
    NS_ASSERT (rc >= 0);

    for (auto rid=m_dataRx.begin(); rid !=m_dataRx.end();)
    {
      if ((*rid)->GetIface() == nid)
      {
        (*rid)->Write (h);
        m_dataRx.erase(rid);
      } else {
        rid++;
      }
    }

    rc = xmlTextWriterEndElement (h);
    NS_ASSERT (rc >= 0);

    rc = xmlTextWriterEndElement(h);
    NS_ASSERT (rc >= 0);
  }
  rc = xmlTextWriterEndElement (h);
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterStartElement (h, BAD_CAST "dataTx");
  //broadcast
  NS_ASSERT (rc >= 0);

  for (auto rid=m_dataTx.begin(); rid !=m_dataTx.end();rid++) (*rid)->Write (h);

  rc = xmlTextWriterEndElement (h);
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterStartElement (h, BAD_CAST "dataRx");
  NS_ASSERT (rc >= 0);

  for (auto rid=m_dataRx.begin(); rid !=m_dataRx.end();rid++) (*rid)->Write (h);

  rc = xmlTextWriterEndElement (h);
  NS_ASSERT (rc >= 0);
 //////

  rc = xmlTextWriterStartElement (h, BAD_CAST "cumulativeDataTx");
  NS_ASSERT (rc >= 0);

  for (auto& dataStatsTx : m_cumulativeDataTx)
    dataStatsTx->Write (h);

  rc = xmlTextWriterEndElement (h);
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterStartElement (h, BAD_CAST "cumulativeDataRx");
  NS_ASSERT (rc >= 0);

  for (auto& dataStatsRx : m_cumulativeDataRx)
    dataStatsRx->Write (h);

  rc = xmlTextWriterEndElement (h);
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterEndElement (h);
  NS_ASSERT (rc >= 0);
}

void
ReportDrone::DoInitializeTrajectoryMonitor ()
{
  NS_LOG_FUNCTION_NOARGS ();
  /* set CourseChange callback using ns-3 XPath addressing system */
  std::stringstream xPathCallback;

  xPathCallback << "/NodeList/" << m_reference
                << "/$ns3::MobilityModel/CourseChange";
  Config::ConnectWithoutContext (xPathCallback.str (),
                                 MakeCallback (&ReportDrone::DoMonitorTrajectory, this));
}

void
ReportDrone::DoMonitorTrajectory (const Ptr<const MobilityModel> mobility)
{
  NS_LOG_FUNCTION (mobility);
  Ptr<Node> drone = NodeList::GetNode(m_reference);
  Vector position = mobility->GetPosition ();

  if (m_trajectory.size() == 0 || m_trajectory.back().GetPosition() != position)
  {
    m_trajectory.push_back (ReportLocation (position, Simulator::Now (), irc->IsInRegions(position)));
    m_roi.push_back (irc->IsInRegions(position));
  }
}

void
ReportDrone::DoInitializePeripherals ()
{
  Ptr<Drone> drone = DroneList::GetDrone(m_reference);
  auto percont = drone->getPeripherals();

  for (auto p=percont->Begin(); p != percont->End(); p++)
  {
    m_peripherals.push_back(ReportPeripheral((*p)->GetInstanceTypeId().GetName(),(*p)->GetPowerConsumptionStates(),(*p)->GetRegionsOfInterest()));
    if ((*p)->GetInstanceTypeId().GetName() == "ns3::StoragePeripheral") m_peripherals.back().AddAttribute({"Capacity",std::to_string(StaticCast<StoragePeripheral,DronePeripheral>((*p))->GetCapacity())});
    if ((*p)->GetInstanceTypeId().GetName() == "ns3::InputPeripheral")
    {
      Ptr<InputPeripheral> inper = StaticCast<InputPeripheral,DronePeripheral>((*p));
      m_peripherals.back().AddAttribute({"DataRate",std::to_string(inper->GetDatarate())});
      m_peripherals.back().AddAttribute({"DataAcquisitionTimeInterval",std::to_string(inper->GetAcquisitionTimeInterval().GetSeconds())});
      m_peripherals.back().AddAttribute({"HasStorage",inper->HasStorage()? "true" : "false"});
    }
  }
}

} // namespace ns3
