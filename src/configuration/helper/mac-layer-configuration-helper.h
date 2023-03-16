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
#ifndef MAC_LAYER_CONFIGURATION_HELPER_H
#define MAC_LAYER_CONFIGURATION_HELPER_H

#include <rapidjson/document.h>

#include <ns3/mac-layer-configuration.h>
#include <ns3/model-configuration.h>

namespace ns3 {

/**
 * Helper to decode a MAC Layer from a JSON configuration file.
 */
class MacLayerConfigurationHelper
{
public:
  /**
   * Parse a MAC Layer configuration from a given JSON tree and map it on a MacLayerConfiguration data class.
   *
   * \param jsonMacLayer The JSON tree to parse.
   * \return The configuration as a pointer to MacLayerConfiguration to easily retrieve parsed data.
   */
  static Ptr<MacLayerConfiguration> GetConfiguration (const rapidjson::Value& jsonMacLayer);

private:
  MacLayerConfigurationHelper();
  static const ModelConfiguration DecodeModelConfiguration (const rapidjson::Value& jsonModel);
};

} // namespace ns3

#endif /* MAC_LAYER_CONFIGURATION_HELPER_H */
