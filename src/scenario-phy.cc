#include "scenario.h"

#include <filesystem>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ScenarioPhy");

void
Scenario::ConfigurePhy()
{
    NS_LOG_FUNCTION_NOARGS();

    const auto phyLayerConfs = CONFIGURATOR->GetPhyLayers();

    size_t phyId = 0;
    for (auto& phyLayerConf : phyLayerConfs)
    {
        if (phyLayerConf->GetType() == "wifi")
        {
            YansWifiChannelHelper wifiChannel;
            const auto wifiConf =
                StaticCast<WifiPhyLayerConfiguration, PhyLayerConfiguration>(phyLayerConf);
            const auto wifiSim = CreateObject<WifiPhySimulationHelper>();
            YansWifiPhyHelper* wifiPhy = wifiSim->GetWifiPhyHelper();

            wifiSim->GetWifiHelper()->SetStandard(wifiConf->GetStandard());

            for (auto& attr : wifiConf->GetAttributes())
            {
                wifiPhy->Set(attr.name, *attr.value);
            }

            // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
            wifiPhy->SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

            WifiPhyFactoryHelper::SetPropagationDelay(wifiChannel,
                                                      wifiConf->GetChannelPropagationDelayModel());
            WifiPhyFactoryHelper::AddPropagationLoss(wifiChannel,
                                                     wifiConf->GetChannelPropagationLossModel());

            wifiPhy->SetChannel(wifiChannel.Create());

            m_protocolStacks[PHY_LAYER].push_back(wifiSim);
        }
        else if (phyLayerConf->GetType() == "lte")
        {
            auto lteSim = CreateObject<LtePhySimulationHelper>(phyId);
            auto lteConf =
                StaticCast<LtePhyLayerConfiguration, PhyLayerConfiguration>(phyLayerConf);
            auto lteHelper = lteSim->GetLteHelper();

            auto pathlossConf = lteConf->GetChannelPropagationLossModel();
            if (pathlossConf)
            {
                lteHelper->SetAttribute("PathlossModel", StringValue(pathlossConf->GetName()));
                for (auto& attr : pathlossConf->GetAttributes())
                {
                    lteHelper->SetPathlossModelAttribute(attr.name, *attr.value);
                }
            }

            auto spectrumConf = lteConf->GetChannelSpectrumModel();
            lteHelper->SetSpectrumChannelType(spectrumConf.GetName());
            for (auto& attr : spectrumConf.GetAttributes())
            {
                lteHelper->SetSpectrumChannelAttribute(attr.name, *attr.value);
            }

            for (auto& attr : lteConf->GetAttributes())
            {
                lteHelper->SetAttribute(attr.name, *attr.value);
            }

            lteHelper->Initialize();

            m_backbone.Add(lteSim->GetEpcHelper()->GetPgwNode());

            m_protocolStacks[PHY_LAYER].push_back(lteSim);
        }
        else if (phyLayerConf->GetType() == "none")
        {
            const auto conf =
                StaticCast<NonePhyLayerConfiguration, PhyLayerConfiguration>(phyLayerConf);
        }
        else if (phyLayerConf->GetType() == "3GPP")
        {
            const auto conf =
                StaticCast<ThreeGppPhyLayerConfiguration, PhyLayerConfiguration>(phyLayerConf);

            auto channelCond = [&conf]() -> Ptr<ThreeGppChannelConditionModel> {
                ObjectFactory factory;

                factory.SetTypeId(conf->GetConditionModel().GetName());
                for (auto& attr : conf->GetConditionModel().GetAttributes())
                {
                    factory.Set(attr.name, *attr.value);
                }

                return factory.Create<ThreeGppChannelConditionModel>();
            }();
            auto propagationLoss = [&conf, &channelCond]() -> Ptr<ThreeGppPropagationLossModel> {
                ObjectFactory factory;

                factory.SetTypeId(conf->GetPropagationLossModel().GetName());
                for (auto& attr : conf->GetPropagationLossModel().GetAttributes())
                {
                    factory.Set(attr.name, *attr.value);
                }

                auto model = factory.Create<ThreeGppPropagationLossModel>();
                model->SetChannelConditionModel(channelCond);

                return model;
            }();
            auto spectrumLoss = [&conf,
                                 &channelCond,
                                 &propagationLoss]() -> Ptr<ThreeGppSpectrumPropagationLossModel> {
                auto model = CreateObject<ThreeGppSpectrumPropagationLossModel>();
                model->SetChannelModelAttribute("ChannelConditionModel", PointerValue(channelCond));

                // propagate PropagationLoss Frequency to SpectrumLoss ChannelModel to ensure
                // property alignment
                model->SetChannelModelAttribute("Frequency",
                                                DoubleValue(propagationLoss->GetFrequency()));
                model->SetChannelModelAttribute("Scenario", StringValue(conf->GetEnvironment()));

                for (auto& attr : conf->GetAttributes())
                {
                    model->SetChannelModelAttribute(attr.name, *attr.value);
                }

                return model;
            }();

            auto simHelper = CreateObject<ThreeGppPhySimulationHelper>(phyId,
                                                                       channelCond,
                                                                       propagationLoss,
                                                                       spectrumLoss);
            m_protocolStacks[PHY_LAYER].push_back(simHelper);
        }
        else if (phyLayerConf->GetType() == "nr")
        {
            auto nrSim = CreateObject<NrPhySimulationHelper>(phyId);
            auto nrConf = StaticCast<NrPhyLayerConfiguration, PhyLayerConfiguration>(phyLayerConf);

            // Set Nr Helpers
            for (auto& attr : nrConf->GetAttributes())
            {
                nrSim->GetNrHelper()->SetAttribute(attr.name, *attr.value);
            }

            nrSim->SetEpcHelper(nrConf->GetEpcHelperType(),
                                nrConf->GetEpcAttributes(),
                                nrConf->GetEnablePcap());
            nrSim->SetBeamformingHelper(nrConf->GetBeamformingHelperType(),
                                        nrConf->GetBeamformingAttributes());
            nrSim->SetBeamformingMethod(nrConf->GetBeamformingMethod(),
                                        nrConf->GetBeamformingAlgorithmAttributes());

            // Setup configs on helpers
            nrSim->GetNrHelper()->SetUeBwpManagerAlgorithmTypeId(nrConf->GetUeBwpManagerType());
            for (auto& attr : nrConf->GetUeBwpManagerAttributes())
            {
                nrSim->GetNrHelper()->SetUeBwpManagerAlgorithmAttribute(attr.name, *attr.value);
            }

            nrSim->GetNrHelper()->SetGnbBwpManagerAlgorithmTypeId(nrConf->GetGnbBwpManagerType());
            for (auto& attr : nrConf->GetGnbBwpManagerAttributes())
            {
                nrSim->GetNrHelper()->SetGnbBwpManagerAlgorithmAttribute(attr.name, *attr.value);
            }

            // Handover
            if (nrConf->GetHandoverAlgorithmType().GetUid() != 0)
            {
                nrSim->GetNrHelper()->SetHandoverAlgorithmType(
                    nrConf->GetHandoverAlgorithmType().GetName());
                for (auto& attr : nrConf->GetHandoverAlgorithmAttributes())
                {
                    nrSim->GetNrHelper()->SetHandoverAlgorithmAttribute(attr.name, *attr.value);
                }
            }

            // UE Channel Access Manager
            if (nrConf->GetUeChannelAccessManagerType().GetUid() != 0)
            {
                nrSim->GetNrHelper()->SetUeChannelAccessManagerTypeId(
                    nrConf->GetUeChannelAccessManagerType());
                for (auto& attr : nrConf->GetUeChannelAccessManagerAttributes())
                {
                    nrSim->GetNrHelper()->SetUeChannelAccessManagerAttribute(attr.name,
                                                                             *attr.value);
                }
            }

            // gNB Channel Access Manager
            if (nrConf->GetGnbChannelAccessManagerType().GetUid() != 0)
            {
                nrSim->GetNrHelper()->SetGnbChannelAccessManagerTypeId(
                    nrConf->GetGnbChannelAccessManagerType());
                for (auto& attr : nrConf->GetGnbChannelAccessManagerAttributes())
                {
                    nrSim->GetNrHelper()->SetGnbChannelAccessManagerAttribute(attr.name,
                                                                              *attr.value);
                }
            }

            for (const auto& bandConf : nrConf->GetBandsConfiguration())
            {
                std::vector<NrPhySimulationHelper::ChannelOperationBandConf> channelOpBands;

                for (auto& opBand : bandConf.bands)
                {
                    NrPhySimulationHelper::ChannelOperationBandConf cobc;
                    cobc.contiguous = (opBand.type == NrOperationBand::CONTIGUOUS);

                    for (auto& attr : opBand.carriers)
                    {
                        auto conf =
                            CcBwpCreator::SimpleOperationBandConf(attr.centralFrequency,
                                                                  attr.bandwidth,
                                                                  attr.numComponentCarriers);
                        conf.m_numBwp = attr.numBandwidthParts;
                        cobc.carrierConfs.push_back(conf);
                    }
                    channelOpBands.push_back(cobc);
                }

                nrSim->CreateChannel(channelOpBands,
                                     bandConf.channel.scenario,
                                     bandConf.channel.conditionModel,
                                     bandConf.channel.propagationModel,
                                     bandConf.channelConditionAttributes,
                                     bandConf.pathlossAttributes,
                                     bandConf.phasedSpectrumAttributes,
                                     bandConf.channel.configFlags);
            }

            nrSim->SetScheduler(nrConf->GetSchedulerType(), nrConf->GetSchedulerAttributes());

            // Configure DL error model if specified
            if (nrConf->GetDlErrorModelType().GetUid() != 0)
            {
                nrSim->SetDlErrorModel(nrConf->GetDlErrorModelType(),
                                       nrConf->GetDlErrorModelAttributes());
            }

            // Configure UL error model if specified
            if (nrConf->GetUlErrorModelType().GetUid() != 0)
            {
                nrSim->SetUlErrorModel(nrConf->GetUlErrorModelType(),
                                       nrConf->GetUlErrorModelAttributes());
            }

            nrSim->SetGnbPhyAttributes(nrConf->GetGnbPhyAttributes());
            nrSim->SetUePhyAttributes(nrConf->GetUePhyAttributes());

            m_backbone.Add(nrSim->GetNrEpcHelper()->GetPgwNode());
            m_protocolStacks[PHY_LAYER].push_back(nrSim);
        }
        else
        {
            NS_FATAL_ERROR("Unsupported PHY Layer Type: " << phyLayerConf->GetType());
        }

        phyId++;
    }
}

