#include "scenario.h"

#include "ns3/nr-epc-x2.h"
#include "ns3/nr-gnb-net-device.h"
#include "ns3/nr-ue-rrc.h"
#include "ns3/nr-no-backhaul-epc-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/string.h"
#include "helper/nr-radio-geo-environment-map-helper.h"

#include <filesystem>
#include <queue>
#include <utility>

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

    BuildIslGraph(netId);
    auto& cache = m_islCaches[netId];

    if (!cache.valid || cache.indexToNode.empty())
    {
        Simulator::Schedule(idmConfig->precision, &Scenario::UpdateIslDelay, this, netId, config);
        return;
    }

    const double SPEED_OF_LIGHT = 2.99792458e8;
    size_t numSats = cache.indexToNode.size();
    size_t virtualEarthNode = numSats;

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

    auto gnbIt = m_nrGnbDevices.find(netId);
    if (gnbIt == m_nrGnbDevices.end())
    {
        Simulator::Schedule(idmConfig->precision, &Scenario::UpdateIslDelay, this, netId, config);
        return;
    }

    for (const auto& gnbContainer : gnbIt->second)
    {
        for (uint32_t i = 0; i < gnbContainer.GetN(); ++i)
        {
            Ptr<Node> gnbNode = gnbContainer.Get(i)->GetNode();

            auto it = cache.nodeToIndex.find(gnbNode);
            if (it == cache.nodeToIndex.end())
            {
                continue;
            }

            size_t satIdx = it->second;

            Time totalDelay = Time::Max();
            bool isAttached = false;
            double minEarthDist = cache.minEarthDist[satIdx];

            if (minEarthDist != std::numeric_limits<double>::infinity())
            {
                totalDelay = Seconds(minEarthDist / SPEED_OF_LIGHT) + idmConfig->additionalDelay;
                isAttached = true;
            }
            else
            {
                totalDelay = Seconds(-1.0);
            }

            if (idmConfig->updateLog && traceFile.is_open())
            {
                Vector pos = cache.indexToNode[satIdx]->GetObject<MobilityModel>()->GetPosition();
                std::ostringstream pathStream;
                std::string gsCoordsStr = "N/A";

                if (isAttached)
                {
                    size_t curr = satIdx;
                    bool firstHop = true;
                    while (curr != virtualEarthNode)
                    {
                        size_t p = cache.earthParent[curr];
                        if (p == virtualEarthNode)
                        {
                            if (!firstHop)
                            {
                                pathStream << " -> ";
                            }

                            double directDist =
                                cache.nodeToGsDirectDist[curr][cache.closestGsIdx[curr]];
                            pathStream << "[Ground;" << directDist << "m;"
                                       << (directDist / SPEED_OF_LIGHT) << "s]";
                            auto gs = idmConfig->groundStations[cache.closestGsIdx[curr]];
                            gsCoordsStr =
                                std::to_string(gs.first) + "," + std::to_string(gs.second);
                            break;
                        }
                        else
                        {
                            double dist = cache.adjMatrix[curr][p];
                            if (!firstHop)
                            {
                                pathStream << " -> ";
                            }
                            pathStream << "[Node_" << cache.indexToNode[p]->GetId() << ";" << dist
                                       << "m;" << (dist / SPEED_OF_LIGHT) << "s]";
                        }
                        curr = p;
                        firstHop = false;
                    }
                }

                traceFile << Simulator::Now().GetSeconds() << "," << gnbNode->GetId() << ","
                          << (isAttached ? "true" : "false") << ",\"" << pathStream.str() << "\","
                          << (isAttached ? totalDelay.GetSeconds() : 3600.0) << ",\"" << gsCoordsStr
                          << "\"," << pos.x << "," << pos.y << "," << pos.z << "\n";
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
                        if (totalDelay < Seconds(0))
                        {
                            // A negative delay means the link is down. PointToPointChannel cannot handle negative delays.
                            // We set an artificially huge delay to practically drop the packets in the simulation time.
                            channel->SetAttribute("Delay", TimeValue(Seconds(3600.0)));
                        }
                        else
                        {
                            channel->SetAttribute("Delay", TimeValue(totalDelay));
                        }
                    }
                }
#endif
            }
        }
    }

    Simulator::Schedule(idmConfig->precision, &Scenario::UpdateIslDelay, this, netId, config);
}

