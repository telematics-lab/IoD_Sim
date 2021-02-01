/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2020 The IoD_Sim Authors.
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
#include <ns3/object-factory.h>
#include <ns3/system-path.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE_MASK ("ScenarioConfigurationHelper", LOG_PREFIX_ALL);

void
ScenarioConfigurationHelper::Initialize (int argc,
                                         char **argv,
                                         const std::string name)
{
  auto now = std::chrono::system_clock::now ();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::stringstream dateTime;

  dateTime << std::put_time (std::localtime (&in_time_t), "%Y-%m-%d-%H-%M-%S");

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
ScenarioConfigurationHelper::GetName () const
{
  return m_name;
}

const std::string
ScenarioConfigurationHelper::GetCurrentDateTime () const
{
  return m_dateTime;
}

const std::string
ScenarioConfigurationHelper::GetResultsPath () const
{
  std::stringstream path;

  path << "../results/" << m_name << "-" << m_dateTime;

  SystemPath::MakeDirectories (path.str ());

  path << "/";

  return path.str ();
}

const std::string
ScenarioConfigurationHelper::GetLoggingFilePath () const
{
  std::stringstream ss;

  ss << GetResultsPath () << GetName () << ".log";

  return ss.str ();
}

const double
ScenarioConfigurationHelper::GetDuration () const
{
  NS_ASSERT (m_config.HasMember ("duration"));
  NS_ASSERT_MSG (m_config["duration"].IsDouble (),
                 "Please define duration in configuration file.");

  return m_config["duration"].GetDouble ();
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
  for (auto& el : jAttrArr)
    {
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

  for (auto& el : jAttrArr)
    {
      const std::string attrKey = el["key"].GetString();
      const float attrVal = el["value"].GetFloat();

      attrList.push_back({attrKey, attrVal});
    }

  return attrList;
}

const std::string
ScenarioConfigurationHelper::GetPhyMode () const
{
  NS_ASSERT_MSG (m_config.HasMember ("phyMode")
                 && m_config["phyMode"].IsString (),
                 "Please define phyMode in configuration file.");

  return m_config["phyMode"].GetString ();
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

const std::string
ScenarioConfigurationHelper::GetDronesMobilityModel () const
{
  NS_ASSERT_MSG (m_config.HasMember ("dronesMobilityModel")
                 && m_config["dronesMobilityModel"].IsString (),
                 "Please define dronesMobilityModel in configuration file.");

  return m_config["dronesMobilityModel"].GetString ();
}

void
ScenarioConfigurationHelper::GetDronesPosition (Ptr<ListPositionAllocator> allocator) const
{
  for (auto drone = m_config["drones"].Begin (); drone != m_config["drones"].End (); drone++)
    {
      NS_ASSERT_MSG (GetDronesMobilityModel () == "ns3::ConstantPositionMobilityModel",
                     "Drones position parameter can be used only when dronesMobilityModel is ns3::ConstantPositionMobilityModel");
      NS_ASSERT_MSG (drone->IsObject (),
                     "Each drone must be a JSON object.");
      NS_ASSERT_MSG (drone->HasMember ("position"),
                     "One or more drones do not have defined position.");

      NS_ASSERT_MSG ((*drone)["position"].IsArray ()
                     && (*drone)["position"].Size () == 3
                     && (*drone)["position"][0].IsDouble ()
                     && (*drone)["position"][1].IsDouble ()
                     && (*drone)["position"][2].IsDouble (),
                     "Please check that each drone position is an array of 3 doubles.");

      Vector v ((*drone)["position"][0].GetDouble (),
                (*drone)["position"][1].GetDouble (),
                (*drone)["position"][2].GetDouble ());

      NS_LOG_LOGIC ("Allocating a drone in space at " << v);
      allocator->Add (v);
    }
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

      auto protoPoint = CreateObjectWithAttributes<ProtoPoint>(
          "Position", VectorValue ({(*point)["position"][0].GetDouble (),
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
  const auto drones = m_config["drones"].GetArray ();
  const auto drone  = drones[i].GetObject ();

  if (drone.HasMember ("applicationStartTime") && drone["applicationStartTime"].IsDouble ())
    return drone["applicationStartTime"].GetDouble ();
  else
    return 0.0;
}

const double
ScenarioConfigurationHelper::GetDroneApplicationStopTime (uint32_t i) const
{
  const auto drones = m_config["drones"].GetArray ();
  const auto drone  = drones[i].GetObject ();

  if (drone.HasMember ("applicationStopTime") && drone["applicationStopTime"].IsDouble ())
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

  if (zsp.HasMember ("applicationStartTime") && zsp["applicationStartTime"].IsDouble ())
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

  if (zsp.HasMember ("applicationStopTime") && zsp["applicationStopTime"].IsDouble ())
    return zsp["applicationStopTime"].GetDouble ();
  else
    return GetDuration ();
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
  if (m_config.HasMember ("zsps"))
    NS_LOG_LOGIC ("Number of ZSPs: "   << GetZspsN ());
  if (m_config.HasMember ("antennas"))
    NS_LOG_LOGIC ("Number of antennas: "   << GetAntennasN ());
  if (m_config.HasMember ("remotes"))
    NS_LOG_LOGIC ("Number of remotes: "   << GetRemotesN ());
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


const uint32_t
ScenarioConfigurationHelper::GetAntennasN () const
{
  NS_ASSERT (m_config.HasMember ("antennas"));
  NS_ASSERT (m_config["antennas"].IsArray ());
  NS_ASSERT_MSG (m_config["antennas"].Size () > 0,
                 "Please define at least one antenna in configuration file.");

  return m_config["antennas"].Size ();
}

void
ScenarioConfigurationHelper::GetAntennasPosition (Ptr<ListPositionAllocator> allocator) const
{
  for (auto i = m_config["antennas"].Begin (); i != m_config["antennas"].End (); i++)
    {
      NS_ASSERT_MSG (i->IsObject (),
                     "Each antenna must be a JSON object.");
      NS_ASSERT_MSG (i->HasMember ("position"),
                     "One or more antennas do not have defined position.");

      NS_ASSERT_MSG ((*i)["position"].IsArray ()
                     && (*i)["position"].Size () == 3
                     && (*i)["position"][0].IsDouble ()
                     && (*i)["position"][1].IsDouble ()
                     && (*i)["position"][2].IsDouble (),
                     "Please check that each antenna position is an array of 3 doubles.");

      Vector v ((*i)["position"][0].GetDouble (),
                (*i)["position"][1].GetDouble (),
                (*i)["position"][2].GetDouble ());

      NS_LOG_LOGIC ("Allocating an antenna in space at " << v);
      allocator->Add (v);
    }
}

const uint32_t
ScenarioConfigurationHelper::GetRemotesN () const
{
  NS_ASSERT (m_config.HasMember ("remotes"));
  NS_ASSERT (m_config["remotes"].IsArray ());
  NS_ASSERT_MSG (m_config["remotes"].Size () > 0,
                 "Please define at least one remote host in configuration file.");

  return m_config["remotes"].Size ();
}

const double
ScenarioConfigurationHelper::GetRemoteApplicationStartTime (uint32_t i) const
{
  const auto remotes = m_config["remotes"].GetArray ();
  const auto remote  = remotes[i].GetObject ();

  if (remote.HasMember ("applicationStartTime") && remote["applicationStartTime"].IsDouble ())
    return remote["applicationStartTime"].GetDouble ();
  else
    return 0.0;
}

const double
ScenarioConfigurationHelper::GetRemoteApplicationStopTime (uint32_t i) const
{
  const auto remotes = m_config["remotes"].GetArray ();
  const auto remote  = remotes[i].GetObject ();

  if (remote.HasMember ("applicationStopTime") && remote["applicationStopTime"].IsDouble ())
    return remote["applicationStopTime"].GetDouble ();
  else
    return GetDuration ();
}

const std::string
ScenarioConfigurationHelper::GetProtocol() const
{
  NS_ASSERT_MSG (m_config.HasMember ("protocol")
                 && m_config["protocol"].IsString (),
                 "Please define protocol in configuration file.");

  return m_config["protocol"].GetString ();
}

const std::vector<std::pair<std::string, std::string>>
ScenarioConfigurationHelper::GetProtocolGlobalSettings() const
{
  std::vector<std::pair<std::string, std::string>> settings;

  if (m_config.HasMember ("protocolSettings"))
    {
      NS_ASSERT (m_config["protocolSettings"].IsArray ());

      NS_ASSERT_MSG (m_config["protocolSettings"].Size () % 2 == 0,
                     "Check protocolSettings: elements in list are not an even number, something is missing.");

      for (auto i = m_config["protocolSettings"].Begin (); i != m_config["protocolSettings"].End (); i += 2)
        {
          NS_ASSERT ((*i).IsString ());
          NS_ASSERT ((*(i+1)).IsString());

          if ((*i).GetString ()[0] == '/') continue;
          settings.push_back ({(*i).GetString (), (*(i+1)).GetString ()});
        }
    }

  return settings;
}

const std::vector<std::pair<std::string, std::string>>
ScenarioConfigurationHelper::GetProtocolDeviceSettings() const
{
  std::vector<std::pair<std::string, std::string>> settings;

  if (m_config.HasMember ("protocolSettings"))
    {
      NS_ASSERT (m_config["protocolSettings"].IsArray ());

      NS_ASSERT_MSG (m_config["protocolSettings"].Size () % 2 == 0,
                     "Check protocolSettings: elements in list are not an even number, something is missing.");

      for (auto i = m_config["protocolSettings"].Begin (); i != m_config["protocolSettings"].End (); i += 2)
        {
          NS_ASSERT ((*i).IsString ());
          NS_ASSERT ((*(i+1)).IsString());

          if ((*i).GetString ()[0] != '/') continue;
          settings.push_back ({(*i).GetString (), (*(i+1)).GetString ()});
        }
    }

  return settings;
}

const std::vector<Ptr<Building>>
ScenarioConfigurationHelper::GetBuildings () const
{
  std::vector<Ptr<Building>> buildings;

  NS_ASSERT_MSG (m_config.HasMember ("buildings"),
                 "No building has been defined, check the configuration file.");
  NS_ASSERT_MSG (m_config["buildings"].IsArray (),
                 "'buildings' needs to be an array of objects, check the configuration file.");

  auto arr = m_config["buildings"].GetArray ();

  for (auto b = arr.Begin (); b != arr.End (); ++b)
  {
    Ptr<Building> building = CreateObject<Building> ();

    NS_ASSERT_MSG (b->HasMember ("boundaries") && (*b)["boundaries"].IsArray () && (*b)["boundaries"].Size () == 6,
                   "'boundaries' must be defined as an array of 6 doubles for each building.");
    auto bounds = (*b)["boundaries"].GetArray ();
    for (uint8_t i = 0; i < 6; ++i) { NS_ASSERT_MSG (bounds[i].IsDouble (), "'boundaries' elements must be doubles."); }
    building->SetBoundaries (Box (bounds[0].GetDouble (), bounds[1].GetDouble (),
                                  bounds[2].GetDouble (), bounds[3].GetDouble (),
                                  bounds[4].GetDouble (), bounds[5].GetDouble ()));

    if (b->HasMember("type"))
    {
      auto type = std::string ((*b)["type"].GetString ());
      NS_ASSERT_MSG (type == "residential" || type == "office" || type == "commercial",
                     "Unknown type of building: '" << type << "'.");
      if (type == "residential") building->SetBuildingType (Building::Residential);
      if (type == "office") building->SetBuildingType (Building::Office);
      if (type == "commercial") building->SetBuildingType (Building::Commercial);
    }

    if (b->HasMember("walls"))
    {
      auto walls = std::string ((*b)["walls"].GetString ());
      NS_ASSERT_MSG (walls == "wood" || walls == "concreteWithWindows" || walls == "concreteWithoutWindows" || walls == "stoneBlocks",
                     "Unknown type of walls for a building: '" << walls << "'.");
      if (walls == "wood") building->SetExtWallsType (Building::Wood);
      if (walls == "concreteWithWindows") building->SetExtWallsType (Building::ConcreteWithWindows);
      if (walls == "concreteWithoutWindows") building->SetExtWallsType (Building::ConcreteWithoutWindows);
      if (walls == "stoneBlocks") building->SetExtWallsType (Building::StoneBlocks);
    }

    if (b->HasMember("floors"))
    {
      NS_ASSERT_MSG ((*b)["floors"].IsInt (), "'floors' must be an integer.");
      building->SetNFloors ((*b)["floors"].GetInt ());
    }

    if (b->HasMember("rooms"))
    {
      auto rooms = (*b)["rooms"].GetArray ();
      NS_ASSERT_MSG (rooms.Size () == 2 && rooms[0].IsInt () && rooms[1].IsInt (),
                     "'rooms' needs to be an array of 2 integers.");
      building->SetNRoomsX (rooms[0].GetInt ());
      building->SetNRoomsY (rooms[1].GetInt ());
    }

    buildings.push_back(building);
  }

  return buildings;
}

} // namespace ns3
