#include "scenario.h"
#include "ns3/nstime.h"
#include "ns3/pointer.h"
#include <ns3/address-utils.h>
#include <ns3/ipv6.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ScenarioEntities");

Ptr<Node>
Scenario::GetNodeByKey(std::string key, uint32_t index)
{
    std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    Ptr<Node> targetNode = nullptr;
    if (key == "leo-sats")
    {
        if (index < m_leoSats.GetN())
        {
            targetNode = m_leoSats.Get(index);
        }
    }
    else if (key == "vehicles")
    {
        if (index < m_vehicles.GetN())
        {
            targetNode = m_vehicles.Get(index);
        }
    }
    else if (key == "nodes")
    {
        if (index < m_plainNodes.GetN())
        {
            targetNode = m_plainNodes.Get(index);
        }
    }
    else if (key == "drones")
    {
        if (index < m_drones.GetN())
        {
            targetNode = m_drones.Get(index);
        }
    }
    else if (key == "zsps")
    {
        if (index < m_zsps.GetN())
        {
            targetNode = m_zsps.Get(index);
        }
    }
    else if (key == "remotes")
    {
        if (index < m_remoteNodes.GetN())
        {
            targetNode = m_remoteNodes.Get(index);
        }
    }
    else if (key == "backbone")
    {
        if (index < m_backbone.GetN())
        {
            targetNode = m_backbone.Get(index);
        }
    }
    else
    {
        NS_FATAL_ERROR("Unknown node key: " << key);
    }
    return targetNode;
}

void
Scenario::ConfigureEntities(const std::string& entityKey, NodeContainer& nodes)
{
    NS_LOG_FUNCTION(entityKey);

    const auto entityConfs = CONFIGURATOR->GetEntitiesConfiguration(entityKey);

    size_t entityId = 0;

    for (auto& entityConf : entityConfs)
    {
        size_t deviceId = 0;

        auto entityNode = nodes.Get(entityId);
        ConfigureEntityMobility(entityKey, entityConf, entityId);

        for (auto& entityNetDev : entityConf->GetNetDevices())
        {
            const auto netId = entityNetDev->GetNetworkLayerId();
            Ptr<NetDevice> installedDev = nullptr;

            if (entityNetDev->GetType() == "wifi")
            {
                NS_ASSERT_MSG(netId, "Wifi NetDevice must have a Network Layer ID");

                auto devContainer = ConfigureEntityWifiStack(entityKey,
                                                             entityNetDev,
                                                             entityNode,
                                                             entityId,
                                                             deviceId,
                                                             *netId);
                installedDev = devContainer.Get(0);
                InstallEntityIpv4(entityNode, devContainer, *netId);
                ConfigureEntityIpv4(entityNode, devContainer, deviceId, *netId);
            }
            else if (entityNetDev->GetType() == "lte")
            {
                NS_ASSERT_MSG(netId, "LTE NetDevice must have a Network Layer ID");

                const auto entityLteDevConf =
                    StaticCast<LteNetdeviceConfiguration, NetdeviceConfiguration>(entityNetDev);
                const auto role = entityLteDevConf->GetRole();
                const auto antennaModels = entityLteDevConf->GetAntennaModels();
                const auto phyConf = entityLteDevConf->GetPhyModel();

                switch (role)
                {
                case eNB:
                    installedDev = ConfigureLteEnb(entityNode, *netId, antennaModels, phyConf);
                    break;
                case UE:
                    installedDev = ConfigureLteUe(entityNode,
                                                  entityLteDevConf->GetBearers(),
                                                  *netId,
                                                  antennaModels,
                                                  phyConf);
                    break;
                default:
                    NS_FATAL_ERROR("Unrecognized LTE role for entity ID " << entityId);
                }
            }
            else if (entityNetDev->GetType() == "nr")
            {
                NS_ASSERT_MSG(netId, "NR NetDevice must have a Network Layer ID");

                const auto entityNrDevConf =
                    StaticCast<NrNetdeviceConfiguration, NetdeviceConfiguration>(entityNetDev);
                const auto role = entityNrDevConf->GetRole();
                const auto antennaModels = entityNrDevConf->GetAntennaModels();
                const auto phyConf = entityNrDevConf->GetPhyProperties();
                const auto rrcConf = entityNrDevConf->GetRrcProperties();
                const auto outputLinks = entityNrDevConf->GetOutputLinks();
                const auto x2Neighbors = entityNrDevConf->GetX2Neighbors();

                switch (role)
                {
                case NrRole::gNB:
                    installedDev = ConfigureNrGnb(entityNode,
                                                  *netId,
                                                  antennaModels,
                                                  phyConf,
                                                  rrcConf,
                                                  outputLinks,
                                                  x2Neighbors,
                                                  entityNrDevConf->GetChannelId(),
                                                  entityNrDevConf->GetChannelBands());
                    break;
                case NrRole::nrUE:
                    installedDev = ConfigureNrUe(entityNode,
                                                 entityNrDevConf->GetQosFlows(),
                                                 *netId,
                                                 antennaModels,
                                                 phyConf,
                                                 rrcConf,
                                                 outputLinks,
                                                 entityNrDevConf->GetChannelId(),
                                                 entityNrDevConf->GetChannelBands());
                    break;
                default:
                    NS_FATAL_ERROR("Unrecognized NR role for entity ID " << entityId);
                }
            }
            else if (entityNetDev->GetType() == "simple")
            {
                auto netDevice = CreateObject<SimpleNetDevice>();
                const auto antennaModels = entityNetDev->GetAntennaModels();

                if (!antennaModels.empty())
                {
                    const auto antennaConf = antennaModels.front().model;
                    ObjectFactory factory;
                    factory.SetTypeId(antennaConf.GetName());
                    for (auto& attr : antennaConf.GetAttributes())
                    {
                        factory.Set(attr.name, *attr.value);
                    }

                    auto antenna = factory.Create();
                    netDevice->AggregateObject(antenna);
                }

                entityNode->AddDevice(netDevice);
                netDevice->SetNode(entityNode);
                installedDev = netDevice;
            }
            else
            {
                NS_FATAL_ERROR(
                    "Unsupported Drone Network Device Type: " << entityNetDev->GetType());
            }

            auto dirConfigs = entityNetDev->GetDirectivity();
            for (const auto& dirConfig : dirConfigs)
            {
                NS_LOG_INFO("Configuring Directivity for Entity " << entityId << " Device "
                                                                  << deviceId);
                // Schedule initial directivity update
                auto mob = entityNode->GetObject<MobilityModel>();
                if (mob && installedDev)
                {
                    Scenario::UpdateAntennaDirectivity(installedDev,
                                                       dirConfig,
                                                       entityNetDev->GetType(),
                                                       entityNetDev->GetNetworkLayerId(),
                                                       mob);
                }
            }

            ++deviceId;
        }

        if (entityKey == "drones")
        {
            DroneEnergyModelHelper energyModel;
            Ptr<energy::EnergySource> energySource;

            ConfigureEntityMechanics(entityKey, entityConf, entityId);
            energySource = ConfigureEntityBattery(entityKey, entityConf, entityId);
            /// Installing Energy Model on Drone
            energyModel.Install(StaticCast<Drone, Node>(entityNode), energySource);
            ConfigureEntityPeripherals(entityKey, entityConf, entityId);
        }

        ++entityId;
    }

    BuildingsHelper::Install(nodes);
}