void
Scenario::BuildIslGraph(uint32_t netId)
{
    auto phyLayerConfs = CONFIGURATOR->GetPhyLayers();
    if (netId >= phyLayerConfs.size())
    {
        return;
    }
    auto config = StaticCast<NrPhyLayerConfiguration, PhyLayerConfiguration>(phyLayerConfs[netId]);
    if (!config)
    {
        return;
    }

    auto idmConfig = config->GetIslDelayModeConfig();
    if (!idmConfig)
    {
        return;
    }

    auto& cache = m_islCaches[netId];
    if (cache.valid && (Simulator::Now() - cache.lastUpdate) < idmConfig->precision)
    {
        return;
    }

    cache.Clear();
    cache.lastUpdate = Simulator::Now();
    cache.valid = true;

    auto gnbIt = m_nrGnbDevices.find(netId);
    if (gnbIt == m_nrGnbDevices.end())
    {
        return;
    }

    std::vector<Ptr<Node>> configLeoNodes;
    for (const auto& gnbContainer : gnbIt->second)
    {
        for (uint32_t i = 0; i < gnbContainer.GetN(); ++i)
        {
            Ptr<Node> gnbNode = gnbContainer.Get(i)->GetNode();

            // Check if it's a LEO satellite
            bool isLeo = false;
            for (uint32_t j = 0; j < m_leoSats.GetN(); ++j)
            {
                if (m_leoSats.Get(j) == gnbNode)
                {
                    isLeo = true;
                    break;
                }
            }

            if (isLeo)
            {
                configLeoNodes.push_back(gnbNode);
            }
        }
    }

    // create a numeric association between satellite nodes and their indices in the matrixes
    for (uint32_t i = 0; i < configLeoNodes.size(); ++i)
    {
        Ptr<Node> node = configLeoNodes[i];
        cache.nodeToIndex[node] = i;
        cache.indexToNode.push_back(node);
    }

    size_t numSats = cache.indexToNode.size();
    if (numSats == 0)
    {
        return;
    }

    // Inizializing the matrixes with infinity distances (no connection)
    cache.adjMatrix.assign(numSats,
                           std::vector<double>(numSats, std::numeric_limits<double>::infinity()));
    cache.shortestPaths.assign(
        numSats,
        std::vector<double>(numSats, std::numeric_limits<double>::infinity()));
    cache.parents.assign(numSats, std::vector<size_t>(numSats, numSats));
    cache.nodeToGsDirectDist.assign(numSats,
                                    std::vector<double>(idmConfig->groundStations.size(),
                                                        std::numeric_limits<double>::infinity()));

    // Virtual Earth Node structures
    cache.minEarthDist.assign(numSats, std::numeric_limits<double>::infinity());
    cache.earthParent.assign(numSats, numSats);
    cache.closestGsIdx.assign(numSats, 0);

    // Computing the shortest path distances between satellites
    for (size_t i = 0; i < numSats; ++i)
    {
        Ptr<Node> n1 = cache.indexToNode[i];
        Vector p1 = n1->GetObject<MobilityModel>()->GetPosition();

        // distance from a node to itself is 0
        cache.adjMatrix[i][i] = 0;
        cache.shortestPaths[i][i] = 0;

        // Compute distances between satellites
        for (size_t j = i + 1; j < numSats; ++j)
        {
            Ptr<Node> n2 = cache.indexToNode[j];
            Vector p2 = n2->GetObject<MobilityModel>()->GetPosition();

            double dx = p1.x - p2.x;
            double dy = p1.y - p2.y;
            double dz = p1.z - p2.z;
            double dist = std::sqrt(dx * dx + dy * dy + dz * dz);

            if (dist <= idmConfig->maxISLSatDistance)
            {
                cache.adjMatrix[i][j] = dist;
                cache.adjMatrix[j][i] = dist;
                cache.shortestPaths[i][j] = dist;
                cache.shortestPaths[j][i] = dist;
                cache.parents[i][j] = i;
                cache.parents[j][i] = j;
            }
        }

        // Compute distances between satellites and ground stations
        for (uint32_t gsIdx = 0; gsIdx < idmConfig->groundStations.size(); ++gsIdx)
        {
            auto gs = idmConfig->groundStations[gsIdx];
            Vector gsPos =
                GeographicPositions::GeographicToCartesianCoordinates(gs.first,
                                                                      gs.second,
                                                                      0,
                                                                      GeographicPositions::SPHERE);

            double dx = p1.x - gsPos.x;
            double dy = p1.y - gsPos.y;
            double dz = p1.z - gsPos.z;
            double dist = std::sqrt(dx * dx + dy * dy + dz * dz);

            if (dist <= idmConfig->maxGroundStationDistance)
            {
                cache.nodeToGsDirectDist[i][gsIdx] = dist;
            }
        }
    }

    // Dijkstra's Algorithm for All-Pairs Shortest Path
    using NodeDist = std::pair<double, size_t>;
    for (size_t s = 0; s < numSats; ++s)
    {
        std::vector<double>& dist = cache.shortestPaths[s];
        std::vector<size_t>& p = cache.parents[s];
        std::fill(dist.begin(), dist.end(), std::numeric_limits<double>::infinity());
        std::fill(p.begin(), p.end(), numSats);
        dist[s] = 0;

        std::priority_queue<NodeDist, std::vector<NodeDist>, std::greater<>> pq;
        pq.emplace(0.0, s);

        while (!pq.empty())
        {
            auto [d, u] = pq.top();
            pq.pop();

            if (d > dist[u])
            {
                continue;
            }

            for (size_t v = 0; v < numSats; ++v)
            {
                if (cache.adjMatrix[u][v] != std::numeric_limits<double>::infinity())
                {
                    double newDist = d + cache.adjMatrix[u][v];
                    if (newDist < dist[v])
                    {
                        dist[v] = newDist;
                        p[v] = u;
                        pq.emplace(newDist, v);
                    }
                }
            }
        }
    }

    // Dijkstra's Algorithm for Virtual Earth Node
    std::priority_queue<NodeDist, std::vector<NodeDist>, std::greater<>> earthPq;

    for (size_t i = 0; i < numSats; ++i)
    {
        double minDist = std::numeric_limits<double>::infinity();
        uint32_t bestGsIdx = 0;
        for (uint32_t gsIdx = 0; gsIdx < idmConfig->groundStations.size(); ++gsIdx)
        {
            if (cache.nodeToGsDirectDist[i][gsIdx] < minDist)
            {
                minDist = cache.nodeToGsDirectDist[i][gsIdx];
                bestGsIdx = gsIdx;
            }
        }

        if (minDist != std::numeric_limits<double>::infinity())
        {
            cache.minEarthDist[i] = minDist;
            cache.earthParent[i] = numSats; // Directly connected to earth
            cache.closestGsIdx[i] = bestGsIdx;
            earthPq.emplace(minDist, i);
        }
    }

    while (!earthPq.empty())
    {
        auto [d, u] = earthPq.top();
        earthPq.pop();

        if (d > cache.minEarthDist[u])
        {
            continue;
        }

        for (size_t v = 0; v < numSats; ++v)
        {
            if (cache.adjMatrix[u][v] != std::numeric_limits<double>::infinity())
            {
                double newDist = d + cache.adjMatrix[u][v];
                if (newDist < cache.minEarthDist[v])
                {
                    cache.minEarthDist[v] = newDist;
                    cache.earthParent[v] = u;
                    cache.closestGsIdx[v] = cache.closestGsIdx[u];
                    earthPq.emplace(newDist, v);
                }
            }
        }
    }
}

