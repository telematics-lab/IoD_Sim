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
#ifndef SCENARIO_CONFIGURATION_HELPER_H
#define SCENARIO_CONFIGURATION_HELPER_H

#include <string>
#include <fstream>
#include <sstream>

#include <rapidjson/document.h>
#include <ns3/position-allocator.h>
#include <ns3/log.h>
#include <ns3/flight-plan.h>
#include <ns3/speed-coefficients.h>
#include <ns3/singleton.h>
#include <ns3/building.h>
#include <ns3/drone-network.h>

#define CONFIGURATOR ScenarioConfigurationHelper::Get ()

namespace ns3 {

/**
 * \brief Configuration Helper for a scenario.
 *
 * This is a Helper to ease the configuration of a typical scenario by reading
 * configuration options via command line or a JSON file.
 * This helper is heavily dependant to Scenario "D" architecture, but it can
 * always be extended to be more general and broad by adding new methods to it.
 */
class ScenarioConfigurationHelper : public Singleton<ScenarioConfigurationHelper>
{
public:
  /**
   * \brief Bootstrap Singleton with basic data.
   * \param argc The number of command line arguments.
   * \param argv The list of command line arguments.
   * \param name The name of the scenario.
   */
  void Initialize (int argc, char ** argv, const std::string name);

  /**
   * \return The name of the scenario.
   */
  const std::string GetName () const;
  /**
   * \return The current date and time as a human-readable string.
   */
  const std::string GetCurrentDateTime () const;
  /**
   * \brief Gets the preconfigured path from the configuration file, then makes a new
   *        folder at <path>/<scenario_name>-<datetime> where to place files
   * \return The full path of the folder where to put result files
   */
  const std::string GetResultsPath () const;
  /**
   * \return The file path of the logging file.
   */
  const std::string GetLoggingFilePath () const;
  /**
   * \return The duration of the simulation in seconds.
   */
  const double GetDuration () const;


// DRONE RELATED CONFIGURATORS

  /**
   * \return the number of drones to be simulated.
   */
  const uint32_t GetDronesN () const;
  /**
   * \return a string identifying the mobility model for the drones.
   */
  const std::string GetDronesMobilityModel () const;
  /**
   * \param n the index of the drone in the list
   * \return a string identifying the mobility model of the n-th drone
   */
  const std::string GetDroneMobilityModel (uint32_t n) const;
  /**
   * \param n the index of the drone in the list
   * \return a ns3::Vector with the coordinates of the position of the n-th drone
   */
  const Vector GetDronePosition (uint32_t n) const;
  /**
   * \brief allocate the position of the drones (if dronesMobilityModel is
   *        set to "ns3::ConstantPositionMobilityModel")
   * \param allocator the allocator to be filled with drones positions.
   */
  void GetDronesPosition (Ptr<ListPositionAllocator> allocator) const;
  /**
   * \return the step of the curve to be generated.
   */
  const float GetCurveStep () const;
  /**
   * \param i the drone index number.
   * \return the flight plan of drone with index i
   */
  const FlightPlan GetDroneFlightPlan (uint32_t i) const;
  /**
   * \param i the drone index number.
   * \return the constant acceleration of the drone with index i.
   */
  const double GetDroneAcceleration (uint32_t i) const;
  /**
   * \param i the drone index number.
   * \return the maximum speed of the drone with index i.
   */
  const double GetDroneMaxSpeed (uint32_t i) const;
  /**
   * \param i the drone index number.
   * \return the speed coefficients in a specific `SpeedCoefficients` class object
   */
  const SpeedCoefficients GetDroneSpeedCoefficients (uint32_t i) const;
  /**
   * \param i the drone index number.
   * \return the instant, in seconds, indicating the start of the application.
   */
  const double GetDroneApplicationStartTime (uint32_t i) const;
  /**
   * \param i the drone index number.
   * \return the instant, in seconds, indicating the end of the application.
   */
  const double GetDroneApplicationStopTime (uint32_t i) const;


// WIFI SPECIFIC CONFIGURATORS

  /**
   * \return the phy mode for WiFi communications.
   */
  const std::string GetPhyMode () const;
  /**
   * \return the phy path loss to use
   */
  const std::string GetPhyPropagationLossModel () const;
  /**
   * \return the phy parameters
   */
  const std::vector<std::pair<std::string, float> > GetThreeLogDistancePropagationLossModelAttributes () const;

  /**
   * \return the number of ZSPs to be simulated.
   */
  const uint32_t GetZspsN () const;
  /**
   * \brief allocate the position of the ZSPs
   * \param allocator the allocator to be filled with ZSPs positions.
   */
  void GetZspsPosition (Ptr<ListPositionAllocator> allocator) const;
  /**
   * \param i the ZSP index number.
   * \return the instant, in seconds, indicating the start of the application.
   */
  const double GetZspApplicationStartTime (uint32_t i) const;
  /**
   * \param i the ZSP index number.
   * \return the instant, in seconds, indicating the end of the application.
   */
  const double GetZspApplicationStopTime (uint32_t i) const;
  /**
   * \return the number of ENBs to be simulated.
   */
  const uint32_t GetEnbsN () const;
  /**
   * \brief allocate the position of the ENBs
   *
   * \param allocator the allocator to be filled with ENBs positions.
   */
  void GetEnbsPosition (Ptr<ListPositionAllocator> allocator) const;


//  LTE SPECIFIC CONFIGURATORS

