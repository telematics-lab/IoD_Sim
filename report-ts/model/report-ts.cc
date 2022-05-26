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
#include <ns3/drone-list.h>
#include <ns3/log.h>
#include <ns3/mobility-model.h>
#include <ns3/node-list.h>
#include <ns3/object-factory.h>
#include <ns3/scenario-configuration-helper.h>
#include <ns3/simulator.h>
#include <ns3/wifi-phy.h>

#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ReportTs");

void
ReportTs::Initialize ()
{
  NS_LOG_FUNCTION (this);

  /*  TODO:
   *  Can we "plug" into ScenarioConfigurationHelper by providing out decoders?
   *  In this way we don't pollute drone module with report-ts decoders.
  */

  Simulator::ScheduleNow (&ReportTs::SimulationStarted, this);
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
  ProbeDroneInitialPositions ();
}

void
ReportTs::InitTraces ()
{
  NS_LOG_FUNCTION (this);
  // trace drones trajectory
  InitTrace ("/NodeList/*/$ns3::MobilityModel/CourseChange",
             MakeCallback (&ReportTs::TraceDroneTrajectory, this));
  InitTrace ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxPsduBegin",
             MakeCallback (&ReportTs::WifiPhyPsduTxBeginCallback, this));
  InitTrace ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxBegin",
             MakeCallback (&ReportTs::WifiPhyRxBeginCallback, this));
  InitTrace ("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportCurrentCellRsrpSinr",
             MakeCallback (&ReportTs::LteReportCurrentCellRsrpSinrCallback, this));
  InitTrace ("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportUlPhyResourceBlocks",
             MakeCallback (&ReportTs::LteReportUlPhyResourceBlocksCallback, this));
  InitTrace ("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportPowerSpectralDensity",
             MakeCallback (&ReportTs::LteReportPowerSpectralDensityCallback, this));
  InitTrace ("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportUeMeasurements",
             MakeCallback (&ReportTs::LteReportUeMeasurementsCallback, this));
}

template<typename ReturnT, typename... ArgT>
void
ReportTs::InitTrace(const std::string& ctx, const Callback<ReturnT, ArgT...>& cb)
{
  NS_LOG_FUNCTION (this << ctx);

  const bool ret = Config::ConnectFailSafe (ctx, cb);
  if (!ret)
    NS_LOG_WARN ("Could not register trace for " << ctx << "."
                 << "Ensure drones are correctly initialized in the simulation.");
}

