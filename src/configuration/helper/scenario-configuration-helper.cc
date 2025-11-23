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
#include "scenario-configuration-helper.h"

#include "mac-layer-configuration-helper.h"
#include "model-configuration-helper.h"
#include "network-layer-configuration-helper.h"
#include "phy-layer-configuration-helper.h"
#include "remote-configuration-helper.h"

#include "ns3/constellation-expander.h"
#include "ns3/json-importer.h"
#include <ns3/command-line.h>
#include <ns3/integer.h>
#include <ns3/log.h>
#include <ns3/object-factory.h>
#include <ns3/system-path.h>

#include <chrono>
#include <filesystem>
#include <iomanip> /* put_time */
#include <iostream>
#include <unistd.h>

#if defined(__clang__)
_Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
#define SUPPRESS_DEPRECATED_POP _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
_Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define SUPPRESS_DEPRECATED_POP _Pragma("GCC diagnostic pop")
#else
#define SUPPRESS_DEPRECATED_POP
#endif

#include <rapidjson/error/en.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/pointer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

    SUPPRESS_DEPRECATED_POP

    namespace ns3
{
    NS_LOG_COMPONENT_DEFINE_MASK("ScenarioConfigurationHelper", LOG_PREFIX_ALL);

    void ScenarioConfigurationHelper::Initialize(int argc, char** argv)
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream dateTime;

        dateTime << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d.%H-%M-%S");

        m_dateTime = dateTime.str();
        char* cwd = getcwd(nullptr, 0);
        m_currentPath = std::string(cwd);
        free(cwd);

