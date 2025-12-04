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
#include "phy-layer-configuration-helper.h"

#include "model-configuration-helper.h"

#include <ns3/assert.h>
#include <ns3/beamforming-helper-base.h>
#include <ns3/double.h>
#include <ns3/fatal-error.h>
#include <ns3/lte-phy-layer-configuration.h>
#include <ns3/none-phy-layer-configuration.h>
#include <ns3/nr-amc.h>
#include <ns3/nr-channel-helper.h>
#include <ns3/nr-epc-helper.h>
#include <ns3/nr-phy-layer-configuration.h>
#include <ns3/string.h>
#include <ns3/three-gpp-phy-layer-configuration.h>
#include <ns3/type-id.h>
#include <ns3/wifi-phy-layer-configuration.h>

namespace ns3
{

Ptr<PhyLayerConfiguration>
PhyLayerConfigurationHelper::GetConfiguration(const rapidjson::Value& jsonPhyLayer)

{
    NS_ASSERT_MSG(jsonPhyLayer.IsObject(),
                  "PHY Layer definition must be an object, got " << jsonPhyLayer.GetType());
    NS_ASSERT_MSG(jsonPhyLayer.HasMember("type"),
                  "PHY Layer definition must have 'type' property.");
    NS_ASSERT_MSG(jsonPhyLayer["type"].IsString(), "PHY Layer 'type' property must be a string.");

    const std::string phyType = jsonPhyLayer["type"].GetString();
    Ptr<PhyLayerConfiguration> phyConfig{nullptr};

    if (phyType == "wifi")
    {
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("standard"),
                      "Wi-Fi PHY Layer definition must have 'standard' property.");
        NS_ASSERT_MSG(jsonPhyLayer["standard"].IsString(),
                      "Wi-Fi PHY Layer 'standard' property must be a string.");
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("attributes"),
                      "Wi-Fi PHY Layer definition must have 'attributes' property.");
        NS_ASSERT_MSG(jsonPhyLayer["attributes"].IsArray(),
                      "Wi-Fi PHY Layer 'attributes' must be an object.");
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("channel"),
                      "Wi-Fi PHY Layer definition must have 'channel' property.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"].IsObject(),
                      "Wi-Fi PHY Layer 'mode' property must be an object.");
        NS_ASSERT_MSG(
            jsonPhyLayer["channel"].HasMember("propagationDelayModel"),
            "Wi-Fi PHY Layer channel definition must have 'propagationDelayModel' property.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"]["propagationDelayModel"].IsObject(),
                      "Wi-Fi PHY Layer channel 'propagationDelayModel' must be an object");
        NS_ASSERT_MSG(
            jsonPhyLayer["channel"].HasMember("propagationLossModel"),
            "Wi-Fi PHY Layer channel definition must have 'propagationLossModel' property.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"]["propagationLossModel"].IsObject(),
                      "Wi-Fi PHY Layer channel 'propagationLossModel' must be an object");

        const auto phyAttributes =
            ModelConfigurationHelper::GetAttributes(TypeId::LookupByName("ns3::WifiPhy"),
                                                    jsonPhyLayer["attributes"].GetArray());
        const auto propagationDelayModel =
            ModelConfigurationHelper::Get(jsonPhyLayer["channel"]["propagationDelayModel"]);
        const auto propagationLossModel =
            ModelConfigurationHelper::Get(jsonPhyLayer["channel"]["propagationLossModel"]);

        return Create<WifiPhyLayerConfiguration>(phyType,
                                                 jsonPhyLayer["standard"].GetString(),
                                                 phyAttributes,
                                                 propagationDelayModel,
                                                 propagationLossModel);
    }
    if (phyType == "none")
    {
        return Create<NonePhyLayerConfiguration>(phyType,
                                                 std::vector<ModelConfiguration::Attribute>());
    }
    else if (phyType == "lte")
    {
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("attributes"),
                      "LTE PHY Layer definition must have 'attributes' property.");
        NS_ASSERT_MSG(jsonPhyLayer["attributes"].IsArray(),
                      "LTE PHY Layer 'attributes' must be an object.");
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("channel"),
                      "LTE PHY Layer definition must have 'channel' property.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"].IsObject(),
                      "LTE PHY Layer 'mode' property must be an object.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"].HasMember("spectrumModel"),
                      "LTE PHY Layer channel definition must have 'spectrumModel' property.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"]["spectrumModel"].IsObject(),
                      "LTE PHY Layer channel 'spectrumModel' must be an object");

        const auto phyAttributes =
            ModelConfigurationHelper::GetAttributes(TypeId::LookupByName("ns3::LteHelper"),
                                                    jsonPhyLayer["attributes"].GetArray());
        const auto spectrumModel =
            ModelConfigurationHelper::Get(jsonPhyLayer["channel"]["spectrumModel"]);
        const auto propagationLossModel =
            ModelConfigurationHelper::GetOptional(jsonPhyLayer["channel"].GetObject(),
                                                  "propagationLossModel");

        phyConfig = Create<LtePhyLayerConfiguration>(phyType,
                                                     phyAttributes,
                                                     propagationLossModel,
                                                     spectrumModel);
    }
    else if (phyType == "3GPP")
    {
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("attributes"),
                      "3GPP PHY Layer definition must have 'attributes' property.");
        NS_ASSERT_MSG(jsonPhyLayer["attributes"].IsArray(),
                      "3GPP PHY Layer 'attributes' must be an object.");
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("channel"),
                      "3GPP PHY Layer definition must have 'channel' property.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"].IsObject(),
                      "3GPP PHY Layer 'mode' property must be an object.");
        NS_ASSERT_MSG(
            jsonPhyLayer["channel"].HasMember("propagationLossModel"),
            "3GPP PHY Layer channel definition must have 'propagationLossModel' property.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"]["propagationLossModel"].IsObject(),
                      "3GPP PHY Layer channel 'propagationLossModel' must be an object");
        NS_ASSERT_MSG(jsonPhyLayer["channel"].HasMember("conditionModel"),
                      "3GPP PHY Layer channel definition must have 'conditionModel' property.");
        NS_ASSERT_MSG(jsonPhyLayer["channel"]["conditionModel"].IsObject(),
                      "3GPP PHY Layer channel 'conditionModel' must be an object");
        NS_ASSERT_MSG(jsonPhyLayer.HasMember("environment"),
                      "3GPP PHY Layer definition must have 'environment' property.");

        const auto phyAttributes = ModelConfigurationHelper::GetAttributes(
            TypeId::LookupByName("ns3::ThreeGppChannelModel"),
            jsonPhyLayer["attributes"].GetArray());
        const auto propagationLossModel =
            ModelConfigurationHelper::Get(jsonPhyLayer["channel"]["propagationLossModel"]);
        const auto conditionModel =
            ModelConfigurationHelper::Get(jsonPhyLayer["channel"]["conditionModel"]);
        const auto environment = jsonPhyLayer["environment"].GetString();

        phyConfig = Create<ThreeGppPhyLayerConfiguration>(phyType,
                                                          phyAttributes,
                                                          propagationLossModel,
                                                          conditionModel,
                                                          environment);
    }
    else if (phyType == "nr")
    {
        Ptr<NrPhyLayerConfiguration> nrConfig;

        if (!jsonPhyLayer.HasMember("attributes"))
        {
            nrConfig =
                Create<NrPhyLayerConfiguration>(phyType, ModelConfiguration::AttributeVector());
        }
        else
        {
            NS_ASSERT_MSG(jsonPhyLayer["attributes"].IsArray(),
                          "NR PHY Layer 'attributes' must be an array.");
            const auto phyAttributes =
                ModelConfigurationHelper::GetAttributes(TypeId::LookupByName("ns3::NrHelper"),
                                                        jsonPhyLayer["attributes"].GetArray());
            nrConfig = Create<NrPhyLayerConfiguration>(phyType, phyAttributes);
        }

        if (jsonPhyLayer.HasMember("attachMethod"))
        {
            NS_ASSERT_MSG(jsonPhyLayer["attachMethod"].IsString(),
                          "NR PHY Layer 'attachMethod' must be a string.");
            std::string attachMethod = jsonPhyLayer["attachMethod"].GetString();
            if (attachMethod != "closest" && attachMethod != "max-rsrp" && attachMethod != "none")
            {
                NS_FATAL_ERROR("Invalid attachMethod: " << attachMethod
                                                        << ". Valid values are: closest, max-rsrp, none");
            }
            nrConfig->SetAttachMethod(attachMethod);
        }
        // Parse beamforming configuration
        if (jsonPhyLayer.HasMember("beamforming") && jsonPhyLayer["beamforming"].IsObject())
        {
            const auto& beamforming = jsonPhyLayer["beamforming"];
            if (beamforming.HasMember("helper") && beamforming["helper"].IsString())
            {
                auto helperType = TypeId::LookupByName(beamforming["helper"].GetString());
                if (!helperType.IsChildOf(BeamformingHelperBase::GetTypeId()))
                {
                    NS_FATAL_ERROR("Beamforming helper "
                                   << beamforming["helper"].GetString()
                                   << " is not a valid BeamformingHelperBase");
                }
                nrConfig->SetBeamformingHelper(helperType);
            }

            if (beamforming.HasMember("method") && beamforming["method"].IsString())
            {
                nrConfig->SetBeamformingMethod(
                    TypeId::LookupByName(beamforming["method"].GetString()));
            }
            else
            {
                nrConfig->SetBeamformingMethod(TypeId::LookupByName("ns3::DirectPathBeamforming"));
            }

            if (beamforming.HasMember("attributes") && beamforming["attributes"].IsArray())
            {
                const auto bfAttributes =
                    ModelConfigurationHelper::GetAttributes(nrConfig->GetBeamformingHelperType(),
                                                            beamforming["attributes"].GetArray());
                nrConfig->SetBeamformingAttributes(bfAttributes);
            }

            if (beamforming.HasMember("algorithmAttributes") &&
                beamforming["algorithmAttributes"].IsArray())
            {
                const auto bfMethodAttributes = ModelConfigurationHelper::GetAttributes(
                    nrConfig->GetBeamformingMethod(),
                    beamforming["algorithmAttributes"].GetArray());
                nrConfig->SetBeamformingAlgorithmAttributes(bfMethodAttributes);
            }
        }
        else
        {
            nrConfig->SetBeamformingMethod(TypeId::LookupByName("ns3::DirectPathBeamforming"));
        }

        if (jsonPhyLayer.HasMember("epc") && jsonPhyLayer["epc"].IsObject())
        {
            const auto& epc = jsonPhyLayer["epc"];
            if (epc.HasMember("helper") && epc["helper"].IsString())
            {
                auto helperType = TypeId::LookupByName(epc["helper"].GetString());
                if (!helperType.IsChildOf(NrEpcHelper::GetTypeId()))
                {
                    NS_FATAL_ERROR("EPC helper " << epc["helper"].GetString()
                                                 << " is not a valid NrEpcHelper");
                }
                nrConfig->SetEpcHelper(helperType);
            }
            if (epc.HasMember("attributes") && epc["attributes"].IsArray())
            {
                const auto epcAttributes =
                    ModelConfigurationHelper::GetAttributes(nrConfig->GetEpcHelperType(),
                                                            epc["attributes"].GetArray());
                nrConfig->SetEpcAttributes(epcAttributes);
            }
        }

        // Parse scheduler configuration
        if (jsonPhyLayer.HasMember("scheduler") && jsonPhyLayer["scheduler"].IsObject())
        {
            const auto& scheduler = jsonPhyLayer["scheduler"];
            if (scheduler.HasMember("type") && scheduler["type"].IsString())
            {
                nrConfig->SetSchedulerTypeId(TypeId::LookupByName(scheduler["type"].GetString()));
            }
            if (scheduler.HasMember("attributes") && scheduler["attributes"].IsArray())
            {
                const auto schedulerAttrs =
                    ModelConfigurationHelper::GetAttributes(nrConfig->GetSchedulerType(),
                                                            scheduler["attributes"].GetArray());
                nrConfig->SetSchedulerAttributes(schedulerAttrs);
            }
        }

        // Parse error-model configuration
        if (jsonPhyLayer.HasMember("error-model") && jsonPhyLayer["error-model"].IsArray())
        {
            const auto errorModels = jsonPhyLayer["error-model"].GetArray();
            for (rapidjson::SizeType i = 0; i < errorModels.Size(); ++i)
            {
                const auto& errorModel = errorModels[i];
                NS_ASSERT_MSG(errorModel.IsObject(), "Each error-model entry must be an object.");

                std::string direction = "both"; // default

                // Get direction if specified
                if (errorModel.HasMember("direction") && errorModel["direction"].IsString())
                {
                    direction = errorModel["direction"].GetString();
                }

                if (errorModel.HasMember("type") && errorModel["type"].IsString())
                {
                    TypeId errorModelType = TypeId::LookupByName(errorModel["type"].GetString());
                    std::vector<ModelConfiguration::Attribute> errorModelAttrs;

                    if (errorModel.HasMember("amcAttributes") &&
                        errorModel["amcAttributes"].IsArray())
                    {
                        errorModelAttrs = ModelConfigurationHelper::GetAttributes(
                            ns3::NrAmc::GetTypeId(),
                            errorModel["amcAttributes"].GetArray());
                    }

                    // Set based on direction
                    if (direction == "both")
                    {
                        nrConfig->SetDlErrorModelType(errorModelType);
                        nrConfig->SetDlErrorModelAttributes(errorModelAttrs);
                        nrConfig->SetUlErrorModelType(errorModelType);
                        nrConfig->SetUlErrorModelAttributes(errorModelAttrs);
                    }
                    else if (direction == "downlink")
                    {
                        nrConfig->SetDlErrorModelType(errorModelType);
                        nrConfig->SetDlErrorModelAttributes(errorModelAttrs);
                    }
                    else if (direction == "uplink")
                    {
                        nrConfig->SetUlErrorModelType(errorModelType);
                        nrConfig->SetUlErrorModelAttributes(errorModelAttrs);
                    }
                    else
                    {
                        NS_FATAL_ERROR("Invalid error-model direction: "
                                       << direction
                                       << ". Valid values are: both, downlink, uplink");
                    }
                }
            }
        }

        if (jsonPhyLayer.HasMember("gnbBwpManager") && jsonPhyLayer["gnbBwpManager"].IsObject())
        {
            const auto bwpManagerType =
                TypeId::LookupByName(jsonPhyLayer["gnbBwpManager"].HasMember("type") &&
                                             jsonPhyLayer["gnbBwpManager"]["type"].IsString()
                                         ? jsonPhyLayer["gnbBwpManager"]["type"].GetString()
                                         : "ns3::BwpManagerAlgorithmStatic");
            nrConfig->SetGnbBwpManager(bwpManagerType);
            if (jsonPhyLayer["gnbBwpManager"].HasMember("attributes") &&
                jsonPhyLayer["gnbBwpManager"]["attributes"].IsArray())
            {
                const auto gnbBwpManagerAttributes = ModelConfigurationHelper::GetAttributes(
                    bwpManagerType,
                    jsonPhyLayer["gnbBwpManager"]["attributes"].GetArray());
                nrConfig->SetGnbBwpManagerAttributes(gnbBwpManagerAttributes);
            }
        }

        if (jsonPhyLayer.HasMember("ueBwpManager") && jsonPhyLayer["ueBwpManager"].IsObject())
        {
            const auto bwpManagerType =
                TypeId::LookupByName(jsonPhyLayer["ueBwpManager"].HasMember("type") &&
                                             jsonPhyLayer["ueBwpManager"]["type"].IsString()
                                         ? jsonPhyLayer["ueBwpManager"]["type"].GetString()
                                         : "ns3::BwpManagerAlgorithmStatic");
            nrConfig->SetUeBwpManager(bwpManagerType);
            if (jsonPhyLayer["ueBwpManager"].HasMember("attributes") &&
                jsonPhyLayer["ueBwpManager"]["attributes"].IsArray())
            {
                const auto ueBwpManagerAttributes = ModelConfigurationHelper::GetAttributes(
                    bwpManagerType,
                    jsonPhyLayer["ueBwpManager"]["attributes"].GetArray());
                nrConfig->SetUeBwpManagerAttributes(ueBwpManagerAttributes);
            }
        }

        // Parse Handover configuration
        if (jsonPhyLayer.HasMember("handover") && jsonPhyLayer["handover"].IsObject())
        {
            const auto& handover = jsonPhyLayer["handover"];
            TypeId algoType = TypeId::LookupByName("ns3::NrNoOpHandoverAlgorithm"); // Default

            if (handover.HasMember("algorithm") && handover["algorithm"].IsString())
            {
                algoType = TypeId::LookupByName(handover["algorithm"].GetString());
            }

            nrConfig->SetHandoverAlgorithmType(algoType);

            if (handover.HasMember("attributes") && handover["attributes"].IsArray())
            {
                const auto handoverAttributes = ModelConfigurationHelper::GetAttributes(
                    algoType,
                    handover["attributes"].GetArray());
                nrConfig->SetHandoverAlgorithmAttributes(handoverAttributes);
            }
        }

        // Parse Channel Access Manager configuration
        if (jsonPhyLayer.HasMember("channelAccessManager") &&
            jsonPhyLayer["channelAccessManager"].IsObject())
        {
            const auto& cam = jsonPhyLayer["channelAccessManager"];

            // UE
            if (cam.HasMember("ue") && cam["ue"].IsObject())
            {
                const auto& ueCam = cam["ue"];
                TypeId ueCamType = TypeId::LookupByName("ns3::NrAlwaysOnAccessManager"); // Default

                if (ueCam.HasMember("type") && ueCam["type"].IsString())
                {
                    ueCamType = TypeId::LookupByName(ueCam["type"].GetString());
                }

                nrConfig->SetUeChannelAccessManagerType(ueCamType);

                if (ueCam.HasMember("attributes") && ueCam["attributes"].IsArray())
                {
                    const auto ueCamAttributes = ModelConfigurationHelper::GetAttributes(
                        ueCamType,
                        ueCam["attributes"].GetArray());
                    nrConfig->SetUeChannelAccessManagerAttributes(ueCamAttributes);
                }
            }

            // gNB
            if (cam.HasMember("gnb") && cam["gnb"].IsObject())
            {
                const auto& gnbCam = cam["gnb"];
                TypeId gnbCamType = TypeId::LookupByName("ns3::NrAlwaysOnAccessManager"); // Default

                if (gnbCam.HasMember("type") && gnbCam["type"].IsString())
                {
                    gnbCamType = TypeId::LookupByName(gnbCam["type"].GetString());
                }

                nrConfig->SetGnbChannelAccessManagerType(gnbCamType);

                if (gnbCam.HasMember("attributes") && gnbCam["attributes"].IsArray())
                {
                    const auto gnbCamAttributes = ModelConfigurationHelper::GetAttributes(
                        gnbCamType,
                        gnbCam["attributes"].GetArray());
                    nrConfig->SetGnbChannelAccessManagerAttributes(gnbCamAttributes);
                }
            }
        }

        // Parse UE antenna configuration
        if (jsonPhyLayer.HasMember("ueAntenna") && jsonPhyLayer["ueAntenna"].IsObject())
        {
            const auto& ueAntenna = jsonPhyLayer["ueAntenna"];
            std::string antennaType = "ns3::IsotropicAntennaModel";
            if (ueAntenna.HasMember("type") && ueAntenna["type"].IsString())
            {
                antennaType = ueAntenna["type"].GetString();
            }

            std::vector<ModelConfiguration::Attribute> antennaProps;
            std::vector<ModelConfiguration::Attribute> arrayProps;

            if (ueAntenna.HasMember("properties") && ueAntenna["properties"].IsArray())
            {
                antennaProps =
                    ModelConfigurationHelper::GetAttributes(TypeId::LookupByName(antennaType),
                                                            ueAntenna["properties"].GetArray());
            }

            if (ueAntenna.HasMember("arrayProperties") && ueAntenna["arrayProperties"].IsArray())
            {
                arrayProps = ModelConfigurationHelper::GetAttributes(
                    TypeId::LookupByName("ns3::UniformPlanarArray"),
                    ueAntenna["arrayProperties"].GetArray());
            }

            nrConfig->SetUeAntenna(NrAntennaConfiguration{
                .type = antennaType,
                .properties = antennaProps,
                .arrayProperties = arrayProps,
            });
        }

        if (jsonPhyLayer.HasMember("uePhyAttributes") && jsonPhyLayer["uePhyAttributes"].IsArray())
        {
            const auto uePhyAttributes =
                ModelConfigurationHelper::GetAttributes(TypeId::LookupByName("ns3::NrUePhy"),
                                                        jsonPhyLayer["uePhyAttributes"].GetArray());
            nrConfig->SetUePhyAttributes(uePhyAttributes);
        }

        // Parse gNB antenna configuration
        if (jsonPhyLayer.HasMember("gnbAntenna") && jsonPhyLayer["gnbAntenna"].IsObject())
        {
            const auto& gnbAntenna = jsonPhyLayer["gnbAntenna"];
            std::string antennaType = "ns3::IsotropicAntennaModel";
            if (gnbAntenna.HasMember("type") && gnbAntenna["type"].IsString())
            {
                antennaType = gnbAntenna["type"].GetString();
            }

            std::vector<ModelConfiguration::Attribute> antennaProps;
            std::vector<ModelConfiguration::Attribute> arrayProps;

            if (gnbAntenna.HasMember("properties") && gnbAntenna["properties"].IsArray())
            {
                antennaProps =
                    ModelConfigurationHelper::GetAttributes(TypeId::LookupByName(antennaType),
                                                            gnbAntenna["properties"].GetArray());
            }

            if (gnbAntenna.HasMember("arrayProperties") && gnbAntenna["arrayProperties"].IsArray())
            {
                arrayProps = ModelConfigurationHelper::GetAttributes(
                    TypeId::LookupByName("ns3::UniformPlanarArray"),
                    gnbAntenna["arrayProperties"].GetArray());
            }

            nrConfig->SetGnbAntenna(NrAntennaConfiguration{
                .type = antennaType,
                .properties = antennaProps,
                .arrayProperties = arrayProps,
            });
        }

        if (jsonPhyLayer.HasMember("gnbPhyAttributes") &&
            jsonPhyLayer["gnbPhyAttributes"].IsArray())
        {
            const auto gnbPhyAttributes = ModelConfigurationHelper::GetAttributes(
                TypeId::LookupByName("ns3::NrGnbPhy"),
                jsonPhyLayer["gnbPhyAttributes"].GetArray());
            nrConfig->SetGnbPhyAttributes(gnbPhyAttributes);
        }

        if (jsonPhyLayer.HasMember("bands") && jsonPhyLayer["bands"].IsArray())
        {
            if (!jsonPhyLayer.HasMember("bands") || !jsonPhyLayer["bands"].IsArray() ||
                jsonPhyLayer["bands"].GetArray().Size() == 0)
            {
                NS_FATAL_ERROR(
                    "NR PHY Layer definition must have at least one band in 'bands' property.");
            }
            const auto bands = jsonPhyLayer["bands"].GetArray();
            for (rapidjson::SizeType i = 0; i < bands.Size(); ++i)
            {
                const auto& band = bands[i];
                NS_ASSERT_MSG(band.IsObject(), "NR band must be an object.");

                NrBandConfiguration bandConfig;
                bandConfig.contiguousCc =
                    band.HasMember("contiguousCc") && band["contiguousCc"].IsBool()
                        ? band["contiguousCc"].GetBool()
                        : true;
                bandConfig.channel.scenario =
                    band.HasMember("scenario") && band["scenario"].IsString()
                        ? band["scenario"].GetString()
                        : "UMi-StreetCanyon";
                bandConfig.channel.conditionModel =
                    band.HasMember("conditionModel") && band["conditionModel"].IsString()
                        ? band["conditionModel"].GetString()
                        : "Default";
                bandConfig.channel.propagationModel =
                    band.HasMember("propagationModel") && band["propagationModel"].IsString()
                        ? band["propagationModel"].GetString()
                        : "ThreeGpp";
                bandConfig.channel.configFlags =
                    band.HasMember("configFlags") && band["configFlags"].IsUint()
                        ? static_cast<uint8_t>(band["configFlags"].GetUint())
                        : (NrChannelHelper::INIT_PROPAGATION | NrChannelHelper::INIT_FADING);

                // We should know what is the typeid of the channel and pathloss model to set
                // and check the attributes
                auto nrChannel = CreateObject<NrChannelHelper>();
                nrChannel->ConfigureFactories(bandConfig.channel.scenario,
                                              bandConfig.channel.conditionModel,
                                              bandConfig.channel.propagationModel);

                // Needed to access private members of NrChannelHelper
                class NrChannelHelperQuirk : public Object
                {
                  public:
                    enum class Scenario;
                    enum class ChannelModel;
                    enum class Condition;
                    Scenario m_scenario;
                    ChannelModel m_channelModel;
                    Condition m_condition;
                    std::set<std::tuple<ChannelModel, Scenario>> m_supportedCombinations;
                    ObjectFactory m_pathLossModel;
                    ObjectFactory m_spectrumModel;
                    ObjectFactory m_channelConditionModel;
                };

                auto nrChannelQ = reinterpret_cast<NrChannelHelperQuirk*>(&(*nrChannel));
                auto propagationLossTypeId = nrChannelQ->m_pathLossModel.GetTypeId();
                auto phasedSpectrumTypeId = nrChannelQ->m_spectrumModel.GetTypeId();
                auto channelConditionTypeId = nrChannelQ->m_channelConditionModel.GetTypeId();

                if (band.HasMember("channelConditionAttributes") &&
                    band["channelConditionAttributes"].IsArray())
                {
                    const auto channelConditionAttributes = ModelConfigurationHelper::GetAttributes(
                        channelConditionTypeId,
                        band["channelConditionAttributes"].GetArray());
                    bandConfig.channelConditionAttributes = channelConditionAttributes;
                }

                if (band.HasMember("pathlossAttributes") && band["pathlossAttributes"].IsArray())
                {
                    const auto pathlossAttributes = ModelConfigurationHelper::GetAttributes(
                        propagationLossTypeId,
                        band["pathlossAttributes"].GetArray());
                    bandConfig.pathlossAttributes = pathlossAttributes;
                }

                if (band.HasMember("phasedSpectrumAttributes") &&
                    band["phasedSpectrumAttributes"].IsArray())
                {
                    const auto phasedSpectrumAttributes = ModelConfigurationHelper::GetAttributes(
                        phasedSpectrumTypeId,
                        band["phasedSpectrumAttributes"].GetArray());
                    bandConfig.phasedSpectrumAttributes = phasedSpectrumAttributes;
                }

                for (rapidjson::SizeType j = 0; j < band["frequencyBands"].GetArray().Size(); ++j)
                {
                    const auto& freqBand = band["frequencyBands"].GetArray()[j];
                    NS_ASSERT_MSG(freqBand.IsObject(), "NR frequency band must be an object.");

                    NrFrequencyBand freqBandConfig;
                    freqBandConfig.centralFrequency =
                        freqBand.HasMember("centralFrequency") &&
                                freqBand["centralFrequency"].IsNumber()
                            ? freqBand["centralFrequency"].GetDouble()
                            : 28e9;
                    freqBandConfig.bandwidth =
                        freqBand.HasMember("bandwidth") && freqBand["bandwidth"].IsNumber()
                            ? freqBand["bandwidth"].GetDouble()
                            : 100e6;
                    freqBandConfig.numComponentCarriers =
                        freqBand.HasMember("numComponentCarriers") &&
                                freqBand["numComponentCarriers"].IsUint()
                            ? static_cast<uint8_t>(freqBand["numComponentCarriers"].GetUint())
                            : 1;
                    freqBandConfig.numBandwidthParts =
                        freqBand.HasMember("numBandwidthParts") &&
                                freqBand["numBandwidthParts"].IsUint()
                            ? static_cast<uint8_t>(freqBand["numBandwidthParts"].GetUint())
                            : 1;

                    bandConfig.frequencyBands.push_back(freqBandConfig);
                }
                nrConfig->AddBandConfiguration(bandConfig);
            }
        }

        phyConfig = nrConfig;
    }
    else
    {
        NS_FATAL_ERROR("PHY Layer of Type " << phyType << " is not supported!");
    }

    return phyConfig;
}

} // namespace ns3
