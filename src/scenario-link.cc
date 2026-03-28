#include "scenario.h"
#include <filesystem>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ScenarioLink");

void
Scenario::InitializeIslDelayMode()
{
    NS_LOG_FUNCTION_NOARGS();

    auto phyLayerConfs = CONFIGURATOR->GetPhyLayers();
    for (size_t netId = 0; netId < m_protocolStacks[PHY_LAYER].size(); ++netId)
    {
        if (phyLayerConfs[netId]->GetType() == "nr")
        {
            auto nrConf =
                StaticCast<NrPhyLayerConfiguration, PhyLayerConfiguration>(phyLayerConfs[netId]);
            if (nrConf->GetIslDelayModeConfig())
            {
                UpdateIslDelay(netId, nrConf);
            }
        }
    }
}

void
Scenario::UpdateIslDelay(uint32_t netId, Ptr<NrPhyLayerConfiguration> config)
{
    auto idmConfig = config->GetIslDelayModeConfig();
    if (!idmConfig)
    {
        return;
    }

    // Phase 1: Precompute ground station coordinates
    std::vector<std::pair<Vector, std::pair<double, double>>> gsData;
    for (auto& gs : idmConfig->groundStations)
    {
        Vector pos =
            GeographicPositions::GeographicToCartesianCoordinates(gs.first,
                                                                  gs.second,
                                                                  0,
                                                                  GeographicPositions::SPHERE);
        gsData.emplace_back(pos, gs);
    }

    struct SatInfo
    {
        Ptr<Node> node;
        Vector pos;
        double minEarthDist = std::numeric_limits<double>::max();
        Time earthDelay = Time::Max();
        std::pair<double, double> closestGsLatLon;
    };

    std::vector<SatInfo> sats;

    auto gnbIt = m_nrGnbDevices.find(netId);
    if (gnbIt != m_nrGnbDevices.end())
    {
        for (const auto& gnbContainer : gnbIt->second)
        {
            for (uint32_t i = 0; i < gnbContainer.GetN(); ++i)
            {
                Ptr<NetDevice> gnbDev = gnbContainer.Get(i);
                Ptr<Node> gnbNode = gnbDev->GetNode();

                // Only consider LEO satellites
                bool isLeo = false;
                for (uint32_t j = 0; j < m_leoSats.GetN(); ++j)
                {
                    if (m_leoSats.Get(j) == gnbNode)
                    {
                        isLeo = true;
                        break;
                    }
                }

                if (!isLeo)
                {
                    continue;
                }

                Ptr<MobilityModel> gnbMob = gnbNode->GetObject<MobilityModel>();
                if (!gnbMob)
                {
                    continue;
                }

                SatInfo info;
                info.node = gnbNode;
                info.pos = gnbMob->GetPosition();
                sats.push_back(info);
            }
        }
    }

    if (sats.empty())
    {
        Simulator::Schedule(idmConfig->precision, &Scenario::UpdateIslDelay, this, netId, config);
        return;
    }

    // Phase 2: Ground Station Links
    const double SPEED_OF_LIGHT = 2.99792458e8;
    for (auto& sat : sats)
    {
        double minDistSq = std::numeric_limits<double>::max();
        std::pair<double, double> bestGsLatLon;
        for (auto& gs : gsData)
        {
            Vector gsPos = gs.first;
            double dx = sat.pos.x - gsPos.x;
            double dy = sat.pos.y - gsPos.y;
            double dz = sat.pos.z - gsPos.z;
            double distSq = dx * dx + dy * dy + dz * dz;
            if (distSq < minDistSq)
            {
                minDistSq = distSq;
                bestGsLatLon = gs.second;
            }
        }

        if (minDistSq != std::numeric_limits<double>::max() && !gsData.empty())
        {
            double minDist = std::sqrt(minDistSq);
            sat.minEarthDist = minDist;
            if (minDist <= idmConfig->maxGroundStationDistance)
            {
                sat.earthDelay = Seconds(minDist / SPEED_OF_LIGHT);
                sat.closestGsLatLon = bestGsLatLon;
            }
        }
    }

    // Phase 3: Dijkstra's Algorithm
    size_t numSats = sats.size();
    size_t virtualEarthNode = numSats;
    size_t numNodes = numSats + 1;

    std::vector<Time> minDelay(numNodes, Time::Max());
    std::vector<bool> visited(numNodes, false);
    std::vector<size_t> parent(numNodes, numNodes);

    // Initialize Virtual Earth Node
    minDelay[virtualEarthNode] = Seconds(0);

    // Find shortest paths
    for (size_t count = 0; count < numNodes - 1; ++count)
    {
        // Pick minimum delay node
        Time uDelay = Time::Max();
        size_t uOffset = numNodes;

        for (size_t v = 0; v < numNodes; ++v)
        {
            if (!visited[v] && minDelay[v] <= uDelay)
            {
                uDelay = minDelay[v];
                uOffset = v;
            }
        }

        if (uOffset == numNodes || uDelay == Time::Max())
        {
            break; // Unreachable nodes remain
        }

        visited[uOffset] = true;

        // Update adjacent vertices
        for (size_t v = 0; v < numNodes; ++v)
        {
            if (visited[v])
            {
                continue;
            }

            Time edgeDelay = Time::Max();

            if (uOffset == virtualEarthNode)
            {
                // Edge from Virtual Earth to Satellite
                if (v < numSats)
                {
                    edgeDelay = sats[v].earthDelay;
                }
            }
            else if (v == virtualEarthNode)
            {
                // Edge from Satellite to Virtual Earth
                if (uOffset < numSats)
                {
                    edgeDelay = sats[uOffset].earthDelay;
                }
            }
            else
            {
                // ISL Edge between Satellites
                double dx = sats[uOffset].pos.x - sats[v].pos.x;
                double dy = sats[uOffset].pos.y - sats[v].pos.y;
                double dz = sats[uOffset].pos.z - sats[v].pos.z;
                double dist = std::sqrt(dx * dx + dy * dy + dz * dz);

                if (dist <= idmConfig->maxISLSatDistance)
                {
                    edgeDelay = Seconds(dist / SPEED_OF_LIGHT);
                }
            }

            if (edgeDelay != Time::Max())
            {
                Time altDelay = uDelay + edgeDelay;
                if (altDelay < minDelay[v])
                {
                    minDelay[v] = altDelay;
                    parent[v] = uOffset;
                }
            }
        }
    }

    // Phase 5: Apply Delays
    std::ofstream traceFile;
    if (idmConfig->updateLog)
    {
        std::string traceFilePath = CONFIGURATOR->GetResultsPath() + "isl-delay-trace.csv";
        bool fileExists = std::filesystem::exists(traceFilePath);
        traceFile.open(traceFilePath, std::ios_base::app);
        if (!fileExists)
        {
            traceFile << "Time,GNbNodeId,Attached,NextHopPath,TotalDelay,GsCoords,SatX,SatY,SatZ\n";
        }
    }

    for (size_t i = 0; i < numSats; ++i)
    {
        Time totalDelay = Time::Max();
        bool isAttached = false;
        if (minDelay[i] != Time::Max())
        {
            totalDelay = minDelay[i] + idmConfig->additionalDelay;
            isAttached = true;
        }
        else
        {
            // Unreachable satellite: close the link by dropping data entirely with a negative delay
            totalDelay = Seconds(-1.0);
        }

        Ptr<Node> gnbNode = sats[i].node;

        if (idmConfig->updateLog && traceFile.is_open())
        {
            std::ostringstream pathStream;
            std::string gsCoordsStr = "N/A";

            if (isAttached)
            {
                size_t curr = i;
                bool firstHop = true;
                while (curr != virtualEarthNode)
                {
                    size_t p = parent[curr];
                    if (p == virtualEarthNode)
                    {
                        if (!firstHop)
                        {
                            pathStream << " -> ";
                        }
                        pathStream << "[Ground;" << sats[curr].minEarthDist << "m;"
                                   << sats[curr].earthDelay.GetSeconds() << "s]";
                        gsCoordsStr = std::to_string(sats[curr].closestGsLatLon.first) + "," +
                                      std::to_string(sats[curr].closestGsLatLon.second);
                        break;
                    }
                    else
                    {
                        double dx = sats[curr].pos.x - sats[p].pos.x;
                        double dy = sats[curr].pos.y - sats[p].pos.y;
                        double dz = sats[curr].pos.z - sats[p].pos.z;
                        double dist = std::sqrt(dx * dx + dy * dy + dz * dz);
                        Time delay = Seconds(dist / SPEED_OF_LIGHT);

                        if (!firstHop)
                        {
                            pathStream << " -> ";
                        }
                        pathStream << "[Node_" << sats[p].node->GetId() << ";" << dist << "m;"
                                   << delay.GetSeconds() << "s]";
                    }
                    curr = p;
                    firstHop = false;
                }
            }

            traceFile << Simulator::Now().GetSeconds() << "," << gnbNode->GetId() << ","
                      << (isAttached ? "Yes" : "No") << ",\"" << pathStream.str() << "\","
                      << (isAttached ? totalDelay.GetSeconds() : 3600.0) << ",\"" << gsCoordsStr
                      << "\"," << sats[i].pos.x << "," << sats[i].pos.y << "," << sats[i].pos.z
                      << "\n";
        }
        for (uint32_t d = 0; d < gnbNode->GetNDevices(); ++d)
        {
            Ptr<NetDevice> nodeDev = gnbNode->GetDevice(d);
#if APPLY_ISL_DELAY_ONLY_ON_DATA
            if (Ptr<NrGnbNetDevice> gnbDev = DynamicCast<NrGnbNetDevice>(nodeDev))
            {
                uint32_t numBwps = gnbDev->GetCcMapSize();
                for (uint32_t bwpIndex = 0; bwpIndex < numBwps; ++bwpIndex)
                {
                    Ptr<NrGnbMac> mac = gnbDev->GetMac(bwpIndex);
                    if (mac)
                    {
                        mac->SetAttribute("DataDelay", TimeValue(totalDelay));
                    }
                }
            }
#else
            if (Ptr<PointToPointNetDevice> ptpDev = DynamicCast<PointToPointNetDevice>(nodeDev))
            {
                Ptr<PointToPointChannel> channel =
                    DynamicCast<PointToPointChannel>(ptpDev->GetChannel());
                if (channel)
                {
                    channel->SetAttribute("Delay", TimeValue(totalDelay));
                }
            }
#endif
        }
    }

    // Schedule next update
    Simulator::Schedule(idmConfig->precision, &Scenario::UpdateIslDelay, this, netId, config);
}