NetDeviceContainer
Scenario::ConfigureEntityWifiStack(const std::string entityKey,
                                   Ptr<NetdeviceConfiguration> entityNetDev,
                                   Ptr<Node> entityNode,
                                   const uint32_t entityId,
                                   const uint32_t deviceId,
                                   const uint32_t netId)
{
    NS_LOG_FUNCTION(entityNetDev << entityNode << entityId << deviceId << netId);

    auto wifiPhy = StaticCast<WifiPhySimulationHelper, Object>(m_protocolStacks[PHY_LAYER][netId]);
    auto wifiMac = StaticCast<WifiMacSimulationHelper, Object>(m_protocolStacks[MAC_LAYER][netId]);
    auto wifiNetDev = StaticCast<WifiNetdeviceConfiguration, NetdeviceConfiguration>(entityNetDev);
    auto wifiMacAttrs = wifiNetDev->GetMacLayer().GetAttributes();

    if (wifiMacAttrs.size() == 0)
    {
        wifiMac->GetMacHelper().SetType(wifiNetDev->GetMacLayer().GetName());
    }
    else if (wifiMacAttrs.size() == 1)
    {
        wifiMac->GetMacHelper().SetType(wifiNetDev->GetMacLayer().GetName(),
                                        wifiMacAttrs[0].name,
                                        *wifiMacAttrs[0].value);
    }

    NetDeviceContainer devContainer =
        wifiPhy->GetWifiHelper()->Install(*wifiPhy->GetWifiPhyHelper(),
                                          wifiMac->GetMacHelper(),
                                          entityNode);

    if (CONFIGURATOR->GetLogOnFile())
    {
        // Configure WiFi PHY Logging
        std::stringstream phyTraceLog;
        std::stringstream pcapLog;
        AsciiTraceHelper ascii;

        // Configure WiFi TXT PHY Logging
        phyTraceLog << CONFIGURATOR->GetResultsPath() << "wifi-phy-" << netId << "-" << entityKey
                    << "-host-" << entityId << "-" << deviceId << ".log";
        wifiPhy->GetWifiPhyHelper()->EnableAscii(ascii.CreateFileStream(phyTraceLog.str()),
                                                 entityNode->GetId(),
                                                 devContainer.Get(0)->GetIfIndex());

        // Configure WiFi PCAP Logging
        if (CONFIGURATOR->GetPcapLog())
        {
            pcapLog << CONFIGURATOR->GetResultsPath() << "wifi-phy-" << netId << "-" << entityKey
                    << "-host";
            wifiPhy->GetWifiPhyHelper()->EnablePcap(pcapLog.str(),
                                                    entityNode->GetId(),
                                                    devContainer.Get(0)->GetIfIndex());
        }
    }

    return devContainer;
}



