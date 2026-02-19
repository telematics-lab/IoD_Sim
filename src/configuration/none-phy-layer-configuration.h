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
#ifndef NONE_PHY_LAYER_CONFIGURATION_H
#define NONE_PHY_LAYER_CONFIGURATION_H

#include "phy-layer-configuration.h"

#include <ns3/model-configuration.h>
#include <ns3/wifi-phy.h>

#include <optional>
#include <string>
#include <vector>

namespace ns3
{

/**
 * Data class to store information about the None PHY Layer of a Scenario.
 */
class NonePhyLayerConfiguration : public PhyLayerConfiguration
{
  public:
    /**
     * Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * Create a new object instance.
     *
     * \param phyType The type of the PHY Layer to be configured. It should be "none".
     */
    NonePhyLayerConfiguration(std::string phyType,
                              std::vector<ModelConfiguration::Attribute> attributes);
};

} // namespace ns3

#endif /* NONE_PHY_LAYER_CONFIGURATION_H */
