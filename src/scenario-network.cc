#include "scenario.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ScenarioNetwork");

void
Scenario::ConfigureNetwork()
{
    NS_LOG_FUNCTION_NOARGS();

    const auto layerConfs = CONFIGURATOR->GetNetworkLayers();
    for (auto& layerConf : layerConfs)
    {
        if (layerConf->GetType() == "ipv4")
        {
            const auto ipv4Conf =
                StaticCast<Ipv4NetworkLayerConfiguration, NetworkLayerConfiguration>(layerConf);
            const auto ipv4Sim = CreateObject<Ipv4SimulationHelper>(ipv4Conf->GetMask(),
                                                                    ipv4Conf->GetGatewayAddress());

            ipv4Sim->GetIpv4Helper().SetBase(Ipv4Address(ipv4Conf->GetAddress().c_str()),
                                             Ipv4Mask(ipv4Conf->GetMask().c_str()));

            if (CONFIGURATOR->GetLogOnFile())
            {
                EnableIpv4RoutingTableReporting();
            }

            m_protocolStacks[NET_LAYER].push_back(ipv4Sim);
        }
        else
        {
            NS_FATAL_ERROR("Unsupported Network Layer Type: " << layerConf->GetType());
        }
    }
}

void
Scenario::EnableIpv4RoutingTableReporting()
{
    // this method should be called once
    static bool hasBeenCalled = false;
    if (hasBeenCalled)
    {
        return;
    }
    else
    {
        hasBeenCalled = true;
    }

    NS_LOG_FUNCTION_NOARGS();

    std::stringstream routingTableFilePath;
    routingTableFilePath << CONFIGURATOR->GetResultsPath() << "ipv4-routing-tables.txt";
    auto routingTableSink = Create<OutputStreamWrapper>(routingTableFilePath.str(), std::ios::out);
    Ipv4RoutingHelper::PrintRoutingTableAllAt(Seconds(1.0), routingTableSink);
}

void
Scenario::InstallEntityIpv4(Ptr<Node> entityNode,
                            NetDeviceContainer netDevices,
                            const uint32_t netId)
{
    NS_LOG_FUNCTION(entityNode << netId);

    for (NetDeviceContainer::Iterator i = netDevices.Begin(); i != netDevices.End(); i++)
    {
        InstallEntityIpv4(entityNode, *i, netId);
    }
}

void
Scenario::InstallEntityIpv4(Ptr<Node> entityNode, Ptr<NetDevice> netDevice, const uint32_t netId)
{
    NS_LOG_FUNCTION(entityNode << netId);

    auto ipv4Obj = entityNode->GetObject<Ipv4>();

    if (!ipv4Obj)
    {
        auto netLayer =
            StaticCast<Ipv4SimulationHelper, Object>(m_protocolStacks[NET_LAYER][netId]);
        netLayer->GetInternetHelper().Install(entityNode);
    }
    else
    {
        ipv4Obj->AddInterface(netDevice);
    }
}

void
Scenario::ConfigureEntityIpv4(Ptr<Node> entityNode,
                              NetDeviceContainer devContainer,
                              const uint32_t deviceId,
                              const uint32_t netId)
{
    NS_LOG_FUNCTION(entityNode << deviceId << netId);
    NS_ASSERT_MSG(1 == devContainer.GetN(),
                  "ConfigureEntityIpv4 assumes to receive a NetDeviceContainer with only one "
                  "NetDevice, but there are "
                      << devContainer.GetN());

    auto netLayer = StaticCast<Ipv4SimulationHelper, Object>(m_protocolStacks[NET_LAYER][netId]);
    auto assignedIPs = netLayer->GetIpv4Helper().Assign(devContainer);

    Ipv4StaticRoutingHelper routingHelper;
    auto deviceIP = assignedIPs.Get(0);
    Ptr<Ipv4StaticRouting> ueStaticRoute = routingHelper.GetStaticRouting(deviceIP.first);

    const Ipv4Address nodeAddr = assignedIPs.GetAddress(0, 0);
    if (nodeAddr != netLayer->GetGatewayAddress())
    {
        ueStaticRoute->SetDefaultRoute(netLayer->GetGatewayAddress(), deviceIP.second);
    }
}

void
Scenario::ConfigureInternetRemotes()
{
    NS_LOG_FUNCTION_NOARGS();

    const auto remoteConfs = CONFIGURATOR->GetRemotesConfiguration();
    // Applications are now installed in ConfigureAllApplications
}

void
Scenario::ConfigureInternetBackbone()
{
    NS_LOG_FUNCTION_NOARGS();

    InternetStackHelper internetHelper;
    Ipv4StaticRoutingHelper routingHelper;

    internetHelper.Install(m_remoteNodes);

    // setup a CSMA LAN between all the remotes and network gateways in the backbone
    CsmaHelper csma;
    NetDeviceContainer backboneDevs = csma.Install(m_backbone);

    // set a new address base for the backbone
    Ipv4AddressHelper ipv4H;
    ipv4H.SetBase(Ipv4Address("200.0.0.0"), Ipv4Mask("255.0.0.0"));
    ipv4H.Assign(backboneDevs);

    // Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    // create static routes between each remote node to each network gateway
    internetHelper.SetRoutingHelper(routingHelper);

    for (uint32_t i = 0; i < m_remoteNodes.GetN(); i++)
    {
        Ptr<Node> remoteNode = m_backbone.Get(i);
        for (uint32_t j = m_remoteNodes.GetN(); j < m_backbone.GetN(); j++)
        {
            Ptr<Node> netGwNode = m_backbone.Get(j);
            uint32_t netGwBackboneDevIndex = netGwNode->GetNDevices() - 1;

            // since network gateway have at least 2 ipv4 addresses (one in the network and the
            // other for the backbone) we should create a route to the internal ip using the
            // backbone ip as next hop
            Ipv4Address netGwBackboneIpv4 =
                netGwNode->GetObject<Ipv4>()->GetAddress(netGwBackboneDevIndex, 0).GetLocal();
            Ipv4Address netGwInternalIpv4 =
                netGwNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

            NS_LOG_LOGIC("Setting-up static route: REMOTE "
                         << i << " " << "("
                         << remoteNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << ") "
                         << "-> NETWORK " << j - m_remoteNodes.GetN() << " " << "("
                         << netGwBackboneIpv4 << " -> " << netGwInternalIpv4 << ")");

            Ptr<Ipv4StaticRouting> staticRouteRem =
                routingHelper.GetStaticRouting(remoteNode->GetObject<Ipv4>());
            staticRouteRem->AddNetworkRouteTo(netGwInternalIpv4,
                                              Ipv4Mask("255.0.0.0"),
                                              netGwBackboneIpv4,
                                              1);
        }
    }

    if (CONFIGURATOR->GetLogOnFile())
    {
        std::stringstream logFilePathBuilder;
        logFilePathBuilder << CONFIGURATOR->GetResultsPath() << "internet";
        const auto logFilePath = logFilePathBuilder.str();

        csma.EnablePcapAll(logFilePath, true);
        csma.EnableAsciiAll(logFilePath);
    }
}

} // namespace ns3
