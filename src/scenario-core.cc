#include "scenario.h"

namespace ns3
{

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

    ConfigureFullMeshX2Links();
    AttachAllNrUesToGnbs();
    InitializeIslDelayMode();
    ConfigureScheduling();

    EnablePhyLteTraces();
    EnablePhyNrTraces();

    // Configure application statistics helper
    m_appStatsHelper.SetOutputPath(CONFIGURATOR->GetResultsPath() + "app-statistics.txt");
    m_appStatsHelper.SetReportingInterval(Seconds(CONFIGURATOR->GetAppStatisticsReportInterval()));
    m_appStatsHelper.InstallFlowMonitor(NodeContainer::GetGlobal()); // Install on all nodes

    // Initialize LeoSat trace CSV file
    if (m_leoSats.GetN() > 0)
    {
        std::ostringstream leoSatTraceFilePath;
        leoSatTraceFilePath << CONFIGURATOR->GetResultsPath() << "leo-sat-trace.csv";
        m_leoSatTraceStream = Create<OutputStreamWrapper>(leoSatTraceFilePath.str(), std::ios::out);
        *m_leoSatTraceStream->GetStream()
            << "Time,Node,X,Y,Z,Latitude,Longitude,Altitude" << std::endl;
    }

    // Inizialize Vehicles trace CSV file
    if (m_vehicles.GetN() > 0)
    {
        std::ostringstream vehicleTraceFilePath;
        vehicleTraceFilePath << CONFIGURATOR->GetResultsPath() << "vehicle-trace.csv";
        m_vehicleTraceStream =
            Create<OutputStreamWrapper>(vehicleTraceFilePath.str(), std::ios::out);
        *m_vehicleTraceStream->GetStream()
            << "Time,Node,X,Y,Z,Latitude,Longitude,Altitude,NearestSatId,NearestSatElevationAngle"
            << std::endl;
    }

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

