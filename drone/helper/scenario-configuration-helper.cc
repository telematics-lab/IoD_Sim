/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2021 The IoD_Sim Authors.
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

#include <chrono>
#include <iomanip>  /* put_time */
#include <iostream>

#include <rapidjson/filereadstream.h>
#include <ns3/command-line.h>
#include <ns3/integer.h>
#include <ns3/log.h>
#include <ns3/mac-layer-configuration-helper.h>
#include <ns3/network-layer-configuration-helper.h>
#include <ns3/object-factory.h>
#include <ns3/phy-layer-configuration-helper.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE_MASK ("ScenarioConfigurationHelper", LOG_PREFIX_ALL);

void
ScenarioConfigurationHelper::Initialize (int argc,
                                         char **argv)
{
  auto now = std::chrono::system_clock::now ();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::stringstream dateTime;

  dateTime << std::put_time (std::localtime (&in_time_t), "%Y-%m-%d.%H-%M-%S");

  m_dateTime = dateTime.str ();

  InitializeConfiguration (argc, argv);
  InitializeLogging (GetLogOnFile ());
}

void
ScenarioConfigurationHelper::Initialize (int argc,
                                         char **argv,
                                         const std::string name)
{
  auto now = std::chrono::system_clock::now ();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::stringstream dateTime;

  dateTime << std::put_time (std::localtime (&in_time_t), "%Y-%m-%d.%H-%M-%S");

  m_dateTime = dateTime.str ();
  m_name = name;

  InitializeConfiguration (argc, argv);
  InitializeLogging (GetLogOnFile ());
}

ScenarioConfigurationHelper::~ScenarioConfigurationHelper ()
{
  DisposeLogging ();
  DisposeConfiguration ();
}

const std::string
ScenarioConfigurationHelper::GetName ()
{
  if (m_name.empty()) {
    NS_ASSERT (m_config.HasMember ("name"));
    NS_ASSERT_MSG (m_config["name"].IsString(),
                   "Please define scenario name in configuration file.");

    m_name = m_config["name"].GetString();
  }

  return m_name;
}

const std::string
ScenarioConfigurationHelper::GetCurrentDateTime () const
{
  return m_dateTime;
}

const std::string
ScenarioConfigurationHelper::GetResultsPath ()
{
  std::stringstream ss;

  ss << "../results/"
     << GetName() << "-"
     << m_dateTime;

  return ss.str ();
}

const std::string
ScenarioConfigurationHelper::GetLoggingFilePath ()
{
  std::stringstream ss;

  ss << GetResultsPath () << ".log";

  return ss.str ();
}

const std::vector<std::pair<std::string, std::string>>
ScenarioConfigurationHelper::GetStaticConfig ()
{
  if (m_staticConfig.empty ())
    {
      NS_ASSERT (m_config.HasMember ("staticNs3Config"));
      NS_ASSERT_MSG (m_config["staticNs3Config"].IsArray (),
                     "Please define staticNs3Config in configuration file.");

      std::vector<std::pair<std::string, std::string>> staticConfigsDecoded = {};
      const auto staticConfigsArr = m_config["staticNs3Config"].GetArray ();
      for (auto& sc : staticConfigsArr)
        {
          NS_ASSERT_MSG (sc.IsObject (),
                        "A static config definition is invalid.");

          const auto obj = sc.GetObject ();
          NS_ASSERT (obj.HasMember ("name"));
          NS_ASSERT_MSG (obj["name"].IsString (),
                         "'name' is required in staticNs3Config definition.");
          NS_ASSERT (obj.HasMember ("value"));
          NS_ASSERT_MSG (obj["value"].IsString (),
                         "'value' is required in staticNs3Config definiton.");

          staticConfigsDecoded.push_back({
            obj["name"].GetString (),
            obj["value"].GetString ()
          });
        }

      m_staticConfig = staticConfigsDecoded;
    }

  return m_staticConfig;
}

const std::vector<Ptr<PhyLayerConfiguration>>
ScenarioConfigurationHelper::GetPhyLayers () const
{
  NS_ASSERT (m_config.HasMember ("phyLayer"));
  NS_ASSERT_MSG (m_config["phyLayer"].IsArray (),
                 "Please define phyLayer in your JSON configuration.");

  const auto arr = m_config["phyLayer"].GetArray ();
  std::vector<Ptr<PhyLayerConfiguration>> phyConfs;
  for(auto& el : arr)
    {
      auto conf = PhyLayerConfigurationHelper::GetConfiguration (el);
      phyConfs.emplace_back (conf);
    }

  return phyConfs;
}

