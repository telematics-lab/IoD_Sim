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
#include "three-gpp-phy-layer-configuration.h"

#include <ns3/abort.h>

namespace ns3
{

ThreeGppPhyLayerConfiguration::ThreeGppPhyLayerConfiguration(
    std::string phyType,
    std::vector<ModelConfiguration::Attribute> attributes,
    ModelConfiguration propagationLossModel,
    ModelConfiguration conditionModel)
    : PhyLayerConfiguration{phyType, attributes},
      m_propagationLossModel{propagationLossModel},
      m_conditionModel{conditionModel}
{
}

ThreeGppPhyLayerConfiguration::~ThreeGppPhyLayerConfiguration()
{
}

const ModelConfiguration
ThreeGppPhyLayerConfiguration::GetPropagationLossModel()
{
    return m_propagationLossModel;
}

const ModelConfiguration
ThreeGppPhyLayerConfiguration::GetConditionModel()
{
    return m_conditionModel;
}

} // namespace ns3