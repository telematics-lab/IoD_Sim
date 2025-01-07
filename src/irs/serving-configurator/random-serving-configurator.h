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
#ifndef RANDOM_SERVING_CONFIGURATOR_H
#define RANDOM_SERVING_CONFIGURATOR_H

#include "serving-configurator.h"

#include <ns3/double-vector.h>
#include <ns3/object.h>
#include <ns3/pointer.h>
#include <ns3/random-variable-stream.h>
#include <ns3/str-vec.h>

namespace ns3
{

/**
 * \ingroup irs
 *
 * \brief Define a random serving configurator for the IRS.
 *
 * Defines an object which, aggregated to an Irs Patch, updates the pair of nodes to be served
 * during the patch life time. The update of the serving pair is performed with a round robin
 * approach, choosing randomly a pair from the m_servingpairs vector
 */
class RandomServingConfigurator : public ServingConfigurator
{
  public:
    /**
     * \brief Register this configurator as a type in ns-3 TypeId System.
     */
    static TypeId GetTypeId(void);

    RandomServingConfigurator();

    /**
     * \brief When invoked, it schedules the updates of the nodes to be served over time with a
     * period of one m_timeslot, until the end of patch lifetime is reached
     */
    void ScheduleUpdates();
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
        m_servingpairs;               ///< Vector of pairs to be served
    double m_timeslot;                ///< The duration of a time slot
    Ptr<UniformRandomVariable> m_rng; ///< Random number generator used to choose the next pair
};

} // namespace ns3

#endif /* RANDOM_SERVING_CONFIGURATOR_H */
