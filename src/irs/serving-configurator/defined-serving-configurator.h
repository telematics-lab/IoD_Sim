/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2023 The IoD_Sim Authors.
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
#ifndef DEFINED_SERVING_CONFIGURATOR_H
#define DEFINED_SERVING_CONFIGURATOR_H

#include "serving-configurator.h"

#include <ns3/double-vector.h>
#include <ns3/object.h>
#include <ns3/str-vec.h>

namespace ns3
{

/**
 * \ingroup irs
 * Defines an object which, aggregated to an Irs Patch, updates the pair of nodes to be served
 * during the patch life time. The different pairs are used for a duration equal to
 * the one defined in m_periods vector.
 */
class DefinedServingConfigurator : public ServingConfigurator
{
  public:
    /**
     * \brief Register this configurator as a type in ns-3 TypeId System.
     */
    static TypeId GetTypeId(void);

    /**
     * \brief Default constructor
     */
    DefinedServingConfigurator();
    /**
     * \brief Default destructor
     */
    ~DefinedServingConfigurator();

    /**
     * \brief When invoked, it schedules the updates of the nodes to be served over time, following
     * the periods defined in the m_periods vector
     */
    void ScheduleUpdates();
    /**
     * \brief Set the m_periods vector
     * \param periods a DoubleVector containing the periods
     */
    void SetPeriods(const DoubleVector& periods);
    /**
     * \brief Set the vector of pairs to be served using a string vector containing the path of the
     * objects
     * \param pairs a string vector containing the pairs to be served
     */
    void SetServingPairs(const StrVec& pairs);

  protected:
    void DoDispose(void);
    void DoInitialize(void);

  private:
    std::vector<std::pair<std::string, std::string>>
        m_servingpairs;            ///< Vector of pairs to be served
    std::vector<double> m_periods; ///< Vector of periods for which each pair is valid
};

} // namespace ns3

#endif /* DEFINED_SERVING_CONFIGURATOR_H */
