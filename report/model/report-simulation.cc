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
 * Authors: Giovanni Grieco <giovanni.grieco@poliba.it>, Giovanni Iacovelli <giovanni.iacovelli@poliba.it>
 */
#include "report-simulation.h"

#include <ns3/config.h>
//#include <ns3/iodsim-node.h>
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/string.h>

#include <libxml/xmlwriter.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ReportSimulation");
NS_OBJECT_ENSURE_REGISTERED (ReportSimulation);

TypeId
ReportSimulation::GetTypeId ()
{
  NS_LOG_FUNCTION_NOARGS ();

  static TypeId tid = TypeId ("ns3::ReportSimulation")
    .AddConstructor<ReportSimulation> ()
    .SetParent<Object> ()

    /* XML Attributes */
    .AddAttribute ("Scenario", "The name of the scenario",
                   StringValue ("unnamed"),
                   MakeStringAccessor (&ReportSimulation::m_scenario),
                   MakeStringChecker ())
    .AddAttribute ("ExecutedAt", "The datetime of execution",
                   StringValue (),
                   MakeStringAccessor (&ReportSimulation::m_executedAt),
                   MakeStringChecker ())
    ;

  return tid;
}

ReportSimulation::ReportSimulation () :
  m_drones {"Drones"},
  m_zsps {"ZSPs"},
  m_remotes {"Remotes"}
{
  NS_LOG_FUNCTION (this);
}

ReportSimulation::~ReportSimulation ()
{
  NS_LOG_FUNCTION (this);
}

void
ReportSimulation::DoInitialize()
{
  NS_LOG_FUNCTION_NOARGS ();

  // use schedule now to be sure we are probing during
  // an active simulation
  Simulator::ScheduleNow (&ReportSimulation::ProbeSimulation,
                          this);
}

void
ReportSimulation::Write (xmlTextWriterPtr h) const
{
  NS_LOG_FUNCTION (h);
  if (h == nullptr)
    {
      NS_LOG_WARN ("Passed handler is not valid: " << h << ". "
                   "Data will be discarded.");
      return;
    }

  int rc;

  rc = xmlTextWriterStartElement (h, BAD_CAST "simulation");
  NS_ASSERT (rc >= 0);

  /* Attributes */
  xmlTextWriterWriteAttribute (h,
                               BAD_CAST "scenario",
                               BAD_CAST m_scenario.c_str ());
  xmlTextWriterWriteAttribute (h,
                               BAD_CAST "executedAt",
                               BAD_CAST m_executedAt.c_str ());

  /* Nested Elements */
  m_duration.Write (h);
  m_world.Write (h);
  m_zsps.Write (h);
  m_drones.Write (h);
  m_remotes.Write (h);

  rc = xmlTextWriterEndElement (h);
  NS_ASSERT (rc >= 0);
}

void
ReportSimulation::ProbeSimulation ()
{
  NS_LOG_FUNCTION_NOARGS ();

  PopulateEntities ("/DroneList/*", &m_drones);
  PopulateEntities ("/ZspList/*", &m_zsps);
  PopulateEntities ("/RemoteList/*", &m_remotes);
}

template <class EntityContainer>
void
ReportSimulation::PopulateEntities (const std::string nodeQuery,
                                    EntityContainer *entities)
{
  NS_LOG_FUNCTION (nodeQuery << entities);
  const auto eFound = Config::LookupMatches (nodeQuery);

  for (auto i = eFound.Begin (); i != eFound.End (); i++)
    {
      const auto node = DynamicCast<Node> (*i);

      if (node != nullptr)
        {
          const uint32_t uid = node->GetId ();
          entities->Add (uid);
        }
    }
}

} // namespace ns3