double
Scenario::GetISLMinimumDistance(Ptr<Node> n1, Ptr<Node> n2, uint32_t netId)
{
    BuildIslGraph(netId);
    auto& cache = m_islCaches[netId];
    if (!cache.valid)
    {
        return std::numeric_limits<double>::infinity();
    }

    auto it1 = cache.nodeToIndex.find(n1);
    auto it2 = cache.nodeToIndex.find(n2);

    if (it1 == cache.nodeToIndex.end() || it2 == cache.nodeToIndex.end())
    {
        return std::numeric_limits<double>::infinity();
    }

    return cache.shortestPaths[it1->second][it2->second];
}

double
Scenario::GetISLMinimumDistance(Ptr<Node> n, uint32_t gsIndex, uint32_t netId)
{
    BuildIslGraph(netId);
    auto& cache = m_islCaches[netId];
    if (!cache.valid)
    {
        return std::numeric_limits<double>::infinity();
    }

    auto it = cache.nodeToIndex.find(n);
    if (it == cache.nodeToIndex.end())
    {
        return std::numeric_limits<double>::infinity();
    }

    size_t u = it->second;
    double minDist = std::numeric_limits<double>::infinity();

    for (size_t v = 0; v < cache.indexToNode.size(); ++v)
    {
        if (cache.shortestPaths[u][v] != std::numeric_limits<double>::infinity() &&
            cache.nodeToGsDirectDist[v][gsIndex] != std::numeric_limits<double>::infinity())
        {
            double dist = cache.shortestPaths[u][v] + cache.nodeToGsDirectDist[v][gsIndex];
            if (dist < minDist)
            {
                minDist = dist;
            }
        }
    }

    return minDist;
}