const std::vector<Ptr<MacLayerConfiguration>>
ScenarioConfigurationHelper::GetMacLayers ()const
{
  NS_ASSERT (m_config.HasMember ("macLayer"));
  NS_ASSERT_MSG (m_config["macLayer"].IsArray (),
                 "Please define macLayer in your JSON configuration.");

  const auto arr = m_config["macLayer"].GetArray ();
  std::vector<Ptr<MacLayerConfiguration>> macConfs;
  for(auto& el : arr)
    {
      auto conf = MacLayerConfigurationHelper::GetConfiguration (el);
      macConfs.emplace_back (conf);
    }

  return macConfs;
}

const std::vector<Ptr<NetworkLayerConfiguration>>
ScenarioConfigurationHelper::GetNetworkLayers () const
{
  NS_ASSERT (m_config.HasMember ("networkLayer"));
  NS_ASSERT_MSG (m_config["networkLayer"].IsArray (),
                 "Please define networkLayer in your JSON configuration.");

  const auto arr = m_config["networkLayer"].GetArray ();
  std::vector<Ptr<NetworkLayerConfiguration>> netConfs;
  for(auto& el : arr)
    {
      auto conf = NetworkLayerConfigurationHelper::GetConfiguration (el);
      netConfs.emplace_back (conf);
    }

  return netConfs;
}

const std::vector<Ptr<EntityConfiguration>>
ScenarioConfigurationHelper::GetEntitiesConfiguration (const std::string& entityKey) const
{
  const char* entityKeyCStr = entityKey.c_str ();
  NS_ASSERT (m_config.HasMember (entityKeyCStr));
  NS_ASSERT_MSG (m_config[entityKeyCStr].IsArray (),
                 "Please define drones in your JSON configuration.");

  const auto arr = m_config[entityKeyCStr].GetArray ();
  std::vector<Ptr<EntityConfiguration>> entityConf;

  for (auto& el : arr)
    {
      auto conf = EntityConfigurationHelper::GetConfiguration(el);
      entityConf.push_back (conf);
    }

  return entityConf;
}

const std::string
ScenarioConfigurationHelper::GetPhyPropagationLossModel () const
{
  constexpr const char* jRootKey = "phyPropagationLoss";
  constexpr const char* jModelKey = "model";

  NS_ASSERT (m_config.HasMember (jRootKey));
  NS_ASSERT_MSG (m_config[jRootKey].IsObject (),
                 jRootKey << " must be a valid JSON object per specification.");

  NS_ASSERT (m_config[jRootKey].HasMember (jModelKey));
  NS_ASSERT_MSG (m_config[jRootKey][jModelKey].IsString (),
                 jModelKey << " must be a valid JSON string per specification.");

  return m_config[jRootKey][jModelKey].GetString ();
}

const std::vector<std::pair<std::string, float>>
ScenarioConfigurationHelper::GetThreeLogDistancePropagationLossModelAttributes () const
{
  constexpr const char* jRootKey = "phyPropagationLoss";
  constexpr const char* jAttrKey = "attributes";

  NS_ASSERT (m_config.HasMember (jRootKey));
  NS_ASSERT_MSG (m_config[jRootKey].IsObject (),
                 jRootKey << " must be a valid JSON object per specification.");

  NS_ASSERT (m_config[jRootKey].HasMember (jAttrKey));
  NS_ASSERT_MSG (m_config[jRootKey][jAttrKey].IsArray (),
                 jAttrKey << " must be a valid JSON array per specification.");

  const auto jAttrArr = m_config[jRootKey][jAttrKey].GetArray ();

  // check if we have an array of objects
  for (auto& el : jAttrArr) {
    NS_ASSERT_MSG (el.IsObject (),
                   jAttrKey << " must be a valid array of objects.");

    NS_ASSERT_MSG (el.HasMember("key")
                   && el["key"].IsString ()
                   && el.HasMember("value")
                   && !el["value"].IsArray()
                   && !el["value"].IsObject(),
                   jAttrKey << " contains a malformed structure.");
  }

  // "safe" to decode the data
  std::vector<std::pair<std::string, float>> attrList;

  for (auto& el : jAttrArr) {
    const std::string attrKey = el["key"].GetString();
    const float attrVal = el["value"].GetFloat();

    attrList.push_back({attrKey, attrVal});
  }

  return attrList;
}

