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
#include "model-netdevice-configuration.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(ModelNetdeviceConfiguration);

TypeId
ModelNetdeviceConfiguration::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ModelNetdeviceConfiguration")
                            .SetParent<NetdeviceConfiguration>()
                            .SetGroupName("IoD_Sim");
    return tid;
}

ModelNetdeviceConfiguration::ModelNetdeviceConfiguration(const std::string type,
                                                         const ModelConfiguration macLayer)
    : NetdeviceConfiguration{type},
      m_macLayer{macLayer}
{
}

const ModelConfiguration
ModelNetdeviceConfiguration::GetMacLayer() const
{
    return m_macLayer;
}

} // namespace ns3
