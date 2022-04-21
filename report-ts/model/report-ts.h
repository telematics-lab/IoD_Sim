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

#include <ns3/drone.h>
#include <ns3/mobility-model.h>
#include <ns3/ptr.h>
#include <ns3/singleton.h>

#include <pqxx/pqxx>

namespace ns3 {

class ReportTs : public Singleton<ReportTs>
{
public:
  void Initialize (/** TODO: configuration (object?) already decoded from JSON file */);
  ~ReportTs ();

private:
  void SimulationStarted ();
  void SimulationEnded ();
  void Probe ();
  void InitTraces ();
  void TraceDrones (Ptr<const MobilityModel> model);
  Ptr<const Drone> GetReferenceDrone (Ptr<const Object> aggregate);

  const std::string DbRegisterScenarioExecution (const std::string name);
  void DbNotifyScenarioEnded (const std::string uid);
  void DbInsertDroneLocation (const double time, const uint32_t droneId, const Vector3D position, const Vector3D velocity);

  pqxx::connection m_conn;
  std::string m_scenarioUid;
};

} // namespace ns3

#endif /** REPORT_TS_H */
