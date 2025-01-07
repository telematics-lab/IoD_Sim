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
#ifndef PHY_LAYER_CONFIGURATION_HELPER_H
#define PHY_LAYER_CONFIGURATION_HELPER_H

#include <ns3/model-configuration.h>
#include <ns3/phy-layer-configuration.h>

#include <rapidjson/document.h>

namespace ns3
{

/**
 * \brief Helper to decode a PHY Layer from a JSON configuration file.
 */
class PhyLayerConfigurationHelper
{
  public:
    /**
     * PhyLayerConfigurationHelper can't be instantiated, as it is an utility class.
     */
    PhyLayerConfigurationHelper() = delete;
    /**
     * Parse a PHY configuration from a given JSON tree and map it on a PhyLayerConfiguration data
     * class.
     *
     * \param jsonPhyLayer The JSON tree to parse.
     * \return The configuration as a pointer to PhyLayerConfiguration to easily retrieve parsed
     * data.
     */
    static Ptr<PhyLayerConfiguration> GetConfiguration(const rapidjson::Value& jsonPhyLayer);
};

} // namespace ns3

#endif /* PHY_LAYER_CONFIGURATION_HELPER_H */
