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
#ifndef NR_PHY_LAYER_CONFIGURATION_H
#define NR_PHY_LAYER_CONFIGURATION_H

#include "phy-layer-configuration.h"

#include <ns3/cc-bwp-helper.h>
#include <ns3/model-configuration.h>
#include <ns3/nr-channel-helper.h>
#include <ns3/object-factory.h>
#include <ns3/uinteger.h>
#include <ns3/wifi-phy.h>

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// Forward declarations for NR components
namespace ns3
{
class NrHelper;
class NrPointToPointEpcHelper;
class IdealBeamformingHelper;
struct OperationBandInfo; // struct, not class
class NetDeviceContainer;
class NodeContainer;
// BandwidthPartInfoPtr is a typedef, not a class - removing forward declaration
} // namespace ns3

namespace ns3
{

/**
 * Data structure for NR channel configuration
 */
struct NrChannelConfiguration
{
    // Channel scenario (Urban Macro, Urban Micro, etc.)
    std::string scenario = "UMi-StreetCanyon";

    // Channel condition model
    std::string conditionModel = "Default";

    // Propagation model type
    std::string propagationModel = "ThreeGpp";

    // Channel configuration flags
    uint8_t configFlags = NrChannelHelper::INIT_PROPAGATION | NrChannelHelper::INIT_FADING;
};

/**
 * Data structure for NR frequency band configuration
 */
struct NrFrequencyBand
{
    // Central frequency in Hz
    double centralFrequency = 28e9;

    // Bandwidth in Hz
    double bandwidth = 100e6;

    // Number of component carriers
    uint8_t numComponentCarriers = 1;

    // Number of bandwidth parts per CC
    uint8_t numBandwidthParts = 1;
};

/**
 * Data structure for NR band configuration
 */
struct NrBandConfiguration
{
    std::vector<NrFrequencyBand> frequencyBands;
    bool contiguousCc = true;
    // Channel configuration for this band
    NrChannelConfiguration channel;
    std::vector<ModelConfiguration::Attribute> channelConditionAttributes;
    std::vector<ModelConfiguration::Attribute> pathlossAttributes;
    std::vector<ModelConfiguration::Attribute> phasedSpectrumAttributes;
};

/**
 * Data structure for antenna configuration
 */
struct NrAntennaConfiguration
{
    std::string type = "ns3::IsotropicAntennaModel";
    std::vector<ModelConfiguration::Attribute> properties;
    std::vector<ModelConfiguration::Attribute> arrayProperties;
};

/**
 * Data class to store information about the NR PHY Layer of a Scenario.
 * This class only stores configuration data and does not create or manage NR helpers.
 */
class NrPhyLayerConfiguration : public PhyLayerConfiguration
{
  public:
    /**
     * Create a new object instance.
     *
     * \param phyType The type of the PHY Layer to be configured. It should be "nr".
     * \param attributes General PHY attributes
     */
    NrPhyLayerConfiguration(std::string phyType,
                            std::vector<ModelConfiguration::Attribute> attributes);
    /** Default destructor */
    ~NrPhyLayerConfiguration();

    /**
     * Set the beamforming method
     */
    void SetBeamformingMethod(const TypeId& beamformingMethod);

    /**
     * Set the scheduler type
     */
    void SetSchedulerTypeId(const TypeId& schedulerType);

    /**
     * Get scheduler attributes
     */
    std::vector<ModelConfiguration::Attribute> GetSchedulerAttributes() const;

    /**
     * Set scheduler attributes
     * \param attributes The list of attributes that configures the scheduler.
     */
    void SetSchedulerAttributes(const std::vector<ModelConfiguration::Attribute>& attributes);

    /**
     * Get DL error model type
     */
    const TypeId& GetDlErrorModelType() const;

    /**
     * Set DL error model type
     */
    void SetDlErrorModelType(const TypeId& dlErrorModelType);

    /**
     * Get DL error model attributes
     */
    std::vector<ModelConfiguration::Attribute> GetDlErrorModelAttributes() const;

    /**
     * Set DL error model attributes
     * \param attributes The list of attributes that configures the DL error model.
     */
    void SetDlErrorModelAttributes(const std::vector<ModelConfiguration::Attribute>& attributes);

    /**
     * Get UL error model type
     */
    const TypeId& GetUlErrorModelType() const;

    /**
     * Set UL error model type
     */
    void SetUlErrorModelType(const TypeId& ulErrorModelType);

    /**
     * Get UL error model attributes
     */
    std::vector<ModelConfiguration::Attribute> GetUlErrorModelAttributes() const;

