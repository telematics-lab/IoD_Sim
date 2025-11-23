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
#include "constellation-expander.h"

#include <ns3/assert.h>
#include <ns3/fatal-error.h>
#include <ns3/log.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <sstream>
#include <string>

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("ConstellationExpander");

// Static cache for "now" timestamp
static std::time_t g_nowTimestamp = 0;

// Helper to fetch content from a URL using curl.
static std::string
GetUrlContent(const std::string& url)
{
    // Check if curl is available
    if (std::system("which curl > /dev/null 2>&1") != 0)
    {
        NS_FATAL_ERROR("The 'curl' command is required but not found. Please install it to "
                       "download the files.");
    }

    std::string result;
    // Quote the URL to handle special characters.
    // -L: Follow redirects
    // --fail: Fail silently (no output) on server errors (but we want to detect failure via exit
    // code) We do NOT use -s so progress is shown on stderr.
    std::string cmd = "curl -L --fail \"" + url + "\"";

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        NS_FATAL_ERROR("Failed to execute curl for URL: " << url);
    }

    std::array<char, 128> buffer{};
    size_t bytesRead;
    while ((bytesRead = std::fread(buffer.data(), 1, buffer.size(), pipe)) > 0)
    {
        result.append(buffer.data(), bytesRead);
    }

    int ret = pclose(pipe);
    if (ret != 0)
    {
        NS_FATAL_ERROR("Failed to download file from " << url << ". curl exited with code " << ret);
    }

    return result;
}

std::time_t
GetReferenceTime(const std::string& timeRef)
{
    if (timeRef == "now")
    {
        if (g_nowTimestamp == 0)
        {
            g_nowTimestamp = std::time(nullptr);
            NS_LOG_INFO("Set 'now' reference time to: "
                        << std::put_time(std::gmtime(&g_nowTimestamp), "%c %Z"));
        }
        return g_nowTimestamp;
    }
    else
    {
        std::tm tm = {};
        std::istringstream ss(timeRef);
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ"); // Parse ISO 8601
        if (ss.fail())
        {
            // Try without Z
            ss.clear();
            ss.str(timeRef);
            ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
            if (ss.fail())
            {
                NS_FATAL_ERROR("Failed to parse tleTimeReference: " << timeRef);
            }
        }
        return timegm(&tm); // Convert to time_t (UTC)
    }
}

bool
ConstellationExpander::HasConstellationParameter(const rapidjson::Value& satelliteConfig)
{
    return satelliteConfig.IsObject() && satelliteConfig.HasMember("!constellation");
}

std::vector<rapidjson::Document>
ConstellationExpander::ExpandConstellation(const rapidjson::Value& templateSat,
                                           const std::string& scenarioPath)
{
    NS_LOG_FUNCTION_NOARGS();

    NS_ASSERT_MSG(HasConstellationParameter(templateSat),
                  "Template satellite must have !constellation parameter");
    NS_ASSERT_MSG(templateSat["!constellation"].IsObject(), "!constellation must be a JSON object");

    const auto& constellationDef = templateSat["!constellation"];

    NS_ASSERT_MSG(constellationDef.HasMember("distribution"),
                  "!constellation must have a 'distribution' field");
    NS_ASSERT_MSG(constellationDef["distribution"].IsString(), "'distribution' must be a string");

    std::string distribution = constellationDef["distribution"].GetString();

    if (distribution == "file")
    {
        return ExpandFromFile(constellationDef, templateSat, scenarioPath);
    }
    else if (distribution == "uniform-orbits")
    {
        return ExpandUniformOrbits(constellationDef, templateSat);
    }
    else if (distribution == "one")
    {
        return ExpandSingleSat(constellationDef, templateSat);
    }
    else
    {
        NS_FATAL_ERROR("Unknown constellation distribution type: " << distribution);
        return {};
    }
}

