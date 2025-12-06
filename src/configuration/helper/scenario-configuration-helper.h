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
#ifndef SCENARIO_CONFIGURATION_HELPER_H
#define SCENARIO_CONFIGURATION_HELPER_H

#include "entity-configuration-helper.h"

#include <ns3/building.h>
#include <ns3/double-vector.h>
#include <ns3/flight-plan.h>
#include <ns3/log.h>
#include <ns3/mac-layer-configuration.h>
#include <ns3/network-layer-configuration.h>
#include <ns3/phy-layer-configuration.h>
#include <ns3/position-allocator.h>
#include <ns3/remote-configuration.h>
#include <ns3/singleton.h>
#include <ns3/waypoint.h>

#include <fstream>
#include <rapidjson/document.h>
#include <sstream>
#include <string>

#define CONFIGURATOR ScenarioConfigurationHelper::Get()

namespace ns3
{
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
        ~ScenarioConfigurationHelper();

        /**
         * \brief Bootstrap Singleton with basic data.
         * \param argc The number of command line arguments.
         * \param argv The list of command line arguments.
         */
        void Initialize(int argc, char** argv);

        /**
         * \brief Bootstrap Singleton with basic data.
         *
         * \param argc The number of command line arguments.
         * \param argv The list of command line arguments.
         * \param name The name of the scenario.
         */
        void Initialize(int argc, char** argv, const std::string name);

        /**
         * \return The name of the scenario.
         */
        const std::string GetName();

        /**
         * \return The current date and time as a human-readable string.
         */
        const std::string GetCurrentDateTime() const;

        /**
         * \brief Gets the preconfigured path from the configuration file, then makes a new
         *        folder at <path>/<scenario_name>-<datetime> where to place files
         * \return The full path of the folder where to put result files
         */
        const std::string GetResultsPath();

        /**
         * \brief Check if the user wants to save trace results or not.
         */
        const bool GetLogOnFile() const;

        /**
         * \return The file path of the logging file.
         */
        const std::string GetLoggingFilePath();

        /**
         * \brief Retrieve Static Configuration Parameters as a key/value pair.
         *
         * \return Static configuration defined in the configuration file.
         */
        const std::vector<std::pair<std::string, Ptr<AttributeValue>>> GetStaticConfig();

        /**
         * \brief Retrieve the list of PHY Layers defined for this simulation.
         *
         * \return The list of PHY Layers to be defined for this simulation.
         */
        const std::vector<Ptr<PhyLayerConfiguration>> GetPhyLayers() const;

        /**
         * \brief Retrieve the list of MAC Layers defined for this simulation.
         *
         * \return The list of MAC Layers to be defined for this simulation.
         */
        const std::vector<Ptr<MacLayerConfiguration>> GetMacLayers() const;

        /**
         * \brief Retrieve the list of Network Layers defined for this simulation.
         *
         * \return The list of Network Layers to be defined for this simulation.
         */
        const std::vector<Ptr<NetworkLayerConfiguration>> GetNetworkLayers() const;

        /**
         * \brief Retrieve the list of generic enetities to be defined for this simulation.
         *
         * \return The list of Entities to be defined for this simulation.
         */
        const std::vector<Ptr<EntityConfiguration>> GetEntitiesConfiguration(
            const std::string& entityKey) const;

        /**
         * \brief Retrieve the list of remotes to be defined for this simulation.
         *
         * \return The list of remotes to be defined for this simulation.
         */
        const std::vector<Ptr<RemoteConfiguration>> GetRemotesConfiguration() const;

        /**
         * \return The duration of the simulation in seconds.
         */
        const double GetDuration() const;

        /**
         * \return The reporting interval for application statistics in seconds.
         */
        const double GetAppStatisticsReportInterval() const;

        /**
         * \return The number of entities in the given entityKey category.
         */
        std::size_t GetN(const char* entityKey) const;

        /**
         * \return The number of plain nodes to be simulated.
         */
        std::size_t GetNodesN() const;

        // DRONE RELATED CONFIGURATORS