void
Scenario::AttachAllNrUesToGnbs()
{
    NS_LOG_FUNCTION_NOARGS();

    for (size_t netId = 0; netId < m_protocolStacks[PHY_LAYER].size(); ++netId)
    {
        auto nrPhy = DynamicCast<NrPhySimulationHelper>(m_protocolStacks[PHY_LAYER][netId]);
        if (!nrPhy)
        {
            continue;
        }

        auto nrHelper = nrPhy->GetNrHelper();

        // Check if we have gNBs for this netId
        auto gnbIt = m_nrGnbDevices.find(netId);
        if (gnbIt == m_nrGnbDevices.end() || gnbIt->second.empty())
        {
            NS_LOG_INFO("No gNBs available for attachment in stack " << netId);
            continue;
        }

        // Collect all gNB devices for this network stack
        NetDeviceContainer allGnbDevices;
        for (const auto& gnbContainer : gnbIt->second)
        {
            allGnbDevices.Add(gnbContainer);
        }

        // Check if we have UEs for this netId
        auto ueIt = m_nrUeDevices.find(netId);
        if (ueIt == m_nrUeDevices.end() || ueIt->second.empty())
        {
            NS_LOG_INFO("No UEs available for attachment in stack " << netId);
            continue;
        }

        // Attach all UEs to closest gNB
        NetDeviceContainer ueDevices;
        for (const auto& ueDevice : ueIt->second)
        {
            ueDevices.Add(ueDevice);
        }

        // Retrieve the configuration for this PHY layer to check the attachment method
        auto phyLayerConfs = CONFIGURATOR->GetPhyLayers();
        // Assuming netId maps directly to the index in the PHY layer configuration vector
        // This assumption holds based on how m_protocolStacks[PHY_LAYER] is populated in
        // ConfigurePhy
        auto nrConf =
            StaticCast<NrPhyLayerConfiguration, PhyLayerConfiguration>(phyLayerConfs[netId]);

        // Check if SINR-Distance Attachment is configured
        if (nrConf->GetSinrDistanceAttachConfig())
        {
            if (m_sinrAttachmentRunning.find(netId) == m_sinrAttachmentRunning.end())
            {
                NS_LOG_INFO(
                    "Using SINR-Distance Attachment logic. Skipping 'attachMethod' configuration.");
                Simulator::Schedule(Seconds(0),
                                    &Scenario::EvaluateSinrDistanceAttachment,
                                    this,
                                    netId);
                m_sinrAttachmentRunning.insert(netId);
            }
            continue;
        }

        std::string attachMethod = nrConf->GetAttachMethod();

        NS_LOG_INFO("Attaching " << ueDevices.GetN()
                                 << " UE devices to gNBs using method: " << attachMethod);

        if (attachMethod == "closest")
        {
            nrHelper->AttachToClosestGnb(ueDevices, allGnbDevices);
        }
        else if (attachMethod == "max-rsrp")
        {
            // Workaround for segfault in NrHelper::AttachToMaxRsrpGnb(container)
            // The helper captures an iterator to the container, which becomes invalid if the
            // container is destroyed. We create a persistent container for each UE to ensure
            // validity.
            for (auto i = ueDevices.Begin(); i != ueDevices.End(); ++i)
            {
                Ptr<NetDevice> ueDevice = *i;
                // Create a persistent container using shared_ptr and store it in the list
                // This ensures automatic deallocation when Scenario is destroyed.
                auto persistentContainer = std::make_shared<NetDeviceContainer>(ueDevice);
                m_persistentContainers.push_back(persistentContainer);

                // We call the container overload with a single device.
                // The helper will schedule the attachment.
                nrHelper->AttachToMaxRsrpGnb(*persistentContainer, allGnbDevices);
            }
        }
        else if (attachMethod == "none")
        {
            NS_LOG_INFO("Skipping attachment as per configuration.");
        }
        else
        {
            NS_FATAL_ERROR("Unknown attachment method: " << attachMethod);
        }

        // Check if SINR-Distance Attachment is configured and schedule it
        if (nrConf->GetSinrDistanceAttachConfig())
        {
            Simulator::Schedule(Seconds(0), &Scenario::EvaluateSinrDistanceAttachment, this, netId);
        }
    }
}

