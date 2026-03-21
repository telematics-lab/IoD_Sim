#include "scenario.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ScenarioMac");

void
Scenario::ConfigureMac()
{
    NS_LOG_FUNCTION_NOARGS();

    const auto macLayerConfs = CONFIGURATOR->GetMacLayers();

    size_t i = 0;
    for (auto& macLayerConf : macLayerConfs)
    {
        if (macLayerConf->GetType() == "wifi")
        {
            const auto wifiPhy =
                StaticCast<WifiPhySimulationHelper, Object>(m_protocolStacks[PHY_LAYER][i]);
            const auto wifiMac = CreateObject<WifiMacSimulationHelper>();
            const auto wifiConf =
                StaticCast<WifiMacLayerConfiguration, MacLayerConfiguration>(macLayerConf);
            Ssid ssid = Ssid(wifiConf->GetSsid());

            WifiMacFactoryHelper::SetRemoteStationManager(
                *(wifiPhy->GetWifiHelper()),
                wifiConf->GetRemoteStationManagerConfiguration());

            m_protocolStacks[MAC_LAYER].emplace_back(wifiMac);
        }
        else if (macLayerConf->GetType() == "lte")
        {
            // NO OPERATION NEEDED HERE
            m_protocolStacks[MAC_LAYER].emplace_back(nullptr);
        }
        else if (macLayerConf->GetType() == "nr")
        {
            // NO OPERATION NEEDED HERE
            m_protocolStacks[MAC_LAYER].emplace_back(nullptr);
        }
        else if (macLayerConf->GetType() == "NullNtnDemo")
        {
            const auto phy =
                StaticCast<ThreeGppPhySimulationHelper, Object>(m_protocolStacks[PHY_LAYER][i]);
            const auto macConf =
                StaticCast<NullNtnDemoMacLayerConfiguration, MacLayerConfiguration>(macLayerConf);
            const auto mac = CreateObject<NullNtnDemoMacLayerSimulationHelper>(macConf, phy);

            Simulator::ScheduleNow(&NullNtnDemoMacLayerSimulationHelper::Setup,
                                   mac,
                                   CONFIGURATOR->GetDuration());

            m_protocolStacks[MAC_LAYER].push_back(mac);
        }
        else
        {
            NS_FATAL_ERROR("Unsupported MAC Layer Type: " << macLayerConf->GetType());
        }

        ++i;
    }
}

