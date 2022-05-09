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
 */
#include "report-ts.h"

#include <ns3/config.h>
#include <ns3/drone.h>
#include <ns3/log.h>
#include <ns3/mobility-model.h>
#include <ns3/node-list.h>
#include <ns3/object-factory.h>
#include <ns3/scenario-configuration-helper.h>
#include <ns3/simulator.h>
#include <ns3/wifi-phy.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ReportTs");

void
ReportTs::Initialize ()
{
  NS_LOG_FUNCTION (this);

  /** TODO:
   *  Can we "plug" into ScenarioConfigurationHelper by providing out decoders?
   *  In this way we don't pollute drone module with report-ts decoders.
  */

  Simulator::ScheduleNow (&ReportTs::SimulationStarted, this);
  Simulator::ScheduleNow (&ReportTs::Probe, this);
  Simulator::ScheduleNow (&ReportTs::InitTraces, this);
}

ReportTs::~ReportTs ()
{
  NS_LOG_FUNCTION (this);

  DbNotifyScenarioEnded (m_scenarioUid);
}

void
ReportTs::SimulationStarted ()
{
  NS_LOG_FUNCTION (this);

  const auto scenarioName = CONFIGURATOR->GetName ();
  m_scenarioUid = DbRegisterScenarioExecution (scenarioName);
}

void
ReportTs::Probe ()
{
  NS_LOG_FUNCTION (this);

  // Probe into the simulation at its start
  for (uint32_t i = 0; i < Config::GetRootNamespaceObjectN (); i++)
    {
      auto obj = Config::GetRootNamespaceObject (i);
      NS_LOG_DEBUG ("Discovered TypeId: " << obj->GetInstanceTypeId ());
      auto oTid = obj->GetInstanceTypeId ();

      //! I don't believe we can do a fully general solution due to missing interfaces. How can we be sure that X
      //! has Y method with a predictable signature?
      // lookup for trace sources
      //auto nTraces = oTid.GetTraceSourceN ();
      // TODO: for each trace... https://www.nsnam.org/doxygen/structns3_1_1_type_id_1_1_trace_source_information.html

      // lookup inner attributes
      for (uint32_t iAttr = 0; iAttr < oTid.GetAttributeN (); iAttr++)
        {
          NS_LOG_DEBUG ("Discovered attribute: " << oTid.GetAttribute (iAttr).name);
        }
    }
}

void
ReportTs::InitTraces ()
{
  NS_LOG_FUNCTION (this);
  bool ret = false;

  // trace drones trajectory
  ret = Config::ConnectWithoutContextFailSafe ("/DroneList/*/$ns3::MobilityModel/CourseChange", // TODO: isn't better /NodeList/ ?
                                               MakeCallback (&ReportTs::TraceDrones, this));
  if (!ret)
    NS_LOG_WARN ("Could not register traces for drones. Ensure drones are correctly initialized in the simulation.");

  ret = Config::ConnectWithoutContextFailSafe ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxPsduBegin",
                                               MakeCallback (&ReportTs::WifiPhyPsduTxBeginCallback, this));
  if (!ret)
    NS_LOG_WARN ("Could not register traces for drones. Ensure drones are correctly initialized in the simulation.");

  ret = Config::ConnectFailSafe ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxBegin",
                                 MakeCallback (&ReportTs::WifiPhyRxBeginCallback, this));
  if (!ret)
    NS_LOG_WARN ("Could not register traces for drones. Ensure drones are correctly initialized in the simulation.");
}

void
ReportTs::TraceDrones (Ptr<const MobilityModel> m)
{
  NS_LOG_FUNCTION (this << m);

  auto time = Simulator::Now ().GetSeconds ();
  auto droneId = GetReferenceDrone (m)->GetId ();
  auto pos = m->GetPosition ();
  auto vel = m->GetVelocity ();

  NS_LOG_DEBUG ("Drone PosLine: " << time << " | " << droneId << " | " << pos << " | " << vel);
  DbInsertDroneLocation (time, droneId, pos, vel);
}

