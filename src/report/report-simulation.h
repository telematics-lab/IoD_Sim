/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2024 The IoD_Sim Authors.
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
 * Authors: Giovanni Grieco <giovanni.grieco@poliba.it>, Giovanni Iacovelli
 * <giovanni.iacovelli@poliba.it>
 */
#ifndef REPORT_SIMULATION_H
#define REPORT_SIMULATION_H

#include "report-container.h"
#include "report-world.h"
#include "simulation-duration.h"

#include <ns3/event-id.h>
#include <ns3/object.h>

#include <libxml/xmlwriter.h>

namespace ns3
{

/**
 * \ingroup report
 *
 * \brief Report module for the simulation.
 *
 * Report a simulation by constructing a tree of data during the simulation
 * and then save all of it in a structured XML file.
 *
 * A probe event is used to gather simulation information. No need to pass any
 * data from a scenario standpoint, in order to not confuse what is expected
 * and what is effectively done.
 */
class ReportSimulation : public Object
{
  public:
    /**
     * Register the type using ns-3 TypeId System.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    ReportSimulation();

    /**
     * Write Zsp report data to a XML file with a given handler
     *
     * \param handle the handler to communicate data to the opened XML file
     */
    void Write(xmlTextWriterPtr handle) const;

  protected:
    /**
     * Initialize this Object by acquiring simulation data
     */
    virtual void DoInitialize(void);

  private:
    /**
     * Probe Callback to retrieve simulation information
     */
    void ProbeSimulation();

    /**
     * Generic function to populate simulated entities, which can be drones
     * or ZSPs
     *
     * \param nodeQuery the query to perform to find simulated Nodes
     * \param entities the container object to be populated
     */
    template <class EntityContainer>
    void PopulateEntities(const std::string nodeQuery, EntityContainer* entities);

    std::string m_scenario;        /// The name of the scenario
    std::string m_executedAt;      /// Datetime of execution
    SimulationDuration m_duration; /// Duration of the simulation

    ReportContainer<ReportDrone> m_drones;   /// Report of drones
    ReportContainer<ReportZsp> m_zsps;       /// Report of ZSPs
    ReportContainer<ReportRemote> m_remotes; /// Report of Remotes
    ReportWorld m_world;                     /// Report of World
};

} // namespace ns3

#endif /* REPORT_SIMULATION_H */
