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
#include "nr-netdevice-configuration.h"

namespace ns3
{

class NrNetdeviceConfigurationPriv
{
  public:
    static const NrRole ParseRole(const std::string rawRole)
    {
        if (rawRole == "UE")
            return NrRole::nrUE;
        else if (rawRole == "gNB")
            return NrRole::gNB;
        else
            NS_FATAL_ERROR("Unsupported NR Role: " << rawRole);
    }
};

NrNetdeviceConfiguration::NrNetdeviceConfiguration(
    const std::string type,
    const std::string rawRole,
    const std::vector<NrBearerConfiguration> bearers,
    const std::vector<NrPhyProperty> phyProperties,
    const std::optional<uint32_t> networkLayerId,
    const std::optional<ModelConfiguration> antennaModel,
    const std::vector<OutputLinkConfiguration> outputLinks,
    const std::optional<DirectivityConfiguration> directivity,
    const uint32_t channelId,
    const std::vector<uint32_t> channelBands,
    const std::vector<NrPhyProperty> rrcProperties,
    const std::vector<X2NeighborConfiguration> x2Neighbors)
    : NetdeviceConfiguration{type, networkLayerId, antennaModel, directivity},
      m_role{NrNetdeviceConfigurationPriv::ParseRole(rawRole)},
      m_bearers{bearers},
      m_phyProperties{phyProperties},
      m_rrcProperties{rrcProperties},
      m_outputLinks{outputLinks},
      m_x2Neighbors{x2Neighbors},
      m_channelId{channelId},
      m_channelBands{channelBands}
{
}

NrNetdeviceConfiguration::~NrNetdeviceConfiguration()
{
}

const NrRole
NrNetdeviceConfiguration::GetRole() const
{
    return m_role;
}

const std::vector<NrBearerConfiguration>
NrNetdeviceConfiguration::GetBearers() const
{
    return m_bearers;
}

const std::vector<NrPhyProperty>
NrNetdeviceConfiguration::GetPhyProperties() const
{
    return m_phyProperties;
}

const std::vector<NrPhyProperty>
NrNetdeviceConfiguration::GetRrcProperties() const
{
    return m_rrcProperties;
}

const std::vector<X2NeighborConfiguration>
NrNetdeviceConfiguration::GetX2Neighbors() const
{
    return m_x2Neighbors;
}

const std::vector<OutputLinkConfiguration>
NrNetdeviceConfiguration::GetOutputLinks() const
{
    return m_outputLinks;
}

const uint32_t
NrNetdeviceConfiguration::GetChannelId() const
{
    return m_channelId;
}

const std::vector<uint32_t>
NrNetdeviceConfiguration::GetChannelBands() const
{
    return m_channelBands;
}

} // namespace ns3