void
Scenario::ConfigureEntityMechanics(const std::string& entityKey,
                                   Ptr<EntityConfiguration> entityConf,
                                   const uint32_t entityId)
{
    NS_LOG_FUNCTION_NOARGS();
    const auto mechanics = entityConf->GetMechanics();
    for (auto attr : mechanics.GetAttributes())
    {
        m_drones.Get(entityId)->SetAttribute(attr.name, *attr.value);
    }
}

Ptr<energy::EnergySource>
Scenario::ConfigureEntityBattery(const std::string& entityKey,
                                 Ptr<EntityConfiguration> entityConf,
                                 const uint32_t entityId)
{
    NS_LOG_FUNCTION_NOARGS();
    const auto battery = entityConf->GetBattery();
    ObjectFactory batteryFactory;
    batteryFactory.SetTypeId(entityConf->GetBattery().GetName());

    for (auto attr : battery.GetAttributes())
    {
        batteryFactory.Set(attr.name, *attr.value);
    }
    auto mountedBattery = batteryFactory.Create<energy::EnergySource>();

    mountedBattery->SetNode(m_drones.Get(entityId));
    m_drones.Get(entityId)->AggregateObject(mountedBattery);
    return mountedBattery;
}

void
Scenario::ConfigureEntityPeripherals(const std::string& entityKey,
                                     const Ptr<EntityConfiguration>& conf,
                                     const uint32_t& entityId)
{
    NS_LOG_FUNCTION(entityKey << entityId << conf);
    auto dronePeripheralsContainer = m_drones.Get(entityId)->GetPeripherals();

    if (conf->GetPeripherals().size() == 0)
    {
        return;
    }

    ObjectFactory factory;

    for (const auto& perConf : conf->GetPeripherals())
    {
        NS_LOG_INFO("Configuring peripheral " << perConf.GetName());
        dronePeripheralsContainer->Add(perConf.GetName());
        for (auto attr : perConf.GetAttributes())
        {
            dronePeripheralsContainer->Set(attr.name, *attr.value);
        }

        NS_LOG_INFO("Peripheral configured");
        auto peripheral = dronePeripheralsContainer->Create();

        for (auto aggIt = perConf.AggregatesBegin(); aggIt != perConf.AggregatesEnd(); aggIt++)
        {
            NS_LOG_INFO("Aggregating " << aggIt->GetName() << " to "
                                       << peripheral->GetTypeId().GetName());
            factory = ObjectFactory{aggIt->GetName()};

            for (auto attr : aggIt->GetAttributes())
            {
                factory.Set(attr.name, *attr.value);
            }

            auto aggObject = factory.Create<Object>();
            peripheral->AggregateObject(aggObject);
        }

        peripheral->Initialize();

        for (uint32_t i = 0; i < (uint32_t)peripheral->GetNRoI(); i++)
        {
            auto reg = irc->GetRoI(i);
            if (!irc->GetRoI(i))
            {
                NS_FATAL_ERROR("Region of Interest #" << i << " does not exist.");
            }
        }
    }
    dronePeripheralsContainer->InstallAll(m_drones.Get(entityId));
}

void
Scenario::ConfigureRegionsOfInterest()
{
    const auto regions = CONFIGURATOR->GetRegionsOfInterest();
    Ptr<InterestRegion> reg;
    for (const auto& region : regions)
    {
        reg = irc->Create(region);
    }
}

void
Scenario::ConfigureAllApplications()
{
    NS_LOG_FUNCTION(this);
    
    std::vector<std::string> keys = {"drones", "ZSPs", "vehicles", "nodes", "leo-sats"};
    for (const auto& entityKey : keys)
    {
        uint32_t entityId = 0;
        const auto entities = CONFIGURATOR->GetEntitiesConfiguration(entityKey);
        
        for (const auto& conf : entities)
        {
            Ptr<Node> targetNode;
            if (entityKey == "drones") targetNode = m_drones.Get(entityId);
            else if (entityKey == "ZSPs") targetNode = m_zsps.Get(entityId);
            else if (entityKey == "remotes") targetNode = m_remoteNodes.Get(entityId);
            else if (entityKey == "vehicles") targetNode = m_vehicles.Get(entityId);
            else if (entityKey == "nodes") targetNode = m_plainNodes.Get(entityId);
            else if (entityKey == "leo-sats") targetNode = m_leoSats.Get(entityId);

            if (targetNode)
            {
                InstallApplications(conf->GetApplications(), targetNode);
            }
            entityId++;
        }
    }

    uint32_t remoteId = 0;
    const auto remotes = CONFIGURATOR->GetRemotesConfiguration();
    for (const auto& conf : remotes)
    {
        Ptr<Node> targetNode = m_remoteNodes.Get(remoteId);
        if (targetNode)
        {
            InstallApplications(conf->GetApplications(), targetNode);
        }
        remoteId++;
    }
}

