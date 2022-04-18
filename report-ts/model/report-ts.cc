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
#include <ns3/log.h>
#include <ns3/object-factory.h>
#include <ns3/simulator.h>

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

  Simulator::ScheduleNow (&ReportTs::Probe, this);
  Simulator::ScheduleNow (&ReportTs::InitTraces, this);
}

ReportTs::~ReportTs ()
{
  NS_LOG_FUNCTION (this);
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

  // trace drones trajectory
  const auto ret = Config::ConnectWithoutContextFailSafe ("/DroneList/*/$ns3::MobilityModel/CourseChange",
                                                          MakeCallback (&ReportTs::TraceDrones, this));
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
  DbInsert (time, droneId, pos, vel);
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
ReportTs::DbInsert (const double time_sim, const uint32_t droneId, const Vector3D position, const Vector3D velocity)
{
  try
    {
      pqxx::work w {m_conn};
      std::stringstream insertQuery;

      insertQuery << "INSERT INTO drones_position"
                  << " (time_sim, drone_id, pos_x, pos_y, pos_z, vel_x, vel_y, vel_z)"
                  << " VALUES"
                  << " (" << time_sim << ","
                          << droneId << ","
                          << position.x << ","
                          << position.y << ","
                          << position.z << ","
                          << velocity.x << ","
                          << velocity.y << ","
                          << velocity.z << ")";
      auto res = w.exec(insertQuery, "Update drone position and its velocity");
      w.commit();

      NS_LOG_DEBUG ("INSERTed " << res.affected_rows () << " row(s).");
    }
  catch (std::exception const &e)
    {
      NS_LOG_ERROR ("An error occurred during the insertion of new drone position/velocity in DB: " << e.what());
    }
}

} // namespace ns3
