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
#ifndef THREE_GPP_PHY_SIMULATION_HELPER_H
#define THREE_GPP_PHY_SIMULATION_HELPER_H

#include <ns3/channel-condition-model.h>
#include <ns3/three-gpp-propagation-loss-model.h>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>

namespace ns3
{

/**
 * \brief A data class to store information about a 3GPP PHY layer configuration for a simulation.
 */
class ThreeGppPhySimulationHelper : public Object
{
  public:
    ThreeGppPhySimulationHelper(
        const std::size_t stackId,
        Ptr<ThreeGppChannelConditionModel> channelConditionModel,
        Ptr<ThreeGppPropagationLossModel> propagationLossModel,
        Ptr<ThreeGppSpectrumPropagationLossModel> spectrumPropagationLossModel);

    Ptr<ThreeGppChannelConditionModel> GetChannelConditionModel();
    Ptr<ThreeGppPropagationLossModel> GetPropagationLossModel();
    Ptr<ThreeGppSpectrumPropagationLossModel> GetSpectrumPropagationLossModel();

  private:
    [[maybe_unused]] const std::size_t m_stackId;
    Ptr<ThreeGppChannelConditionModel> m_channelConditionModel;
    Ptr<ThreeGppPropagationLossModel> m_propagationLossModel;
    Ptr<ThreeGppSpectrumPropagationLossModel> m_spectrumPropagationLossModel;
};

} // namespace ns3

#endif /* THREE_GPP_PHY_SIMULATION_HELPER_H */