void
Scenario::ConfigureLteEnb(Ptr<Node> entityNode,
                          const uint32_t netId,
                          const std::vector<AntennaModelConfiguration> antennaModels,
                          const std::optional<ModelConfiguration> phyConf)
{
    // !NOTICE: no checks are made for backbone/netid combination that do not represent an LTE
    // backbone!
    static std::vector<NodeContainer> backbonePerStack(m_protocolStacks[PHY_LAYER].size());
    auto ltePhy = StaticCast<LtePhySimulationHelper, Object>(m_protocolStacks[PHY_LAYER][netId]);
    auto lteHelper = ltePhy->GetLteHelper();

    if (!antennaModels.empty())
    {
        ltePhy->GetLteHelper()->SetEnbAntennaModelType(antennaModels.front().model.GetName());
        for (auto& attr : antennaModels.front().model.GetAttributes())
        {
            ltePhy->GetLteHelper()->SetEnbAntennaModelAttribute(attr.name, *attr.value);
        }
    }

    auto dev = StaticCast<LteEnbNetDevice, NetDevice>(
        LteSetupHelper::InstallSingleEnbDevice(lteHelper, entityNode));

    if (phyConf)
    {
        for (const auto& attr : phyConf->GetAttributes())
        {
            dev->GetPhy()->SetAttribute(attr.name, *attr.value);
        }
    }

    for (NodeContainer::Iterator eNB = backbonePerStack[netId].Begin();
         eNB != backbonePerStack[netId].End();
         eNB++)
    {
        ltePhy->GetLteHelper()->AddX2Interface(entityNode, *eNB);
    }
    backbonePerStack[netId].Add(entityNode);
}