        /**
         * \return the number of drones to be simulated.
         */
        const uint32_t GetDronesN() const;
        /**
         * \return a string identifying the mobility model for the drones.
         */
        const std::string GetDronesMobilityModel() const;
        /**
         * \param n the index of the drone in the list
         * \return a string identifying the mobility model of the n-th drone
         */
        const std::string GetDroneMobilityModel(uint32_t n) const;
        /**
         * \param n the index of the drone in the list
         * \return a ns3::Vector with the coordinates of the position of the n-th drone
         */
        const Vector GetDronePosition(uint32_t n) const;
        /**
         * \brief allocate the position of the drones (if dronesMobilityModel is
         *        set to "ns3::ConstantPositionMobilityModel")
         * \param allocator the allocator to be filled with drones positions.
         */
        void GetDronesPosition(Ptr<ListPositionAllocator> allocator) const;
        /**
         * \param n the index of the drone in the list
         * \return the list of waypoints of drone with index i
         */
        const std::vector<Waypoint> GetDroneWaypoints(uint32_t i) const;
        /**
         * \return the step of the curve to be generated.
         */
        const float GetCurveStep() const;
        /**
         * \param i the drone index number.
         * \return the flight plan of drone with index i
         */
        const FlightPlan GetDroneFlightPlan(uint32_t i) const;
        /**
         * \param i the drone index number.
         * \return the constant acceleration of the drone with index i.
         */
        const double GetDroneAcceleration(uint32_t i) const;
        /**
         * \param i the drone index number.
         * \return the maximum speed of the drone with index i.
         */
        const double GetDroneMaxSpeed(uint32_t i) const;
        /**
         * \param i the drone index number.
         * \return the polynomial speed coeffients.
         */
        const DoubleVector GetDroneSpeedCoefficients(uint32_t i) const;
        /**
         * \param i the drone index number.
         * \return the instant, in seconds, indicating the start of the application.
         */
        const double GetDroneApplicationStartTime(uint32_t i) const;
        /**
         * \param i the drone index number.
         * \return the instant, in seconds, indicating the end of the application.
         */
        const double GetDroneApplicationStopTime(uint32_t i) const;

        // WIFI SPECIFIC CONFIGURATORS

        /**
         * \return the phy mode for WiFi communications.
         */
        const std::string GetPhyMode() const;
        /**
         * \return the phy path loss to use
         */
        const std::string GetPhyPropagationLossModel() const;
        /**
         * \return the phy parameters
         */
        const std::vector<std::pair<std::string, float>>
        GetThreeLogDistancePropagationLossModelAttributes() const;

        /**
         * \return the number of ZSPs to be simulated.
         */
        const uint32_t GetZspsN() const;
        /**
         * \brief allocate the position of the ZSPs
         * \param allocator the allocator to be filled with ZSPs positions.
         */
        void GetZspsPosition(Ptr<ListPositionAllocator> allocator) const;
        /**
         * \param i the ZSP index number.
         * \return the instant, in seconds, indicating the start of the application.
         */
        const double GetZspApplicationStartTime(uint32_t i) const;
        /**
         * \param i the ZSP index number.
         * \return the instant, in seconds, indicating the end of the application.
         */
        const double GetZspApplicationStopTime(uint32_t i) const;
        /**
         * \return the number of ENBs to be simulated.
         */
        const uint32_t GetEnbsN() const;
        /**
         * \brief allocate the position of the ENBs
         *
         * \param allocator the allocator to be filled with ENBs positions.
         */
        void GetEnbsPosition(Ptr<ListPositionAllocator> allocator) const;
        /**
         * \brief Check if dry run is wanted.
         */
        const bool IsDryRun() const;
        /**
         * \return a structure containing points describing regions of interest
         */
        const std::vector<DoubleVector> GetRegionsOfInterest() const;

        //  LTE SPECIFIC CONFIGURATORS

        /**
         * \return Radio Environment Map generation code, 0 for no generation, 1 for generation, 2
         * for generation with map plot.
         */
        const uint32_t RadioMap() const;

        /**
         * \return a list of parameters for the Radio Environment Map generator
         */
        const std::vector<std::pair<std::string, std::string>> GetRadioMapParameters() const;

        /**
         * \return the number of antennas to be simulated.
         */
        const uint32_t GetAntennasN() const;
        /**
         * \brief allocate the position of the antennas
         * \param allocator the allocator to be filled with antennas positions
         */
        void GetAntennasPosition(Ptr<ListPositionAllocator> allocator) const;

        /**
         * \return the number of remote hosts to be simulated.
         */
        const uint32_t GetRemotesN() const;
        /**
         * \param i the remote index number.
         * \return the instant, in seconds, indicating the start of the application.
         */
        const double GetRemoteApplicationStartTime(uint32_t i) const;
        /**
         * \param i the remote index number.
         * \return the instant, in seconds, indicating the end of the application.
         */
        const double GetRemoteApplicationStopTime(uint32_t i) const;

        /**
         * \brief Returns a vector with all the global configuration strings for scenario.
         *        Use with Config::SetDefault(pair.first, pair.second) at the beginning of the
         * script.
         * \return a vector of pairs containing the key and the value extracted from config
         */
        const std::vector<std::pair<std::string, std::string>> GetGlobalSettings() const;
        /**
         * \brief Returns a vector with all the per-object individual settings.
         *        Use with Config::Set(pair.first, pair.second) after creating all the object before
         * simulation start. \return a vector of pairs containing the key and the value extracted
         * from config
         */
        const std::vector<std::pair<std::string, std::string>> GetIndividualSettings() const;

        /**
         * \return a vector of Ptr<Building> created with the attributes in config
         */
        const std::vector<Ptr<Building>> GetBuildings() const;

        /**
         * \param field the name of the field to search into
         * \param index the index of the element to which retrieve the name
         * \return the name associated to the object at the index in the field
         */
        const std::string GetObjectName(const char* field, uint32_t index) const;