void
Scenario::ConfigureFullMeshX2Links()
{
    NS_LOG_FUNCTION(this);

    // Retrieve configuration
    auto phyLayerConfs = CONFIGURATOR->GetPhyLayers();

    for (size_t netId = 0; netId < m_protocolStacks[PHY_LAYER].size(); ++netId)
    {
        if (netId >= phyLayerConfs.size())
        {
            continue;
        }

        auto nrConf = DynamicCast<NrPhyLayerConfiguration>(phyLayerConfs[netId]);
        if (!nrConf || !nrConf->GetFullMeshX2Links())
        {
            continue;
        }

        auto nrPhySim = DynamicCast<NrPhySimulationHelper>(m_protocolStacks[PHY_LAYER][netId]);
        if (!nrPhySim)
        {
            continue;
        }

        NS_LOG_INFO("Configuring full mesh X2 links for netId " << netId);

        NodeContainer gnbNodes;
        auto it = m_nrGnbDevices.find(netId);
        if (it != m_nrGnbDevices.end())
        {
            for (const auto& devContainer : it->second)
            {
                for (uint32_t i = 0; i < devContainer.GetN(); ++i)
                {
                    Ptr<NetDevice> dev = devContainer.Get(i);
                    if (dev)
                    {
                        gnbNodes.Add(dev->GetNode());
                    }
                }
            }
        }

        // AddX2Interface creates links between all pairs in the container
        if (gnbNodes.GetN() > 1)
        {
            nrPhySim->GetNrHelper()->AddX2Interface(gnbNodes);
        }
    }
}