const std::string
ScenarioConfigurationHelper::GetPhyMode () const
{
  NS_ASSERT (m_config.HasMember ("phyMode"));
  NS_ASSERT_MSG (m_config["phyMode"].IsString (),
                 "Please define phyMode in configuration file.");

  return m_config["phyMode"].GetString ();
}

const double
ScenarioConfigurationHelper::GetDuration () const
{
  NS_ASSERT (m_config.HasMember ("duration"));
  NS_ASSERT_MSG (m_config["duration"].IsUint (),
                 "Please define duration in configuration file.");

  return m_config["duration"].GetDouble ();
}

const float
ScenarioConfigurationHelper::GetCurveStep () const
{
  if (m_config.HasMember ("curveStep"))
    {
      NS_ASSERT (m_config["curveStep"].IsFloat ());
      return m_config["curveStep"].GetFloat ();
    }
  else
    {
      return 0.001;
    }
}

const uint32_t
ScenarioConfigurationHelper::GetDronesN () const
{
  NS_ASSERT (m_config.HasMember ("drones"));
  NS_ASSERT (m_config["drones"].IsArray ());
  NS_ASSERT_MSG (m_config["drones"].Size () > 0,
                 "Please define at least one drone in configuration file.");

  return m_config["drones"].Size ();
}

const FlightPlan
ScenarioConfigurationHelper::GetDroneFlightPlan (uint32_t i) const
{
  // checks for drones were already made in ::ConfGetNumDrones.
  // Let's skip them.
  const auto drones = m_config["drones"].GetArray ();
  const auto drone  = drones[i].GetObject ();

  FlightPlan flightPlan;

  for (auto point = drone["trajectory"].Begin ();
       point != drone["trajectory"].End ();
       point++)
    {
      NS_ASSERT_MSG (point->IsObject (),
                     "Each point in drone must be a JSON object.");
      NS_ASSERT_MSG (point->HasMember ("position")
                     && (*point)["position"].IsArray ()
                     && (*point)["position"][0].IsDouble ()
                     && (*point)["position"][1].IsDouble ()
                     && (*point)["position"][2].IsDouble ()
                     && point->HasMember ("interest")
                     && (*point)["interest"].IsUint (),
                     "Cannot decode trajectory, please check that each point has at least position and interest.");

      // restTime is optional
      double restTime = 0.0;
      if (point->HasMember("restTime"))
        {
          NS_ASSERT ((*point)["restTime"].IsDouble ());
          restTime = (*point)["restTime"].GetDouble ();
        }

      auto protoPoint = CreateObjectWithAttributes<ProtoPoint>
        ("Position", VectorValue ({(*point)["position"][0].GetDouble (),
                                   (*point)["position"][1].GetDouble (),
                                   (*point)["position"][2].GetDouble ()}),
          "Interest", IntegerValue ((*point)["interest"].GetUint ()),
          "RestTime", TimeValue (Seconds (restTime)));

      flightPlan.Add (protoPoint);
    }

  return flightPlan;
}

const double
ScenarioConfigurationHelper::GetDroneAcceleration (uint32_t i) const
{
  // checks for drones were already made in ::ConfGetNumDrones.
  // Let's skip them.
  const auto drones = m_config["drones"].GetArray ();
  const auto drone  = drones[i].GetObject ();

  NS_ASSERT_MSG (drone.HasMember ("acceleration")
                 && drone["acceleration"].IsDouble (),
                 "Every done must have defined the parameter \"acceleration\".");

  return drone["acceleration"].GetDouble ();
}

const double
ScenarioConfigurationHelper::GetDroneMaxSpeed (uint32_t i) const
{
  // checks for drones were already made in ::ConfGetNumDrones.
  // Let's skip them.
  const auto drones = m_config["drones"].GetArray ();
  const auto drone  = drones[i].GetObject ();

  NS_ASSERT_MSG (drone.HasMember ("maxSpeed")
                 && drone["maxSpeed"].IsDouble (),
                 "Every done must have defined the parameter \"maxSpeed\".");

  return drone["maxSpeed"].GetDouble ();
}

