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
#ifndef REPORT_TS_H
#define REPORT_TS_H

#include <ns3/ptr.h>
#include <ns3/singleton.h>
#include <ns3/he-frame-exchange-manager.h> // TODO: hide in specialized obj

#include <pqxx/pqxx>

// Quick class declarations for private methods
namespace ns3 { // TODO: hide in specialized obj
  class Drone;
  class MobilityModel;
  class Object;
  class Vector3D;
  class WifiTxVector;
}

namespace ns3 {

class ReportTs : public Singleton<ReportTs>
{
public:
  void Initialize (); // TODO: configuration (object?) already decoded from JSON file
  ~ReportTs ();

private:
  void SimulationStarted ();
  void SimulationEnded ();
  void Probe ();

  void InitTraces ();
  template<typename ReturnT, typename... ArgT>
  void InitTrace(const std::string& contextFilter, const Callback<ReturnT, ArgT...>& callback);

  void TraceDroneTrajectory (std::string context, Ptr<const MobilityModel> droneMobility);
  Ptr<const Drone> GetReferenceDrone (Ptr<const Object> aggregate);
  void ProbeDroneInitialPositions ();
  const bool isDroneMobilityModel (const std::string& mobilityModelName);

  // Callback Utilities
  void WifiPhyPsduTxBeginCallback (std::string context,
                                   WifiConstPsduMap psduMap,
                                   WifiTxVector txVector,
                                   double txPowerW);
  void WifiPhyRxBeginCallback (std::string callbackContext,
                               Ptr<const Packet> pkt,
                               RxPowerWattPerChannelBand rxPowersW);
  void LteReportCurrentCellRsrpSinrCallback (std::string context,
                                             uint16_t cellId,
                                             uint16_t rnti,
                                             double rsrp,
                                             double sinr,
                                             uint8_t componentCarrierId);
  void LteReportUlPhyResourceBlocksCallback (std::string context,
                                             uint16_t rnti,
                                             const std::vector<int>& rbs);
  void LteReportPowerSpectralDensityCallback (std::string context,
                                              uint16_t rnti,
                                              Ptr<SpectrumValue> psd);
  void LteReportUeMeasurementsCallback (std::string context,
                                        uint16_t rnti,
                                        uint16_t cellId,
                                        double rsrp,
                                        double rsrq,
                                        bool isServingCell,
                                        uint8_t componentCarrierId);

  // Utilities for information decoding
  const std::optional<uint32_t> ExtractId (std::string ctx, const std::string& key);
  const Mac48Address GetHostAddress (const uint32_t nodeId,
                                     const uint32_t deviceId);
  const bool IsDestinationHost (const Mac48Address& deviceHostAddress,
                                const WifiMacHeader& rxPacketHeader);
  const Mac48Address GetWifiSender (const WifiMacHeader& rxPacketHeader);
  const double GetRssi (const RxPowerWattPerChannelBand& rxPowersW);

  // Utilities to push information to TSDB
  const std::string DbRegisterScenarioExecution (const std::string name);
  void DbNotifyScenarioEnded (const std::string uid);
  void DbInsertDroneLocation (const double time,
                              const uint32_t droneId,
                              const Vector3D position,
                              const Vector3D velocity);
  void DbInsertDroneRssi (const double time,
                          const uint32_t rxId,
                          const Mac48Address txAddr,
                          const double rssi);
  void DbInsertLteCurrentCellRsrpSinr (const double time,
                                       const uint32_t droneId,
                                       const uint16_t cellId,
                                       const uint16_t rnti,
                                       const double rsrp,
                                       const double sinr,
                                       const uint8_t componentCarrierId);
  void DbInsertLteUlPhyResourceBlocks (const double time,
                                       const uint32_t droneId,
                                       const uint16_t rnti,
                                       const std::vector<int>& rbs);
  void DbInsertLtePowerSpectralDensity (const double time,
                                        const uint32_t droneId,
                                        const uint16_t rnti,
                                        Ptr<SpectrumValue> psd);
  void DbInsertLteUeMeasurements (const double time,
                                  const uint32_t droneId,
                                  const uint16_t rnti,
                                  const uint16_t cellId,
                                  const double rsrp,
                                  const double rsrq,
                                  const bool isServingCell,
                                  const uint8_t componentCarrierId);

  // other SQL utils
  const std::string ToSqlArray (const std::vector<int> arr);
  const std::string ToSqlArray (const Ptr<SpectrumValue> arr);
  const std::string ToSqlDouble (const double n);
  const std::string ToSqlBoolean (const bool x);

  pqxx::connection m_conn;
  std::string m_scenarioUid;
};

} // namespace ns3

#endif /** REPORT_TS_H */
