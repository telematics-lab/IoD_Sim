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
#include "ns3/ideal-beamforming-algorithm.h"
#include <ns3/app-statistics-helper.h>
#include <ns3/buildings-helper.h>
#include <ns3/config.h>
#include <ns3/csma-module.h>
#include <ns3/debug-helper.h>
#include <ns3/drone-client-application.h>
#include <ns3/drone-container.h>
#include <ns3/drone-energy-model-helper.h>
#include <ns3/drone-list.h>
#include <ns3/drone-peripheral-container.h>
#include <ns3/drone-peripheral.h>
#include <ns3/drone-server-application.h>
#include <ns3/energy-source.h>
#include <ns3/geocentric-mobility-model.h>
#include <ns3/ideal-beamforming-helper.h>
#include <ns3/input-peripheral.h>
#include <ns3/interest-region-container.h>
#include <ns3/internet-module.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/ipv4-network-layer-configuration.h>
#include <ns3/ipv4-routing-helper.h>
#include <ns3/ipv4-simulation-helper.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/leo-sat-list.h>
#include <ns3/log.h>
#include <ns3/lte-enb-net-device.h>
#include <ns3/lte-netdevice-configuration.h>
#include <ns3/lte-phy-layer-configuration.h>
#include <ns3/lte-phy-simulation-helper.h>
#include <ns3/lte-setup-helper.h>
#include <ns3/lte-ue-net-device.h>
#include <ns3/mobility-factory-helper.h>
#include <ns3/mobility-helper.h>
#include <ns3/nat-application.h>
#include <ns3/net-device-container.h>
#include <ns3/node-list.h>
#include <ns3/none-phy-layer-configuration.h>
#include <ns3/nr-bearer-configuration.h>
#include <ns3/nr-bearer-stats-calculator.h>
#include <ns3/nr-eps-bearer.h>
#include <ns3/nr-gnb-net-device.h>
#include <ns3/nr-helper.h>
#include <ns3/nr-netdevice-configuration.h>
#include <ns3/nr-phy-layer-configuration.h>
#include <ns3/nr-phy-rx-trace.h>
#include <ns3/nr-phy-simulation-helper.h>
#include <ns3/nr-ue-net-device.h>
#include <ns3/nr-ue-phy.h>
#include <ns3/null-ntn-demo-mac-layer-configuration.h>
#include <ns3/null-ntn-demo-mac-layer-simulation-helper.h>
#include <ns3/object-factory.h>
#include <ns3/ptr.h>
#include <ns3/radio-environment-map-helper.h>
#include <ns3/remote-list.h>
#include <ns3/report.h>
#include <ns3/rng-seed-manager.h>
#include <ns3/scenario-configuration-helper.h>
#include <ns3/show-progress.h>
#include <ns3/simple-net-device.h>
#include <ns3/ssid.h>
#include <ns3/string.h>
#include <ns3/three-dimensional-rem-helper.h>
#include <ns3/three-gpp-phy-layer-configuration.h>
#include <ns3/three-gpp-phy-simulation-helper.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/traced-value.h>
#include <ns3/wifi-mac-factory-helper.h>
#include <ns3/wifi-mac-layer-configuration.h>
#include <ns3/wifi-mac-simulation-helper.h>
#include <ns3/wifi-netdevice-configuration.h>
#include <ns3/wifi-phy-factory-helper.h>
#include <ns3/wifi-phy-layer-configuration.h>
#include <ns3/wifi-phy-simulation-helper.h>
#include <ns3/zsp-list.h>

#include <algorithm>
#include <filesystem>
#include <map>
#include <sys/resource.h>
#include <vector>

namespace ns3
{

constexpr int N_LAYERS = 4;
constexpr int PHY_LAYER = 0;
constexpr int MAC_LAYER = 1;
constexpr int NET_LAYER = 2;
// constexpr int APP_LAYER = 3;
constexpr int PROGRESS_REFRESH_INTERVAL_SECONDS = 1;

class Scenario
{
  public:
    Scenario(int argc, char** argv);
    virtual ~Scenario();

    void operator()();

