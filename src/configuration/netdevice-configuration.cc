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
#include "netdevice-configuration.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(NetdeviceConfiguration);

TypeId
NetdeviceConfiguration::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NetdeviceConfiguration")
                            .SetParent<Object>()
                            .SetGroupName("IoD_Sim");
    return tid;
}

NetdeviceConfiguration::NetdeviceConfiguration(
    const std::string type,
    const std::optional<uint32_t> networkLayerId,
    const std::optional<ModelConfiguration> antennaModel,
    const std::optional<DirectivityConfiguration> directivity)
    : m_type{type},
      m_networkLayerId{networkLayerId},
      m_antennaModel{antennaModel},
      m_directivity{directivity}
{
}

const std::string
NetdeviceConfiguration::GetType() const
{
    return m_type;
}

const std::optional<uint32_t>
NetdeviceConfiguration::GetNetworkLayerId() const
{
    return m_networkLayerId;
}

std::optional<ModelConfiguration>
NetdeviceConfiguration::GetAntennaModel() const
{
    return m_antennaModel;
}

std::optional<DirectivityConfiguration>
NetdeviceConfiguration::GetDirectivity() const
{
    return m_directivity;
}

} // namespace ns3