const SpeedCoefficients
ScenarioConfigurationHelper::GetDroneSpeedCoefficients (uint32_t i) const
{
  // checks for drones were already made in ::ConfGetNumDrones.
  // Let's skip them.

  const auto drones = m_config["drones"].GetArray ();
  const auto drone  = drones[i].GetObject ();

  SpeedCoefficients speedCoefficients;

  NS_ASSERT (drone.HasMember ("speedCoefficients")
             && drone["speedCoefficients"].IsArray ()
             && drone["speedCoefficients"].Size () > 0);

  for (auto coefficient = drone["speedCoefficients"].Begin ();
       coefficient != drone["speedCoefficients"].End ();
       coefficient++)
    {
      NS_ASSERT ((*coefficient).IsDouble ());

      speedCoefficients.Add ((*coefficient).GetDouble ());
    }

  return speedCoefficients;
}

const double
ScenarioConfigurationHelper::GetDroneApplicationStartTime (uint32_t i) const
{
  // checks for drones were already made in ::ConfGetNumDrones.
  // Let's skip them.
  const auto drones = m_config["drones"].GetArray ();
  const auto drone  = drones[i].GetObject ();

  if (drone.HasMember ("applicationStartTime")
      && drone["applicationStartTime"].IsDouble ())
    return drone["applicationStartTime"].GetDouble ();
  else
    return 0.0;
}

const double
ScenarioConfigurationHelper::GetDroneApplicationStopTime (uint32_t i) const
{
  // checks for drones were already made in ::ConfGetNumDrones.
  // Let's skip them.
  const auto drones = m_config["drones"].GetArray ();
  const auto drone  = drones[i].GetObject ();

  if (drone.HasMember ("applicationStopTime")
      && drone["applicationStopTime"].IsDouble ())
    return drone["applicationStopTime"].GetDouble ();
  else
    return GetDuration ();
}

const uint32_t
ScenarioConfigurationHelper::GetZspsN () const
{
  NS_ASSERT (m_config.HasMember ("ZSPs"));
  NS_ASSERT (m_config["ZSPs"].IsArray ());
  NS_ASSERT_MSG (m_config["ZSPs"].Size () > 0,
                 "Please define at least one ZSP in configuration file.");

  return m_config["ZSPs"].Size ();
}

void
ScenarioConfigurationHelper::GetZspsPosition (Ptr<ListPositionAllocator> allocator) const
{
  // checks for ZSP array were already made in ::ConfGetNumZsps.
  // Let's skip them.
  for (auto i = m_config["ZSPs"].Begin (); i != m_config["ZSPs"].End (); i++)
    {
      NS_ASSERT_MSG (i->IsObject (),
                     "Each ZSP must be a JSON object.");
      NS_ASSERT_MSG (i->HasMember ("position"),
                     "One or more ZSPs do not have defined position.");

      NS_ASSERT_MSG ((*i)["position"].IsArray ()
                     && (*i)["position"].Size () == 3
                     && (*i)["position"][0].IsDouble ()
                     && (*i)["position"][1].IsDouble ()
                     && (*i)["position"][2].IsDouble (),
                     "Please check that each ZSP position is an array of 3 doubles.");

      Vector v ((*i)["position"][0].GetDouble (),
                (*i)["position"][1].GetDouble (),
                (*i)["position"][2].GetDouble ());

      NS_LOG_LOGIC ("Allocating a ZSP in space at " << v);
      allocator->Add (v);
    }
}

const double
ScenarioConfigurationHelper::GetZspApplicationStartTime (uint32_t i) const
{
  // checks for drones were already made in ::ConfGetNumDrones.
  // Let's skip them.
  const auto zsps = m_config["ZSPs"].GetArray ();
  const auto zsp  = zsps[i].GetObject ();

  if (zsp.HasMember ("applicationStartTime")
      && zsp["applicationStartTime"].IsDouble ())
    return zsp["applicationStartTime"].GetDouble ();
  else
    return 0.0;
}

const double
ScenarioConfigurationHelper::GetZspApplicationStopTime (uint32_t i) const
{
  // checks for drones were already made in ::ConfGetNumDrones.
  // Let's skip them.
  const auto zsps = m_config["ZSPs"].GetArray ();
  const auto zsp  = zsps[i].GetObject ();

  if (zsp.HasMember ("applicationStopTime")
      && zsp["applicationStopTime"].IsDouble ())
    return zsp["applicationStopTime"].GetDouble ();
  else
    return GetDuration ();
}

const uint32_t
ScenarioConfigurationHelper::GetEnbsN () const
{
  NS_ASSERT (m_config.HasMember ("ENBs"));
  NS_ASSERT (m_config["ENBs"].IsArray ());
  NS_ASSERT_MSG (m_config["ENBs"].Size () > 0,
                 "Please define at least one ENB in configuration file.");

  return m_config["ENBs"].Size ();
}