std::vector<rapidjson::Document>
ConstellationExpander::ExpandFromFile(const rapidjson::Value& constellationDef,
                                      const rapidjson::Value& templateSat,
                                      const std::string& scenarioPath)
{
    NS_LOG_FUNCTION_NOARGS();

    NS_ASSERT_MSG(constellationDef.HasMember("file"), "'file' distribution requires 'file' field");

    std::vector<std::string> filePaths;

    // Support both single file path (string) and array of paths
    if (constellationDef["file"].IsString())
    {
        filePaths.push_back(constellationDef["file"].GetString());
    }
    else if (constellationDef["file"].IsArray())
    {
        for (const auto& fileEntry : constellationDef["file"].GetArray())
        {
            NS_ASSERT_MSG(fileEntry.IsString(), "Each file path must be a string");
            filePaths.push_back(fileEntry.GetString());
        }
    }
    else
    {
        NS_FATAL_ERROR("'file' field must be a string or array of strings");
    }

    std::vector<rapidjson::Document> satellites;

    for (const auto& relativePath : filePaths)
    {
        // Resolve relative path
        std::string fullPath;
        if (relativePath[0] == '/')
        {
            fullPath = relativePath;
        }
        else
        {
            // Concatenate scenario path with relative path
            fullPath = scenarioPath;
            if (!fullPath.empty() && fullPath.back() != '/')
            {
                fullPath += "/";
            }
            fullPath += relativePath;
        }

        NS_LOG_INFO("Loading TLE file: " << fullPath);

        // Read TLE data either from a local file or a remote URL
        std::vector<std::pair<std::string, std::string>> tleEntries;
        std::string line;
        std::string tleLine1, tleLine2;
        std::istringstream dataStream;
        if (relativePath.rfind("http://", 0) == 0 || relativePath.rfind("https://", 0) == 0)
        {
            // Fetch remote content
            std::string urlContent = GetUrlContent(relativePath);
            dataStream.str(urlContent);
        }
        else
        {
            // Load local file content
            std::ifstream tleFile(fullPath);
            if (!tleFile.is_open())
            {
                NS_FATAL_ERROR("Failed to open TLE file: " << fullPath);
            }
            std::string fileContent((std::istreambuf_iterator<char>(tleFile)),
                                    std::istreambuf_iterator<char>());
            tleFile.close();
            dataStream.str(fileContent);
        }

        while (std::getline(dataStream, line))
        {
            // Strip trailing whitespace (including \r from Windows line endings)
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            // Check if line starts with "1 " (first TLE line)
            if (line.length() >= 2 && line[0] == '1' && line[1] == ' ')
            {
                tleLine1 = line;
            }
            // Check if line starts with "2 " (second TLE line)
            else if (line.length() >= 2 && line[0] == '2' && line[1] == ' ')
            {
                tleLine2 = line;
                // We have a complete TLE pair
                if (!tleLine1.empty())
                {
                    tleEntries.push_back({tleLine1, tleLine2});
                    tleLine1.clear();
                    tleLine2.clear();
                }
            }
        }

        NS_LOG_INFO("Loaded " << tleEntries.size() << " satellites from " << fullPath);

        // Determine reference time
        // Determine reference time
        std::string timeRef = "tleEpoch";
        if (constellationDef.HasMember("tleTimeReference"))
        {
            timeRef = constellationDef["tleTimeReference"].GetString();
        }

        std::string tleStartTime;
        if (timeRef != "tleEpoch")
        {
            std::time_t refTime = GetReferenceTime(timeRef);

            // Format reference time for TleStartTime (ISO 8601: YYYY-MM-DD HH:MM:SS)
            std::tm* tm = std::gmtime(&refTime);
            std::stringstream ss;
            ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
            tleStartTime = ss.str();
        }

        // Determine mobility model type (circular default, sgp4 optional)
        std::string model = "circular";
        if (constellationDef.HasMember("model"))
        {
            model = constellationDef["model"].GetString();
        }

        // Create a satellite configuration for each TLE entry
        for (const auto& tleEntry : tleEntries)
        {
            rapidjson::Document satDoc;
            satDoc.SetObject();
            auto& allocator = satDoc.GetAllocator();

            // Copy template without !constellation
            rapidjson::Value satConfig = CreateSatelliteTemplate(templateSat, allocator);

            // Get TLE lines (already cleaned)
            const std::string& tle1 = tleEntry.first;
            const std::string& tle2 = tleEntry.second;

            // Create or update mobilityModel based on selected model
            if (!satConfig.HasMember("mobilityModel"))
            {
                rapidjson::Value mobilityModel(rapidjson::kObjectType);
                if (model == "sgp4")
                {
                    mobilityModel.AddMember("name",
                                            rapidjson::Value("ns3::GeoSGP4Mobility", allocator),
                                            allocator);
                    mobilityModel.AddMember(
                        "attributes",
                        CreateMobilityAttributes(tle1, tle2, tleStartTime, allocator),
                        allocator);
                }
                else // circular (default)
                {
                    mobilityModel.AddMember("name",
                                            rapidjson::Value("ns3::GeoLeoOrbitMobility", allocator),
                                            allocator);
                    // Reuse the same attribute creator for circular (altitude/inclination not
                    // needed here) For TLE based circular case we still use the TLE attribute
                    // overload
                    mobilityModel.AddMember(
                        "attributes",
                        CreateMobilityAttributes(tle1, tle2, tleStartTime, allocator),
                        allocator);
                }
                satConfig.AddMember("mobilityModel", mobilityModel, allocator);
            }
            else
            {
                auto& mobilityModel = satConfig["mobilityModel"];
                mobilityModel["attributes"] =
                    CreateMobilityAttributes(tle1, tle2, tleStartTime, allocator);
            }

            satDoc.CopyFrom(satConfig, allocator);
            satellites.push_back(std::move(satDoc));
        }
    }

    NS_LOG_INFO("Expanded " << satellites.size() << " satellites from TLE files");
    return satellites;
}