        /**
         * \param field the name of the field to search into
         * \param name the name of the element to which retrieve the index
         * \return the index of the object of given name
         */
        const uint32_t GetObjectIndex(const char* field, const std::string& name) const;

        //  NETWORKS SPECIFIC CONFIGURATORS

        /**
         * \param id the index of the drone to query
         * \return a list of the network IDs the drone is connected to
         */
        const std::vector<uint32_t> GetDroneNetworks(uint32_t id) const;

        /**
         * \param name the name of the drone to query
         * \return a list of the network IDs the drone is connected to
         */
        const std::vector<uint32_t> GetDroneNetworks(const std::string& name) const;

        /**
         * \param id the index of the antenna to query
         * \return a list of the network IDs the antenna is connected to
         */
        const std::vector<uint32_t> GetAntennaNetworks(uint32_t id) const;

        /**
         * \param name the name of the antenna to query
         * \return a list of the network IDs the antenna is connected to
         */
        const std::vector<uint32_t> GetAntennaNetworks(const std::string& name) const;

        /**
         * \param id the index of the network
         * \return a list of id of all the drones connected to a network
         */
        const std::vector<uint32_t> GetDronesInNetwork(uint32_t id) const;

        /**
         * \param net_name the name of the network
         * \return a list of id of all the drones connected to a network
         */
        const std::vector<uint32_t> GetDronesInNetwork(const std::string& net_name) const;

        /**
         * \param id the index of the network
         * \return a list of id of all the antennas connected to a network
         */
        const std::vector<uint32_t> GetAntennasInNetwork(uint32_t id) const;

        /**
         * \param net_name the name of the network
         * \return a list of id of all the antennas connected to a network
         */
        const std::vector<uint32_t> GetAntennasInNetwork(const std::string& net_name) const;

        /// General purpose value retrieving methods

        /**
         * \param path1 the first part of the path
         * \param path2 the second part of the path
         * \return the concatenation of those 2 paths adding a '/' if necessary
         */
        const std::string MakePath(const std::string& path1, const std::string& path2 = "") const;

        /**
         * \param path the first part of the path
         * \param index the index of the element at the path
         * \return the concatenation of path and index, adding a '/' if necessary
         */
        const std::string MakePath(const std::string& path, uint32_t index) const;

        /**
         * \param path the path to check in form eg "/parent/children/index/key"
         * \return true if the path is valid else false
         */
        bool CheckPath(const std::string& path) const;

        /**
         * \param path the path of the value to retrieve in form eg "/parent/children/index/key"
         * \returns if the path is valid returns a pair <true, int32_t at path, else <false, 0>
         */
        const std::pair<bool, int32_t> GetInt(const std::string& path) const;

        /**
         * \param path the path of the value to retrieve in form eg "/parent/children/index/key"
         * \returns if the path is valid returns a pair <true, uint32_t at path, else <false, 0>
         */
        const std::pair<bool, uint32_t> GetUint(const std::string& path) const;

        /**
         * \param path the path of the value to retrieve in form eg "/parent/children/index/key"
         * \returns if the path is valid returns a pair <true, double at path, else <false, 0.0>
         */
        const std::pair<bool, double> GetDouble(const std::string& path) const;

        /**
         * \param path the path of the value to retrieve in form eg "/parent/children/index/key"
         * \returns if the path is valid returns a pair <true, bool at path>, else <false, false>
         */
        const std::pair<bool, bool> GetBool(const std::string& path) const;

        /**
         * \param path the path of the value to retrieve in form eg "/parent/children/index/key"
         * \returns if the path is valid returns a pair <true, std::string at path>, else <false,
         * "">
         */
        const std::pair<bool, std::string> GetString(const std::string& path) const;

      private:
        /**
         * \brief part of the constructor, it focuses on command line decoding and JSON file
         * parsing.
         * \param argc the command line argument count number.
         * \param argv the list of command line arguments
         */
        void InitializeConfiguration(int argc, char** argv);
        /**
         * \brief part of the destructor, it releases any pointer bound to the command line and JSON
         * files.
         */
        void DisposeConfiguration();

        /**
         * \brief redirect clog to a proper log file or standard out
         * \param onFile whether clog should print on a file or on standard output console.
         */
        void InitializeLogging(const bool& onFile);
        /**
         * \brief recover previous state of clog output buffer and closes any log file opened.
         */
        void DisposeLogging();
        /**
         * \brief Optional array of log components to be enabled for the simulation.
         */
        void EnableLogComponents() const;

        std::FILE* m_configFilePtr;   /// pointer to the JSON file
        rapidjson::Document m_config; /// decoded JSON structure
        std::ofstream m_out;          /// output stream for clog
        std::string m_name;           /// name of the simulation
        std::string m_dateTime;       /// cache for the current datetime
        std::vector<std::pair<std::string, Ptr<AttributeValue>>>
            m_staticConfig;        /// cache for ns-3 static config params
        uint32_t m_radioMap;       /// a code for radio map generation options
        std::string m_currentPath; /// cache for the current path at initialization
    };

} // namespace ns3

#endif /* SCENARIO_CONFIGURATION_HELPER_H */