Ptr<const Drone>
ReportTs::GetReferenceDrone (Ptr<const Object> aggregate)
{
  NS_LOG_FUNCTION (this << aggregate);

  auto it = aggregate->GetAggregateIterator ();
  while (it.HasNext ())
    {
      auto obj = it.Next ();
      auto oTid = obj->GetInstanceTypeId ();

      if (oTid.GetName () == "ns3::Drone")
        return DynamicCast<const Drone, const Object> (obj);
    }

  NS_FATAL_ERROR ("Cannot find ns3::Drone aggregated to " << aggregate);
}

void
ReportTs::WifiPhyPsduTxBeginCallback (WifiConstPsduMap psduMap, WifiTxVector txVector, double txPowerW)
{
  NS_LOG_FUNCTION (this << psduMap << txVector << txPowerW);

}

void
ReportTs::WifiPhyRxBeginCallback (std::string ctx, Ptr<const Packet> pkt, RxPowerWattPerChannelBand rxPowersW)
{
  NS_LOG_FUNCTION (this << ctx << pkt << rxPowersW);

  const auto nodeId = ExtractId (ctx, "NodeList/");
  // we are in a shared medium, so STA can receive other signals
  if (!nodeId)
    return;

  const auto deviceId = ExtractId (ctx, "DeviceList/");
  NS_ASSERT (deviceId);

  const auto hostAddr = GetHostAddress (*nodeId, *deviceId);

  WifiMacHeader hdr;
  pkt->PeekHeader (hdr);

  if (IsDestinationHost (hostAddr, hdr))
    {
      const auto senderAddr = GetWifiSender (hdr);
      // filter out any sender, which happens in case of CTL ACK frames
      if (senderAddr == Mac48Address () /* 00:00:00:00:00:00 */)
        return;

      const auto rssi = GetRssi (rxPowersW);

      DbInsertDroneRssi (Simulator::Now ().GetSeconds (),
                         *nodeId,
                         senderAddr,
                         rssi);
    }
}

const std::optional<uint32_t>
ReportTs::ExtractId (std::string ctx, const std::string& key)
{
  NS_LOG_FUNCTION (this << ctx << key);

  size_t pos = ctx.find (key);
  if (pos == std::string::npos)
    return {};

  const size_t posNodeIdx = pos + key.size ();
  ctx.erase (0, posNodeIdx);

  try
    {
      return std::stoi (ctx);
    }
  catch (std::exception const&)
    {
      return {};
    }
}

const Mac48Address
ReportTs::GetHostAddress (const uint32_t nodeId, const uint32_t deviceId)
{
  NS_LOG_FUNCTION (this << nodeId << deviceId);
  auto n = NodeList::GetNode (nodeId);
  auto d = n->GetDevice (deviceId);
  auto a = d->GetAddress ();
  return Mac48Address::ConvertFrom (a);
}

const bool
ReportTs::IsDestinationHost (const Mac48Address& addr, const WifiMacHeader& hdr)
{
  NS_LOG_FUNCTION (this << addr << hdr);

  // check if packet is for this net device
  const auto da = (hdr.IsToDs ()) ? hdr.GetAddr3 () : hdr.GetAddr1 ();
  return da != addr;
}

const Mac48Address
ReportTs::GetWifiSender (const WifiMacHeader& hdr)
{
  NS_LOG_FUNCTION (this << hdr);
  Mac48Address senderAddr;

  // See IEEE 802.11 standard
  if ((!hdr.IsToDs () && !hdr.IsFromDs ()) || (hdr.IsToDs () && !hdr.IsFromDs ()))
    senderAddr = hdr.GetAddr2 ();
  else if (!hdr.IsToDs () && hdr.IsFromDs ())
    senderAddr = hdr.GetAddr3 ();
  else if (hdr.IsToDs () && hdr.IsFromDs ())
    senderAddr = hdr.GetAddr4 ();

  return senderAddr;
}

const double
ReportTs::GetRssi (const RxPowerWattPerChannelBand& rxPowersW)
{
  NS_LOG_FUNCTION (this << rxPowersW);

  //The total RX power corresponds to the maximum over all the bands
  auto it = std::max_element (rxPowersW.begin (), rxPowersW.end (),
                              [] (const std::pair<WifiSpectrumBand, double> &p1,
                                  const std::pair<WifiSpectrumBand, double> &p2) {
                                return p1.second < p2.second;
                              });
  return 10.0 * log (it->second / 1e-3 /* to mW */) /* to dBm */;
}