void
Scenario::InstallApplications(const std::vector<ModelConfiguration>& apps, const Ptr<Node>& targetNode)
{
    for (const auto& appConf : apps)
    {
        ObjectFactory factory;
        factory.SetTypeId(appConf.GetName());

        for (const auto& attr : appConf.GetAttributes())
        {
            factory.Set(attr.name, *attr.value);
        }

        Ptr<Application> application = factory.Create<Application>();

        for (const auto& defIp : appConf.GetDeferredIps())
        {
            // Resolve the actual IP address
            auto targetNetworkNode = GetNodeByKey(defIp.key, defIp.index);
            NS_ABORT_MSG_IF(!targetNetworkNode, "Target node not found for key: " << defIp.key << " index: " << defIp.index);

            uint32_t targetInterfaceIndex = 0;
            uint32_t deviceCount = 0;
            Address targetAddress;

            if (defIp.isIpv6)
            {
                auto targetIpv6 = targetNetworkNode->GetObject<Ipv6>();
                NS_ABORT_MSG_IF(!targetIpv6, "Target node for IPv6 resolution does not have an Ipv6 object.");

                for (uint32_t i = 0; i < targetIpv6->GetNInterfaces(); ++i)
                {
                    if (targetIpv6->GetNetDevice(i)->GetInstanceTypeId().GetName() == "ns3::LoopbackNetDevice")
                    {
                        continue;
                    }

                    if (deviceCount == defIp.device)
                    {
                        targetInterfaceIndex = i;
                        break;
                    }
                    deviceCount++;
                }

                NS_ABORT_MSG_IF(targetInterfaceIndex == 0 && defIp.device > 0,
                                "Target device index " << defIp.device << " out of bounds for IPv6.");
                
                // Usually address 0 is loopback, address 1 is link-local, address 2 is global for IPv6. We use the global or link-local address.
                // We'll get address 1 if it exists as it is typically the valid one for the interface.
                uint32_t addressIndex;
                if (defIp.addressIndex.has_value())
                {
                    addressIndex = defIp.addressIndex.value();
                    NS_ABORT_MSG_IF(addressIndex >= targetIpv6->GetNAddresses(targetInterfaceIndex),
                                    "IPv6 addressIndex " << addressIndex << " out of bounds.");
                }
                else
                {
                    addressIndex = targetIpv6->GetNAddresses(targetInterfaceIndex) > 1 ? 1 : 0;
                }
                targetAddress = targetIpv6->GetAddress(targetInterfaceIndex, addressIndex).GetAddress();
            }
            else
            {
                auto targetIpv4 = targetNetworkNode->GetObject<Ipv4>();
                NS_ABORT_MSG_IF(!targetIpv4, "Target node for IPv4 resolution does not have an Ipv4 object.");

                for (uint32_t i = 0; i < targetIpv4->GetNInterfaces(); ++i)
                {
                    if (targetIpv4->GetNetDevice(i)->GetInstanceTypeId().GetName() == "ns3::LoopbackNetDevice")
                    {
                        continue;
                    }

                    if (deviceCount == defIp.device)
                    {
                        targetInterfaceIndex = i;
                        break;
                    }
                    deviceCount++;
                }

                NS_ABORT_MSG_IF(targetInterfaceIndex == 0 && defIp.device > 0,
                                "Target device index " << defIp.device << " out of bounds for IPv4.");

                uint32_t addressIndex;
                if (defIp.addressIndex.has_value())
                {
                    addressIndex = defIp.addressIndex.value();
                    NS_ABORT_MSG_IF(addressIndex >= targetIpv4->GetNAddresses(targetInterfaceIndex),
                                    "IPv4 addressIndex " << addressIndex << " out of bounds.");
                }
                else
                {
                    addressIndex = 0;
                }
                targetAddress = targetIpv4->GetAddress(targetInterfaceIndex, addressIndex).GetLocal();
            }

            auto targetPort = defIp.port.value_or(0);

            AddressValue resolvedAddressValue(
                addressUtils::ConvertToSocketAddress(targetAddress, targetPort));
            application->SetAttribute(defIp.attrName, resolvedAddressValue);
        }

        targetNode->AddApplication(application);
    }
}

} // namespace ns3
