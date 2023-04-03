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
#include "random-serving-configurator.h"

#include <ns3/irs-patch.h>
#include <ns3/random-variable-stream.h>
#include <ns3/simulator.h>
#include <ns3/str-vec.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("RandomServingConfigurator");
NS_OBJECT_ENSURE_REGISTERED(RandomServingConfigurator);

TypeId
RandomServingConfigurator::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::RandomServingConfigurator")
            .SetParent<ServingConfigurator>()
            .SetGroupName("Irs")
            .AddConstructor<RandomServingConfigurator>()
            .AddAttribute(
                "Timeslot",
                "Duration in seconds of the timeslot in which is used the same serving nodes pair",
                DoubleValue(0.1),
                MakeDoubleAccessor(&RandomServingConfigurator::m_timeslot),
                MakeDoubleChecker<double>())
            .AddAttribute(
                "ServingPairs",
                "List of serving nodes pairs to be applied to the Irs Patch during the simulation",
                StrVecValue(),
                MakeStrVecAccessor(&RandomServingConfigurator::SetServingPairs),
                MakeStrVecChecker());
    return tid;
}

RandomServingConfigurator::RandomServingConfigurator()
{
    m_rng = CreateObject<UniformRandomVariable>();
}

RandomServingConfigurator::~RandomServingConfigurator()
{
}

void
RandomServingConfigurator::DoDispose()
{
    NS_LOG_FUNCTION(this);
    Object::DoDispose();
}

void
RandomServingConfigurator::DoInitialize(void)
{
    NS_LOG_FUNCTION(this);
    Object::DoInitialize();
    ScheduleUpdates();
}

void
RandomServingConfigurator::ScheduleUpdates()
{
    double lifetime = GetObject<IrsPatch>()->GetLifeTime();
    double nextupdate = 0;
    double periodstart = Simulator::Now().GetSeconds();
    double periodend = periodstart + lifetime;
    uint32_t j = 0;
    while (true)
    {
        if (periodend <= periodstart + nextupdate)
        {
            // TODO: schedule patch and serving configurator destruction?
            // Simulator::Schedule (Seconds (nextupdate), &PeriodicServingConfigurator::DoDispose,
            // this);
            break; // we have reached the end of the period, no need to continue
        }
        if (nextupdate > 0)
        {
            Simulator::Schedule(Seconds(nextupdate),
                                &RandomServingConfigurator::UpdateServingNodes,
                                this,
                                m_servingpairs.at(m_rng->GetInteger(0, m_servingpairs.size() - 1)));
        }
        else
        {
            UpdateServingNodes(m_servingpairs.at(m_rng->GetInteger(0, m_servingpairs.size() - 1)));
        }
        nextupdate += m_timeslot;
        j++;
    }
}

void
RandomServingConfigurator::SetServingPairs(const StrVec& pairs)
{
    // The string vector containing the pairs of serving nodes must have as its length an even
    // number
    NS_ASSERT_MSG((pairs.GetN() % 2 == 0),
                  "Length of the vector wich contains serving nodes is not a multiple of 2");
    m_servingpairs.clear();
    for (uint32_t i = 0; i < pairs.GetN(); i += 2)
    {
        m_servingpairs.push_back(std::make_pair(pairs.Get(i), pairs.Get(i + 1)));
    }
}

} // namespace ns3