std::vector<rapidjson::Document>
ConstellationExpander::ExpandUniformOrbits(const rapidjson::Value& constellationDef,
                                           const rapidjson::Value& templateSat)
{
    NS_LOG_FUNCTION_NOARGS();

    NS_ASSERT_MSG(constellationDef.HasMember("orbits"),
                  "'uniform-orbits' distribution requires 'orbits' field");
    NS_ASSERT_MSG(constellationDef["orbits"].IsArray(), "'orbits' must be an array");
    // Determine mobility model type (circular default, sgp4 optional)
    std::string model = "circular";
    if (constellationDef.HasMember("model"))
    {
        model = constellationDef["model"].GetString();
    }

    std::vector<rapidjson::Document> satellites;

    for (const auto& orbitDef : constellationDef["orbits"].GetArray())
    {
        NS_ASSERT_MSG(orbitDef.IsObject(), "Each orbit definition must be an object");
        NS_ASSERT_MSG(orbitDef.HasMember("height") && orbitDef["height"].IsNumber(),
                      "Orbit must have numeric 'height' field");
        NS_ASSERT_MSG(orbitDef.HasMember("inclination") && orbitDef["inclination"].IsNumber(),
                      "Orbit must have numeric 'inclination' field");
        NS_ASSERT_MSG(orbitDef.HasMember("orbits-per-longitude") &&
                          orbitDef["orbits-per-longitude"].IsNumber(),
                      "Orbit must have numeric 'orbits-per-longitude' field");
        NS_ASSERT_MSG(orbitDef.HasMember("sats-per-orbit") && orbitDef["sats-per-orbit"].IsNumber(),
                      "Orbit must have numeric 'sats-per-orbit' field");

        double altitude = orbitDef["height"].GetDouble();
        double inclination = orbitDef["inclination"].GetDouble();
        uint32_t orbitsPerLongitude = orbitDef["orbits-per-longitude"].GetUint();
        uint32_t satsPerOrbit = orbitDef["sats-per-orbit"].GetUint();

        // Optional retrograde parameter
        bool retrograde = false;
        if (orbitDef.HasMember("retrograde-orbit") && orbitDef["retrograde-orbit"].IsBool())
        {
            retrograde = orbitDef["retrograde-orbit"].GetBool();
        }

        // Distribute orbits uniformly across longitudes
        for (uint32_t orbitIdx = 0; orbitIdx < orbitsPerLongitude; ++orbitIdx)
        {
            double longitude = (orbitIdx * 360.0) / orbitsPerLongitude;
            // Normalize longitude to range -180 to 180
            if (longitude > 180.0)
            {
                longitude -= 360.0;
            }

            // Distribute satellites uniformly within each orbit
            for (uint32_t satIdx = 0; satIdx < satsPerOrbit; ++satIdx)
            {
                double offset = (satIdx * 360.0) / satsPerOrbit;

                rapidjson::Document satDoc;
                satDoc.SetObject();
                auto& allocator = satDoc.GetAllocator();

                // Copy template without !constellation
                rapidjson::Value satConfig = CreateSatelliteTemplate(templateSat, allocator);

                // Create or update mobilityModel
                if (!satConfig.HasMember("mobilityModel"))
                {
                    rapidjson::Value mobilityModel(rapidjson::kObjectType);
                    if (model == "sgp4")
                    {
                        mobilityModel.AddMember("name",
                                                rapidjson::Value("ns3::GeoSGP4Mobility", allocator),
                                                allocator);
                        // For uniform-orbits with sgp4 we still need TLE lines; assume they are
                        // provided elsewhere. Here we fallback to circular attributes as
                        // placeholder.
                        mobilityModel.AddMember("attributes",
                                                CreateMobilityAttributes(altitude,
                                                                         inclination,
                                                                         longitude,
                                                                         offset,
                                                                         retrograde,
                                                                         allocator),
                                                allocator);
                    }
                    else // circular default
                    {
                        mobilityModel.AddMember(
                            "name",
                            rapidjson::Value("ns3::GeoLeoOrbitMobility", allocator),
                            allocator);
                        mobilityModel.AddMember("attributes",
                                                CreateMobilityAttributes(altitude,
                                                                         inclination,
                                                                         longitude,
                                                                         offset,
                                                                         retrograde,
                                                                         allocator),
                                                allocator);
                    }
                    satConfig.AddMember("mobilityModel", mobilityModel, allocator);
                }
                else
                {
                    auto& mobilityModel = satConfig["mobilityModel"];
                    mobilityModel["attributes"] = CreateMobilityAttributes(altitude,
                                                                           inclination,
                                                                           longitude,
                                                                           offset,
                                                                           retrograde,
                                                                           allocator);
                }

                satDoc.CopyFrom(satConfig, allocator);
                satellites.push_back(std::move(satDoc));
            }
        }
    }

    NS_LOG_INFO("Expanded uniform-orbits constellation to " << satellites.size() << " satellites");
    return satellites;
}

