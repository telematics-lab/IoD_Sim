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
    const std::optional<uint32_t> networkLayerId,
    const std::optional<ModelConfiguration> antennaModel,
    const std::optional<ModelConfiguration> phyModel)
    : NetdeviceConfiguration{type, networkLayerId, antennaModel},
      m_role{NrNetdeviceConfigurationPriv::ParseRole(rawRole)},
      m_bearers{bearers},
      m_phyModel{phyModel}
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

const std::optional<ModelConfiguration>
NrNetdeviceConfiguration::GetPhyModel() const
{
    return m_phyModel;
}

} // namespace ns3
