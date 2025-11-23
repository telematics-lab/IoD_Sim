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
#ifndef CONSTELLATION_EXPANDER_H
#define CONSTELLATION_EXPANDER_H

#include <ns3/log.h>

#if defined(__clang__)
_Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
#define SUPPRESS_DEPRECATED_POP _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
_Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define SUPPRESS_DEPRECATED_POP _Pragma("GCC diagnostic pop")
#else
#define SUPPRESS_DEPRECATED_POP
#endif

#include <rapidjson/document.h>

    SUPPRESS_DEPRECATED_POP

#include <string>
#include <vector>

    namespace ns3
{
    /**
     * \brief Helper class to expand constellation definitions into individual satellite
     * configurations.
     *
     * This class provides utilities to process the special "!constellation" parameter in leo-sats
     * JSON configurations and expand them into multiple individual satellite configurations based
     * on different distribution patterns (file, uniform-orbits, one).
     */
    class ConstellationExpander
    {
      public:
        /**
         * \brief Check if a satellite configuration contains a constellation parameter.
         * \param satelliteConfig The JSON value representing a satellite configuration
         * \return true if the configuration has a "!constellation" member, false otherwise
         */
        static bool HasConstellationParameter(const rapidjson::Value& satelliteConfig);

        /**
         * \brief Expand a constellation definition into individual satellite configurations.
         * \param templateSat The JSON value containing the constellation template
         * \param scenarioPath The base path for resolving relative file paths
         * \return A vector of JSON values, each representing an individual satellite configuration
         */
        static std::vector<rapidjson::Document> ExpandConstellation(
            const rapidjson::Value& templateSat,
            const std::string& scenarioPath);

      private:
        /**
         * \brief Expand constellation from TLE/TSE file(s).
         * \param constellationDef The "!constellation" object definition
         * \param templateSat The template satellite configuration (without constellation parameter)
         * \param scenarioPath The base path for resolving relative file paths
         * \return A vector of satellite configurations, one per TLE entry
         */
        static std::vector<rapidjson::Document> ExpandFromFile(
            const rapidjson::Value& constellationDef,
            const rapidjson::Value& templateSat,
            const std::string& scenarioPath);

        /**
         * \brief Expand constellation with uniform orbital distribution.
         * \param constellationDef The "!constellation" object definition
         * \param templateSat The template satellite configuration (without constellation parameter)
         * \return A vector of satellite configurations distributed uniformly across orbits
         */
        static std::vector<rapidjson::Document> ExpandUniformOrbits(
            const rapidjson::Value& constellationDef,
            const rapidjson::Value& templateSat);

        /**
         * \brief Create a single satellite configuration.
         * \param constellationDef The "!constellation" object definition
         * \param templateSat The template satellite configuration (without constellation parameter)
         * \return A vector containing a single satellite configuration
         */
        static std::vector<rapidjson::Document> ExpandSingleSat(
            const rapidjson::Value& constellationDef,
            const rapidjson::Value& templateSat);

        /**
         * \brief Create a copy of the template satellite without the constellation parameter.
         * \param templateSat The original template satellite configuration
         * \param allocator The JSON allocator to use for creating the copy
         * \return A JSON value that is a deep copy of templateSat without "!constellation"
         */
        static rapidjson::Value CreateSatelliteTemplate(
            const rapidjson::Value& templateSat,
            rapidjson::Document::AllocatorType& allocator);

        /**
         * \brief Create GeoLeoOrbitMobility attributes from orbital parameters.
         * \param altitude Altitude in kilometers
         * \param inclination Inclination in degrees
         * \param longitude Longitude in degrees
         * \param offset Offset in degrees
         * \param retrograde Whether the orbit is retrograde
         * \param allocator The JSON allocator to use
         * \return A JSON array of mobility model attributes
         */
        static rapidjson::Value CreateMobilityAttributes(
            double altitude,
            double inclination,
            double longitude,
            double offset,
            bool retrograde,
            rapidjson::Document::AllocatorType& allocator);

        /**
         * \brief Create GeoLeoOrbitMobility attributes from TLE parameters.
         * \param tle1 TLE Line 1
         * \param tle2 TLE Line 2
         * \param sgp4Epoch SGP4 Epoch string
         * \param allocator The JSON allocator to use
         * \return A JSON array of mobility model attributes
         */
        static rapidjson::Value CreateMobilityAttributes(
            const std::string& tle1,
            const std::string& tle2,
            const std::string& sgp4Epoch,
            rapidjson::Document::AllocatorType& allocator);
    };

} // namespace ns3

#endif /* CONSTELLATION_EXPANDER_H */