std::vector<rapidjson::Document>
ConstellationExpander::ExpandSingleSat(const rapidjson::Value& constellationDef,
                                       const rapidjson::Value& templateSat)
{
    NS_LOG_FUNCTION_NOARGS();

    NS_ASSERT_MSG(constellationDef.HasMember("orbit"), "'one' distribution requires 'orbit' field");
    NS_ASSERT_MSG(constellationDef["orbit"].IsObject(), "'orbit' must be an object");

    const auto& orbitDef = constellationDef["orbit"];

    NS_ASSERT_MSG(orbitDef.HasMember("altitude") && orbitDef["altitude"].IsNumber(),
                  "Orbit must have numeric 'altitude' field");
    NS_ASSERT_MSG(orbitDef.HasMember("inclination") && orbitDef["inclination"].IsNumber(),
                  "Orbit must have numeric 'inclination' field");
    NS_ASSERT_MSG(orbitDef.HasMember("longitude") && orbitDef["longitude"].IsNumber(),
                  "Orbit must have numeric 'longitude' field");
    NS_ASSERT_MSG(orbitDef.HasMember("offset") && orbitDef["offset"].IsNumber(),
                  "Orbit must have numeric 'offset' field");
    // Determine mobility model type (circular default, sgp4 optional)
    std::string model = "circular";
    if (constellationDef.HasMember("model"))
    {
        model = constellationDef["model"].GetString();
    }

    double altitude = orbitDef["altitude"].GetDouble();
    double inclination = orbitDef["inclination"].GetDouble();
    double longitude = orbitDef["longitude"].GetDouble();
    double offset = orbitDef["offset"].GetDouble();
    bool retrograde = false;

    if (orbitDef.HasMember("retrograde-orbit") && orbitDef["retrograde-orbit"].IsBool())
    {
        retrograde = orbitDef["retrograde-orbit"].GetBool();
    }

    rapidjson::Document satDoc;
    satDoc.SetObject();
    auto& allocator = satDoc.GetAllocator();

    // Copy template without !constellation
    rapidjson::Value satConfig = CreateSatelliteTemplate(templateSat, allocator);

    // Create or update mobilityModel
    if (!satConfig.HasMember("mobilityModel"))
    {
        rapidjson::Value mobilityModel(rapidjson::kObjectType);
        if (model == "sgp4")
        {
            mobilityModel.AddMember("name",
                                    rapidjson::Value("ns3::GeoSGP4Mobility", allocator),
                                    allocator);
            // For single satellite sgp4 case we still need TLE lines; placeholder uses same
            // attribute creator.
            mobilityModel.AddMember("attributes",
                                    CreateMobilityAttributes(altitude,
                                                             inclination,
                                                             longitude,
                                                             offset,
                                                             retrograde,
                                                             allocator),
                                    allocator);
        }
        else // circular default
        {
            mobilityModel.AddMember("name",
                                    rapidjson::Value("ns3::GeoLeoOrbitMobility", allocator),
                                    allocator);
            mobilityModel.AddMember("attributes",
                                    CreateMobilityAttributes(altitude,
                                                             inclination,
                                                             longitude,
                                                             offset,
                                                             retrograde,
                                                             allocator),
                                    allocator);
        }
        satConfig.AddMember("mobilityModel", mobilityModel, allocator);
    }
    else
    {
        auto& mobilityModel = satConfig["mobilityModel"];
        mobilityModel["attributes"] = CreateMobilityAttributes(altitude,
                                                               inclination,
                                                               longitude,
                                                               offset,
                                                               retrograde,
                                                               allocator);
    }

    satDoc.CopyFrom(satConfig, allocator);

    std::vector<rapidjson::Document> satellites;
    satellites.push_back(std::move(satDoc));

    NS_LOG_INFO("Expanded single satellite constellation");
    return satellites;
}

