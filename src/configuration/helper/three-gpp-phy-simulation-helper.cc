/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2026 The IoD_Sim Authors.
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
#include "three-gpp-phy-simulation-helper.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(ThreeGppPhySimulationHelper);

TypeId
ThreeGppPhySimulationHelper::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::ThreeGppPhySimulationHelper")
                            .SetParent<Object>()
                            .SetGroupName("IoDSim");
    return tid;
}

ThreeGppPhySimulationHelper::ThreeGppPhySimulationHelper(
    const std::size_t stackId,
    Ptr<ThreeGppChannelConditionModel> channelConditionModel,
    Ptr<ThreeGppPropagationLossModel> propagationLossModel,
    Ptr<ThreeGppSpectrumPropagationLossModel> spectrumPropagationLossModel)
    : m_stackId{stackId},
      m_channelConditionModel{channelConditionModel},
      m_propagationLossModel{propagationLossModel},
      m_spectrumPropagationLossModel{spectrumPropagationLossModel}
{
}

Ptr<ThreeGppChannelConditionModel>
ThreeGppPhySimulationHelper::GetChannelConditionModel()
{
    return m_channelConditionModel;
}

Ptr<ThreeGppPropagationLossModel>
ThreeGppPhySimulationHelper::GetPropagationLossModel()
{
    return m_propagationLossModel;
}

Ptr<ThreeGppSpectrumPropagationLossModel>
ThreeGppPhySimulationHelper::GetSpectrumPropagationLossModel()
{
    return m_spectrumPropagationLossModel;
}

} // namespace ns3