        InitializeConfiguration(argc, argv);
        InitializeLogging(GetLogOnFile());
    }

    void ScenarioConfigurationHelper::Initialize(int argc, char** argv, const std::string name)
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream dateTime;

        dateTime << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%H-%M-%S");

        m_dateTime = dateTime.str();
        m_name = name;

        InitializeConfiguration(argc, argv);
        InitializeLogging(GetLogOnFile());
    }

    ScenarioConfigurationHelper::~ScenarioConfigurationHelper()
    {
        DisposeLogging();
        DisposeConfiguration();
    }

    const std::string ScenarioConfigurationHelper::GetName()
    {
        if (m_name.empty())
        {
            NS_ASSERT_MSG(m_config.HasMember("name"),
                          "Please define scenario 'name' in configuration file.");
            NS_ASSERT_MSG(m_config["name"].IsString(), "Scenario 'name' must be a String.");

            m_name = m_config["name"].GetString();
            NS_ASSERT_MSG(!m_name.empty(), "Scenario 'name' must be non empty.");
        }

        return m_name;
    }

    const std::string ScenarioConfigurationHelper::GetCurrentDateTime() const
    {
        return m_dateTime;
    }

    const std::string ScenarioConfigurationHelper::GetResultsPath()
    {
        NS_ASSERT_MSG(m_config.HasMember("resultsPath"),
                      "Please define 'resultsPath' in configuration file.");
        NS_ASSERT_MSG(m_config["resultsPath"].IsString(), "'resultsPath' must be a string.");

        std::stringstream path;
        auto resPath = m_config["resultsPath"].GetString();
        auto resPathLen = strlen(resPath);
        if (resPathLen > 0 && resPath[0] == '/')
        {
            path << resPath;
        }
        else
        {
            std::string resPathStr = std::string(resPath);
            if (resPathLen > 1 && resPath[resPathLen - 1] == '/')
            {
                resPathStr.pop_back();
            }
            path << m_currentPath << "/" << resPathStr << "/" << GetName() << "-" << m_dateTime;
        }

        SystemPath::MakeDirectories(path.str());

        path << "/";

        return path.str();
    }

    const bool ScenarioConfigurationHelper::GetLogOnFile() const
    {
        // this is an optional parameter. Default to false if not specified.
        if (m_config.HasMember("logOnFile"))
        {
            NS_ASSERT_MSG(m_config["logOnFile"].IsBool(), "'logOnFile' property must be boolean.");
            return m_config["logOnFile"].GetBool();
        }
        else
        {
            return false;
        }
    }

    const std::string ScenarioConfigurationHelper::GetLoggingFilePath()
    {
        std::stringstream ss;

        ss << GetResultsPath() << "scenario.log";

        return ss.str();
    }

    const std::vector<std::pair<std::string, Ptr<AttributeValue>>>
    ScenarioConfigurationHelper::GetStaticConfig()
    {
        if (m_staticConfig.empty())
        {
            if (!m_config.HasMember("staticNs3Config"))
            {
                return {};
            }

            NS_ASSERT_MSG(m_config["staticNs3Config"].IsArray(),
                          "'staticNs3Config' property must be an array.");

            auto decoded = std::vector<std::pair<std::string, Ptr<AttributeValue>>>{};
            decoded.reserve(m_config["staticNs3Config"].Size());

            const auto staticConfigsArr = m_config["staticNs3Config"].GetArray();
            for (auto& sc : staticConfigsArr)
            {
                NS_ASSERT_MSG(sc.IsObject(), "A static config definition is invalid.");

                const auto obj = sc.GetObject();
                NS_ASSERT_MSG(obj.HasMember("name"),
                              "'name' is required in staticNs3Config definition.");
                NS_ASSERT_MSG(obj["name"].IsString(),
                              "'name' property must be a string in staticNs3Config definition.");
                NS_ASSERT_MSG(obj.HasMember("value"),
                              "'value' is required in staticNs3Config definiton.");

                const std::string modelAttr = obj["name"].GetString();
                const std::string delimiter = "::";
                const std::string modelName = modelAttr.substr(0, modelAttr.rfind(delimiter));
                const std::string attrName = modelAttr.substr(modelName.size() + delimiter.size());
                const TypeId model = TypeId::LookupByName(modelName);

                TypeId::AttributeInformation attrInfo;
                NS_ABORT_MSG_IF(!model.LookupAttributeByName(attrName, &attrInfo),
                                "Cannot find attribute name "
                                    << attrName << " of model " << modelName
                                    << ". Please check your static ns3 config parameters.");
                const auto attrValue = ModelConfigurationHelper::DecodeAttributeValue(modelName,
                                                                                      obj["value"],
                                                                                      attrInfo);

                decoded.push_back({modelAttr, attrValue});
            }

            m_staticConfig = decoded;
        }

        return m_staticConfig;
    }

    const std::vector<Ptr<PhyLayerConfiguration>> ScenarioConfigurationHelper::GetPhyLayers() const
    {
        NS_ASSERT_MSG(m_config.HasMember("phyLayer"),
                      "Please define phyLayer in your JSON configuration.");
        NS_ASSERT_MSG(m_config["phyLayer"].IsArray(), "'phyLayer' property must be an array.");

        const auto arr = m_config["phyLayer"].GetArray();
        std::vector<Ptr<PhyLayerConfiguration>> phyConfs;
        for (auto& el : arr)
        {
            auto conf = PhyLayerConfigurationHelper::GetConfiguration(el);
            phyConfs.emplace_back(conf);
        }

        return phyConfs;
    }

    const std::vector<Ptr<MacLayerConfiguration>> ScenarioConfigurationHelper::GetMacLayers() const
    {
        NS_ASSERT_MSG(m_config.HasMember("macLayer"),
                      "Please define macLayer in your JSON configuration.");
        NS_ASSERT_MSG(m_config["macLayer"].IsArray(), "'macLayer' property must be an array.");

        const auto arr = m_config["macLayer"].GetArray();
        std::vector<Ptr<MacLayerConfiguration>> macConfs;
        for (auto& el : arr)
        {
            auto conf = MacLayerConfigurationHelper::GetConfiguration(el);
            macConfs.emplace_back(conf);
        }

        return macConfs;
    }

    const std::vector<Ptr<NetworkLayerConfiguration>>
    ScenarioConfigurationHelper::GetNetworkLayers() const
    {
        if (!m_config.HasMember("networkLayer"))
        {
            return {};
        }

        NS_ASSERT_MSG(m_config["networkLayer"].IsArray(),
                      "'networkLayer' property must be an array.");

        const auto arr = m_config["networkLayer"].GetArray();
        std::vector<Ptr<NetworkLayerConfiguration>> netConfs;
        for (auto& el : arr)
        {
            auto conf = NetworkLayerConfigurationHelper::GetConfiguration(el);
            netConfs.emplace_back(conf);
        }

        return netConfs;
    }

    const std::vector<Ptr<EntityConfiguration>>
    ScenarioConfigurationHelper::GetEntitiesConfiguration(const std::string& entityKey) const
    {
        const char* entityKeyCStr = entityKey.c_str();
        if (!m_config.HasMember(entityKeyCStr))
        {
            return {};
        }

        NS_ASSERT_MSG(m_config[entityKeyCStr].IsArray(),
                      "JSON property '" << entityKey << "' must be an array.");

        const auto arr = m_config[entityKeyCStr].GetArray();
        std::vector<Ptr<EntityConfiguration>> entityConf{};

        entityConf.reserve(arr.Size());
        for (auto& el : arr)
        {
            auto conf = EntityConfigurationHelper::GetConfiguration(el);
            entityConf.push_back(conf);
        }

        return entityConf;
    }

    const std::vector<Ptr<RemoteConfiguration>>
    ScenarioConfigurationHelper::GetRemotesConfiguration() const
    {
        std::vector<Ptr<RemoteConfiguration>> remoteConf;

        if (!m_config.HasMember("remotes"))
        {
            return remoteConf;
        }

        NS_ASSERT_MSG(m_config["remotes"].IsArray(), "JSON property 'remotes' must be an array.");

        const auto arr = m_config["remotes"].GetArray();
        for (auto& el : arr)
        {
            auto conf = RemoteConfigurationHelper::GetConfiguration(el);
            remoteConf.push_back(conf);
        }

        return remoteConf;
    }

    const double ScenarioConfigurationHelper::GetDuration() const
    {
        NS_ASSERT(m_config.HasMember("duration"));
        NS_ASSERT_MSG(m_config["duration"].IsNumber(),
                      "Please define duration in configuration file.");

        double duration = 0.0;

        if (m_config["duration"].IsUint())
        {
            duration = static_cast<double>(m_config["duration"].GetUint());
        }
        else
        {
            duration = m_config["duration"].GetDouble();
        }

        return duration;
    }

    const double ScenarioConfigurationHelper::GetAppStatisticsReportInterval() const
    {
        // Default to 1 second if not specified
        if (!m_config.HasMember("appStatisticsReportInterval"))
        {
            return 1;
        }

        NS_ASSERT_MSG(m_config["appStatisticsReportInterval"].IsNumber(),
                      "appStatisticsReportInterval must be a number.");

        double interval = 0.0;

        if (m_config["appStatisticsReportInterval"].IsUint())
        {
            interval = static_cast<double>(m_config["appStatisticsReportInterval"].GetUint());
        }
        else
        {
            interval = m_config["appStatisticsReportInterval"].GetDouble();
        }

        return interval;
    }

    std::size_t ScenarioConfigurationHelper::GetN(const char* ek) const
    {
        if (!m_config.HasMember(ek))
        {
            return 0;
        }

        NS_ASSERT_MSG(m_config[ek].IsArray(),
                      "'" << ek
                          << "' property in the configuration file must be an array of objects.");

        return m_config[ek].Size();
    }

    std::size_t ScenarioConfigurationHelper::GetNodesN() const
    {
        // optional field
        if (!m_config.HasMember("nodes"))
        {
            return 0;
        }

        NS_ASSERT_MSG(m_config["nodes"].IsArray(),
                      "Expected \"nodes\" property to be an array of objects.");

        return m_config["nodes"].Size();
    }

    const std::string ScenarioConfigurationHelper::GetPhyPropagationLossModel() const
    {
        constexpr const char* jRootKey = "phyPropagationLoss";
        constexpr const char* jModelKey = "model";

        NS_ASSERT(m_config.HasMember(jRootKey));
        NS_ASSERT_MSG(m_config[jRootKey].IsObject(),
                      jRootKey << " must be a valid JSON object per specification.");

        NS_ASSERT(m_config[jRootKey].HasMember(jModelKey));
        NS_ASSERT_MSG(m_config[jRootKey][jModelKey].IsString(),
                      jModelKey << " must be a valid JSON string per specification.");

        return m_config[jRootKey][jModelKey].GetString();
    }

    const std::vector<std::pair<std::string, float>>
    ScenarioConfigurationHelper::GetThreeLogDistancePropagationLossModelAttributes() const
    {
        constexpr const char* jRootKey = "phyPropagationLoss";
        constexpr const char* jAttrKey = "attributes";

        NS_ASSERT(m_config.HasMember(jRootKey));
        NS_ASSERT_MSG(m_config[jRootKey].IsObject(),
                      jRootKey << " must be a valid JSON object per specification.");

        NS_ASSERT(m_config[jRootKey].HasMember(jAttrKey));
        NS_ASSERT_MSG(m_config[jRootKey][jAttrKey].IsArray(),
                      jAttrKey << " must be a valid JSON array per specification.");

        const auto jAttrArr = m_config[jRootKey][jAttrKey].GetArray();

        // check if we have an array of objects
        for (auto& el : jAttrArr)
        {
            NS_ASSERT_MSG(el.IsObject(), jAttrKey << " must be a valid array of objects.");

            NS_ASSERT_MSG(el.HasMember("key") && el["key"].IsString() && el.HasMember("value") &&
                              !el["value"].IsArray() && !el["value"].IsObject(),
                          jAttrKey << " contains a malformed structure.");
        }

        // "safe" to decode the data
        std::vector<std::pair<std::string, float>> attrList;

        for (auto& el : jAttrArr)
        {
            const std::string attrKey = el["key"].GetString();
            const float attrVal = el["value"].GetFloat();

            attrList.push_back({attrKey, attrVal});
        }

        return attrList;
    }

    const std::string ScenarioConfigurationHelper::GetPhyMode() const
    {
        NS_ASSERT_MSG(m_config.HasMember("phyMode") && m_config["phyMode"].IsString(),
                      "Please define phyMode in configuration file.");

        return m_config["phyMode"].GetString();
    }

    const uint32_t ScenarioConfigurationHelper::GetDronesN() const
    {
        if (!m_config.HasMember("drones"))
        {
            return 0;
        }

        NS_ASSERT_MSG(m_config["drones"].IsArray(),
                      "'drones' property in the configuration file must be an array of objects.");

        return m_config["drones"].Size();
    }

    const std::string ScenarioConfigurationHelper::GetDronesMobilityModel() const
    {
        NS_ASSERT_MSG(m_config.HasMember("dronesMobilityModel") &&
                          m_config["dronesMobilityModel"].IsString(),
                      "Please define dronesMobilityModel in configuration file.");

        return m_config["dronesMobilityModel"].GetString();
    }

    void ScenarioConfigurationHelper::GetDronesPosition(Ptr<ListPositionAllocator> allocator) const
    {
        NS_ASSERT_MSG(GetDronesMobilityModel() == "ns3::ConstantPositionMobilityModel",
                      "Drones position parameter can be used only when dronesMobilityModel is "
                      "ns3::ConstantPositionMobilityModel");

        for (uint32_t i = 0; i < m_config["drones"].GetArray().Size(); ++i)
        {
            Vector v = GetDronePosition(i);
            NS_LOG_LOGIC("Allocating a drone in space at " << v);
            allocator->Add(v);
        }
    }

    const std::vector<Waypoint> ScenarioConfigurationHelper::GetDroneWaypoints(uint32_t i) const
    {
        NS_ASSERT_MSG(GetDroneMobilityModel(i) == "ns3::WaypointMobilityModel",
                      "Waypoints are usable only with ns3::WaypointMobilityModel");

        const auto drones = m_config["drones"].GetArray();
        const auto drone = drones[i].GetObject();

        std::vector<Waypoint> waypoints;

        Time prevTime = Seconds(0);

        for (auto point = drone["trajectory"].Begin(); point != drone["trajectory"].End(); point++)
        {
            NS_ASSERT_MSG(point->IsObject(),
                          "Each waypoint in drone trajectory must be a JSON object.");
            NS_ASSERT_MSG(
                point->HasMember("position") && (*point)["position"].IsArray() &&
                    (*point)["position"][0].IsDouble() && (*point)["position"][1].IsDouble() &&
                    (*point)["position"][2].IsDouble() && point->HasMember("time") &&
                    (*point)["time"].IsDouble(),
                "Cannot decode waypoint, please check that each point in trajectory has at "
                "least position and time.");

            Time time = Seconds((*point)["time"].GetDouble());

            NS_ASSERT_MSG(
                time >= prevTime,
                "Check waypoints, time of a waypoint must be greater or equal to the previous");

            Vector position = Vector({(*point)["position"][0].GetDouble(),
                                      (*point)["position"][1].GetDouble(),
                                      (*point)["position"][2].GetDouble()});

            waypoints.emplace_back(Waypoint(time, position));

            prevTime = time;
        }

        return waypoints;
    }

    const float ScenarioConfigurationHelper::GetCurveStep() const
    {
        if (m_config.HasMember("curveStep"))
        {
            NS_ASSERT(m_config["curveStep"].IsFloat());
            return m_config["curveStep"].GetFloat();
        }
        else
        {
            return 0.001;
        }
    }

    const FlightPlan ScenarioConfigurationHelper::GetDroneFlightPlan(uint32_t i) const
    {
        // checks for drones were already made in ::ConfGetNumDrones.
        // Let's skip them.
        const auto drones = m_config["drones"].GetArray();
        const auto drone = drones[i].GetObject();

        FlightPlan flightPlan;

        for (auto point = drone["trajectory"].Begin(); point != drone["trajectory"].End(); point++)
        {
            NS_ASSERT_MSG(point->IsObject(), "Each point in drone must be a JSON object.");
            NS_ASSERT_MSG(point->HasMember("position") && (*point)["position"].IsArray() &&
                              (*point)["position"][0].IsDouble() &&
                              (*point)["position"][1].IsDouble() &&
                              (*point)["position"][2].IsDouble() && point->HasMember("interest") &&
                              (*point)["interest"].IsUint(),
                          "Cannot decode trajectory, please check that each point has at least "
                          "position and interest.");

            // restTime is optional
            double restTime = 0.0;
            if (point->HasMember("restTime"))
            {
                NS_ASSERT((*point)["restTime"].IsDouble());
                restTime = (*point)["restTime"].GetDouble();
            }

            auto protoPoint = CreateObjectWithAttributes<ProtoPoint>(
                "Position",
                VectorValue({(*point)["position"][0].GetDouble(),
                             (*point)["position"][1].GetDouble(),
                             (*point)["position"][2].GetDouble()}),
                "Interest",
                IntegerValue((*point)["interest"].GetUint()),
                "RestTime",
                TimeValue(Seconds(restTime)));

            flightPlan.Add(protoPoint);
        }

        return flightPlan;
    }

    const double ScenarioConfigurationHelper::GetDroneAcceleration(uint32_t i) const
    {
        // checks for drones were already made in ::ConfGetNumDrones.
        // Let's skip them.
        const auto drones = m_config["drones"].GetArray();
        const auto drone = drones[i].GetObject();

        NS_ASSERT_MSG(drone.HasMember("acceleration") && drone["acceleration"].IsDouble(),
                      "Every done must have defined the parameter \"acceleration\".");

        return drone["acceleration"].GetDouble();
    }

    const double ScenarioConfigurationHelper::GetDroneMaxSpeed(uint32_t i) const
    {
        // checks for drones were already made in ::ConfGetNumDrones.
        // Let's skip them.
        const auto drones = m_config["drones"].GetArray();
        const auto drone = drones[i].GetObject();

        NS_ASSERT_MSG(drone.HasMember("maxSpeed") && drone["maxSpeed"].IsDouble(),
                      "Every done must have defined the parameter \"maxSpeed\".");

        return drone["maxSpeed"].GetDouble();
    }

    const DoubleVector ScenarioConfigurationHelper::GetDroneSpeedCoefficients(uint32_t i) const
    {
        // checks for drones were already made in ::ConfGetNumDrones.
        // Let's skip them.
        const auto drones = m_config["drones"].GetArray();
        const auto drone = drones[i].GetObject();

        DoubleVector speedCoefficients;

        NS_ASSERT(drone.HasMember("speedCoefficients") && drone["speedCoefficients"].IsArray() &&
                  drone["speedCoefficients"].Size() > 0);

        for (auto coefficient = drone["speedCoefficients"].Begin();
             coefficient != drone["speedCoefficients"].End();
             coefficient++)
        {
            NS_ASSERT((*coefficient).IsDouble());
            speedCoefficients.Add((*coefficient).GetDouble());
        }

        return speedCoefficients;
    }

    const double ScenarioConfigurationHelper::GetDroneApplicationStartTime(uint32_t i) const
    {
        const auto drones = m_config["drones"].GetArray();
        const auto drone = drones[i].GetObject();

        if (drone.HasMember("applicationStartTime") && drone["applicationStartTime"].IsDouble())
        {
            return drone["applicationStartTime"].GetDouble();
        }
        else
        {
            return 0.0;
        }
    }

    const double ScenarioConfigurationHelper::GetDroneApplicationStopTime(uint32_t i) const
    {
        const auto drones = m_config["drones"].GetArray();
        const auto drone = drones[i].GetObject();

        if (drone.HasMember("applicationStopTime") && drone["applicationStopTime"].IsDouble())
        {
            return drone["applicationStopTime"].GetDouble();
        }
        else
        {
            return GetDuration();
        }
    }

    const uint32_t ScenarioConfigurationHelper::GetZspsN() const
    {
        if (!m_config.HasMember("ZSPs"))
        {
            return 0;
        }

        NS_ASSERT_MSG(m_config["ZSPs"].IsArray(),
                      "'ZSPs' property in the configuration file must be an array of objects.");

        return m_config["ZSPs"].Size();
    }

    void ScenarioConfigurationHelper::GetZspsPosition(Ptr<ListPositionAllocator> allocator) const
    {
        // checks for ZSP array were already made in ::ConfGetNumZsps.
        // Let's skip them.
        for (auto i = m_config["ZSPs"].Begin(); i != m_config["ZSPs"].End(); i++)
        {
            NS_ASSERT_MSG(i->IsObject(), "Each ZSP must be a JSON object.");
            NS_ASSERT_MSG(i->HasMember("position"),
                          "One or more ZSPs do not have defined position.");

            NS_ASSERT_MSG((*i)["position"].IsArray() && (*i)["position"].Size() == 3 &&
                              (*i)["position"][0].IsDouble() && (*i)["position"][1].IsDouble() &&
                              (*i)["position"][2].IsDouble(),
                          "Please check that each ZSP position is an array of 3 doubles.");

            Vector v((*i)["position"][0].GetDouble(),
                     (*i)["position"][1].GetDouble(),
                     (*i)["position"][2].GetDouble());

            NS_LOG_LOGIC("Allocating a ZSP in space at " << v);
            allocator->Add(v);
        }
    }

    const double ScenarioConfigurationHelper::GetZspApplicationStartTime(uint32_t i) const
    {
        // checks for drones were already made in ::ConfGetNumDrones.
        // Let's skip them.
        const auto zsps = m_config["ZSPs"].GetArray();
        const auto zsp = zsps[i].GetObject();

        if (zsp.HasMember("applicationStartTime") && zsp["applicationStartTime"].IsDouble())
        {
            return zsp["applicationStartTime"].GetDouble();
        }
        else
        {
            return 0.0;
        }
    }

    const double ScenarioConfigurationHelper::GetZspApplicationStopTime(uint32_t i) const
    {
        // checks for drones were already made in ::ConfGetNumDrones.
        // Let's skip them.
        const auto zsps = m_config["ZSPs"].GetArray();
        const auto zsp = zsps[i].GetObject();

        if (zsp.HasMember("applicationStopTime") && zsp["applicationStopTime"].IsDouble())
        {
            return zsp["applicationStopTime"].GetDouble();
        }
        else
        {
            return GetDuration();
        }
    }

    const uint32_t ScenarioConfigurationHelper::GetEnbsN() const
    {
        NS_ASSERT(m_config.HasMember("ENBs"));
        NS_ASSERT(m_config["ENBs"].IsArray());
        NS_ASSERT_MSG(m_config["ENBs"].Size() > 0,
                      "Please define at least one ENB in configuration file.");

        return m_config["ENBs"].Size();
    }

    void ScenarioConfigurationHelper::GetEnbsPosition(Ptr<ListPositionAllocator> allocator) const
    {
        // checks for ZSP array were already made in ::ConfGetNumZsps.
        // Let's skip them.
        for (auto i = m_config["ENBs"].Begin(); i != m_config["ENBs"].End(); i++)
        {
            NS_ASSERT_MSG(i->IsObject(), "Each ENB must be a JSON object.");
            NS_ASSERT_MSG(i->HasMember("position"),
                          "One or more ENBs do not have defined position.");

            NS_ASSERT_MSG((*i)["position"].IsArray() && (*i)["position"].Size() == 3 &&
                              (*i)["position"][0].IsDouble() && (*i)["position"][1].IsDouble() &&
                              (*i)["position"][2].IsDouble(),
                          "Please check that each ENB position is an array of 3 doubles.");

            Vector v((*i)["position"][0].GetDouble(),
                     (*i)["position"][1].GetDouble(),
                     (*i)["position"][2].GetDouble());

            NS_LOG_LOGIC("Allocating a ENB in space at " << v);
            allocator->Add(v);
        }
    }

    const bool ScenarioConfigurationHelper::IsDryRun() const
    {
        // This is an optional parameter, so no asserts!
        if (m_config.HasMember("dryRun"))
        {
            NS_ASSERT_MSG(m_config["dryRun"].IsBool(), "'dryRun' flag must be boolean type.");
            return m_config["dryRun"].GetBool();
        }
        else
        {
            return false;
        }
    }

    void ScenarioConfigurationHelper::InitializeConfiguration(int argc, char** argv)
    {
        std::string configFilePath = "";
        std::string expandOutputPath = "";
        bool doExpand = false;
        constexpr const ssize_t configFileBufferSize = 64 * 1024; // KiB
        char configFileBuffer[configFileBufferSize];
        m_radioMap = 0; // no generation as default
        CommandLine cmd;
        cmd.AddValue("name", "Name of the scenario", m_name);
        cmd.AddValue("config", "Configuration file path", configFilePath);
        cmd.AddValue("radioMap", "Enables the generation of the EnvironmentRadioMap", m_radioMap);
        cmd.AddValue("expand", "Expand JSON configuration and exit", doExpand);
        cmd.AddValue("output",
                     "Output file path for expanded JSON (optional, default stdout)",
                     expandOutputPath);
        cmd.Parse(argc, argv);

        if (configFilePath.empty())
        {
            NS_FATAL_ERROR(
                "Please specify a non-empty, JSON formatted configuration with --config!");
        }

        // open configuration file and decode JSON data
        m_configFilePtr = fopen(configFilePath.c_str(), "rb");
        if (!m_configFilePtr)
        {
            NS_FATAL_ERROR("Cannot open " << configFilePath << ": " << std::strerror(errno));
        }

        rapidjson::FileReadStream jsonFileStream(m_configFilePtr,
                                                 configFileBuffer,
                                                 configFileBufferSize);
        m_config.ParseStream<rapidjson::kParseCommentsFlag>(jsonFileStream);

        NS_ABORT_MSG_IF(m_config.HasParseError(),
                        "The given configuration schema is not valid JSON: "
                            << rapidjson::GetParseError_En(m_config.GetParseError()));

        // Process !importJson commands
        std::string scenarioPath = std::filesystem::path(configFilePath).parent_path().string();
        JsonImporter::Process(m_config, scenarioPath);

        // Expand constellation definitions if present
        if (m_config.HasMember("leo-sats") && m_config["leo-sats"].IsArray())
        {
            rapidjson::Value newSatArray(rapidjson::kArrayType);
            auto& allocator = m_config.GetAllocator();
            bool expanded = false;

            for (auto& sat : m_config["leo-sats"].GetArray())
            {
                if (ConstellationExpander::HasConstellationParameter(sat))
                {
                    std::vector<rapidjson::Document> expandedSats =
                        ConstellationExpander::ExpandConstellation(sat, scenarioPath);
                    for (auto& expandedSat : expandedSats)
                    {
                        rapidjson::Value satVal(rapidjson::kObjectType);
                        satVal.CopyFrom(expandedSat, allocator);
                        newSatArray.PushBack(satVal, allocator);
                    }
                    expanded = true;
                }
                else
                {
                    rapidjson::Value satVal(rapidjson::kObjectType);
                    satVal.CopyFrom(sat, allocator);
                    newSatArray.PushBack(satVal, allocator);
                }
            }

            if (expanded)
            {
                m_config["leo-sats"] = newSatArray;
            }
        }

        // Handle expansion export
        if (doExpand)
        {
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            m_config.Accept(writer);

            if (!expandOutputPath.empty())
            {
                std::ofstream ofs(expandOutputPath);
                ofs << buffer.GetString();
                ofs.close();
            }
            else
            {
                std::cout << buffer.GetString() << std::endl;
            }
            exit(0);
        }
    }

    void ScenarioConfigurationHelper::DisposeConfiguration()
    {
        if (m_configFilePtr)
        {
            std::fclose(m_configFilePtr);
        }
    }

    void ScenarioConfigurationHelper::InitializeLogging(const bool& onFile)
    {
        EnableLogComponents();

        if (onFile)
        {
            m_out = std::ofstream(GetLoggingFilePath());
            std::clog.rdbuf(m_out.rdbuf());
        }

        NS_LOG_INFO("####");
        NS_LOG_INFO("# Drone Simulation");
        NS_LOG_INFO("# Scenario: " << GetName());
        NS_LOG_INFO("# Date: " << GetCurrentDateTime());
        NS_LOG_INFO("####");

        NS_LOG_LOGIC("Number of drones: " << GetN("drones"));
        NS_LOG_LOGIC("Number of ZSPs: " << GetN("ZSPs"));
        NS_LOG_LOGIC("Number of plain nodes: " << GetN("nodes"));
        NS_LOG_LOGIC("Number of remotes: " << GetN("remotes"));
        NS_LOG_LOGIC("Duration: " << GetDuration() << "s");
    }

    void ScenarioConfigurationHelper::DisposeLogging()
    {
        std::clog.rdbuf(std::cout.rdbuf());
        m_out.close();
    }

    void ScenarioConfigurationHelper::EnableLogComponents() const
    {
        if (m_config.HasMember("logComponents"))
        {
            NS_ASSERT_MSG(m_config["logComponents"].IsArray(),
                          "'logComponents' property must be an array.");

            for (auto i = m_config["logComponents"].Begin(); i != m_config["logComponents"].End();
                 i++)
            {
                NS_ASSERT_MSG((*i).IsString(), "'logComponents' elements must be strings.");
                LogComponentEnable((*i).GetString(), LOG_ALL);
            }
        }

        LogComponentEnableAll(LOG_PREFIX_ALL);
    }

    const uint32_t ScenarioConfigurationHelper::GetAntennasN() const
    {
        NS_ASSERT(m_config.HasMember("antennas"));
        NS_ASSERT(m_config["antennas"].IsArray());
        NS_ASSERT_MSG(m_config["antennas"].Size() > 0,
                      "Please define at least one antenna in configuration file.");

        return m_config["antennas"].Size();
    }

    void ScenarioConfigurationHelper::GetAntennasPosition(Ptr<ListPositionAllocator> allocator)
        const
    {
        for (auto i = m_config["antennas"].Begin(); i != m_config["antennas"].End(); i++)
        {
            NS_ASSERT_MSG(i->IsObject(), "Each antenna must be a JSON object.");
            NS_ASSERT_MSG(i->HasMember("position"),
                          "One or more antennas do not have defined position.");

            NS_ASSERT_MSG((*i)["position"].IsArray() && (*i)["position"].Size() == 3 &&
                              (*i)["position"][0].IsDouble() && (*i)["position"][1].IsDouble() &&
                              (*i)["position"][2].IsDouble(),
                          "Please check that each antenna position is an array of 3 doubles.");

            Vector v((*i)["position"][0].GetDouble(),
                     (*i)["position"][1].GetDouble(),
                     (*i)["position"][2].GetDouble());

            NS_LOG_LOGIC("Allocating an antenna in space at " << v);
            allocator->Add(v);
        }
    }

    const uint32_t ScenarioConfigurationHelper::GetRemotesN() const
    {
        if (!m_config.HasMember("remotes"))
        {
            return 0;
        }

        NS_ASSERT_MSG(m_config["remotes"].IsArray(), "JSON property 'remotes' must be an array.");

        return m_config["remotes"].Size();
    }

    const double ScenarioConfigurationHelper::GetRemoteApplicationStartTime(uint32_t i) const
    {
        const auto remotes = m_config["remotes"].GetArray();
        const auto remote = remotes[i].GetObject();

        if (remote.HasMember("applicationStartTime") && remote["applicationStartTime"].IsDouble())
        {
            return remote["applicationStartTime"].GetDouble();
        }
        else
        {
            return 0.0;
        }
    }

    const double ScenarioConfigurationHelper::GetRemoteApplicationStopTime(uint32_t i) const
    {
        const auto remotes = m_config["remotes"].GetArray();
        const auto remote = remotes[i].GetObject();

        if (remote.HasMember("applicationStopTime") && remote["applicationStopTime"].IsDouble())
        {
            return remote["applicationStopTime"].GetDouble();
        }
        else
        {
            return GetDuration();
        }
    }

    const std::vector<std::pair<std::string, std::string>>
    ScenarioConfigurationHelper::GetGlobalSettings() const
    {
        NS_LOG_FUNCTION_NOARGS();

        std::vector<std::pair<std::string, std::string>> settings;

        if (m_config.HasMember("settings"))
        {
            NS_ASSERT(m_config["settings"].IsArray());

            NS_ASSERT_MSG(m_config["settings"].Size() % 2 == 0,
                          "Check \"settings\": elements in list are not an even number, something "
                          "is missing.");

            for (auto i = m_config["settings"].Begin(); i != m_config["settings"].End(); i += 2)
            {
                NS_ASSERT((*i).IsString());
                NS_ASSERT((*(i + 1)).IsString());

                if ((*i).GetString()[0] == '/')
                {
                    continue;
                }
                settings.push_back({(*i).GetString(), (*(i + 1)).GetString()});
            }
        }

        return settings;
    }

    const std::vector<std::pair<std::string, std::string>>
    ScenarioConfigurationHelper::GetIndividualSettings() const
    {
        NS_LOG_FUNCTION_NOARGS();

        std::vector<std::pair<std::string, std::string>> settings;

        if (m_config.HasMember("settings"))
        {
            NS_ASSERT(m_config["settings"].IsArray());

            NS_ASSERT_MSG(m_config["settings"].Size() % 2 == 0,
                          "Check \"settings\": elements in list are not an even number, something "
                          "is missing.");

            for (auto i = m_config["settings"].Begin(); i != m_config["settings"].End(); i += 2)
            {
                NS_ASSERT((*i).IsString());
                NS_ASSERT((*(i + 1)).IsString());

                if ((*i).GetString()[0] != '/')
                {
                    continue;
                }
                settings.push_back({(*i).GetString(), (*(i + 1)).GetString()});
            }
        }

        return settings;
    }

    const std::vector<Ptr<Building>> ScenarioConfigurationHelper::GetBuildings() const
    {
        NS_LOG_FUNCTION_NOARGS();

        if (!m_config.HasMember("world"))
        {
            return {};
        }

        std::vector<Ptr<Building>> buildings;

        NS_ASSERT_MSG(m_config["world"].IsObject(),
                      "'world' defined in the JSON configuration must be an object.");

        if (!m_config["world"].HasMember("buildings"))
        {
            return {};
        }

        NS_ASSERT_MSG(m_config["world"]["buildings"].IsArray(),
                      "'buildings' needs to be an array of objects, check the configuration file.");

        auto arr = m_config["world"]["buildings"].GetArray();
        for (auto b = arr.Begin(); b != arr.End(); ++b)
        {
            Ptr<Building> building = CreateObject<Building>();

            NS_ASSERT_MSG(
                b->HasMember("boundaries") && (*b)["boundaries"].IsArray() &&
                    (*b)["boundaries"].Size() == 6,
                "'boundaries' must be defined as an array of 6 doubles for each building.");
            auto bounds = (*b)["boundaries"].GetArray();
            for (uint8_t i = 0; i < 6; ++i)
            {
                NS_ASSERT_MSG(bounds[i].IsDouble(), "'boundaries' elements must be doubles.");
            }
            building->SetBoundaries(Box(bounds[0].GetDouble(),
                                        bounds[1].GetDouble(),
                                        bounds[2].GetDouble(),
                                        bounds[3].GetDouble(),
                                        bounds[4].GetDouble(),
                                        bounds[5].GetDouble()));

            if (b->HasMember("type"))
            {
                auto type = std::string((*b)["type"].GetString());
                NS_ASSERT_MSG(type == "residential" || type == "office" || type == "commercial",
                              "Unknown type of building: '" << type << "'.");
                if (type == "residential")
                {
                    building->SetBuildingType(Building::Residential);
                }
                if (type == "office")
                {
                    building->SetBuildingType(Building::Office);
                }
                if (type == "commercial")
                {
                    building->SetBuildingType(Building::Commercial);
                }
            }

            if (b->HasMember("walls"))
            {
                auto walls = std::string((*b)["walls"].GetString());
                NS_ASSERT_MSG(walls == "wood" || walls == "concreteWithWindows" ||
                                  walls == "concreteWithoutWindows" || walls == "stoneBlocks",
                              "Unknown type of walls for a building: '" << walls << "'.");
                if (walls == "wood")
                {
                    building->SetExtWallsType(Building::Wood);
                }
                if (walls == "concreteWithWindows")
                {
                    building->SetExtWallsType(Building::ConcreteWithWindows);
                }
                if (walls == "concreteWithoutWindows")
                {
                    building->SetExtWallsType(Building::ConcreteWithoutWindows);
                }
                if (walls == "stoneBlocks")
                {
                    building->SetExtWallsType(Building::StoneBlocks);
                }
            }

            if (b->HasMember("floors"))
            {
                NS_ASSERT_MSG((*b)["floors"].IsInt(), "'floors' must be an integer.");
                building->SetNFloors((*b)["floors"].GetInt());
            }

            if (b->HasMember("rooms"))
            {
                auto rooms = (*b)["rooms"].GetArray();
                NS_ASSERT_MSG(rooms.Size() == 2 && rooms[0].IsInt() && rooms[1].IsInt(),
                              "'rooms' needs to be an array of 2 integers.");
                building->SetNRoomsX(rooms[0].GetInt());
                building->SetNRoomsY(rooms[1].GetInt());
            }

            buildings.push_back(building);
        }

        return buildings;
    }

    const std::string ScenarioConfigurationHelper::GetDroneMobilityModel(uint32_t n) const
    {
        const auto drones = m_config["drones"].GetArray();
        const auto drone = drones[n].GetObject();

        if (drone.HasMember("mobilityModel") && drone["mobilityModel"].IsString())
        {
            return drone["mobilityModel"].GetString();
        }
        return "";
    }

    const Vector ScenarioConfigurationHelper::GetDronePosition(uint32_t n) const
    {
        const auto drones = m_config["drones"].GetArray();
        const auto drone = drones[n].GetObject();

        NS_ASSERT_MSG(GetDroneMobilityModel(n) == "ns3::ConstantPositionMobilityModel" ||
                          GetDronesMobilityModel() == "ns3::ConstantPositionMobilityModel",
                      "Drone position parameter can be used only when the mobility model is "
                      "ns3::ConstantPositionMobilityModel");

        NS_ASSERT_MSG(drone.HasMember("position"),
                      "Drone #" << n << " does not have a position defined.");

        NS_ASSERT_MSG(drone["position"].IsArray() && drone["position"].Size() == 3 &&
                          drone["position"][0].IsDouble() && drone["position"][1].IsDouble() &&
                          drone["position"][2].IsDouble(),
                      "Please check that drone #" << n << " position is an array of 3 doubles.");

        Vector v(drone["position"][0].GetDouble(),
                 drone["position"][1].GetDouble(),
                 drone["position"][2].GetDouble());

        return v;
    }

    const std::string ScenarioConfigurationHelper::GetObjectName(const char* field, uint32_t index)
        const
    {
        NS_ASSERT(index < m_config[field].GetArray().Size());
        return m_config[field].GetArray()[index].GetObject()["name"].GetString();
    }

    const uint32_t ScenarioConfigurationHelper::GetObjectIndex(const char* field,
                                                               const std::string& name) const
    {
        auto array = m_config[field].GetArray();
        for (uint32_t i = 0; i < array.Size(); i++)
        {
            std::string objName = array[i].GetObject()["name"].GetString();
            if (objName == name)
            {
                return i;
            }
        }
        // NS_LOG_ERROR("No element found with name \"" << name << "\" in field \"" << field <<
        // "\".");
        return array.Size();
    }

    const std::vector<uint32_t> ScenarioConfigurationHelper::GetDroneNetworks(uint32_t id) const
    {
        const auto drones = m_config["drones"].GetArray();
        const auto drone = drones[id].GetObject();

        NS_ASSERT_MSG(drone.HasMember("interfaces"),
                      "The drone " << id << " has no key \"interfaces\".");

        const auto nets = drone["interfaces"].GetArray();

        std::vector<uint32_t> droneNetworks;

        for (auto net = nets.Begin(); net != nets.End(); net++)
        {
            if (net->IsInt())
            {
                droneNetworks.push_back(net->GetUint());
            }
            else if (net->IsString())
            {
                droneNetworks.push_back(GetObjectIndex("networks", net->GetString()));
            }
            else
            {
                NS_LOG_ERROR("A network in array is neither a string nor an integer.");
            }
        }

        return droneNetworks;
    }

    const std::vector<uint32_t> ScenarioConfigurationHelper::GetDroneNetworks(
        const std::string& name) const
    {
        return GetDroneNetworks(GetObjectIndex("drones", name));
    }

    const std::vector<uint32_t> ScenarioConfigurationHelper::GetAntennaNetworks(uint32_t id) const
    {
        const auto antennas = m_config["antennas"].GetArray();
        const auto antenna = antennas[id].GetObject();

        NS_ASSERT_MSG(antenna.HasMember("interfaces"),
                      "The antenna " << id << " has no key \"interfaces\".");

        const auto nets = antenna["interfaces"].GetArray();

        std::vector<uint32_t> antennaNetworks;

        for (auto net = nets.Begin(); net != nets.End(); net++)
        {
            if (net->IsInt())
            {
                antennaNetworks.push_back(net->GetUint());
            }
            else if (net->IsString())
            {
                antennaNetworks.push_back(GetObjectIndex("networks", net->GetString()));
            }
            else
            {
                NS_LOG_ERROR("A network in array is neither a string nor an integer.");
            }
        }

        return antennaNetworks;
    }

    const std::vector<uint32_t> ScenarioConfigurationHelper::GetAntennaNetworks(
        const std::string& name) const
    {
        return GetAntennaNetworks(GetObjectIndex("antennas", name));
    }

    const std::vector<uint32_t> ScenarioConfigurationHelper::GetDronesInNetwork(uint32_t id) const
    {
        std::vector<uint32_t> dronesInNet;
        auto drones = m_config["drones"].GetArray();
        for (uint32_t i = 0; i < drones.Size(); i++)
        {
            auto nets = drones[i].GetObject()["interfaces"].GetArray();
            for (auto el = nets.Begin(); el != nets.End(); el++)
            {
                if ((el->IsInt() && el->GetUint() == id) ||
                    (el->IsString() && GetObjectIndex("networks", el->GetString()) == id))
                {
                    dronesInNet.push_back(i);
                }
            }
        }
        return dronesInNet;
    }

    const std::vector<uint32_t> ScenarioConfigurationHelper::GetDronesInNetwork(
        const std::string& net_name) const
    {
        return GetDronesInNetwork(GetObjectIndex("networks", net_name));
    }

    const std::vector<uint32_t> ScenarioConfigurationHelper::GetAntennasInNetwork(uint32_t id) const
    {
        std::vector<uint32_t> antennasInNet;
        auto antennas = m_config["antennas"].GetArray();
        for (uint32_t i = 0; i < antennas.Size(); i++)
        {
            auto nets = antennas[i].GetObject()["interfaces"].GetArray();
            for (auto el = nets.Begin(); el != nets.End(); el++)
            {
                if ((el->IsInt() && el->GetUint() == id) ||
                    (el->IsString() && GetObjectIndex("networks", el->GetString()) == id))
                {
                    antennasInNet.push_back(i);
                }
            }
        }
        return antennasInNet;
    }

    const std::vector<uint32_t> ScenarioConfigurationHelper::GetAntennasInNetwork(
        const std::string& net_name) const
    {
        return GetAntennasInNetwork(GetObjectIndex("networks", net_name));
    }

    const uint32_t ScenarioConfigurationHelper::RadioMap() const
    {
        return m_radioMap;
    }

    const std::vector<std::pair<std::string, std::string>>
    ScenarioConfigurationHelper::GetRadioMapParameters() const
    {
        std::vector<std::pair<std::string, std::string>> parameters;
        NS_ASSERT_MSG(m_config.HasMember("radioMapParameters"),
                      "'radioMapParameters' key is not present in the configuration file.");

        NS_ASSERT_MSG(m_config["radioMapParameters"].IsArray() &&
                          m_config["radioMapParameters"].Size() % 2 == 0,
                      "Check 'radioMapParameters': should be an array of even number elements.");

        for (auto i = m_config["radioMapParameters"].Begin();
             i != m_config["radioMapParameters"].End();
             i += 2)
        {
            NS_ASSERT((*i).IsString());
            NS_ASSERT((*(i + 1)).IsString());

            parameters.push_back({(*i).GetString(), (*(i + 1)).GetString()});
        }

        return parameters;
    }

    const std::string ScenarioConfigurationHelper::MakePath(const std::string& path1,
                                                            const std::string& path2 /* ="" */)
        const
    {
        std::string npath(path1);
        if (npath.at(0) != '/')
        {
            npath.insert(0, "/");
        }
        if (!path2.empty())
        {
            if (npath.back() != '/')
            {
                npath.push_back('/');
            }
            npath.append(path2);
        }
        if (npath.back() == '/')
        {
            npath.pop_back();
        }
        return npath;
    }

    const std::string ScenarioConfigurationHelper::MakePath(const std::string& path, uint32_t index)
        const
    {
        return MakePath(path, std::to_string(index));
    }

    bool ScenarioConfigurationHelper::CheckPath(const std::string& path) const
    {
        return rapidjson::Pointer(MakePath(path).c_str()).Get(m_config);
    }

    const std::pair<bool, int32_t> ScenarioConfigurationHelper::GetInt(const std::string& path)
        const
    {
        const rapidjson::Value* value = rapidjson::Pointer(MakePath(path).c_str()).Get(m_config);
        if (!value)
        {
            return std::make_pair<bool, int32_t>(false, 0);
        }

        NS_ASSERT_MSG(value->IsInt(), "Object at path '" << path << "' is not an integer");

        return std::make_pair<bool, int32_t>(true, value->GetInt());
    }

    const std::pair<bool, uint32_t> ScenarioConfigurationHelper::GetUint(const std::string& path)
        const
    {
        const rapidjson::Value* value = rapidjson::Pointer(MakePath(path).c_str()).Get(m_config);
        if (!value)
        {
            return std::make_pair<bool, uint32_t>(false, 0);
        }

        NS_ASSERT_MSG(value->IsUint(),
                      "Object at path '" << path << "' is not an unsigned integer");

        return std::make_pair<bool, uint32_t>(true, value->GetUint());
    }

    const std::pair<bool, double> ScenarioConfigurationHelper::GetDouble(const std::string& path)
        const
    {
        const rapidjson::Value* value = rapidjson::Pointer(MakePath(path).c_str()).Get(m_config);
        if (!value)
        {
            return std::make_pair<bool, double>(false, 0.0);
        }

        NS_ASSERT_MSG(value->IsDouble(), "Object at path '" << path << "' is not a double");

        return std::make_pair<bool, double>(true, value->GetDouble());
    }

    const std::pair<bool, bool> ScenarioConfigurationHelper::GetBool(const std::string& path) const
    {
        const rapidjson::Value* value = rapidjson::Pointer(MakePath(path).c_str()).Get(m_config);
        if (!value)
        {
            return std::make_pair<bool, bool>(false, false);
        }

        NS_ASSERT_MSG(value->IsBool(), "Object at path '" << path << "' is not a boolean");

        return std::make_pair<bool, bool>(true, value->GetBool());
    }

    const std::pair<bool, std::string> ScenarioConfigurationHelper::GetString(
        const std::string& path) const
    {
        const rapidjson::Value* value = rapidjson::Pointer(MakePath(path).c_str()).Get(m_config);
        if (!value)
        {
            return std::make_pair<bool, std::string>(false, "");
        }

        NS_ASSERT_MSG(value->IsString(), "Object at path '" << path << "' is not a string");

        return std::make_pair<bool, std::string>(true, value->GetString());
    }

    const std::vector<DoubleVector> ScenarioConfigurationHelper::GetRegionsOfInterest() const
    {
        std::vector<DoubleVector> final_regions;

        if (!m_config.HasMember("world"))
        {
            return final_regions;
        }

        const auto world = m_config["world"].GetObject();
        if (world.HasMember("regionsOfInterest"))
        {
            const auto regions = world["regionsOfInterest"].GetArray();
            for (auto region = regions.Begin(); region != regions.End(); region++)
            {
                const auto xMin = (*region)[0].GetDouble();
                const auto xMax = (*region)[1].GetDouble();
                const auto yMin = (*region)[2].GetDouble();
                const auto yMax = (*region)[3].GetDouble();
                const auto zMin = (*region)[4].GetDouble();
                const auto zMax = (*region)[5].GetDouble();
                final_regions.push_back(DoubleVector({xMin, xMax, yMin, yMax, zMin, zMax}));
            }
        }
        return final_regions;
    }

} // namespace ns3