  /**
   * \return true if the generation of the Radio Environment Map is required
   */
  const bool RadioMap () const;

  /**
   * \return a list of parameters for the Radio Environment Map generator
   */
  const std::vector<std::pair<std::string, std::string> > GetRadioMapParameters () const;

  /**
   * \return the number of antennas to be simulated.
   */
  const uint32_t GetAntennasN () const;
  /**
   * \brief allocate the position of the antennas
   * \param allocator the allocator to be filled with antennas positions
   */
  void GetAntennasPosition (Ptr<ListPositionAllocator> allocator) const;

  /**
   * \return the number of remote hosts to be simulated.
   */
  const uint32_t GetRemotesN () const;
  /**
 * \param i the remote index number.
 * \return the instant, in seconds, indicating the start of the application.
 */
  const double GetRemoteApplicationStartTime (uint32_t i) const;
  /**
   * \param i the remote index number.
   * \return the instant, in seconds, indicating the end of the application.
   */
  const double GetRemoteApplicationStopTime (uint32_t i) const;

  /**
   * \brief Returns a vector with all the global configuration strings for scenario.
   *        Use with Config::SetDefault(pair.first, pair.second) at the beginning of the script.
   * \return a vector of pairs containing the key and the value extracted from config
   */
  const std::vector<std::pair<std::string, std::string> > GetGlobalSettings () const;
  /**
   * \brief Returns a vector with all the per-object individual settings.
   *        Use with Config::Set(pair.first, pair.second) after creating all the object before simulation start.
   * \return a vector of pairs containing the key and the value extracted from config
   */
  const std::vector<std::pair<std::string, std::string> > GetIndividualSettings () const;

  /**
   * \return a vector of Ptr<Building> created with the attributes in config
   */
  const std::vector<Ptr<Building> > GetBuildings () const;

  /**
   * \param field the name of the field to search into
   * \param index the index of the element to which retrieve the name
   * \return the name associated to the object at the index in the field
   */
  std::string GetObjectName (const char* field, uint32_t index) const;

  /**
   * \param field the name of the field to search into
   * \param name the name of the element to which retrieve the index
   * \return the index of the object of given name
   */
  uint32_t GetObjectIndex (const char* field, std::string name) const;


//  NETWORKS SPECIFIC CONFIGURATORS

  /**
   * \return a container of Ptr<DroneNetwork> already configured with attributes and name, but not generated.
   */
  DroneNetworkContainer GetNetworks () const;

  /**
   * \param id the index of the drone to query
   * \return a list of the network IDs the drone is connected to
   */
  std::vector<uint32_t> GetDroneNetworks (uint32_t id) const;
   /**
   * \param name the name of the drone to query
   * \return a list of the network IDs the drone is connected to
   */
  std::vector<uint32_t> GetDroneNetworks (std::string name) const;

  /**
   * \param id the index of the antenna to query
   * \return a list of the network IDs the antenna is connected to
   */
  std::vector<uint32_t> GetAntennaNetworks (uint32_t id) const;
   /**
   * \param name the name of the antenna to query
   * \return a list of the network IDs the antenna is connected to
   */
  std::vector<uint32_t> GetAntennaNetworks (std::string name) const;

  /**
   * \param id the index of the network
   * \return a list of id of all the drones connected to a network
   */
  std::vector<uint32_t> GetDronesInNetwork (uint32_t id) const;
    /**
   * \param net_name the name of the network
   * \return a list of id of all the drones connected to a network
   */
  std::vector<uint32_t> GetDronesInNetwork (std::string net_name) const;

  /**
   * \param id the index of the network
   * \return a list of id of all the antennas connected to a network
   */
  std::vector<uint32_t> GetAntennasInNetwork (uint32_t id) const;
   /**
   * \param net_name the name of the network
   * \return a list of id of all the antennas connected to a network
   */
  std::vector<uint32_t> GetAntennasInNetwork (std::string net_name) const;

  /**
   * \brief default destructor
   */
  ~ScenarioConfigurationHelper ();

private:
  /**
   * \brief part of the constructor, it focuses on command line decoding and JSON file parsing.
   * \param argc the command line argument count number.
   * \param argv the list of command line arguments
   */
  void InitializeConfiguration (int argc, char **argv);
  /**
   * \brief part of the destructor, it releases any pointer bound to the command line and JSON files.
   */
  void DisposeConfiguration ();

  /**
   * \brief redirect clog to a proper log file or standard out
   * \param onFile whether clog should print on a file or on standard output console.
   */
  void InitializeLogging (const bool &onFile);
  /**
   * \brief recover previous state of clog output buffer and closes any log file opened.
   */
  void DisposeLogging ();
  /**
   * \return optional parameter to be fed to `InitializeLogging()`
   */
  const bool GetLogOnFile () const;
  /**
   * \brief Optional array of log components to be enabled for the simulation.
   */
  void EnableLogComponents () const;


  std::FILE *m_configFilePtr;   /// pointer to the JSON file
  rapidjson::Document m_config; /// decoded JSON structure
  std::ofstream m_out;          /// output stream for clog
  std::string m_name;           /// name of the simulation
  std::string m_dateTime;       /// cache for the current datetime
  bool m_radioMap;              /// a flag for radio map generation
};

} // namespace ns3

#endif /* SCENARIO_CONFIGURATION_HELPER_H */