    static std::vector<Ptr<RadioEnvironmentMapHelper>> lteRemHelpers;
    static std::vector<Ptr<NrRadioEnvironmentMapHelper>> nrRemHelpers;
    static std::vector<Ptr<NrRadioGeoEnvironmentMapHelper>> nrGeoRemHelpers;
    auto radioMaps = CONFIGURATOR->GetRadioMaps();
    if (CONFIGURATOR->GetGenerateRadioMaps() && !radioMaps.empty())
    {
        struct RadioMapGenConfig
        {
            std::string file;
            std::string type;
            bool is3D;
        };

        NS_LOG_INFO("Generating Radio Maps...");

        std::vector<std::string> generatedFiles;
        std::vector<RadioMapGenConfig> plotFiles;

        size_t mapIdx = 0;
        for (const auto& config : radioMaps)
        {
            if (config.type == "nr")
            {
                NetDeviceContainer txDevs;
                Ptr<NetDevice> rxDev;

                // TX Nodes Selection
                if (!config.txNodes.empty())
                {
                    for (const auto& selection : config.txNodes)
                    {
                        if (selection.key == "ALL_GNB")
                        {
                            for (const auto& [netId, containers] : m_nrGnbDevices)
                            {
                                if (netId != config.phyLayerIndex)
                                {
                                    continue;
                                }
                                for (const auto& container : containers)
                                {
                                    txDevs.Add(container);
                                }
                            }
                        }
                        else if (selection.key == "ALL_UE")
                        {
                            for (const auto& [netId, devices] : m_nrUeDevices)
                            {
                                if (netId != config.phyLayerIndex)
                                {
                                    continue;
                                }
                                for (const auto& dev : devices)
                                {
                                    txDevs.Add(dev);
                                }
                            }
                        }
                        else if (selection.key == "FIRST_GNB")
                        {
                            bool found = false;
                            for (const auto& [netId, containers] : m_nrGnbDevices)
                            {
                                if (netId != config.phyLayerIndex)
                                {
                                    continue;
                                }
                                if (!containers.empty())
                                {
                                    txDevs.Add(containers[0].Get(0));
                                    found = true;
                                    break;
                                }
                            }
                            if (!found)
                            {
                                NS_LOG_WARN("FIRST_GNB selected but no gNBs found.");
                            }
                        }
                        else
                        {
                            if (selection.index < 0)
                            {
                                NS_FATAL_ERROR("Tx Node " << selection.key << " index "
                                                          << selection.index
                                                          << " must be non-negative.");
                            }
                            Ptr<Node> node = GetNodeByKey(selection.key, selection.index);
                            if (node)
                            {
                                if (selection.deviceIndex != -1)
                                {
                                    if ((uint32_t)selection.deviceIndex < node->GetNDevices())
                                    {
                                        auto dev = node->GetDevice(selection.deviceIndex);
                                        if (dev && dev->GetInstanceTypeId().IsChildOf(
                                                       NrNetDevice::GetTypeId()))
                                        {
                                            txDevs.Add(dev);
                                        }
                                        else
                                        {
                                            NS_FATAL_ERROR("Tx Node " << selection.key << " index "
                                                                      << selection.index
                                                                      << " deviceIndex "
                                                                      << selection.deviceIndex
                                                                      << " is not an NR device.");
                                        }
                                    }
                                    else
                                    {
                                        NS_FATAL_ERROR("Tx Node " << selection.key << " index "
                                                                  << selection.index
                                                                  << " deviceIndex "
                                                                  << selection.deviceIndex
                                                                  << " out of range.");
                                    }
                                }
                                else
                                {
                                    for (uint32_t i = 0; i < node->GetNDevices(); ++i)
                                    {
                                        auto dev = node->GetDevice(i);
                                        if (dev && dev->GetInstanceTypeId().IsChildOf(
                                                       NrNetDevice::GetTypeId()))
                                        {
                                            txDevs.Add(dev);
                                        }
                                    }
                                }
                            }
                            else
                            {
                                NS_LOG_WARN("Tx Node " << selection.key << " index "
                                                       << selection.index << " not found.");
                            }
                        }
                    }
                }
                else
                {
                    // Default behavior: All gNBs
                    for (const auto& [netId, containers] : m_nrGnbDevices)
                    {
                        if (netId != config.phyLayerIndex)
                        {
                            continue;
                        }
                        for (const auto& container : containers)
                        {
                            txDevs.Add(container);
                        }
                    }
                }

                // RX Node Selection
                // Check if key is set (it's empty by default)
                if (!config.rxNode.key.empty())
                {
                    const auto& selection = config.rxNode;

                    if (selection.key == "FIRST_GNB")
                    {
                        for (const auto& [netId, containers] : m_nrGnbDevices)
                        {
                            if (netId != config.phyLayerIndex)
                            {
                                continue;
                            }
                            if (!containers.empty())
                            {
                                rxDev = containers[0].Get(0);
                                break;
                            }
                        }
                    }
                    else if (selection.key == "FIRST_UE")
                    {
                        for (const auto& [netId, devices] : m_nrUeDevices)
                        {
                            if (netId != config.phyLayerIndex)
                            {
                                continue;
                            }
                            if (!devices.empty())
                            {
                                rxDev = devices.front();
                                break;
                            }
                        }
                    }
                    else
                    {
                        if (selection.index < 0)
                        {
                            NS_FATAL_ERROR("Rx Node " << selection.key << " index "
                                                      << selection.index
                                                      << " must be non-negative.");
                        }
                        Ptr<Node> node = GetNodeByKey(selection.key, selection.index);

                        if (node)
                        {
                            if (selection.deviceIndex != -1)
                            {
                                if ((uint32_t)selection.deviceIndex < node->GetNDevices())
                                {
                                    auto dev = node->GetDevice(selection.deviceIndex);
                                    if (dev && dev->GetInstanceTypeId().IsChildOf(
                                                   NrNetDevice::GetTypeId()))
                                    {
                                        rxDev = dev;
                                    }
                                    else
                                    {
                                        NS_FATAL_ERROR("Rx Node " << selection.key << " index "
                                                                  << selection.index
                                                                  << " deviceIndex "
                                                                  << selection.deviceIndex
                                                                  << " is not an NR device.");
                                    }
                                }
                                else
                                {
                                    NS_FATAL_ERROR("Rx Node " << selection.key << " index "
                                                              << selection.index << " deviceIndex "
                                                              << selection.deviceIndex
                                                              << " out of range.");
                                }
                            }
                            else
                            {
                                for (uint32_t i = 0; i < node->GetNDevices(); ++i)
                                {
                                    auto dev = node->GetDevice(i);
                                    if (dev && dev->GetInstanceTypeId().IsChildOf(
                                                   NrNetDevice::GetTypeId()))
                                    {
                                        rxDev = dev;
                                        break;
                                    }
                                }
                            }
                        }
                        else
                        {
                            NS_FATAL_ERROR("Rx Node " << selection.key << " index "
                                                      << selection.index << " not found.");
                        }
                    }
                }
                else
                {
                    // Default behavior: First UE -> DL
                    for (const auto& [netId, devices] : m_nrUeDevices)
                    {
                        if (netId != config.phyLayerIndex)
                        {
                            continue;
                        }
                        if (!devices.empty())
                        {
                            rxDev = devices.front();
                            break;
                        }
                    }
                }

                if (!rxDev)
                {
                    NS_FATAL_ERROR("Cannot generate REM: No Rx device found.");
                }
                if (txDevs.GetN() == 0)
                {
                    NS_FATAL_ERROR("Cannot generate REM: No Tx devices found.");
                }

                std::stringstream ss;
                ss << "Phy" << config.phyLayerIndex << "-Bwp" << config.bwpId << "-" << mapIdx + 1;

                if (config.coordinatesType == "geocentric")
                {
                    Ptr<NrRadioGeoEnvironmentMapHelper> remHelper =
                        CreateObject<NrRadioGeoEnvironmentMapHelper>();
                    nrGeoRemHelpers.push_back(remHelper); // Keep alive
                    remHelper->SetSimTag(ss.str());
                    remHelper->SetLogGeocentricRem(config.logGeocentricRem);

                    for (const auto& par : config.parameters)
                    {
                        remHelper->SetAttribute(par.first, StringValue(par.second));
                    }

                    remHelper->CreateRem(txDevs, rxDev, config.bwpId);
                }
                else
                {
                    Ptr<NrRadioEnvironmentMapHelper> remHelper =
                        CreateObject<NrRadioEnvironmentMapHelper>();
                    nrRemHelpers.push_back(remHelper); // Keep alive
                    remHelper->SetSimTag(ss.str());

                    for (const auto& par : config.parameters)
                    {
                        remHelper->SetAttribute(par.first, StringValue(par.second));
                    }

                    remHelper->CreateRem(txDevs, rxDev, config.bwpId);
                }

                // New helper outputs: nr-rem- + simTag + ".out"
                std::string filename =
                    CONFIGURATOR->GetResultsPath() + "nr-rem-" + ss.str() + ".out";
                generatedFiles.push_back(filename);
                plotFiles.push_back({.file = filename, .type = "nr", .is3D = false});
            }
            else if (config.type == "lte")
            {
                if (config.is3d)
                {
                    static std::vector<Ptr<ThreeDimensionalRemHelper>> lte3dRemHelpers;
                    Ptr<ThreeDimensionalRemHelper> remHelper =
                        CreateObject<ThreeDimensionalRemHelper>();
                    lte3dRemHelpers.push_back(remHelper); // Keep alive

                    for (const auto& par : config.parameters)
                    {
                        remHelper->SetAttribute(par.first, StringValue(par.second));
                    }

                    std::string filename = CONFIGURATOR->GetResultsPath() +
                                           CONFIGURATOR->GetName() + "-lte-3d-rem-" +
                                           std::to_string(mapIdx) + ".txt";
                    remHelper->SetAttribute("OutputFile", StringValue(filename));
                    remHelper->Install();
                    generatedFiles.push_back(filename);
                    plotFiles.push_back({.file = filename, .type = "lte", .is3D = true});
                }
                else
                {
                    Ptr<RadioEnvironmentMapHelper> remHelper =
                        CreateObject<RadioEnvironmentMapHelper>();
                    lteRemHelpers.push_back(remHelper); // Keep alive

                    for (const auto& par : config.parameters)
                    {
                        remHelper->SetAttribute(par.first, StringValue(par.second));
                    }

                    std::string filename = CONFIGURATOR->GetResultsPath() +
                                           CONFIGURATOR->GetName() + "-lte-rem-" +
                                           std::to_string(mapIdx) + ".txt";
                    remHelper->SetAttribute("OutputFile", StringValue(filename));
                    remHelper->Install();
                    generatedFiles.push_back(filename);
                    plotFiles.push_back({.file = filename, .type = "lte", .is3D = false});
                }
            }
            else
            {
                NS_LOG_WARN("Unknown radio map type: " << config.type);
            }
            mapIdx++;
        }

        Simulator::Run();
        Simulator::Destroy();

        for (const auto& plotConf : plotFiles)
        {
            std::string cmd = "python " + CONFIGURATOR->GetResultsPath() +
                              "../../analysis/txt2ply.py " + plotConf.file;

            bool addNrFlag = plotConf.type == "nr";
            if (addNrFlag)
            {
                cmd += " --nrMap";
            }

            int plyRet = system(cmd.c_str());
            if (plyRet != 0)
            {
                NS_FATAL_ERROR("Something went wrong while generating the ply file for "
                               << plotConf.file);
            }

            std::string plotCmd = plotConf.is3D
                                      ? "python " + CONFIGURATOR->GetResultsPath() +
                                            "../../analysis/rem-3d-preview.py " + plotConf.file
                                      : "python " + CONFIGURATOR->GetResultsPath() +
                                            "../../analysis/rem-2d-preview.py " + plotConf.file;
            if (addNrFlag)
            {
                plotCmd += " --nrMap";
            }
            int plotRet = system(plotCmd.c_str());
            if (plotRet != 0)
            {
                NS_FATAL_ERROR("Something went wrong while generating the plot for "
                               << plotConf.file);
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

} // namespace ns3