    /**
     * Set UL error model attributes
     * \param attributes The list of attributes that configures the UL error model.
     */
    void SetUlErrorModelAttributes(const std::vector<ModelConfiguration::Attribute>& attributes);

    /**
     * Set band configuration
     */
    void AddBandConfiguration(const NrBandConfiguration& bandConfig);
    /**
     * Set gNB antenna configuration
     */
    void SetGnbAntenna(const NrAntennaConfiguration& antennaConfig);

    /**
     * Set UE antenna configuration
     */
    void SetUeAntenna(const NrAntennaConfiguration& antennaConfig);

    /**
     * Get beamforming method
     */
    const TypeId& GetBeamformingMethod() const;

    /**
     * Get scheduler type
     */
    const TypeId& GetSchedulerType() const;

    /**
     * Get gNB antenna configuration
     */
    const NrAntennaConfiguration& GetGnbAntenna() const;
    /**
     * Get UE antenna configuration
     */
    const NrAntennaConfiguration& GetUeAntenna() const;

    /**
     * Get UE PHY attributes
     */
    const std::vector<ModelConfiguration::Attribute>& GetUePhyAttributes() const;

    /**
     * Get gNB PHY attributes
     */
    const std::vector<ModelConfiguration::Attribute>& GetGnbPhyAttributes() const;

    /**
     * Set UE PHY attributes
     * \param attributes The list of attributes that configures the UE PHY.
     */
    void SetUePhyAttributes(const std::vector<ModelConfiguration::Attribute>& attributes);
    /**
     * Set gNB PHY attributes
     * \param attributes The list of attributes that configures the gNB PHY.
     */
    void SetGnbPhyAttributes(const std::vector<ModelConfiguration::Attribute>& attributes);

    /**
     * Get beamforming attributes
     */
    std::vector<ModelConfiguration::Attribute> GetBeamformingAttributes() const;

    /**
     * Set beamforming attributes
     * \param attributes The list of attributes that configures the beamforming method.
     */
    void SetBeamformingAttributes(const std::vector<ModelConfiguration::Attribute>& attributes);

    /**
     * Get bands configuration
     */
    std::vector<NrBandConfiguration> GetBandsConfiguration() const;

    /**
     * Get EPC attributes
     */
    std::vector<ModelConfiguration::Attribute> GetEpcAttributes() const;

    /**
     * Get gNB BWP Manager attributes
     */
    std::vector<ModelConfiguration::Attribute> GetGnbBwpManagerAttributes() const;

    /**
     * Get UE BWP Manager attributes
     */
    std::vector<ModelConfiguration::Attribute> GetUeBwpManagerAttributes() const;

    /**
     * Set gNB BWP Manager attributes
     */
    void SetGnbBwpManagerAttributes(const std::vector<ModelConfiguration::Attribute>& attributes);

    /**
     * Set UE BWP Manager attributes
     */
    void SetUeBwpManagerAttributes(const std::vector<ModelConfiguration::Attribute>& attributes);

    /**
     * Set EPC attributes
     */
    void SetEpcAttributes(const std::vector<ModelConfiguration::Attribute>& attributes);

    /**
     * Set Ue BWP Manager
     */
    void SetUeBwpManager(const TypeId& bwpManagerType);

    /**
     * Set gNB BWP Manager
     */
    void SetGnbBwpManager(const TypeId& bwpManagerType);

    /**
     * Get gNB BWP Manager Type
     */
    TypeId GetGnbBwpManagerType() const;

    /**
     * Get UE BWP Manager Type
     */
    TypeId GetUeBwpManagerType() const;

    /**
     * Set EPC Helper Type
     */
    void SetEpcHelper(const TypeId& epcHelperType);
    /**
     * Get EPC Helper Type
     */
    TypeId GetEpcHelperType() const;
    /**
     * Set Beamforming Helper Type
     */
    void SetBeamformingHelper(const TypeId& beamformingHelperType);
    /**
     * Get Beamforming Helper Type
     */
    TypeId GetBeamformingHelperType() const;

    /**
     * Get Beamforming Method Attributes
     */
    std::vector<ModelConfiguration::Attribute> GetBeamformingAlgorithmAttributes() const;

    /**
     * Set Beamforming Method Attributes
     * \param attributes The list of attributes that configures the beamforming method.
     */
    void SetBeamformingAlgorithmAttributes(
        const std::vector<ModelConfiguration::Attribute>& attributes);

