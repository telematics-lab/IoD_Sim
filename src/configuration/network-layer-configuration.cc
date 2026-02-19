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
#include "network-layer-configuration.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(NetworkLayerConfiguration);

TypeId
NetworkLayerConfiguration::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NetworkLayerConfiguration")
                            .SetParent<Object>()
                            .SetGroupName("IoD_Sim");
    return tid;
}

NetworkLayerConfiguration::NetworkLayerConfiguration(std::string type)
    : m_type{type}
{
}

const std::string
NetworkLayerConfiguration::GetType() const
{
    return m_type;
}

} // namespace ns3