void
Scenario::ConfigureLteUe(Ptr<Node> entityNode,
                         const std::vector<LteBearerConfiguration> bearers,
                         const uint32_t netId,
                         const std::vector<AntennaModelConfiguration> antennaModels,
                         const std::optional<ModelConfiguration> phyConf)
{
    // NOTICE: no checks are made for ue/netid combination that do not represent an LTE backbone!
    static std::vector<NodeContainer> uePerStack(m_protocolStacks[PHY_LAYER].size());
    auto ltePhy = StaticCast<LtePhySimulationHelper, Object>(m_protocolStacks[PHY_LAYER][netId]);
    auto lteHelper = ltePhy->GetLteHelper();
    Ipv4StaticRoutingHelper routingHelper;

    if (!antennaModels.empty())
    {
        ltePhy->GetLteHelper()->SetUeAntennaModelType(antennaModels.front().model.GetName());
        for (auto& attr : antennaModels.front().model.GetAttributes())
        {
            ltePhy->GetLteHelper()->SetUeAntennaModelAttribute(attr.name, *attr.value);
        }
    }

    auto dev = StaticCast<LteUeNetDevice, NetDevice>(
        LteSetupHelper::InstallSingleUeDevice(lteHelper, entityNode));

    if (phyConf)
    {
        for (const auto& attr : phyConf->GetAttributes())
        {
            dev->GetPhy()->SetAttribute(attr.name, *attr.value);
        }
    }

    // Install network layer in order to proceed with IPv4 LTE configuration
    InstallEntityIpv4(entityNode, dev, netId);
    // Register UEs into network 7.0.0.0/8
    // unfortunately this is hardwired into EpcHelper implementation

    auto assignedIpAddrs = ltePhy->GetEpcHelper()->AssignUeIpv4Address(NetDeviceContainer(dev));
    for (auto assignedIpIter = assignedIpAddrs.Begin(); assignedIpIter != assignedIpAddrs.End();
         assignedIpIter++)
    {
        NS_LOG_LOGIC("Assigned IPv4 Address to UE with Node ID "
                     << entityNode->GetId() << ":" << " Iface " << assignedIpIter->second);

        for (uint32_t i = 0; i < assignedIpIter->first->GetNAddresses(assignedIpIter->second); i++)
        {
            NS_LOG_LOGIC(" Addr " << assignedIpIter->first->GetAddress(assignedIpIter->second, i));
        }
    }

    // create a static route for each UE to the SGW/PGW in order to communicate
    // with the internet
    auto nodeIpv4 = entityNode->GetObject<Ipv4>();
    Ptr<Ipv4StaticRouting> ueStaticRoute = routingHelper.GetStaticRouting(nodeIpv4);
    ueStaticRoute->SetDefaultRoute(ltePhy->GetEpcHelper()->GetUeDefaultGatewayAddress(),
                                   nodeIpv4->GetInterfaceForDevice(dev));
    // auto attach each drone UE to the eNB with the strongest signal
    ltePhy->GetLteHelper()->Attach(dev);
    // init bearers on UE
    for (auto& bearerConf : bearers)
    {
        EpsBearer bearer(bearerConf.GetType(), bearerConf.GetQos());
        ltePhy->GetLteHelper()->ActivateDedicatedEpsBearer(dev, bearer, EpcTft::Default());
    }
}

