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
#include "nr-phy-layer-configuration.h"

#include <ns3/abort.h>
#include <ns3/cc-bwp-helper.h>

#include <functional>

namespace ns3
{

NrPhyLayerConfiguration::NrPhyLayerConfiguration(
    std::string phyType,
    std::vector<ModelConfiguration::Attribute> attributes)
    : PhyLayerConfiguration{phyType, attributes}
{
    // Initialize with default values - no helper creation here
}

NrPhyLayerConfiguration::~NrPhyLayerConfiguration()
{
}

void
NrPhyLayerConfiguration::SetBeamformingMethod(const std::string& beamformingMethod)
{
    m_beamformingMethod = beamformingMethod;
}

void
NrPhyLayerConfiguration::SetScheduler(const std::string& schedulerType)
{
    m_schedulerType = schedulerType;
}

void
NrPhyLayerConfiguration::AddBandConfiguration(const NrBandConfiguration& bandConfig)
{
    m_bandsConfig.push_back(bandConfig);
}

void
NrPhyLayerConfiguration::SetGnbAntenna(const NrAntennaConfiguration& antennaConfig)
{
    m_gnbAntenna = antennaConfig;
}

void
NrPhyLayerConfiguration::SetUeAntenna(const NrAntennaConfiguration& antennaConfig)
{
    m_ueAntenna = antennaConfig;
}

/**
 * Get beamforming method
 */
const std::string&
NrPhyLayerConfiguration::GetBeamformingMethod() const
{
    return m_beamformingMethod;
}

/**
 * Get scheduler type
 */
const std::string&
NrPhyLayerConfiguration::GetSchedulerType() const
{
    return m_schedulerType;
}

/**
 * Get gNB antenna configuration
 */
const NrAntennaConfiguration&
NrPhyLayerConfiguration::GetGnbAntenna() const
{
    return m_gnbAntenna;
}

/**
 * Get UE antenna configuration
 */
const NrAntennaConfiguration&
NrPhyLayerConfiguration::GetUeAntenna() const
{
    return m_ueAntenna;
}

std::string
NrPhyLayerConfiguration::GetAttribute(const std::string& name) const
{
    if (name == "BeamformingMethod")
    {
        return m_beamformingMethod;
    }
    else if (name == "SchedulerType")
    {
        return m_schedulerType;
    }
    return "";
}

const std::vector<ModelConfiguration::Attribute>&
NrPhyLayerConfiguration::GetUePhyAttributes() const
{
    return m_uePhyAttributes;
}

const std::vector<ModelConfiguration::Attribute>&
NrPhyLayerConfiguration::GetGnbPhyAttributes() const
{
    return m_gnbPhyAttributes;
}

void
NrPhyLayerConfiguration::SetUePhyAttributes(
    const std::vector<ModelConfiguration::Attribute>& attributes)
{
    m_uePhyAttributes = attributes;
}

void
NrPhyLayerConfiguration::SetGnbPhyAttributes(
    const std::vector<ModelConfiguration::Attribute>& attributes)
{
    m_gnbPhyAttributes = attributes;
}

std::vector<NrBandConfiguration>
NrPhyLayerConfiguration::GetBandsConfiguration() const
{
    return m_bandsConfig;
}

/**
 * Get beamforming attributes
 */
std::vector<ModelConfiguration::Attribute>
NrPhyLayerConfiguration::GetBeamformingAttributes() const
{
    return m_beamformingAttributes;
}

/**
 * Set beamforming attributes
 * \param attributes The list of attributes that configures the beamforming method.
 */
void
NrPhyLayerConfiguration::SetBeamformingAttributes(
    const std::vector<ModelConfiguration::Attribute>& attributes)
{
    m_beamformingAttributes = attributes;
}

/**
 * Set channel condition attributes
 * \param attributes The list of attributes that configures the channel condition model.
 */
void
NrPhyLayerConfiguration::SetChannelConditionAttributes(
    const std::vector<ModelConfiguration::Attribute>& attributes)
{
    m_channelConditionAttributes = attributes;
}

/**
 * Set pathloss attributes
 * \param attributes The list of attributes that configures the pathloss model.
 */
void
NrPhyLayerConfiguration::SetPathlossAttributes(
    const std::vector<ModelConfiguration::Attribute>& attributes)
{
    m_pathlossAttributes = attributes;
}

/**
 * Get channel condition attributes
 */
std::vector<ModelConfiguration::Attribute>
NrPhyLayerConfiguration::GetChannelConditionAttributes() const
{
    return m_channelConditionAttributes;
}

/**
 * Get pathloss attributes
 */
std::vector<ModelConfiguration::Attribute>
NrPhyLayerConfiguration::GetPathlossAttributes() const
{
    return m_pathlossAttributes;
}

std::vector<ModelConfiguration::Attribute>
NrPhyLayerConfiguration::GetEpcAttributes() const
{
    return m_epcAttributes;
}

void
NrPhyLayerConfiguration::SetEpcAttributes(
    const std::vector<ModelConfiguration::Attribute>& attributes)
{
    m_epcAttributes = attributes;
}

std::vector<ModelConfiguration::Attribute>
NrPhyLayerConfiguration::GetGnbBwpManagerAttributes() const
{
    return m_gnbBwpManagerAttributes;
}

void
NrPhyLayerConfiguration::SetGnbBwpManagerAttributes(
    const std::vector<ModelConfiguration::Attribute>& attributes)
{
    m_gnbBwpManagerAttributes = attributes;
}

std::vector<ModelConfiguration::Attribute>
NrPhyLayerConfiguration::GetUeBwpManagerAttributes() const
{
    return m_ueBwpManagerAttributes;
}

void
NrPhyLayerConfiguration::SetUeBwpManagerAttributes(
    const std::vector<ModelConfiguration::Attribute>& attributes)
{
    m_ueBwpManagerAttributes = attributes;
}

} // namespace ns3