rapidjson::Value
ConstellationExpander::CreateSatelliteTemplate(const rapidjson::Value& templateSat,
                                               rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value satelliteConfig(rapidjson::kObjectType);

    // Deep copy all members except "!constellation"
    for (auto it = templateSat.MemberBegin(); it != templateSat.MemberEnd(); ++it)
    {
        if (std::string(it->name.GetString()) != "!constellation")
        {
            rapidjson::Value key(it->name, allocator);
            rapidjson::Value value(it->value, allocator);
            satelliteConfig.AddMember(key, value, allocator);
        }
    }

    return satelliteConfig;
}

rapidjson::Value
ConstellationExpander::CreateMobilityAttributes(double altitude,
                                                double inclination,
                                                double longitude,
                                                double offset,
                                                bool retrograde,
                                                rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value attributes(rapidjson::kArrayType);

    // Altitude
    rapidjson::Value altitudeAttr(rapidjson::kObjectType);
    altitudeAttr.AddMember("name", "Altitude", allocator);
    altitudeAttr.AddMember("value", altitude, allocator);
    attributes.PushBack(altitudeAttr, allocator);

    // Inclination
    rapidjson::Value inclinationAttr(rapidjson::kObjectType);
    inclinationAttr.AddMember("name", "Inclination", allocator);
    inclinationAttr.AddMember("value", inclination, allocator);
    attributes.PushBack(inclinationAttr, allocator);

    // Longitude
    rapidjson::Value longitudeAttr(rapidjson::kObjectType);
    longitudeAttr.AddMember("name", "Longitude", allocator);
    longitudeAttr.AddMember("value", longitude, allocator);
    attributes.PushBack(longitudeAttr, allocator);

    // Offset
    rapidjson::Value offsetAttr(rapidjson::kObjectType);
    offsetAttr.AddMember("name", "Offset", allocator);
    offsetAttr.AddMember("value", offset, allocator);
    attributes.PushBack(offsetAttr, allocator);

    // RetrogradeOrbit
    rapidjson::Value retrogradeAttr(rapidjson::kObjectType);
    retrogradeAttr.AddMember("name", "RetrogradeOrbit", allocator);
    retrogradeAttr.AddMember("value", retrograde, allocator);
    attributes.PushBack(retrogradeAttr, allocator);

    return attributes;
}

rapidjson::Value
ConstellationExpander::CreateMobilityAttributes(const std::string& tle1,
                                                const std::string& tle2,
                                                const std::string& sgp4Epoch,
                                                rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value attributes(rapidjson::kArrayType);

    // Add TLE attributes
    rapidjson::Value tle1Attr(rapidjson::kObjectType);
    tle1Attr.AddMember("name", "TleLine1", allocator);
    tle1Attr.AddMember("value", rapidjson::Value(tle1.c_str(), allocator), allocator);
    attributes.PushBack(tle1Attr, allocator);

    rapidjson::Value tle2Attr(rapidjson::kObjectType);
    tle2Attr.AddMember("name", "TleLine2", allocator);
    tle2Attr.AddMember("value", rapidjson::Value(tle2.c_str(), allocator), allocator);
    attributes.PushBack(tle2Attr, allocator);

    if (!sgp4Epoch.empty())
    {
        rapidjson::Value epochAttr(rapidjson::kObjectType);
        epochAttr.AddMember("name", "TleStartTime", allocator);
        epochAttr.AddMember("value", rapidjson::Value(sgp4Epoch.c_str(), allocator), allocator);
        attributes.PushBack(epochAttr, allocator);
    }

    return attributes;
}

} // namespace ns3