void
Scenario::ConfigureNrGnb(Ptr<Node> entityNode,
                         const uint32_t netId,
                         const std::vector<AntennaModelConfiguration> antennaModels,
                         const std::vector<ns3::NrPhyProperty> phyConf,
                         const std::vector<ns3::NrPhyProperty> rrcConf,
                         const std::vector<OutputLinkConfiguration> outputLinks,
                         const std::vector<X2NeighborConfiguration> x2Neighbors,
                         const uint32_t channelId,
                         const std::vector<uint32_t> channelBands)
{
    static std::vector<NodeContainer> backbonePerStack(m_protocolStacks[PHY_LAYER].size());
    auto nrPhy = StaticCast<NrPhySimulationHelper, Object>(m_protocolStacks[PHY_LAYER][netId]);
    auto nrPhyConf = StaticCast<NrPhyLayerConfiguration, PhyLayerConfiguration>(
        CONFIGURATOR->GetPhyLayers()[netId]);
    auto nrHelper = nrPhy->GetNrHelper();

    nrPhy->ResetGnbAntenna();
    std::vector<AntennaModelConfiguration> effectiveAntennas = antennaModels;
    if (effectiveAntennas.empty())
    {
        for (const auto& antConf : nrPhyConf->GetGnbAntenna())
        {
            AntennaModelConfiguration parsedConf;
            parsedConf.bwpId = antConf.bwpId;

            ObjectFactory antennaElementFactory;
            antennaElementFactory.SetTypeId(TypeId::LookupByName(antConf.type));
            for (const auto& prop : antConf.properties)
            {
                antennaElementFactory.Set(prop.name, *prop.value);
            }

            auto arrayProps = antConf.arrayProperties;
            arrayProps.emplace_back("AntennaElement",
                                    Create<PointerValue>(antennaElementFactory.Create()));

            parsedConf.model = ModelConfiguration("ns3::UniformPlanarArray", arrayProps);
            effectiveAntennas.push_back(parsedConf);
        }
    }

    if (!effectiveAntennas.empty())
    {
        // Setup factory with the first one to ensure the correct class is instantiated
        auto firstModel = effectiveAntennas.front().model;
        nrHelper->SetGnbAntennaTypeId(firstModel.GetName());
        // For backwards compatibility or default behavior, configure it entirely on the factory if
        // only 1 model
        if (effectiveAntennas.size() == 1 && !effectiveAntennas.front().bwpId.has_value())
        {
            for (auto& attr : firstModel.GetAttributes())
            {
                nrHelper->SetGnbAntennaAttribute(attr.name, *attr.value);
            }
        }
    }
    auto entityNodeContainer = NodeContainer(entityNode);
    auto bwps = nrPhy->GetBwps(channelId, channelBands);
    auto dev = StaticCast<NrGnbNetDevice, NetDevice>(
        nrPhy->InstallGnbDevices(entityNodeContainer, bwps).Get(0));

    for (const auto& attr : phyConf)
    {
        auto bwpLen = NrHelper::GetNumberBwp(dev);
        if (attr.bwpId.has_value())
        {
            if (attr.bwpId.value() >= bwpLen)
            {
                NS_FATAL_ERROR("BWP ID " << attr.bwpId.value() << " is out of range");
            }
            NrHelper::GetGnbPhy(dev, attr.bwpId.value())
                ->SetAttribute(attr.attribute.name, *attr.attribute.value);
        }
        else
        {
            for (size_t i = 0; i < bwpLen; i++)
            {
                NrHelper::GetGnbPhy(dev, i)->SetAttribute(attr.attribute.name,
                                                          *attr.attribute.value);
            }
        }
    }

    if (effectiveAntennas.size() > 1 ||
        (!effectiveAntennas.empty() && effectiveAntennas.front().bwpId.has_value()))
    {
        auto bwpLen = NrHelper::GetNumberBwp(dev);
        for (const auto& antConf : effectiveAntennas)
        {
            uint32_t startBwp = 0;
            uint32_t endBwp = bwpLen;
            if (antConf.bwpId.has_value())
            {
                if (antConf.bwpId.value() < bwpLen)
                {
                    startBwp = antConf.bwpId.value();
                    endBwp = startBwp + 1;
                }
                else
                {
                    NS_LOG_WARN("Skipping out-of-bounds bwpId " << antConf.bwpId.value()
                                                                << " for antenna component");
                    continue;
                }
            }

            for (uint32_t i = startBwp; i < endBwp; ++i)
            {
                auto phy = dev->GetPhy(i);
                if (phy)
                {
                    auto specPhy = phy->GetSpectrumPhy();
                    if (specPhy)
                    {
                        auto antenna = specPhy->GetAntenna();
                        if (antenna)
                        {
                            for (auto& attr : antConf.model.GetAttributes())
                            {
                                antenna->SetAttribute(attr.name, *attr.value);
                            }
                        }
                    }
                }
            }
        }
    }

    // Configure RRC attributes
    for (const auto& attr : rrcConf)
    {
        dev->GetRrc()->SetAttribute(attr.attribute.name, *attr.attribute.value);
    }

    // Configure Output Links
    for (const auto& link : outputLinks)
    {
        NrHelper::GetBwpManagerGnb(dev)->SetOutputLink(link.sourceBwp, link.targetBwp);
    }

    // Configure X2 Neighbors
    for (const auto& neighbor : x2Neighbors)
    {
        Ptr<Node> targetNode = GetNodeByKey(neighbor.key, neighbor.index);
        if (targetNode)
        {
            nrPhy->GetNrHelper()->AddX2Interface(entityNode, targetNode);
        }
        else
        {
            NS_LOG_WARN("Could not find target node for X2 link: " << neighbor.key << " index "
                                                                   << neighbor.index);
        }
    }

    backbonePerStack[netId].Add(entityNode);

    // Store gNB device for later attachment operations
    NetDeviceContainer gnbDevContainer(dev);
    m_nrGnbDevices[netId].push_back(gnbDevContainer);
}