const std::string
ReportTs::DbRegisterScenarioExecution (const std::string name)
{
  NS_LOG_FUNCTION (this << name);
  try
    {
      pqxx::work w {m_conn};
      std::stringstream insertQuery;

      insertQuery << "INSERT INTO scenario_executions"
                  << " (name) VALUES ('" << name << "')"
                  << " RETURNING (id)";
      const auto res = w.exec (insertQuery, "Register new scenario execution");
      w.commit ();

      NS_LOG_DEBUG ("INSERTed " << res.affected_rows () << " row(s).");
      NS_ASSERT (res.affected_rows () == 1);

      const std::string uid {res[0][0].view ()};
      NS_LOG_INFO ("Scenario UID: " << uid);
      return uid;
    }
  catch (std::exception const &e)
    {
      NS_FATAL_ERROR ("An error occurred during the registration of new scenario execution.");
    }
}

void
ReportTs::DbNotifyScenarioEnded (const std::string uid)
{
  NS_LOG_FUNCTION (this << uid);
  try
    {
      pqxx::work w {m_conn};
      std::stringstream sqlQuery;

      sqlQuery << "UPDATE scenario_executions"
               << " SET time_end = CURRENT_TIMESTAMP"
               << " WHERE id = '" << uid << "'";
      const auto res = w.exec (sqlQuery, "Notify scenario ended");
      w.commit ();

      NS_LOG_DEBUG ("UPDATEd " << res.affected_rows () << " row(s).");
    }
  catch (std::exception const &e)
    {
      NS_LOG_ERROR ("An error occurred during the notification of scenario ended: " << e.what());
    }
}

void
ReportTs::DbInsertDroneLocation (const double timeSim, const uint32_t droneId, const Vector3D position, const Vector3D velocity)
{
  NS_LOG_FUNCTION (this << timeSim << droneId << position << velocity);
  try
    {
      pqxx::work w {m_conn};
      std::stringstream insertQuery;

      insertQuery << "INSERT INTO drones_position"
                  << " (scenario_id, time_sim, drone_id, pos_x, pos_y, pos_z, vel_x, vel_y, vel_z)"
                  << " VALUES"
                  << " ('" << m_scenarioUid << "',"
                           << timeSim << ","
                           << droneId << ","
                           << position.x << ","
                           << position.y << ","
                           << position.z << ","
                           << velocity.x << ","
                           << velocity.y << ","
                           << velocity.z << ")";
      auto res = w.exec (insertQuery, "Update drone position and its velocity");
      w.commit ();

      NS_LOG_DEBUG ("INSERTed " << res.affected_rows () << " row(s).");
    }
  catch (std::exception const &e)
    {
      NS_LOG_ERROR ("An error occurred during the insertion of new drone position/velocity in DB: " << e.what());
    }
}

void
ReportTs::DbInsertDroneRssi (const double time,
                             const uint32_t rxId,
                             const Mac48Address txAddr,
                             const double rssi)
{
  NS_LOG_FUNCTION (this << time << rxId << txAddr << rssi);
  try
    {
      pqxx::work w {m_conn};
      std::stringstream insertQuery;

      insertQuery << "INSERT INTO wifi_rssi"
                  << " (scenario_id, time_sim, drone_id, from_addr, rssi)"
                  << " VALUES"
                  << " ('" << m_scenarioUid << "',"
                           << time << ","
                           << rxId << ","
                  << "  '" << txAddr << "',"
                           << rssi << ")";
      auto res = w.exec (insertQuery, "Update drone RSSI");
      w.commit ();

      NS_LOG_DEBUG ("INSERTed " << res.affected_rows () << " row(s).");
    }
  catch (std::exception const &e)
    {
      NS_LOG_ERROR ("An error occurred during the insertion of new drone RSSI in DB: " << e.what());
    }
}

std::ostream&
operator<< (std::ostream& os, const WifiSpectrumBand& a)
{
  os << "(" << a.first << "," << a.second << ")";

  return os;
}

std::ostream&
operator<< (std::ostream& os, const RxPowerWattPerChannelBand& a)
{
  os << "[";
  for (auto& el : a)
    os << " { Band: " << el.first << ";"
       << " RxPowerW: " << el.second << "}, ";
  os << "]";

  return os;
}

} // namespace ns3
