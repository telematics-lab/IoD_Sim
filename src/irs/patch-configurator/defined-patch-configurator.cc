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
#include "defined-patch-configurator.h"

#include <ns3/irs.h>
#include <ns3/model-configuration-matrix.h>
#include <ns3/model-configuration.h>
#include <ns3/pointer.h>
#include <ns3/simulator.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("DefinedPatchConfigurator");
NS_OBJECT_ENSURE_REGISTERED(DefinedPatchConfigurator);

TypeId
DefinedPatchConfigurator::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::DefinedPatchConfigurator")
            .SetParent<PatchConfigurator>()
            .SetGroupName("Irs")
            .AddConstructor<DefinedPatchConfigurator>()
            .AddAttribute("Periods",
                          "List of validity periods for each configuration",
                          DoubleVectorValue(),
                          MakeDoubleVectorAccessor(&DefinedPatchConfigurator::SetPeriods),
                          MakeDoubleVectorChecker())
            .AddAttribute(
                "Configurations",
                "List of configurations to be applied to the Irs during the simulation",
                ModelConfigurationMatrixValue(),
                MakeModelConfigurationMatrixAccessor(&DefinedPatchConfigurator::m_configurations),
                MakeModelConfigurationMatrixChecker());
    return tid;
}

void
DefinedPatchConfigurator::DoDispose()
{
    NS_LOG_FUNCTION(this);
    Object::DoDispose();
}

void
DefinedPatchConfigurator::DoInitialize(void)
{
    NS_LOG_FUNCTION(this);
    Object::DoInitialize();
    SetLifeTimes();
    ScheduleUpdates();
}

void
DefinedPatchConfigurator::ScheduleUpdates()
{
    // You need to run this method at the beginning of the simulation since Simulator::Schedule(...)
    // accepts as parameter the delay from now
    if (Simulator::Now().GetSeconds() > 0.0)
        NS_LOG_WARN(
            "ScheduleUpdates has been executed during the simulation, the timing could be shifted");

    double nextupdate = 0;
    for (uint32_t i = 0; i < m_periods.size(); i++)
    {
        Simulator::Schedule(Seconds(nextupdate),
                            &DefinedPatchConfigurator::UpdateConfiguration,
                            this,
                            m_configurations.Get(i));
        nextupdate += m_periods.at(i);
    }
}

void
DefinedPatchConfigurator::SetPeriods(const DoubleVector& periods)
{
    for (auto c = periods.Begin(); c != periods.End(); c++)
    {
        m_periods.push_back(*c);
    }
}

void
DefinedPatchConfigurator::SetLifeTimes()
{
    uint32_t periodsIdx = 0;
    NS_ASSERT_MSG(m_periods.size() == m_configurations.GetN(),
                  "The number of periods and configurations must be equal");

    for (auto confIt = m_configurations.MutableBegin(); confIt != m_configurations.MutableEnd();
         confIt++)
    {
        for (auto patchIt = confIt->MutableBegin(); patchIt != confIt->MutableEnd(); patchIt++)
        {
            auto lifeTimeValue = DoubleValue(m_periods.at(periodsIdx));
            Ptr<AttributeValue> attr = MakeDoubleChecker<double>()->CreateValidValue(lifeTimeValue);
            patchIt->SetAttribute("LifeTime", attr);
        }

        periodsIdx++;
    }
}

} // namespace ns3