void
Scenario::ConfigureNrUe(Ptr<Node> entityNode,
                        const std::vector<NrQosFlowConfiguration> qosFlows,
                        const uint32_t netId,
                        const std::vector<AntennaModelConfiguration> antennaModels,
                        const std::vector<ns3::NrPhyProperty> phyConf,
                        const std::vector<ns3::NrPhyProperty> rrcConf,
                        const std::vector<OutputLinkConfiguration> outputLinks,
                        const uint32_t channelId,
                        const std::vector<uint32_t> channelBands)
{
    static std::vector<NodeContainer> uePerStack(m_protocolStacks[PHY_LAYER].size());
    auto nrPhy = StaticCast<NrPhySimulationHelper, Object>(m_protocolStacks[PHY_LAYER][netId]);
    auto nrPhyConf = StaticCast<NrPhyLayerConfiguration, PhyLayerConfiguration>(
        CONFIGURATOR->GetPhyLayers()[netId]);
    auto nrHelper = nrPhy->GetNrHelper();

    nrPhy->ResetUeAntenna();
    std::vector<AntennaModelConfiguration> effectiveAntennas = antennaModels;
    if (effectiveAntennas.empty())
    {
        for (const auto& antConf : nrPhyConf->GetUeAntenna())
        {
            AntennaModelConfiguration parsedConf;
            parsedConf.bwpId = antConf.bwpId;

            ObjectFactory antennaElementFactory;
            antennaElementFactory.SetTypeId(TypeId::LookupByName(antConf.type));
            for (const auto& prop : antConf.properties)
            {
                antennaElementFactory.Set(prop.name, *prop.value);
            }

            auto arrayProps = antConf.arrayProperties;
            arrayProps.emplace_back("AntennaElement",
                                    Create<PointerValue>(antennaElementFactory.Create()));

            parsedConf.model = ModelConfiguration("ns3::UniformPlanarArray", arrayProps);
            effectiveAntennas.push_back(parsedConf);
        }
    }

    if (!effectiveAntennas.empty())
    {
        auto firstModel = effectiveAntennas.front().model;
        nrHelper->SetUeAntennaTypeId(firstModel.GetName());
        if (effectiveAntennas.size() == 1 && !effectiveAntennas.front().bwpId.has_value())
        {
            for (auto& attr : firstModel.GetAttributes())
            {
                nrHelper->SetUeAntennaAttribute(attr.name, *attr.value);
            }
        }
    }

    Ipv4StaticRoutingHelper routingHelper;
    auto entityNodeContainer = NodeContainer(entityNode);
    auto bwps = nrPhy->GetBwps(channelId, channelBands);
    auto dev = StaticCast<NrUeNetDevice, NetDevice>(
        nrPhy->InstallUeDevices(entityNodeContainer, bwps).Get(0));

    // Install network layer in order to proceed with IPv4 configuration
    InstallEntityIpv4(entityNode, dev, netId);

    // Register UEs into network 7.0.0.0/8
    // unfortunately this is hardwired into EpcHelper implementation
    auto assignedIpAddrs = nrPhy->GetNrEpcHelper()->AssignUeIpv4Address(NetDeviceContainer(dev));

    for (auto assignedIpIter = assignedIpAddrs.Begin(); assignedIpIter != assignedIpAddrs.End();
         assignedIpIter++)
    {
        NS_LOG_LOGIC("Assigned IPv4 Address to UE with Node ID "
                     << entityNode->GetId() << ":" << " Iface " << assignedIpIter->second);

        for (uint32_t i = 0; i < assignedIpIter->first->GetNAddresses(assignedIpIter->second); i++)
        {
            NS_LOG_LOGIC(" Addr " << assignedIpIter->first->GetAddress(assignedIpIter->second, i));
        }
    }

    for (const auto& attr : phyConf)
    {
        auto bwpLen = NrHelper::GetNumberBwp(dev);
        if (attr.bwpId.has_value())
        {
            if (attr.bwpId.value() >= bwpLen)
            {
                NS_FATAL_ERROR("BWP ID " << attr.bwpId.value() << " is out of range");
            }
            NrHelper::GetUePhy(dev, attr.bwpId.value())
                ->SetAttribute(attr.attribute.name, *attr.attribute.value);
        }
        else
        {
            for (size_t i = 0; i < bwpLen; i++)
            {
                NrHelper::GetUePhy(dev, i)->SetAttribute(attr.attribute.name,
                                                         *attr.attribute.value);
            }
        }
    }

    auto bwpLen = NrHelper::GetNumberBwp(dev);
    for (const auto& antConf : effectiveAntennas)
    {
        uint32_t startBwp = 0;
        uint32_t endBwp = bwpLen;
        if (antConf.bwpId.has_value())
        {
            if (antConf.bwpId.value() < bwpLen)
            {
                startBwp = antConf.bwpId.value();
                endBwp = startBwp + 1;
            }
            else
            {
                NS_LOG_WARN("Skipping out-of-bounds bwpId " << antConf.bwpId.value()
                                                            << " for UE antenna component");
                continue;
            }
        }

        for (uint32_t i = startBwp; i < endBwp; ++i)
        {
            auto phy = dev->GetPhy(i);
            if (phy)
            {
                auto specPhy = phy->GetSpectrumPhy();
                if (specPhy)
                {
                    auto antenna = specPhy->GetAntenna();
                    if (antenna)
                    {
                        for (auto& attr : antConf.model.GetAttributes())
                        {
                            antenna->SetAttribute(attr.name, *attr.value);
                        }
                    }
                }
            }
        }
    }

    // Configure RRC attributes
    for (const auto& attr : rrcConf)
    {
        dev->GetRrc()->SetAttribute(attr.attribute.name, *attr.attribute.value);
    }

    // Configure Output Links
    for (const auto& link : outputLinks)
    {
        NrHelper::GetBwpManagerUe(dev)->SetOutputLink(link.sourceBwp, link.targetBwp);
    }

    // create a static route for each UE to the SGW/PGW in order to communicate
    // with the internet
    auto nodeIpv4 = entityNode->GetObject<Ipv4>();
    Ptr<Ipv4StaticRouting> ueStaticRoute = routingHelper.GetStaticRouting(nodeIpv4);
    ueStaticRoute->SetDefaultRoute(nrPhy->GetNrEpcHelper()->GetUeDefaultGatewayAddress(),
                                   nodeIpv4->GetInterfaceForDevice(dev));

    // Store UE device for later attachment operations
    m_nrUeDevices[netId].emplace_back(dev);

    // init QoS flows on UE
    for (auto& qosFlowConf : qosFlows)
    {
        NrQosFlow flow(qosFlowConf.GetType(), qosFlowConf.GetQos());
        nrPhy->GetNrHelper()->ActivateDedicatedQosFlow(dev, flow, NrQosRule::Default());
    }
}