  private:
    void GenerateRadioMap();
    void GenerateThreeDimensionalRem();
    void ApplyStaticConfig();
    void ConfigureWorld();
    void ConfigurePhy();
    void ConfigureMac();
    void ConfigureNetwork();
    void EnableIpv4RoutingTableReporting();
    void ConfigureEntities(const std::string& entityKey, NodeContainer& nodes);
    void ConfigureEntityMobility(const std::string& entityKey,
                                 Ptr<EntityConfiguration> entityConf,
                                 const uint32_t entityId);
    NetDeviceContainer ConfigureEntityWifiStack(std::string entityKey,
                                                Ptr<NetdeviceConfiguration> entityNetDev,
                                                Ptr<Node> entityNode,
                                                const uint32_t entityId,
                                                const uint32_t deviceId,
                                                const uint32_t netId);
    void ConfigureLteEnb(Ptr<Node> entityNode,
                         const uint32_t netId,
                         const std::optional<ModelConfiguration> antennaModel,
                         const std::optional<ModelConfiguration> phyConf);
    void ConfigureLteUe(Ptr<Node> entityNode,
                        const std::vector<LteBearerConfiguration> bearers,
                        const uint32_t netId,
                        const std::optional<ModelConfiguration> antennaModel,
                        const std::optional<ModelConfiguration> phyConf);

    void ConfigureNrGnb(Ptr<Node> entityNode,
                        const uint32_t netId,
                        const std::optional<ModelConfiguration> antennaModel,
                        const std::vector<ns3::NrPhyProperty> phyConf);

    void ConfigureNrUe(Ptr<Node> entityNode,
                       const std::vector<NrBearerConfiguration> bearers,
                       const uint32_t netId,
                       const std::optional<ModelConfiguration> antennaModel,
                       const std::vector<ns3::NrPhyProperty> phyConf);

    void InstallEntityIpv4(Ptr<Node> entityNode,
                           NetDeviceContainer netDevices,
                           const uint32_t netId);
    void InstallEntityIpv4(Ptr<Node> entityNode, Ptr<NetDevice> netDevice, const uint32_t netId);
    void ConfigureEntityIpv4(Ptr<Node> entityNode,
                             NetDeviceContainer devContainer,
                             const uint32_t deviceId,
                             const uint32_t netId);
    void ConfigureEntityApplications(const std::string& entityKey,
                                     const Ptr<EntityConfiguration>& conf,
                                     const uint32_t& entityId);
    void ConfigureEntityMechanics(const std::string& entityKey,
                                  Ptr<EntityConfiguration> entityConf,
                                  const uint32_t entityId);
    Ptr<energy::EnergySource> ConfigureEntityBattery(const std::string& entityKey,
                                                     Ptr<EntityConfiguration> entityConf,
                                                     const uint32_t entityId);
    void ConfigureEntityPeripherals(const std::string& entityKey,
                                    const Ptr<EntityConfiguration>& conf,
                                    const uint32_t& entityId);
    void ConfigureInternetRemotes();
    void ConfigureInternetBackbone();
    void EnablePhyLteTraces();
    void EnablePhyNrTraces();
    void ConfigureRegionsOfInterest();
    void DroneCourseChange(std::string context, Ptr<const MobilityModel> model);
    void LeoSatCourseChange(std::string context, Ptr<const MobilityModel> model);
    void VehicleCourseChange(std::string context, Ptr<const MobilityModel> model);
    void ConfigureSimulator();
    void AttachAllNrUesToClosestGnb(const uint32_t netId);

    NodeContainer m_plainNodes;
    DroneContainer m_drones;
    NodeContainer m_zsps;
    NodeContainer m_remoteNodes;
    NodeContainer m_backbone;
    NodeContainer m_leoSats;
    NodeContainer m_vehicles;

    std::array<std::vector<Ptr<Object>>, N_LAYERS> m_protocolStacks;
    Ptr<OutputStreamWrapper> m_leoSatTraceStream;
    Ptr<OutputStreamWrapper> m_vehicleTraceStream;

    // NR gNB and UE tracking for proper attachment
    std::map<uint32_t, std::vector<NetDeviceContainer>> m_nrGnbDevices;
    std::map<uint32_t, std::vector<Ptr<NetDevice>>> m_nrUeDevices;

    // Application statistics helper
    AppStatisticsHelper m_appStatsHelper;
};

NS_LOG_COMPONENT_DEFINE("Scenario");

Scenario::Scenario(int argc, char** argv)
{
    RngSeedManager::SetRun(1);
    RngSeedManager::SetSeed(33);
    Config::SetDefault("ns3::RandomVariableStream::Stream", IntegerValue(1));
    Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod",
                       TimeValue(MilliSeconds(1))); // update the channel at each iteration
    Config::SetDefault("ns3::ThreeGppChannelConditionModel::UpdatePeriod",
                       TimeValue(MilliSeconds(1))); // do not update the channel condition

    CONFIGURATOR->Initialize(argc, argv);
    m_plainNodes.Create(CONFIGURATOR->GetN("nodes"));
    m_drones.Create(CONFIGURATOR->GetN("drones"));
    m_zsps.Create(CONFIGURATOR->GetN("ZSPs"));
    m_remoteNodes.Create(CONFIGURATOR->GetN("remotes"));
    m_leoSats.Create(CONFIGURATOR->GetN("leo-sats"));
    m_vehicles.Create(CONFIGURATOR->GetN("vehicles"));
    m_backbone.Add(m_remoteNodes);

