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
 */
#ifndef DEFINED_PATCH_CONFIGURATOR_H
#define DEFINED_PATCH_CONFIGURATOR_H

#include "patch-configurator.h"

#include <ns3/double-vector.h>
#include <ns3/model-configuration-matrix.h>
#include <ns3/object.h>
#include <ns3/pointer.h>

namespace ns3
{

/**
 * \ingroup irs
 * Defines an object which, aggregated to an Irs, updates its patch organisation during the
 * simulation. The different configurations are installed for a duration equal to that defined in
 * m_periods vector.
 */
class DefinedPatchConfigurator : public PatchConfigurator
{
  public:
    /**
     * \brief Register this configurator as a type in ns-3 TypeId System.
     */
    static TypeId GetTypeId(void);

    /**
     * \brief Default constructor
     */
    DefinedPatchConfigurator();
    /**
     * \brief Default destructor
     */
    ~DefinedPatchConfigurator();

    /**
     * \brief When invoked, it schedules the patch organisation updates over time, following the
     * periods defined in the m_periods vector
     */
    void ScheduleUpdates();
    /**
     * \brief Set the m_periods vector
     * \param periods a DoubleVector containing the periods
     */
    void SetPeriods(const DoubleVector& periods);
    /**
     * \brief Set the LifeTime attribute for each patch of each configuration. It is used by
     * each dinamic patch to schedule the update of nodes to be served
     */
    void SetLifeTimes();

  protected:
    void DoDispose(void);
    void DoInitialize(void);

  private:
    ModelConfigurationMatrix
        m_configurations; ///< Matrix of configurations (i.e. a vector of vector of patches)
    std::vector<double> m_periods; ///< Vector of periods for which each configuration is valid
};

} // namespace ns3

#endif /* DEFINED_PATCH_CONFIGURATOR_H */