void
Scenario::EnablePhyLteTraces()
{
    NS_LOG_FUNCTION_NOARGS();
    if (!CONFIGURATOR->GetLogOnFile())
    {
        return;
    }

    for (size_t phyId = 0; phyId < m_protocolStacks[PHY_LAYER].size(); phyId++)
    {
        auto obj = m_protocolStacks[PHY_LAYER][phyId];

        if (DynamicCast<LtePhySimulationHelper>(obj))
        {
            /* LteHelperQuirk:
             *  This class is an hack to allow access to private members of LteHelper class,
             *  in particular to the StatsCalculators in order to set their output path.
             *  A structurally identical class is defined with all attributes set to public,
             *  then with a reinterpret_cast we interpret the LteHelper as this new class
             *  so the compiler won't complain about accessing its private members.
             */
            class LteHelperQuirk : public Object
            {
              public:
                Ptr<SpectrumChannel> dlC, ulC;
                Ptr<Object> dlPlM, ulPlM;
                ObjectFactory a, b, c, d, e, f, g, h, i, j, k;
                std::string fMT;
                ObjectFactory fMF;
                Ptr<SpectrumPropagationLossModel> fM;
                bool fSA;
                Ptr<PhyStatsCalculator> phyStat;
                Ptr<PhyTxStatsCalculator> phyTxStat;
                Ptr<PhyRxStatsCalculator> phyRxStat;
                Ptr<MacStatsCalculator> macStat;
                Ptr<RadioBearerStatsCalculator> rlcStat;
                Ptr<RadioBearerStatsCalculator> pdcpStat;
                RadioBearerStatsConnector radioBearerStatsConnector;
                Ptr<EpcHelper> m_epcHelper;
                uint64_t m_imsiCounter;
                uint16_t m_cellIdCounter;
                bool l, m, n, o;
                std::map<uint8_t, ComponentCarrier> m_componentCarrierPhyParams;
                uint16_t m_noOfCcs;
            };

            auto phy = StaticCast<LtePhySimulationHelper, Object>(obj);
            auto lteHelper = phy->GetLteHelper();
            std::stringstream basePath;

            basePath << CONFIGURATOR->GetResultsPath() << "lte-" << phyId << "-";

            lteHelper->EnableTraces();

            auto rlcStat = lteHelper->GetRlcStats();
            rlcStat->SetDlOutputFilename(basePath.str() + "LteRlcDlStats.txt");
            rlcStat->SetUlOutputFilename(basePath.str() + "LteRlcUlStats.txt");

            auto pdcpStat = lteHelper->GetPdcpStats();
            pdcpStat->SetDlPdcpOutputFilename(basePath.str() + "PdcpDlStats.txt");
            pdcpStat->SetUlPdcpOutputFilename(basePath.str() + "PdcpUlStats.txt");

            auto lteHelperQ = reinterpret_cast<LteHelperQuirk*>(&(*lteHelper));

            lteHelperQ->phyStat->SetUeSinrFilename(basePath.str() + "PhySinrUlStats.txt");
            lteHelperQ->phyStat->SetInterferenceFilename(basePath.str() +
                                                         "PhyInterferenceUlStats.txt");
            lteHelperQ->phyStat->SetCurrentCellRsrpSinrFilename(basePath.str() +
                                                                "PhyRsrpSinrDlStats.txt");

            lteHelperQ->phyRxStat->SetDlRxOutputFilename(basePath.str() + "PhyRxDlStats.txt");
            lteHelperQ->phyRxStat->SetUlRxOutputFilename(basePath.str() + "PhyRxUlStats.txt");

            lteHelperQ->phyTxStat->SetDlTxOutputFilename(basePath.str() + "PhyTxDlStats.txt");
            lteHelperQ->phyTxStat->SetUlTxOutputFilename(basePath.str() + "PhyTxUlStats.txt");

            lteHelperQ->macStat->SetDlOutputFilename(basePath.str() + "MacDlStats.txt");
            lteHelperQ->macStat->SetUlOutputFilename(basePath.str() + "MacUlStats.txt");
        }
    }
}

void
Scenario::EnablePhyNrTraces()
{
    NS_LOG_FUNCTION_NOARGS();
    if (!CONFIGURATOR->GetLogOnFile())
    {
        return;
    }

    for (size_t phyId = 0; phyId < m_protocolStacks[PHY_LAYER].size(); phyId++)
    {
        auto obj = m_protocolStacks[PHY_LAYER][phyId];

        if (DynamicCast<NrPhySimulationHelper>(obj))
        {
            auto phy = StaticCast<NrPhySimulationHelper, Object>(obj);
            auto nrHelper = phy->GetNrHelper();

            // Some paths are hadcoded during runtime (overwriting the filename variable, and
            // not allowing setting paths), other also creates the file during the call to
            // EnableTraces, so we can't set it like in LTE We need to change for this reason
            // the application path execution to the results path and keep it also during the
            // simulation GetResultsPath() use as base folder the initial program path, so other
            // next paths will not be affected by the change of the current program path (this
            // change has been done to manage this issue)
            std::filesystem::current_path(CONFIGURATOR->GetResultsPath());
            nrHelper->EnableTraces();
        }
    }
}

} // namespace ns3