void
Scenario::EvaluateSinrDistanceAttachment(const uint32_t netId)
{
    // Retrieve configuration
    auto phyLayerConfs = CONFIGURATOR->GetPhyLayers();
    if (netId >= phyLayerConfs.size())
    {
        return;
    }

    auto nrConf = DynamicCast<NrPhyLayerConfiguration>(phyLayerConfs[netId]);
    if (!nrConf)
    {
        return;
    }

    auto sdaConfigOpt = nrConf->GetSinrDistanceAttachConfig();

    if (!sdaConfigOpt)
    {
        return;
    }

    const auto& sdaConfig = *sdaConfigOpt;

    // Retrieve Devices
    auto gnbIt = m_nrGnbDevices.find(netId);
    auto ueIt = m_nrUeDevices.find(netId);

    if (gnbIt == m_nrGnbDevices.end() || ueIt == m_nrUeDevices.end() || gnbIt->second.empty() ||
        ueIt->second.empty())
    {
        // Reschedule if devices are not yet ready or empty
        Simulator::Schedule(sdaConfig.precision,
                            &Scenario::EvaluateSinrDistanceAttachment,
                            this,
                            netId);
        return;
    }

    // Flatten gNB list for easier access
    NetDeviceContainer allGnbDevices;
    for (const auto& gnbContainer : gnbIt->second)
    {
        allGnbDevices.Add(gnbContainer);
    }

    auto nrPhySim = StaticCast<NrPhySimulationHelper, Object>(m_protocolStacks[PHY_LAYER][netId]);
    auto nrHelper = nrPhySim->GetNrHelper();

    // Prepare REM Helper for SINR calculations
    Ptr<NrRadioGeoEnvironmentMapHelper> remHelper = CreateObject<NrRadioGeoEnvironmentMapHelper>();
    remHelper->SetInterferers(allGnbDevices, 0); // Configure interferers (all gNBs)

    // Iterate over all UEs
    for (const auto& ueDevicePtr : ueIt->second)
    {
        auto ueDevice = DynamicCast<NrUeNetDevice>(ueDevicePtr);
        if (!ueDevice)
        {
            continue;
        }

        Ptr<Node> ueNode = ueDevice->GetNode();
        if (!ueNode)
        {
            continue;
        }

        Ptr<MobilityModel> ueMobility = ueNode->GetObject<MobilityModel>();
        if (!ueMobility)
        {
            continue;
        }

        // Find current gNB
        uint16_t currentCellId = ueDevice->GetRrc()->GetCellId();
        Ptr<NrGnbNetDevice> currentGnb = nullptr;

        if (ueDevice->GetRrc()->GetState() == NrUeRrc::CONNECTED_NORMALLY ||
            ueDevice->GetRrc()->GetState() == NrUeRrc::CONNECTED_HANDOVER)
        {
            for (uint32_t k = 0; k < allGnbDevices.GetN(); ++k)
            {
                auto gnb = DynamicCast<NrGnbNetDevice>(allGnbDevices.Get(k));
                if (gnb && gnb->GetCellId() == currentCellId)
                {
                    currentGnb = gnb;
                    break;
                }
            }
        }

        double currentSnr = -std::numeric_limits<double>::infinity();
        bool currentGnbValid = false;

        // Find best gNB
        Ptr<NrGnbNetDevice> bestGnb = nullptr;
        double bestSnr = -std::numeric_limits<double>::infinity();

        // Check all gNBs
        for (auto gnbDeviceIt = allGnbDevices.Begin(); gnbDeviceIt != allGnbDevices.End();
             ++gnbDeviceIt)
        {
            Ptr<NrGnbNetDevice> gnbDevice = DynamicCast<NrGnbNetDevice>(*gnbDeviceIt);
            if (!gnbDevice)
            {
                continue;
            }

            Ptr<Node> gnbNode = gnbDevice->GetNode();
            if (!gnbNode)
            {
                continue;
            }

            Ptr<MobilityModel> gnbMobility = gnbNode->GetObject<MobilityModel>();
            if (!gnbMobility)
            {
                continue;
            }

            double distance = ueMobility->GetDistanceFrom(gnbMobility);

#ifdef SINR_DISTANCE_PRINT_DEBUG
            std::cout << "UE " << ueDevice->GetNode()->GetId() << " distance to gNB "
                      << gnbNode->GetId() << ": " << distance / 1000 << " km" << std::endl;
#endif

            // Find required min SINR for this distance
            double minSinrRequired = std::numeric_limits<double>::max();

            const SinrDistanceTableEntry* bestEntry = nullptr;
            double rangeDiff = std::numeric_limits<double>::max();

            // Finding the best entry based on maxDistance (the minimum distance that is still
            // within range)
            for (const auto& entry : sdaConfig.table)
            {
                if (distance <= entry.maxDistance)
                {
                    if (entry.maxDistance < rangeDiff)
                    {
                        rangeDiff = entry.maxDistance;
                        bestEntry = &entry;
                    }
                }
            }

            // UE is too far for any rule
            if (!bestEntry)
            {
                continue;
            }

            minSinrRequired = bestEntry->minSinr;
            double estimatedSnr = remHelper->GetSnr(ueDevice, gnbDevice, 0, true);

#ifdef SINR_DISTANCE_PRINT_DEBUG
            std::cout << "UE " << ueDevice->GetNode()->GetId() << " evaluated SNR for gNB "
                      << gnbNode->GetId() << ": " << estimatedSnr
                      << " dB (Required: " << minSinrRequired << " dB)" << std::endl;
#endif

            // Checking if SNR is above the required threshold
            if (estimatedSnr >= minSinrRequired)
            {
                if (currentGnb && gnbDevice == currentGnb)
                {
                    currentSnr = estimatedSnr;
                    currentGnbValid = true;
                }

                if (estimatedSnr > bestSnr)
                {
                    bestSnr = estimatedSnr;
                    bestGnb = gnbDevice;
                }
            }
        }

        if (bestGnb)
        {
            // Check if we need to handover (if configured gNB is different)
            if (currentGnb != nullptr)
            {
                if (currentGnb != bestGnb)
                {
                    // Hysteresis check
                    // Only switch if the new gNB is better by at least 'threshold' dB
                    // AND the current gNB is still valid.
                    // If current gNB is NOT valid (didn't meet min requirements), we MUST switch.

                    bool shouldHandover = true;

                    if (currentGnbValid)
                    {
                        if (bestSnr < currentSnr + sdaConfig.threshold)
                        {
                            shouldHandover = false;
#ifdef SINR_DISTANCE_PRINT_DEBUG
                            if (bestSnr > currentSnr)
                            {
                                std::cout << "UE " << ueDevice->GetImsi()
                                          << " HANDOVER PREVENTED by threshold ("
                                          << sdaConfig.threshold << " dB)"
                                          << " from gNB " << currentGnb->GetNode()->GetId()
                                          << " (SNR: " << currentSnr << " dB)"
                                          << " to gNB " << bestGnb->GetNode()->GetId()
                                          << " (SNR: " << bestSnr << " dB)"
                                          << " Delta: " << bestSnr - currentSnr << " dB"
                                          << std::endl;
                            }
#endif
                        }
                    }

                    if (shouldHandover)
                    {
#ifdef SINR_DISTANCE_PRINT_DEBUG
                        std::cout << "UE " << ueDevice->GetImsi() << " HANDOVER from gNB "
                                  << currentGnb->GetNode()->GetId() << " (SNR: " << currentSnr
                                  << " dB)"
                                  << " to gNB " << bestGnb->GetNode()->GetId()
                                  << " (SNR: " << bestSnr << " dB)"
                                  << " Threshold: " << sdaConfig.threshold << " dB" << std::endl;
#endif
                        nrHelper->HandoverRequest(Seconds(0), ueDevice, currentGnb, bestGnb);
                    }
                }
            }
            else
            {
#ifdef SINR_DISTANCE_PRINT_DEBUG
                std::cout << "UE " << ueDevice->GetNode()->GetId() << " CAN connect to gNB "
                          << bestGnb->GetNode()->GetId() << " (SNR: " << bestSnr << " dB)" << " at "
                          << Simulator::Now().GetSeconds() << std::endl;
#endif

                for (uint32_t i = 0; i < ueDevice->GetCcMapSize(); ++i)
                {
                    auto uePhy = DynamicCast<NrUePhy>(ueDevice->GetPhy(i));
                    if (uePhy && !uePhy->IsPhyEnabled())
                    {
                        uePhy->SetPhyEnabled(true);
                    }
                }
                nrHelper->AttachToGnb(ueDevice, bestGnb);
            }
        }
        else
        {
            // No suitable gNB found. If currently connected, disconnect.
            if (currentGnb != nullptr)
            {
#ifdef SINR_DISTANCE_PRINT_DEBUG
                std::cout << "UE " << ueDevice->GetImsi() << " DISCONNECTING from gNB "
                          << currentGnb->GetCellId() << " (No suitable gNB found)"
                          << " at " << Simulator::Now().GetSeconds() << std::endl;
#endif

                for (uint32_t i = 0; i < ueDevice->GetCcMapSize(); ++i)
                {
                    auto uePhy = DynamicCast<NrUePhy>(ueDevice->GetPhy(i));
                    if (uePhy)
                    {
                        uePhy->SetPhyEnabled(false);
                    }
                }
            }
        }
    }

    // Reschedule
    Simulator::Schedule(sdaConfig.precision,
                        &Scenario::EvaluateSinrDistanceAttachment,
                        this,
                        netId);
}

} // namespace ns3