    // Register created entities in their lists
    for (auto drone = m_drones.Begin(); drone != m_drones.End(); drone++)
    {
        DroneList::Add(*drone);
    }

    for (auto zsp = m_zsps.Begin(); zsp != m_zsps.End(); zsp++)
    {
        ZspList::Add(*zsp);
    }

    for (auto remote = m_remoteNodes.Begin(); remote != m_remoteNodes.End(); remote++)
    {
        RemoteList::Add(*remote);
    }

    for (auto leoSat = m_leoSats.Begin(); leoSat != m_leoSats.End(); leoSat++)
    {
        LeoSatList::Add(*leoSat);
    }

    // Initialize NR device tracking maps
    m_nrGnbDevices.clear();
    m_nrUeDevices.clear();

    ApplyStaticConfig();
    ConfigureWorld();
    ConfigurePhy();
    ConfigureMac();
    ConfigureNetwork();
    ConfigureRegionsOfInterest();
    ConfigureEntities("nodes", m_plainNodes);
    ConfigureEntities("drones", m_drones);
    ConfigureEntities("ZSPs", m_zsps);
    ConfigureEntities("leo-sats", m_leoSats);
    ConfigureEntities("vehicles", m_vehicles);
    ConfigureInternetBackbone();
    ConfigureInternetRemotes();
    EnablePhyLteTraces();
    EnablePhyNrTraces();

    // Configure application statistics helper
    m_appStatsHelper.SetOutputPath(CONFIGURATOR->GetResultsPath() + "app-statistics.txt");
    m_appStatsHelper.SetReportingInterval(Seconds(CONFIGURATOR->GetAppStatisticsReportInterval()));
    m_appStatsHelper.InstallFlowMonitor(NodeContainer::GetGlobal()); // Install on all nodes

    // Initialize LeoSat trace CSV file
    std::ostringstream leoSatTraceFilePath;
    leoSatTraceFilePath << CONFIGURATOR->GetResultsPath() << "leo-sat-trace.csv";
    m_leoSatTraceStream = Create<OutputStreamWrapper>(leoSatTraceFilePath.str(), std::ios::out);
    *m_leoSatTraceStream->GetStream() << "Time,Node,X,Y,Z,Latitude,Longitude,Altitude" << std::endl;
    // Inizialize Vehicles trace CSV file
    std::ostringstream vehicleTraceFilePath;
    vehicleTraceFilePath << CONFIGURATOR->GetResultsPath() << "vehicle-trace.csv";
    m_vehicleTraceStream = Create<OutputStreamWrapper>(vehicleTraceFilePath.str(), std::ios::out);
    *m_vehicleTraceStream->GetStream()
        << "Time,Node,X,Y,Z,Latitude,Longitude,Altitude" << std::endl;

    // DebugHelper::ProbeNodes();
    ConfigureSimulator();
}

Scenario::~Scenario()
{
}

void
Scenario::operator()()
{
    NS_LOG_FUNCTION_NOARGS();
    bool anyLte = false;
    const auto phyLayerConfs = CONFIGURATOR->GetPhyLayers();
    for (auto& phyLayerConf : phyLayerConfs)
    {
        if (phyLayerConf->GetType() == "lte")
        {
            anyLte = true;
        }
    }

    if (CONFIGURATOR->RadioMap() > 0)
    {
        NS_ASSERT_MSG(anyLte,
                      "Environment Radio Map can be generated only if an LTE network is "
                      "present. Aborting simulation.");
        NS_LOG_INFO("Generating Environment Radio Map, simulation will not run.");
        if (CONFIGURATOR->RadioMap() == 2)
        {
            this->GenerateThreeDimensionalRem();
            Simulator::Run();
            Simulator::Destroy();
            int plyRet = system(("python ../analysis/txt2ply.py " + CONFIGURATOR->GetResultsPath() +
                                 CONFIGURATOR->GetName() + "-3D-REM.txt")
                                    .c_str());
            if (plyRet == -1)
            {
                NS_ABORT_MSG("Something went wrong while generating the ply file");
            }
            int plotRet =
                system(("python ../analysis/rem-3d-preview.py " + CONFIGURATOR->GetResultsPath() +
                        CONFIGURATOR->GetName() + "-3D-REM.ply")
                           .c_str());
            if (plotRet != 0)
            {
                NS_ABORT_MSG("Something went wrong while generating the plot");
            }
        }
        else if (CONFIGURATOR->RadioMap() == 1)
        {
            this->GenerateRadioMap();
            Simulator::Run();
            Simulator::Destroy();
            int plyRet = system(("python ../analysis/txt2ply.py " + CONFIGURATOR->GetResultsPath() +
                                 CONFIGURATOR->GetName() + "-2D-REM.txt")
                                    .c_str());
            if (plyRet != 0)
            {
                NS_ABORT_MSG("Something went wrong while generating the ply file");
            }
            int plotRet =
                system(("python ../analysis/rem-2d-preview.py " + CONFIGURATOR->GetResultsPath() +
                        CONFIGURATOR->GetName() + "-2D-REM.txt")
                           .c_str());
            if (plotRet != 0)
            {
                NS_ABORT_MSG("Something went wrong while generating the plot");
            }
        }
    }
    else
    {
        if (CONFIGURATOR->IsDryRun())
        {
            return;
        }

        std::stringstream progressLogFilePath;
        progressLogFilePath << CONFIGURATOR->GetResultsPath() << "progress.log";
        auto progressLogSink =
            Create<OutputStreamWrapper>(progressLogFilePath.str(), std::ios::out);

        ShowProgress progressLog{Seconds(PROGRESS_REFRESH_INTERVAL_SECONDS),
                                 (*progressLogSink->GetStream())};
        ShowProgress progressStdout{Seconds(PROGRESS_REFRESH_INTERVAL_SECONDS), std::cout};

        Simulator::Run();

        // Stop UDP statistics collection
        m_appStatsHelper.Stop();

        if (CONFIGURATOR->GetLogOnFile())
        {
            // Report Module needs the simulator context alive to introspect it
            Report::Get()->Save();
        }
        Simulator::Destroy();
    }
}

