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

    // Configure all applications after IP addresses are resolved
    ConfigureAllApplications();

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
    struct RadioMapGenConfig
    {
        std::string file;
        std::string type;
        bool is3D;
    };

    struct AggregationTask
    {
        std::vector<std::string> inputFiles;
        std::string outputFile;
        std::string type;
        bool is3D;
    };

    std::vector<std::string> generatedFiles;
    std::vector<RadioMapGenConfig> plotFiles;
    std::vector<AggregationTask> aggregationTasks;

    if (CONFIGURATOR->GetGenerateRadioMaps() && !radioMaps.empty())
    {
        NS_LOG_INFO("Generating Radio Maps...");

        Ptr<Object> lastRemHelper = nullptr;

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
                                    uint32_t nrDeviceCount = 0;
                                    bool foundDevice = false;
                                    for (uint32_t i = 0; i < node->GetNDevices(); ++i)
                                    {
                                        auto dev = node->GetDevice(i);
                                        if (dev && dev->GetInstanceTypeId().IsChildOf(
                                                       NrNetDevice::GetTypeId()))
                                        {
                                            if (nrDeviceCount == (uint32_t)selection.deviceIndex)
                                            {
                                                txDevs.Add(dev);
                                                foundDevice = true;
                                                break;
                                            }
                                            nrDeviceCount++;
                                        }
                                    }
                                    if (!foundDevice)
                                    {
                                        NS_FATAL_ERROR("Tx Node "
                                                       << selection.key << " index "
                                                       << selection.index << " deviceIndex "
                                                       << selection.deviceIndex
                                                       << " is out of range. Found only "
                                                       << nrDeviceCount << " NR devices.");
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
                                uint32_t nrDeviceCount = 0;
                                bool foundDevice = false;
                                for (uint32_t i = 0; i < node->GetNDevices(); ++i)
                                {
                                    auto dev = node->GetDevice(i);
                                    if (dev && dev->GetInstanceTypeId().IsChildOf(
                                                   NrNetDevice::GetTypeId()))
                                    {
                                        if (nrDeviceCount == (uint32_t)selection.deviceIndex)
                                        {
                                            rxDev = dev;
                                            foundDevice = true;
                                            break;
                                        }
                                        nrDeviceCount++;
                                    }
                                }
                                if (!foundDevice)
                                {
                                    NS_FATAL_ERROR("Rx Node " << selection.key << " index "
                                                              << selection.index << " deviceIndex "
                                                              << selection.deviceIndex
                                                              << " is out of range. Found only "
                                                              << nrDeviceCount << " NR devices.");
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

                uint32_t startBwp = 0;
                uint32_t endBwp = 1;

                if (config.bwpId.has_value())
                {
                    startBwp = config.bwpId.value();
                    endBwp = startBwp + 1;
                }
                else
                {
                    // get highest bwpLen from txDevs
                    uint32_t maxBwp = 0;
                    for (uint32_t i = 0; i < txDevs.GetN(); ++i)
                    {
                        auto dev = txDevs.Get(i);
                        if (dev->GetInstanceTypeId() == NrGnbNetDevice::GetTypeId() ||
                            dev->GetInstanceTypeId() == NrUeNetDevice::GetTypeId())
                        {
                            uint32_t len = NrHelper::GetNumberBwp(dev);
                            if (len > maxBwp)
                            {
                                maxBwp = len;
                            }
                        }
                    }
                    if (maxBwp == 0)
                    {
                        maxBwp = 1; // Fallback
                    }
                    endBwp = maxBwp;
                }

                for (uint32_t bwpId = startBwp; bwpId < endBwp; bwpId++)
                {
                    std::stringstream ss;
                    ss << "Phy" << config.phyLayerIndex << "-Bwp" << bwpId << "-" << mapIdx + 1;

                    // Check if rx and tx can communicate with each other on this bwpId
                    uint32_t maxRxBwp = 0;

                    if (auto ueDev = DynamicCast<NrUeNetDevice>(rxDev))
                    {
                        maxRxBwp = NrHelper::GetNumberBwp(ueDev);
                    }
                    else if (auto gnbDev = DynamicCast<NrGnbNetDevice>(rxDev))
                    {
                        maxRxBwp = NrHelper::GetNumberBwp(gnbDev);
                    }

                    if (bwpId >= maxRxBwp)
                    {
                        std::cout << "[RadioMap Setup] Rx device does not have BWP " << bwpId
                                  << std::endl;
                        continue;
                    }

                    NetDeviceContainer currentTxDevices;
                    for (uint32_t i = 0; i < txDevs.GetN(); ++i)
                    {
                        auto dev = txDevs.Get(i);

                        uint32_t maxTxBwp = 0;
                        if (auto ueDev = DynamicCast<NrUeNetDevice>(dev))
                        {
                            maxTxBwp = ueDev->GetCcMapSize();
                        }
                        else if (auto gnbDev = DynamicCast<NrGnbNetDevice>(dev))
                        {
                            maxTxBwp = gnbDev->GetCcMapSize();
                        }
                        if (bwpId < maxTxBwp)
                        {
                            currentTxDevices.Add(dev);
                        }
                    }

                    if (currentTxDevices.GetN() == 0)
                    {
                        std::cout << "[RadioMap Setup] No Tx devices found with BWP " << bwpId
                                  << std::endl;
                        continue;
                    }

                    if (config.coordinatesType == "geocentric")
                    {
                        Ptr<NrRadioGeoEnvironmentMapHelper> remHelper =
                            CreateObject<NrRadioGeoEnvironmentMapHelper>();
                        nrGeoRemHelpers.push_back(remHelper); // Keep alive
                        remHelper->SetSimTag(ss.str());
                        remHelper->SetLogGeocentricRem(config.logGeocentricRem);
                        remHelper->SetAttribute("StopWhenDone", BooleanValue(false));
                        lastRemHelper = remHelper;

                        for (const auto& par : config.parameters)
                        {
                            remHelper->SetAttribute(par.first, StringValue(par.second));
                        }

                        remHelper->CreateRem(currentTxDevices, rxDev, bwpId);
                    }
                    else
                    {
                        Ptr<NrRadioEnvironmentMapHelper> remHelper =
                            CreateObject<NrRadioEnvironmentMapHelper>();
                        nrRemHelpers.push_back(remHelper); // Keep alive
                        remHelper->SetSimTag(ss.str());
                        remHelper->SetAttribute("StopWhenDone", BooleanValue(false));
                        lastRemHelper = remHelper;

                        for (const auto& par : config.parameters)
                        {
                            remHelper->SetAttribute(par.first, StringValue(par.second));
                        }

                        remHelper->CreateRem(currentTxDevices, rxDev, bwpId);
                    }

                    // New helper outputs: nr-rem- + simTag + ".out"
                    std::string filename =
                        CONFIGURATOR->GetResultsPath() + "nr-rem-" + ss.str() + ".out";
                    generatedFiles.push_back(filename);

                    if (!config.aggregateBwps)
                    {
                        plotFiles.push_back({.file = filename, .type = "nr", .is3D = false});
                    }
                } // end loop bwps

                if (config.aggregateBwps)
                {
                    std::stringstream ssAgg;
                    ssAgg << "Phy" << config.phyLayerIndex << "-BwpALL-" << mapIdx + 1;
                    std::string aggFilename =
                        CONFIGURATOR->GetResultsPath() + "nr-rem-" + ssAgg.str() + ".out";

                    AggregationTask task;
                    task.outputFile = aggFilename;
                    task.type = "nr";
                    task.is3D = false;
                    for (uint32_t bwpId = startBwp; bwpId < endBwp; bwpId++)
                    {
                        std::stringstream ssBwp;
                        ssBwp << "Phy" << config.phyLayerIndex << "-Bwp" << bwpId << "-"
                              << mapIdx + 1;
                        task.inputFiles.push_back(CONFIGURATOR->GetResultsPath() + "nr-rem-" +
                                                  ssBwp.str() + ".out");
                    }
                    aggregationTasks.push_back(task);
                    plotFiles.push_back({.file = aggFilename, .type = "nr", .is3D = false});
                }
            }
            else if (config.type == "lte")
            {
                if (config.is3d)
                {
                    static std::vector<Ptr<ThreeDimensionalRemHelper>> lte3dRemHelpers;
                    Ptr<ThreeDimensionalRemHelper> remHelper =
                        CreateObject<ThreeDimensionalRemHelper>();
                    lte3dRemHelpers.push_back(remHelper); // Keep alive
                    remHelper->SetAttribute("StopWhenDone", BooleanValue(false));
                    lastRemHelper = remHelper;

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
                    remHelper->SetAttribute("StopWhenDone", BooleanValue(false));
                    lastRemHelper = remHelper;

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

        if (CONFIGURATOR->GetCloseAfterRadioMaps() && lastRemHelper != nullptr)
        {
            lastRemHelper->SetAttribute("StopWhenDone", BooleanValue(true));
        }
    }

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

    if (CONFIGURATOR->GetGenerateRadioMaps() && !radioMaps.empty())
    {
        // Perform aggregation logic right after Simulator ends
        for (const auto& task : aggregationTasks)
        {
            if (task.inputFiles.empty())
            {
                continue;
            }

            NS_LOG_INFO("Aggregating BWP REM maps to " << task.outputFile);

            std::map<std::tuple<double, double, double>, std::vector<double>> aggregatedData;

            for (const auto& inFileName : task.inputFiles)
            {
                std::ifstream inFile(inFileName);
                if (!inFile.is_open())
                {
                    NS_LOG_WARN("Could not open input file for aggregation: " << inFileName);
                    continue;
                }

                std::string line;
                while (std::getline(inFile, line))
                {
                    if (line.empty())
                        continue;
                    std::stringstream ss(line);
                    double x, y, z;
                    if (!(ss >> x >> y >> z))
                        continue;

                    std::vector<double> vals;
                    std::string strVal;
                    while (ss >> strVal)
                    {
                        try
                        {
                            vals.push_back(std::stod(strVal));
                        }
                        catch (...)
                        {
                            vals.push_back(-1e9); // fallback for unparseable floats
                        }
                    }

                    auto key = std::make_tuple(x, y, z);
                    auto it = aggregatedData.find(key);
                    if (it == aggregatedData.end())
                    {
                        aggregatedData[key] = vals;
                    }
                    else
                    {
                        for (size_t i = 0; i < vals.size() && i < it->second.size(); ++i)
                        {
                            it->second[i] = std::max(it->second[i], vals[i]);
                        }
                    }
                }
            }

            std::ofstream outFile(task.outputFile);
            if (!outFile.is_open())
            {
                NS_LOG_WARN("Could not open output file for aggregation: " << task.outputFile);
                continue;
            }

            for (const auto& kv : aggregatedData)
            {
                outFile << std::get<0>(kv.first) << "\t" << std::get<1>(kv.first) << "\t"
                        << std::get<2>(kv.first);
                for (double val : kv.second)
                {
                    outFile << "\t" << val;
                }
                outFile << "\n";
            }
            outFile.close();
        }

#ifdef ENABLE_CLI_COMMANDS
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
#else
        if (!plotFiles.empty())
        {
            NS_LOG_INFO("Skipping GUI preview terminal script execution because ENABLE_CLI_COMMANDS is not defined.");
        }
#endif
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
