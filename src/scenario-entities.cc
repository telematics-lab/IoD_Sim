#include "scenario.h"

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
    else if (key == "remote-nodes")
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

    const auto entityConfs = CONFIGURATOR->GetEntitiesConfiguration(entityKey); // Modificare
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

        ConfigureEntityApplications(entityKey, entityConf, entityId);

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
        pcapLog << CONFIGURATOR->GetResultsPath() << "wifi-phy-" << netId << "-" << entityKey
                << "-host";
        wifiPhy->GetWifiPhyHelper()->EnablePcap(pcapLog.str(),
                                                entityNode->GetId(),
                                                devContainer.Get(0)->GetIfIndex());
    }

    return devContainer;
}

void
Scenario::ConfigureEntityApplications(const std::string& entityKey,
                                      const Ptr<EntityConfiguration>& conf,
                                      const uint32_t& entityId)
{
    NS_LOG_FUNCTION(entityKey << conf << entityId);

    for (const auto& appConf : conf->GetApplications())
    {
        ObjectFactory f{appConf.GetName()};

        for (auto attr : appConf.GetAttributes())
        {
            f.Set(attr.name, *attr.value);
        }

        auto app = StaticCast<Application, Object>(f.Create());

        if (entityKey == "drones")
        {
            m_drones.Get(entityId)->AddApplication(app);
        }
        else if (entityKey == "ZSPs")
        {
            m_zsps.Get(entityId)->AddApplication(app);
        }
        else if (entityKey == "nodes")
        {
            m_plainNodes.Get(entityId)->AddApplication(app);
        }
        else if (entityKey == "leo-sats")
        {
            m_leoSats.Get(entityId)->AddApplication(app);
        }
        else if (entityKey == "vehicles")
        {
            m_vehicles.Get(entityId)->AddApplication(app);
        }
        else
        {
            NS_FATAL_ERROR("Unsupported Entity Type " << entityKey);
        }
    }
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

} // namespace ns3