    /**
     * Set Handover Algorithm Type
     */
    void SetHandoverAlgorithmType(const TypeId& typeId);
    /**
     * Get Handover Algorithm Type
     */
    TypeId GetHandoverAlgorithmType() const;

    /**
     * Set Handover Algorithm Attributes
     */
    void SetHandoverAlgorithmAttributes(
        const std::vector<ModelConfiguration::Attribute>& attributes);
    /**
     * Get Handover Algorithm Attributes
     */
    std::vector<ModelConfiguration::Attribute> GetHandoverAlgorithmAttributes() const;

    /**
     * Set UE Channel Access Manager Type
     */
    void SetUeChannelAccessManagerType(const TypeId& typeId);
    /**
     * Get UE Channel Access Manager Type
     */
    TypeId GetUeChannelAccessManagerType() const;

    /**
     * Set UE Channel Access Manager Attributes
     */
    void SetUeChannelAccessManagerAttributes(
        const std::vector<ModelConfiguration::Attribute>& attributes);
    /**
     * Get UE Channel Access Manager Attributes
     */
    std::vector<ModelConfiguration::Attribute> GetUeChannelAccessManagerAttributes() const;

    /**
     * Set gNB Channel Access Manager Type
     */
    void SetGnbChannelAccessManagerType(const TypeId& typeId);
    /**
     * Get gNB Channel Access Manager Type
     */
    TypeId GetGnbChannelAccessManagerType() const;

    /**
     * Set gNB Channel Access Manager Attributes
     */
    void SetGnbChannelAccessManagerAttributes(
        const std::vector<ModelConfiguration::Attribute>& attributes);
    /**
     * Get gNB Channel Access Manager Attributes
     */
    std::vector<ModelConfiguration::Attribute> GetGnbChannelAccessManagerAttributes() const;

    /**
     * Set the attachment method
     * \param attachMethod The attachment method to use ("closest", "max-rsrp", "none")
     */
    void SetAttachMethod(const std::string& attachMethod);

    /**
     * Get the attachment method
     * \return The attachment method
     */
    std::string GetAttachMethod() const;

  private:
    std::string m_attachMethod = "max-rsrp";
    TypeId m_epcHelperType = TypeId::LookupByName("ns3::NrPointToPointEpcHelper");
    TypeId m_beamformingHelperType = TypeId::LookupByName("ns3::IdealBeamformingHelper");
    TypeId m_beamformingMethod = TypeId::LookupByName("ns3::IdealBeamformingAlgorithm");
    TypeId m_schedulerType = TypeId::LookupByName("ns3::NrMacSchedulerTdmaRR");
    std::vector<ModelConfiguration::Attribute> m_schedulerAttributes;
    TypeId m_dlErrorModelType;
    std::vector<ModelConfiguration::Attribute> m_dlErrorModelAttributes;
    TypeId m_ulErrorModelType;
    std::vector<ModelConfiguration::Attribute> m_ulErrorModelAttributes;
    std::vector<NrBandConfiguration> m_bandsConfig;
    NrAntennaConfiguration m_ueAntenna;
    NrAntennaConfiguration m_gnbAntenna;
    std::vector<ModelConfiguration::Attribute> m_uePhyAttributes;
    std::vector<ModelConfiguration::Attribute> m_gnbPhyAttributes;
    std::vector<ModelConfiguration::Attribute> m_beamformingAttributes;
    std::vector<ModelConfiguration::Attribute> m_beamformingAlgorithmAttributes;
    std::vector<ModelConfiguration::Attribute> m_epcAttributes;
    std::vector<ModelConfiguration::Attribute> m_gnbBwpManagerAttributes;
    std::vector<ModelConfiguration::Attribute> m_ueBwpManagerAttributes;
    TypeId m_gnbBwpManagerType = TypeId::LookupByName("ns3::BwpManagerAlgorithmStatic");
    TypeId m_ueBwpManagerType = TypeId::LookupByName("ns3::BwpManagerAlgorithmStatic");
    TypeId m_handoverAlgorithmType;
    std::vector<ModelConfiguration::Attribute> m_handoverAlgorithmAttributes;
    TypeId m_ueChannelAccessManagerType;
    std::vector<ModelConfiguration::Attribute> m_ueChannelAccessManagerAttributes;
    TypeId m_gnbChannelAccessManagerType;
    std::vector<ModelConfiguration::Attribute> m_gnbChannelAccessManagerAttributes;
};

} // namespace ns3

#endif /* NR_PHY_LAYER_CONFIGURATION_H */