void
ScenarioConfigurationHelper::GetEnbsPosition (Ptr<ListPositionAllocator> allocator) const
{
  // checks for ZSP array were already made in ::ConfGetNumZsps.
  // Let's skip them.
  for (auto i = m_config["ENBs"].Begin (); i != m_config["ENBs"].End (); i++)
    {
      NS_ASSERT_MSG (i->IsObject (),
                     "Each ENB must be a JSON object.");
      NS_ASSERT_MSG (i->HasMember ("position"),
                     "One or more ENBs do not have defined position.");

      NS_ASSERT_MSG ((*i)["position"].IsArray ()
                     && (*i)["position"].Size () == 3
                     && (*i)["position"][0].IsDouble ()
                     && (*i)["position"][1].IsDouble ()
                     && (*i)["position"][2].IsDouble (),
                     "Please check that each ENB position is an array of 3 doubles.");

      Vector v ((*i)["position"][0].GetDouble (),
                (*i)["position"][1].GetDouble (),
                (*i)["position"][2].GetDouble ());

      NS_LOG_LOGIC ("Allocating a ENB in space at " << v);
      allocator->Add (v);
    }
}

void
ScenarioConfigurationHelper::InitializeConfiguration (int argc, char **argv)
{
  std::string configFilePath = "";
  constexpr const ssize_t configFileBufferSize = 64 * 1024;  // KiB
  char configFileBuffer [configFileBufferSize];

  CommandLine cmd;
  cmd.AddValue ("config", "Configuration file path", configFilePath);
  cmd.Parse (argc, argv);

  if (configFilePath.empty ())
    NS_FATAL_ERROR ("Please specify a non-empty, JSON formatted configuration with --config!");

  // open configuration file and decode JSON data
  m_configFilePtr = fopen (configFilePath.c_str (), "rb");
  if (m_configFilePtr == nullptr)
    NS_FATAL_ERROR ("Cannot open " << configFilePath << ": " << std::strerror (errno));

  rapidjson::FileReadStream jsonFileStream (m_configFilePtr,
                                            configFileBuffer,
                                            configFileBufferSize);
  m_config.ParseStream (jsonFileStream);

  // close???

  NS_ABORT_MSG_IF (m_config.HasParseError (),
                   "The given configuration schema is not valid JSON.");
}

void
ScenarioConfigurationHelper::DisposeConfiguration ()
{
  if (m_configFilePtr != nullptr)
    std::fclose (m_configFilePtr);
}

void
ScenarioConfigurationHelper::InitializeLogging (const bool &onFile)
{
  EnableLogComponents ();

  if (onFile)
    {
      m_out = std::ofstream (GetLoggingFilePath ());
      std::clog.rdbuf (m_out.rdbuf ());
    }

  NS_LOG_INFO ("####");
  NS_LOG_INFO ("# Drone Simulation");
  NS_LOG_INFO ("# Scenario: " << GetName ());
  NS_LOG_INFO ("# Date: "     << GetCurrentDateTime ());
  NS_LOG_INFO ("####");

  NS_LOG_LOGIC ("Number of drones: " << GetDronesN ());
  NS_LOG_LOGIC ("Number of ZSPs: "   << GetZspsN ());
  NS_LOG_LOGIC ("Duration: "         << GetDuration () << "s");
}

void
ScenarioConfigurationHelper::DisposeLogging ()
{
  std::clog.rdbuf (std::cout.rdbuf ());
  m_out.close ();
}

const bool
ScenarioConfigurationHelper::GetLogOnFile () const
{
  // this is an optional parameter. Default to false if not specified.
  if (m_config.HasMember ("logOnFile"))
    {
      NS_ASSERT (m_config["logOnFile"].IsBool ());

      return m_config["logOnFile"].GetBool ();
    }
  else
    {
      return false;
    }
}

void
ScenarioConfigurationHelper::EnableLogComponents () const
{
  if (m_config.HasMember ("logComponents"))
    {
      NS_ASSERT (m_config["logComponents"].IsArray ());

      for (auto i = m_config["logComponents"].Begin ();
           i != m_config["logComponents"].End ();
           i++)
        {
          NS_ASSERT ((*i).IsString ());

          LogComponentEnable ((*i).GetString (), LOG_LEVEL_ALL);
        }
    }
}

} // namespace ns3
