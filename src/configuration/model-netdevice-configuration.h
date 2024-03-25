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
#ifndef MODEL_NETDEVICE_CONFIGURATION_H
#define MODEL_NETDEVICE_CONFIGURATION_H

#include "netdevice-configuration.h"

#include <ns3/model-configuration.h>
#include <ns3/object.h>

namespace ns3
{

/**
 * Data class to recnognize and configure a Network Device for an entity to be simulated.
 */
class ModelNetdeviceConfiguration : public NetdeviceConfiguration
{
  public:
    /**
     * Create a new object instance.
     *
     * \param type The type of the network device - in this case "Model" to use a ns-3 TypeId.
     * \param macLayer The configuration of the MAC Layer to be simulated for this network device.
     */
    ModelNetdeviceConfiguration(const std::string type, const ModelConfiguration macLayer);
    /** Default destructor */
    ~ModelNetdeviceConfiguration();

    /**
     * \return The MAC Layer configuration.
     */
    const ModelConfiguration GetMacLayer() const;

  private:
    const ModelConfiguration m_macLayer;
};

} // namespace ns3

#endif /* MODEL_NETDEVICE_CONFIGURATION_H */