std::pair<double, uint32_t>
Scenario::GetISLMinimumDistance(Ptr<Node> n, uint32_t netId)
{
    BuildIslGraph(netId);
    auto& cache = m_islCaches[netId];
    if (!cache.valid)
    {
        return {std::numeric_limits<double>::infinity(), 0};
    }

    auto it = cache.nodeToIndex.find(n);
    if (it == cache.nodeToIndex.end())
    {
        return {std::numeric_limits<double>::infinity(), 0};
    }

    size_t u = it->second;
    return {cache.minEarthDist[u], cache.closestGsIdx[u]};
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
        else if (attachMethod == "sinr-distance-dynamic")
        {
            std::string tableStr = "";
            for (const auto& attr : nrConf->GetHandoverAlgorithmAttributes())
            {
                if (attr.name == "DistanceSinrTable")
                {
                    Ptr<StringValue> stringVal = DynamicCast<StringValue>(attr.value);
                    if (stringVal)
                    {
                        tableStr = stringVal->Get();
                    }
                    break;
                }
            }
            if (tableStr.empty())
            {
                NS_LOG_WARN("DistanceSinrTable attribute missing from Handover algorithm. AttachMethod sinr-distance-dynamic may fail.");
            }

            // Create persistent containers for the periodic attachment
            for (auto i = ueDevices.Begin(); i != ueDevices.End(); ++i)
            {
                Ptr<NetDevice> ueDevice = *i;
                auto persistentContainer = std::make_shared<NetDeviceContainer>(ueDevice);
                m_persistentContainers.push_back(persistentContainer);
            }
            auto persistentGnbContainer = std::make_shared<NetDeviceContainer>(allGnbDevices);
            m_persistentContainers.push_back(persistentGnbContainer);

            // Start the periodic check
            Simulator::ScheduleNow(MakeEvent(&Scenario::PeriodicSinrDistanceAttachmentCheck,
                                             this,
                                             nrHelper,
                                             ueDevices,
                                             allGnbDevices,
                                             tableStr));
        }
        else
        {
            NS_FATAL_ERROR("Unknown attachment method: " << attachMethod);
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

        // Collect all NrGnbNetDevice pointers (one per channelId/device, not per node)
        NetDeviceContainer allGnbDevs;

        auto it = m_nrGnbDevices.find(netId);
        if (it != m_nrGnbDevices.end())
        {
            for (const auto& devContainer : it->second)
            {
                allGnbDevs.Add(devContainer);
            }
        }

        // Full mesh X2 links (standard path via NrHelper, now supports intra-node)
        if (allGnbDevs.GetN() > 1)
        {
            nrPhySim->GetNrHelper()->AddX2Interface(allGnbDevs);
        }
    }
}