void
Scenario::ConfigureScheduling()
{
    NS_LOG_FUNCTION_NOARGS();
    auto schedulingEvents = CONFIGURATOR->GetSchedulingEvents();
    if (schedulingEvents.empty())
    {
        return;
    }

    for (const auto& event : schedulingEvents)
    {
        if (event.action == "handover")
        {
            Ptr<Node> ueNode = GetNodeByKey(event.params.ue.key, event.params.ue.index);
            if (!ueNode)
            {
                NS_FATAL_ERROR("Could not find UE node for handover scheduling.");
                continue;
            }

            Ptr<NrUeNetDevice> ueDevice = nullptr;
            uint32_t ueNetId = 0;
            for (const auto& [netId, devices] : m_nrUeDevices)
            {
                for (const auto& dev : devices)
                {
                    if (dev->GetNode() == ueNode)
                    {
                        ueDevice = DynamicCast<NrUeNetDevice>(dev);
                        ueNetId = netId;
                        break;
                    }
                }
                if (ueDevice)
                {
                    break;
                }
            }

            if (!ueDevice)
            {
                NS_FATAL_ERROR("Could not find NR UE net device for handover scheduling.");
                continue;
            }

            uint16_t targetCellId = 0;
            Ptr<NrGnbNetDevice> targetGnb = nullptr;
            if (event.params.cellId.isDirectId)
            {
                auto it = m_nrGnbDevices.find(ueNetId);
                if (it != m_nrGnbDevices.end())
                {
                    for (const auto& containers : it->second)
                    {
                        for (uint32_t i = 0; i < containers.GetN(); ++i)
                        {
                            auto gnb = DynamicCast<NrGnbNetDevice>(containers.Get(i));
                            if (gnb)
                            {
                                if (gnb->GetCellId() == event.params.cellId.cellId ||
                                    gnb->GetRrc()->HasCellId(event.params.cellId.cellId))
                                {
                                    targetGnb = gnb;
                                    break;
                                }
                            }
                        }
                        if (targetGnb)
                        {
                            break;
                        }
                    }
                }
                if (targetGnb)
                {
                    targetCellId = event.params.cellId.cellId;
                }
            }
            else
            {
                Ptr<Node> gnbNode =
                    GetNodeByKey(event.params.cellId.key, event.params.cellId.index);
                if (gnbNode)
                {
                    if (event.params.cellId.hasDeviceIndex)
                    {
                        targetGnb = DynamicCast<NrGnbNetDevice>(
                            gnbNode->GetDevice(event.params.cellId.deviceIndex));
                    }
                    else
                    {
                        auto it = m_nrGnbDevices.find(ueNetId);
                        if (it != m_nrGnbDevices.end())
                        {
                            for (const auto& containers : it->second)
                            {
                                for (uint32_t i = 0; i < containers.GetN(); ++i)
                                {
                                    auto gnb = DynamicCast<NrGnbNetDevice>(containers.Get(i));
                                    if (gnb && gnb->GetNode() == gnbNode)
                                    {
                                        targetGnb = gnb;
                                        break;
                                    }
                                }
                                if (targetGnb)
                                {
                                    break;
                                }
                            }
                        }
                    }
                }
                if (targetGnb)
                {
                    targetCellId = targetGnb->GetCellId();
                }
            }

            if (!targetGnb)
            {
                NS_FATAL_ERROR("Could not find Target gNB net device for handover scheduling.");
                continue;
            }

            NS_LOG_INFO("Scheduling Handover for UE "
                        << ueNode->GetId() << " to gNB " << targetGnb->GetNode()->GetId()
                        << " cellId " << targetCellId << " at time " << event.time << "s");
            Simulator::Schedule(Seconds(event.time),
                                &Scenario::ExecuteHandoverRequest,
                                this,
                                ueDevice,
                                targetGnb,
                                targetCellId,
                                ueNetId);
        }
    }
}

void
Scenario::ExecuteHandoverRequest(Ptr<NrUeNetDevice> ueDevice,
                                 Ptr<NrGnbNetDevice> targetGnb,
                                 uint16_t targetCellId,
                                 uint32_t ueNetId)
{
    Ptr<NrGnbNetDevice> sourceGnb = nullptr;
    uint16_t currentCellId = ueDevice->GetRrc()->GetCellId();
    auto it = m_nrGnbDevices.find(ueNetId);
    if (it != m_nrGnbDevices.end())
    {
        for (const auto& containers : it->second)
        {
            for (uint32_t i = 0; i < containers.GetN(); ++i)
            {
                auto gnb = DynamicCast<NrGnbNetDevice>(containers.Get(i));
                if (gnb && gnb->GetCellId() == currentCellId)
                {
                    sourceGnb = gnb;
                    break;
                }
            }
            if (sourceGnb)
            {
                break;
            }
        }
    }

    if (!sourceGnb)
    {
        NS_LOG_WARN("ExecuteHandoverRequest: Could not find Source gNB net device.");
        return;
    }

    auto nrPhySim = StaticCast<NrPhySimulationHelper, Object>(m_protocolStacks[PHY_LAYER][ueNetId]);
    auto nrHelper = nrPhySim->GetNrHelper();

    NS_LOG_INFO("Executing Handover for UE "
                << ueDevice->GetNode()->GetId() << " from gNB " << sourceGnb->GetNode()->GetId()
                << " to gNB " << targetGnb->GetNode()->GetId() << " cellId " << targetCellId);

    nrHelper->HandoverRequest(Seconds(0), ueDevice, sourceGnb, targetCellId);
}

} // namespace ns3