void
ReportTs::TraceDroneTrajectory (std::string ctx, Ptr<const MobilityModel> m)
{
  NS_LOG_FUNCTION (this << ctx << m);

  auto time = Simulator::Now ().GetSeconds ();
  auto droneId = GetReferenceDrone (m)->GetId ();
  auto pos = m->GetPosition ();
  auto vel = m->GetVelocity ();

  NS_LOG_DEBUG ("Drone PosLine: " << time
                << " | " << droneId
                << " | " << pos
                << " | " << vel);
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
ReportTs::ProbeDroneInitialPositions ()
{
  NS_LOG_FUNCTION (this);
  const auto time = Simulator::Now ().GetSeconds ();

  for (auto it = DroneList::Begin (); it != DroneList::End (); it++)
    {
      Ptr<MobilityModel> mm = (*it)->GetObject<MobilityModel> ();

      auto aggregates = mm->GetAggregateIterator ();
      while (aggregates.HasNext ())
        {
          auto it = aggregates.Next ();
          NS_LOG_DEBUG ("!! MM Agg: " << it->GetInstanceTypeId ().GetName ());
        }

      // Drone mobility models do not use postition allocators, which may result
      // in the drone being wrongly placed at startup at [0,0,0]
      if (isDroneMobilityModel (mm->GetInstanceTypeId ().GetName ()))
        continue;

      const auto droneId = (*it)->GetId ();
      TypeId::AttributeInformation posAttrInfo, velAttrInfo;

      mm->GetInstanceTypeId ().LookupAttributeByName ("Position", &posAttrInfo);
      mm->GetInstanceTypeId ().LookupAttributeByName ("Velocity", &velAttrInfo);

      auto pos = StaticCast<const Vector3DValue, const AttributeValue>
                 (posAttrInfo.initialValue)->Get ();
      auto vel = StaticCast<const Vector3DValue, const AttributeValue>
                 (velAttrInfo.initialValue)->Get ();

      NS_LOG_DEBUG ("Drone Initial Position: " << time
                    << " | " << droneId
                    << " | " << pos
                    << " | " << vel);
      DbInsertDroneLocation (time, droneId, pos, vel);
    }
}

const bool
ReportTs::isDroneMobilityModel (const std::string& mmName)
{
  NS_LOG_FUNCTION (this << mmName);
  return mmName == "ns3::ConstantAccelerationDroneMobilityModel"
      || mmName == "ns3::ParametricSpeedDroneMobilityModel";
}

void
ReportTs::WifiPhyPsduTxBeginCallback (std::string ctx,
                                      WifiConstPsduMap psduMap,
                                      WifiTxVector txVector,
                                      double txPowerW)
{
  NS_LOG_FUNCTION (this << ctx << psduMap << txVector << txPowerW);
}

void
ReportTs::WifiPhyRxBeginCallback (std::string ctx,
                                  Ptr<const Packet> pkt,
                                  RxPowerWattPerChannelBand rxPowersW)
{
  NS_LOG_FUNCTION (this << ctx << pkt << rxPowersW);

  const auto nodeId = ExtractId (ctx, "NodeList/");
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

void
ReportTs::LteReportCurrentCellRsrpSinrCallback (std::string ctx,
                                                uint16_t cellId,
                                                uint16_t rnti,
                                                double rsrp,
                                                double sinr,
                                                uint8_t componentCarrierId)
{
  NS_LOG_FUNCTION (this << ctx << cellId << rsrp << sinr << componentCarrierId);
  const auto nodeId = *ExtractId (ctx, "NodeList/");
  DbInsertLteCurrentCellRsrpSinr (Simulator::Now ().GetSeconds (),
                                  nodeId,
                                  cellId,
                                  rnti,
                                  rsrp,
                                  sinr,
                                  componentCarrierId);
}

void
ReportTs::LteReportUlPhyResourceBlocksCallback (std::string ctx,
                                                uint16_t rnti,
                                                const std::vector<int>& rbs)
{
  NS_LOG_FUNCTION (this << ctx << rnti << rbs);
  const auto nodeId = *ExtractId (ctx, "NodeList/");
  DbInsertLteUlPhyResourceBlocks (Simulator::Now ().GetSeconds (),
                                  nodeId,
                                  rnti,
                                  rbs);
}

void
ReportTs::LteReportPowerSpectralDensityCallback (std::string ctx,
                                                 uint16_t rnti,
                                                 Ptr<SpectrumValue> psd)
{
  NS_LOG_FUNCTION (this << ctx << rnti << psd);
  const auto nodeId = *ExtractId (ctx, "NodeList/");
  DbInsertLtePowerSpectralDensity (Simulator::Now ().GetSeconds (),
                                   nodeId,
                                   rnti,
                                   psd);
}

void
ReportTs::LteReportUeMeasurementsCallback (std::string ctx,
                                           uint16_t rnti,
                                           uint16_t cellId,
                                           double rsrp,
                                           double rsrq,
                                           bool isServingCell,
                                           uint8_t componentCarrierId)
{
  NS_LOG_FUNCTION (this << ctx << rnti << cellId << rsrp << rsrq
                        << isServingCell << componentCarrierId);
  const auto nodeId = *ExtractId (ctx, "NodeList/");
  DbInsertLteUeMeasurements (Simulator::Now ().GetSeconds (),
                             nodeId,
                             rnti,
                             cellId,
                             rsrp,
                             rsrq,
                             isServingCell,
                             componentCarrierId);
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
                           << ToSqlDouble (timeSim) << ","
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
                           << ToSqlDouble (time) << ","
                           << rxId << ","
                  << "  '" << txAddr << "',"
                           << ToSqlDouble (rssi) << ")";
      auto res = w.exec (insertQuery, "Update drone RSSI");
      w.commit ();

      NS_LOG_DEBUG ("INSERTed " << res.affected_rows () << " row(s).");
    }
  catch (std::exception const &e)
    {
      NS_LOG_ERROR ("An error occurred during the insertion of new drone RSSI in DB: " << e.what());
    }
}

void
ReportTs::DbInsertLteCurrentCellRsrpSinr (const double time,
                                          const uint32_t droneId,
                                          const uint16_t cellId,
                                          const uint16_t rnti,
                                          const double rsrp,
                                          const double sinr,
                                          const uint8_t componentCarrierId)
{
  NS_LOG_FUNCTION (this << time << droneId << cellId << rsrp << sinr
                        << componentCarrierId);
  try
    {
      pqxx::work w {m_conn};
      std::stringstream insertQuery;

      insertQuery << "INSERT INTO lte_current_cell_rsrp_sinr"
                  << " (scenario_id, time_sim, drone_id, cell_id, rsrp, rnti,"
                  << "  sinr, component_carrier_id)"
                  << " VALUES"
                  << " ('" << m_scenarioUid << "',"
                           << ToSqlDouble (time) << ","
                           << droneId << ","
                           << cellId << ","
                           << ToSqlDouble (rnti) << ","
                           << ToSqlDouble (rsrp) << ","
                           << sinr << ","
                           << (unsigned int) componentCarrierId << ")";
      auto res = w.exec (insertQuery, "Update drone LTE CurrentCellRsrpSinr");
      w.commit ();

      NS_LOG_DEBUG ("INSERTed " << res.affected_rows () << " row(s).");
    }
  catch (std::exception const &e)
    {
      NS_LOG_ERROR ("An error occurred during the insertion of new drone LteCurrentCellRsrpSinr in DB: " << e.what());
    }
}

void
ReportTs::DbInsertLteUlPhyResourceBlocks (const double time,
                                          const uint32_t droneId,
                                          const uint16_t rnti,
                                          const std::vector<int>& rbs)
{
  NS_LOG_FUNCTION (this << time << droneId << rnti << rbs);
  try
    {
      pqxx::work w {m_conn};
      std::stringstream insertQuery;

      insertQuery << "INSERT INTO lte_ul_phy_resource_blocks"
                  << " (scenario_id, time_sim, drone_id, rnti, rbs)"
                  << " VALUES"
                  << " ('" << m_scenarioUid << "',"
                           << ToSqlDouble (time) << ","
                           << droneId << ","
                           << rnti << ","
                       "'" << ToSqlArray(rbs) << "')";
      auto res = w.exec (insertQuery, "Update drone LTE UlPhyResourceBlocks");
      w.commit ();

      NS_LOG_DEBUG ("INSERTed " << res.affected_rows () << " row(s).");
    }
  catch (std::exception const &e)
    {
      NS_LOG_ERROR ("An error occurred during the insertion of new drone LteUlPhyResourceBlocks in DB: " << e.what());
    }
}

void
ReportTs::DbInsertLtePowerSpectralDensity (const double time,
                                           const uint32_t droneId,
                                           const uint16_t rnti,
                                           Ptr<SpectrumValue> psd)
{
  NS_LOG_FUNCTION (this << time << droneId << rnti << psd);
  try
    {
      pqxx::work w {m_conn};
      std::stringstream insertQuery;

      insertQuery << "INSERT INTO lte_power_spectral_density"
                  << " (scenario_id, time_sim, drone_id, rnti, psd)"
                  << " VALUES"
                  << " ('" << m_scenarioUid << "',"
                           << ToSqlDouble (time) << ","
                           << droneId << ","
                           << rnti << ","
                       "'" << ToSqlArray(psd) << "')";
      auto res = w.exec (insertQuery, "Update drone LTE PowerSpectralDensity");
      w.commit ();

      NS_LOG_DEBUG ("INSERTed " << res.affected_rows () << " row(s).");
    }
  catch (std::exception const &e)
    {
      NS_LOG_ERROR ("An error occurred during the insertion of new drone LtePowerSpectralDensity in DB: " << e.what());
    }
}

void
ReportTs::DbInsertLteUeMeasurements (const double time,
                                     const uint32_t droneId,
                                     const uint16_t rnti,
                                     const uint16_t cellId,
                                     const double rsrp,
                                     const double rsrq,
                                     const bool isServingCell,
                                     const uint8_t componentCarrierId)
{
  NS_LOG_FUNCTION (this << time << droneId << rnti << cellId << rsrp << rsrq
                        << isServingCell << componentCarrierId);
  try
    {
      pqxx::work w {m_conn};
      std::stringstream insertQuery;

      insertQuery << "INSERT INTO lte_ue_measurements"
                  << " (scenario_id, time_sim, drone_id, rnti, cell_id, rsrp, "
                  << "  rsrq, is_serving_cell, component_carrier_id)"
                  << " VALUES"
                  << " ('" << m_scenarioUid << "',"
                           << ToSqlDouble (time) << ","
                           << droneId << ","
                           << rnti << ","
                           << cellId << ","
                           << ToSqlDouble (rsrp) << ","
                           << ToSqlDouble (rsrq) << ","
                           << ToSqlBoolean (isServingCell) << ","
                           << (unsigned int) componentCarrierId << ")";
      auto res = w.exec (insertQuery, "Update drone LTE UeMeasurements");
      w.commit ();

      NS_LOG_DEBUG ("INSERTed " << res.affected_rows () << " row(s).");
    }
  catch (std::exception const &e)
    {
      NS_LOG_ERROR ("An error occurred during the insertion of new drone LteUeMeasurements in DB: " << e.what());
    }
}

const std::string
ReportTs::ToSqlArray (const std::vector<int> arr)
{
  NS_LOG_FUNCTION (this << arr);
  std::stringstream s;

  s << "{";
  for (auto it = arr.begin (); it != arr.end (); it++)
    {
      if (it != arr.begin ())
        s << ",";

      s << *it;
    }
  s << "}";

  return s.str ();
}

const std::string
ReportTs::ToSqlArray (const Ptr<SpectrumValue> arr)
{
  NS_LOG_FUNCTION (this << arr);
  std::stringstream s;

  s << "{";
  for (auto it = arr->ValuesBegin (); it != arr->ValuesEnd (); it++)
    {
      if (it != arr->ValuesBegin ())
        s << ",";

      s << *it;
    }
  s << "}";

  return s.str ();
}

const std::string
ReportTs::ToSqlDouble (const double n)
{
  NS_LOG_FUNCTION (this << n);
  if (std::isnan(n))
    return "'NaN'";
  else if (n == std::isinf(n))
    return (n > 0) ? "'Infinity'" : "'-Infinity'";
  else
    return std::to_string (n);
}

const std::string
ReportTs::ToSqlBoolean (const bool x)
{
  NS_LOG_FUNCTION (x);
  return (x) ? "TRUE" : "FALSE";
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