struct TableEntry {
    double maxDistance;
    double minSinr;
};

void
Scenario::PeriodicSinrDistanceAttachmentCheck(Ptr<NrHelper> nrHelper, NetDeviceContainer ueDevices, NetDeviceContainer allGnbDevices, std::string tableStr)
{
    // Parse the table
    std::vector<TableEntry> table;
    std::stringstream ss(tableStr);
    std::string token;
    while (std::getline(ss, token, '|')) {
        auto colonPos = token.find(':');
        if (colonPos != std::string::npos) {
            double dist = std::stod(token.substr(0, colonPos));
            double sinr = std::stod(token.substr(colonPos + 1));
            table.push_back({dist, sinr});
        }
    }

    bool anyUnattached = false;

    for (uint32_t i = 0; i < ueDevices.GetN(); ++i)
    {
        Ptr<NetDevice> ueDevice = ueDevices.Get(i);
        Ptr<NrUeNetDevice> ueNetDev = ueDevice->GetObject<NrUeNetDevice>();
        // Check if the UE is attached. Wait, the state enum is NrUeRrc::IDLE_START or IDLE_CAMPED_NORMALLY.
        // Actually, if it has a CellId == 0, it means it's not attached.
        if (!ueNetDev || !ueNetDev->GetRrc() || ueNetDev->GetRrc()->GetCellId() != 0) {
            continue; // Already attached or attaching
        }

        anyUnattached = true;

        Ptr<MobilityModel> ueMobility = ueDevice->GetNode()->GetObject<MobilityModel>();
        Ptr<NrGnbNetDevice> bestGnb = nullptr;
        double bestSnr = -std::numeric_limits<double>::infinity();

        for (uint32_t k = 0; k < allGnbDevices.GetN(); ++k) {
            Ptr<NetDevice> gnbDev = allGnbDevices.Get(k);
            Ptr<NrGnbNetDevice> gnbNetDev = gnbDev->GetObject<NrGnbNetDevice>();
            Ptr<MobilityModel> gnbMobility = gnbDev->GetNode()->GetObject<MobilityModel>();
            double distance = ueMobility->GetDistanceFrom(gnbMobility);

            const TableEntry* bestEntry = nullptr;
            double rangeDiff = std::numeric_limits<double>::max();
            for (const auto& entry : table) {
                if (distance <= entry.maxDistance && entry.maxDistance < rangeDiff) {
                    rangeDiff = entry.maxDistance;
                    bestEntry = &entry;
                }
            }

            if (!bestEntry) continue;

            double minSinrRequired = bestEntry->minSinr;

            // Compute SINR for all BWPs and take the max
            for (uint32_t bwpId = 0; bwpId < gnbNetDev->GetCcMapSize(); ++bwpId) {
                Ptr<NrRadioGeoEnvironmentMapHelper> remHelper = CreateObject<NrRadioGeoEnvironmentMapHelper>();
                remHelper->SetInterferers(allGnbDevices, bwpId);
                double estimatedSnr = remHelper->GetSnr(ueDevice, gnbDev, bwpId, true);
                if (estimatedSnr >= minSinrRequired && estimatedSnr > bestSnr) {
                    bestSnr = estimatedSnr;
                    bestGnb = gnbNetDev;
                }
            }
        }

        if (bestGnb) {
            NS_LOG_INFO("Dynamically attaching UE " << ueDevice->GetNode()->GetId() << " to gNB " << bestGnb->GetNode()->GetId() << " (SINR: " << bestSnr << ")");
            nrHelper->AttachToGnb(ueDevice, bestGnb);
        }
    }

    if (anyUnattached) {
        Simulator::Schedule(Seconds(1.0), MakeEvent(&Scenario::PeriodicSinrDistanceAttachmentCheck, this, nrHelper, ueDevices, allGnbDevices, tableStr));
    }
}

} // namespace ns3