void
Scenario::GenerateRadioMap()
{
    // Making it static in order for it to be alive when simulation run
    static Ptr<RadioEnvironmentMapHelper> m_remHelper = CreateObject<RadioEnvironmentMapHelper>();
    m_remHelper->SetAttribute(
        "OutputFile",
        StringValue(CONFIGURATOR->GetResultsPath() + CONFIGURATOR->GetName() + "-2D-REM.txt"));

    auto parameters = CONFIGURATOR->GetRadioMapParameters();
    for (auto par : parameters)
    {
        m_remHelper->SetAttribute(par.first, StringValue(par.second));
    }

    m_remHelper->Install();
}

void
Scenario::GenerateThreeDimensionalRem()
{
    // Making it static in order for it to be alive when simulation run
    static Ptr<ThreeDimensionalRemHelper> m_remHelper = CreateObject<ThreeDimensionalRemHelper>();
    m_remHelper->SetAttribute(
        "OutputFile",
        StringValue(CONFIGURATOR->GetResultsPath() + CONFIGURATOR->GetName() + "-3D-REM.txt"));

    auto parameters = CONFIGURATOR->GetRadioMapParameters();
    for (auto par : parameters)
    {
        m_remHelper->SetAttribute(par.first, StringValue(par.second));
    }

    m_remHelper->Install();
}

void
Scenario::ApplyStaticConfig()
{
    NS_LOG_FUNCTION_NOARGS();

    for (auto& param : CONFIGURATOR->GetStaticConfig())
    {
        Config::SetDefault(param.first, *param.second);
    }
}

void
Scenario::ConfigureWorld()
{
    NS_LOG_FUNCTION_NOARGS();

    CONFIGURATOR->GetBuildings(); // buildings created here are automatically added to BuildingsList
}

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

            nrSim->SetEpcHelper(nrConf->GetEpcHelperType(), nrConf->GetEpcAttributes());
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

            for (auto bandConf : nrConf->GetBandsConfiguration())
            {
                std::vector<CcBwpCreator::SimpleOperationBandConf> freqs;
                for (auto& attr : bandConf.frequencyBands)
                {
                    auto conf = CcBwpCreator::SimpleOperationBandConf(attr.centralFrequency,
                                                                      attr.bandwidth,
                                                                      attr.numComponentCarriers);
                    conf.m_numBwp = attr.numBandwidthParts;
                    freqs.push_back(conf);
                }
                nrSim->CreateOperationBand(freqs,
                                           bandConf.channel.scenario,
                                           bandConf.channel.conditionModel,
                                           bandConf.channel.propagationModel,
                                           bandConf.contiguousCc,
                                           bandConf.channelConditionAttributes,
                                           bandConf.pathlossAttributes,
                                           bandConf.phasedSpectrumAttributes,
                                           bandConf.channel.configFlags);
            }

            nrSim->SetScheduler(nrConf->GetSchedulerType());

            auto gnbAntennaConf = nrConf->GetGnbAntenna();
            nrSim->SetGnbAntenna(gnbAntennaConf.type,
                                 gnbAntennaConf.properties,
                                 gnbAntennaConf.arrayProperties);
            auto ueAntennaConf = nrConf->GetUeAntenna();
            nrSim->SetUeAntenna(ueAntennaConf.type,
                                ueAntennaConf.properties,
                                ueAntennaConf.arrayProperties);

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

            m_protocolStacks[MAC_LAYER].push_back(wifiMac);
        }
        else if (macLayerConf->GetType() == "lte")
        {
            // NO OPERATION NEEDED HERE
            m_protocolStacks[MAC_LAYER].push_back(nullptr);
        }
        else if (macLayerConf->GetType() == "nr")
        {
            // NO OPERATION NEEDED HERE
            m_protocolStacks[MAC_LAYER].push_back(nullptr);
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

            if (entityNetDev->GetType() == "wifi")
            {
                NS_ASSERT_MSG(netId, "Wifi NetDevice must have a Network Layer ID");

                auto devContainer = ConfigureEntityWifiStack(entityKey,
                                                             entityNetDev,
                                                             entityNode,
                                                             entityId,
                                                             deviceId,
                                                             *netId);
                InstallEntityIpv4(entityNode, devContainer, *netId);
                ConfigureEntityIpv4(entityNode, devContainer, deviceId, *netId);
            }
            else if (entityNetDev->GetType() == "lte")
            {
                NS_ASSERT_MSG(netId, "LTE NetDevice must have a Network Layer ID");

                const auto entityLteDevConf =
                    StaticCast<LteNetdeviceConfiguration, NetdeviceConfiguration>(entityNetDev);
                const auto role = entityLteDevConf->GetRole();
                const auto antennaModel = entityLteDevConf->GetAntennaModel();
                const auto phyConf = entityLteDevConf->GetPhyModel();

                switch (role)
                {
                case eNB:
                    ConfigureLteEnb(entityNode, *netId, antennaModel, phyConf);
                    break;
                case UE:
                    ConfigureLteUe(entityNode,
                                   entityLteDevConf->GetBearers(),
                                   *netId,
                                   antennaModel,
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
                const auto antennaModel = entityNrDevConf->GetAntennaModel();
                const auto phyConf = entityNrDevConf->GetPhyProperties();

                switch (role)
                {
                case NrRole::gNB:
                    ConfigureNrGnb(entityNode, *netId, antennaModel, phyConf);
                    break;
                case NrRole::nrUE:
                    ConfigureNrUe(entityNode,
                                  entityNrDevConf->GetBearers(),
                                  *netId,
                                  antennaModel,
                                  phyConf);
                    break;
                default:
                    NS_FATAL_ERROR("Unrecognized NR role for entity ID " << entityId);
                }
            }
            else if (entityNetDev->GetType() == "simple")
            {
                auto netDevice = CreateObject<SimpleNetDevice>();
                const auto antennaConf = entityNetDev->GetAntennaModel();

                if (antennaConf)
                {
                    ObjectFactory factory;
                    factory.SetTypeId(antennaConf->GetName());
                    for (auto& attr : antennaConf->GetAttributes())
                    {
                        factory.Set(attr.name, *attr.value);
                    }

                    auto antenna = factory.Create();
                    netDevice->AggregateObject(antenna);
                }

                entityNode->AddDevice(netDevice);
                netDevice->SetNode(entityNode);
            }
            else
            {
                NS_FATAL_ERROR(
                    "Unsupported Drone Network Device Type: " << entityNetDev->GetType());
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

void
Scenario::ConfigureEntityMobility(const std::string& entityKey,
                                  Ptr<EntityConfiguration> entityConf,
                                  const uint32_t entityId)
{
    NS_LOG_FUNCTION(entityKey << entityConf << entityId);

    MobilityHelper mobility;
    const auto mobilityConf = entityConf->GetMobilityModel();
    const auto mobilityType = mobilityConf.GetName(); // Configure Entity Mobility
    MobilityFactoryHelper::SetMobilityModel(mobility, mobilityConf);

    if (entityKey == "drones")
    {
        mobility.Install(m_drones.Get(entityId));
        std::ostringstream oss;
        oss << "/DroneList/" << entityId << "/$ns3::MobilityModel/CourseChange";
        Config::Connect(oss.str(), MakeCallback(&Scenario::DroneCourseChange, this));
    }
    else if (entityKey == "ZSPs")
    {
        mobility.Install(m_zsps.Get(entityId));
    }
    else if (entityKey == "nodes")
    {
        mobility.Install(m_plainNodes.Get(entityId));
    }
    else if (entityKey == "leo-sats")
    {
        mobility.Install(m_leoSats.Get(entityId));
        std::ostringstream oss;
        oss << "/LeoSatList/" << entityId << "/$ns3::MobilityModel/CourseChange";
        Config::Connect(oss.str(), MakeCallback(&Scenario::LeoSatCourseChange, this));
    }
    else if (entityKey == "vehicles")
    {
        auto vehicle = m_vehicles.Get(entityId);
        mobility.Install(vehicle);
        std::ostringstream oss;
        oss << "/NodeList/" << vehicle->GetId() << "/$ns3::MobilityModel/CourseChange";
        Config::Connect(oss.str(), MakeCallback(&Scenario::VehicleCourseChange, this));
    }
    else
    {
        NS_FATAL_ERROR("Unsupported Entity Key: " << entityKey);
    }
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
Scenario::ConfigureLteEnb(Ptr<Node> entityNode,
                          const uint32_t netId,
                          const std::optional<ModelConfiguration> antennaModel,
                          const std::optional<ModelConfiguration> phyConf)
{
    // !NOTICE: no checks are made for backbone/netid combination that do not represent an LTE
    // backbone!
    static std::vector<NodeContainer> backbonePerStack(m_protocolStacks[PHY_LAYER].size());
    auto ltePhy = StaticCast<LtePhySimulationHelper, Object>(m_protocolStacks[PHY_LAYER][netId]);
    auto lteHelper = ltePhy->GetLteHelper();

    if (antennaModel)
    {
        lteHelper->SetEnbAntennaModelType(antennaModel->GetName());
        for (auto& attr : antennaModel->GetAttributes())
        {
            lteHelper->SetEnbAntennaModelAttribute(attr.name, *attr.value);
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
                         const std::optional<ModelConfiguration> antennaModel,
                         const std::optional<ModelConfiguration> phyConf)
{
    // NOTICE: no checks are made for ue/netid combination that do not represent an LTE backbone!
    static std::vector<NodeContainer> uePerStack(m_protocolStacks[PHY_LAYER].size());
    auto ltePhy = StaticCast<LtePhySimulationHelper, Object>(m_protocolStacks[PHY_LAYER][netId]);
    auto lteHelper = ltePhy->GetLteHelper();
    Ipv4StaticRoutingHelper routingHelper;

    if (antennaModel)
    {
        lteHelper->SetUeAntennaModelType(antennaModel->GetName());
        for (auto& attr : antennaModel->GetAttributes())
        {
            lteHelper->SetUeAntennaModelAttribute(attr.name, *attr.value);
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
                         const std::optional<ModelConfiguration> antennaModel,
                         const std::vector<ns3::NrPhyProperty> phyConf)
{
    // !NOTICE: no checks are made for backbone/netid combination that do not represent an LTE
    // backbone!
    static std::vector<NodeContainer> backbonePerStack(m_protocolStacks[PHY_LAYER].size());
    auto nrPhy = StaticCast<NrPhySimulationHelper, Object>(m_protocolStacks[PHY_LAYER][netId]);
    auto nrHelper = nrPhy->GetNrHelper();

    // TODO allow better substitution of default antenna model
    if (antennaModel)
    {
        nrHelper->SetGnbAntennaTypeId(antennaModel->GetName());
        for (auto& attr : antennaModel->GetAttributes())
        {
            nrHelper->SetGnbAntennaAttribute(attr.name, *attr.value);
        }
    }
    auto entityNodeContainer = NodeContainer(entityNode);
    auto dev =
        StaticCast<NrGnbNetDevice, NetDevice>(nrPhy->InstallGnbDevices(entityNodeContainer).Get(0));

    for (const auto& attr : phyConf)
    {
        if (attr.bwpId.has_value())
        {
            nrHelper->GetGnbPhy(dev, attr.bwpId.value())
                ->SetAttribute(attr.attribute.name, *attr.attribute.value);
        }
        else
        {
            for (size_t i = 0; i < nrPhy->GetAllBwps().size(); i++)
            {
                nrHelper->GetGnbPhy(dev, i)->SetAttribute(attr.attribute.name,
                                                          *attr.attribute.value);
            }
        }
    }

    for (NodeContainer::Iterator eNB = backbonePerStack[netId].Begin();
         eNB != backbonePerStack[netId].End();
         eNB++)
    {
        nrPhy->GetNrHelper()->AddX2Interface(entityNode, *eNB);
    }
    backbonePerStack[netId].Add(entityNode);

    // Store gNB device for later attachment operations
    NetDeviceContainer gnbDevContainer(dev);
    m_nrGnbDevices[netId].push_back(gnbDevContainer);

    // Re-attach all existing UEs to the closest gNB (including this new one)
    AttachAllNrUesToClosestGnb(netId);
}

void
Scenario::ConfigureNrUe(Ptr<Node> entityNode,
                        const std::vector<NrBearerConfiguration> bearers,
                        const uint32_t netId,
                        const std::optional<ModelConfiguration> antennaModel,
                        const std::vector<ns3::NrPhyProperty> phyConf)
{
    // NOTICE: no checks are made for ue/netid combination that do not represent an LTE backbone!
    static std::vector<NodeContainer> uePerStack(m_protocolStacks[PHY_LAYER].size());
    auto nrPhy = StaticCast<NrPhySimulationHelper, Object>(m_protocolStacks[PHY_LAYER][netId]);
    auto nrHelper = nrPhy->GetNrHelper();
    Ipv4StaticRoutingHelper routingHelper;

    if (antennaModel)
    {
        nrHelper->SetUeAntennaTypeId(antennaModel->GetName());
        for (auto& attr : antennaModel->GetAttributes())
        {
            nrHelper->SetUeAntennaAttribute(attr.name, *attr.value);
        }
    }

    auto entityNodeContainer = NodeContainer(entityNode);
    auto dev =
        StaticCast<NrUeNetDevice, NetDevice>(nrPhy->InstallUeDevices(entityNodeContainer).Get(0));

    for (const auto& attr : phyConf)
    {
        if (attr.bwpId.has_value())
        {
            nrHelper->GetUePhy(dev, attr.bwpId.value())
                ->SetAttribute(attr.attribute.name, *attr.attribute.value);
        }
        else
        {
            for (size_t i = 0; i < nrPhy->GetAllBwps().size(); i++)
            {
                nrHelper->GetUePhy(dev, i)->SetAttribute(attr.attribute.name,
                                                         *attr.attribute.value);
            }
        }
    }

    // Install network layer in order to proceed with IPv4 LTE configuration
    InstallEntityIpv4(entityNode, dev, netId);
    // Register UEs into network 1.0.0.0/8
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

    // create a static route for each UE to the SGW/PGW in order to communicate
    // with the internet
    auto nodeIpv4 = entityNode->GetObject<Ipv4>();
    Ptr<Ipv4StaticRouting> ueStaticRoute = routingHelper.GetStaticRouting(nodeIpv4);
    ueStaticRoute->SetDefaultRoute(nrPhy->GetNrEpcHelper()->GetUeDefaultGatewayAddress(),
                                   nodeIpv4->GetInterfaceForDevice(dev));

    // Store UE device for later attachment operations
    m_nrUeDevices[netId].push_back(dev);

    // Attach this UE to the closest gNB among all available gNBs
    AttachAllNrUesToClosestGnb(netId);

    // init bearers on UE
    for (auto& bearerConf : bearers)
    {
        NrEpsBearer bearer(bearerConf.GetType(), bearerConf.GetQos());
        nrPhy->GetNrHelper()->ActivateDedicatedEpsBearer(dev, bearer, NrEpcTft::Default());
    }
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
Scenario::ConfigureEntityApplications(const std::string& entityKey,
                                      const Ptr<EntityConfiguration>& conf,
                                      const uint32_t& entityId)
{
    NS_LOG_FUNCTION(entityKey << conf << entityId);

    for (auto appConf : conf->GetApplications())
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

    for (auto perConf : conf->GetPeripherals())
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
Scenario::ConfigureInternetRemotes()
{
    NS_LOG_FUNCTION_NOARGS();

    const auto remoteConfs = CONFIGURATOR->GetRemotesConfiguration();
    size_t remoteId = 0;

    for (auto& conf : remoteConfs)
    {
        const auto appConfs = conf->GetApplications();

        for (auto& appConf : appConfs)
        {
            TypeId appTid;
            NS_ASSERT_MSG(TypeId::LookupByNameFailSafe(appConf.GetName(), &appTid),
                          "Failed to initialize application " << appConf.GetName()
                                                              << ". It does not exist!");

            ObjectFactory factory(appTid.GetName());

            for (auto& appAttr : appConf.GetAttributes())
            {
                factory.Set(appAttr.name, *appAttr.value);
            }

            auto app = StaticCast<Application, Object>(factory.Create());
            m_remoteNodes.Get(remoteId)->AddApplication(app);
        }

        remoteId++;
    }
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

void
Scenario::ConfigureRegionsOfInterest()
{
    const auto regions = CONFIGURATOR->GetRegionsOfInterest();
    Ptr<InterestRegion> reg;
    for (auto region : regions)
    {
        reg = irc->Create(region);
    }
}

void
Scenario::LeoSatCourseChange(std::string context, Ptr<const MobilityModel> model)
{
    auto mobility = DynamicCast<const GeocentricMobilityModel>(model);
    if (mobility)
    {
        auto pos = mobility->GetPosition(ns3::PositionType::GEOCENTRIC);
        auto geo = mobility->GetPosition(ns3::PositionType::GEOGRAPHIC);
        Ptr<const Node> node = model->GetObject<Node>();
        // Write to CSV file: Time,Node,X,Y,Z,Latitude,Longitude,Altitude
        *m_leoSatTraceStream->GetStream()
            << Simulator::Now().GetSeconds() << "," << node->GetId() << "," << pos.x << "," << pos.y
            << "," << pos.z << "," << geo.x << "," << geo.y << "," << geo.z << std::endl;
    }
}

void
Scenario::VehicleCourseChange(std::string context, Ptr<const MobilityModel> model)
{
    auto mobility = DynamicCast<const GeocentricMobilityModel>(model);
    if (mobility)
    {
        auto pos = mobility->GetPosition(ns3::PositionType::GEOCENTRIC);
        auto geo = mobility->GetPosition(ns3::PositionType::GEOGRAPHIC);
        Ptr<const Node> node = model->GetObject<Node>();
        // Write to CSV file: Time,Node,X,Y,Z,Latitude,Longitude,Altitude
        *m_vehicleTraceStream->GetStream()
            << Simulator::Now().GetSeconds() << "," << node->GetId() << "," << pos.x << "," << pos.y
            << "," << pos.z << "," << geo.x << "," << geo.y << "," << geo.z << std::endl;
    }
}

void
Scenario::DroneCourseChange(std::string context, Ptr<const MobilityModel> model)
{
    Vector position = model->GetPosition();
    std::string start = "/DroneList/";
    std::string end = "/$ns3::MobilityModel/CourseChange";
    std::string id = context.substr(context.find(start) + start.length(),
                                    context.length() - end.length() - start.length());
    auto dronePeripheralsContainer = m_drones.Get(std::stoi(id))->GetPeripherals();
    Ptr<DronePeripheral> peripheral;
    std::vector<int> regionindex;
    for (DronePeripheralContainer::Iterator i = dronePeripheralsContainer->Begin();
         i != dronePeripheralsContainer->End();
         i++)
    {
        peripheral = *i;
        regionindex = peripheral->GetRegionsOfInterest();
        int status = irc->IsInRegions(regionindex, position);
        if (regionindex.empty())
        {
            continue;
        }
        if (status >= 0 || status == -2)
        {
            if (peripheral->GetState() != DronePeripheral::PeripheralState::ON)
            {
                peripheral->SetState(DronePeripheral::PeripheralState::ON);
            }
        }
        else
        {
            if (peripheral->GetState() == DronePeripheral::PeripheralState::ON)
            {
                peripheral->SetState(DronePeripheral::PeripheralState::IDLE);
            }
        }
    }
}

void
Scenario::ConfigureSimulator()
{
    NS_LOG_FUNCTION_NOARGS();

    if (CONFIGURATOR->GetLogOnFile())
    {
        // Enable Report
        Report::Get()->Initialize(CONFIGURATOR->GetName(),
                                  CONFIGURATOR->GetCurrentDateTime(),
                                  CONFIGURATOR->GetResultsPath());
    }

    // Start application statistics collection
    m_appStatsHelper.Start();

    Simulator::Stop(Seconds(CONFIGURATOR->GetDuration()));
}

void
Scenario::AttachAllNrUesToClosestGnb(const uint32_t netId)
{
    NS_LOG_FUNCTION(netId);

    auto nrPhy = StaticCast<NrPhySimulationHelper, Object>(m_protocolStacks[PHY_LAYER][netId]);
    auto nrHelper = nrPhy->GetNrHelper();

    // Check if we have gNBs for this netId
    auto gnbIt = m_nrGnbDevices.find(netId);
    if (gnbIt == m_nrGnbDevices.end() || gnbIt->second.empty())
    {
        NS_LOG_INFO("No gNBs available for attachment in stack " << netId);
        return;
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
        return;
    }

    // Attach all UEs to closest gNB
    NetDeviceContainer ueDevices;
    for (auto ueDevice : ueIt->second)
    {
        ueDevices.Add(ueDevice);
    }
    NS_LOG_INFO("Attaching " << ueDevices.GetN() << " UE devices to closest gNB among "
                             << allGnbDevices.GetN() << " available gNBs");
    nrHelper->AttachToClosestGnb(ueDevices, allGnbDevices);
}

} // namespace ns3

int
main(int argc, char** argv)
{
    struct rlimit file_limits;
    file_limits.rlim_cur = 65536;
    file_limits.rlim_max = 65536;

    setrlimit(RLIMIT_NOFILE, &file_limits);

    ns3::Scenario s(argc, argv);
    s(); // run the scenario as soon as it is ready

    return 0;
}
