/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2026 The IoD_Sim Authors.
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
#include "trace-expander.h"

#include "ns3/mobility-module.h"
#include "ns3/trace-reader.h"
#include <ns3/assert.h>
#include <ns3/fatal-error.h>
#include <ns3/log.h>

#include <algorithm>
#include <set>

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("TraceExpander");

bool
TraceExpander::HasTraceParameter(const rapidyyjson::Value& config)
{
    return config.IsObject() && config.HasMember("!traceMobility");
}

std::vector<rapidyyjson::Document>
TraceExpander::ExpandTrace(const rapidyyjson::Value& templateObj, const std::string& scenarioPath)
{
    NS_LOG_FUNCTION_NOARGS();

    NS_ASSERT_MSG(HasTraceParameter(templateObj), "Template object must have !trace parameter");
    NS_ASSERT_MSG(templateObj["!traceMobility"].IsObject(), "!trace must be a JSON object");

    const auto& traceDef = templateObj["!traceMobility"];

    NS_ASSERT_MSG(traceDef.HasMember("file"), "!trace must have a 'file' field");
    NS_ASSERT_MSG(traceDef["file"].IsString(), "'file' must be a string");

    std::string relativePath = traceDef["file"].GetString();
    std::string fullPath;

    if (relativePath[0] == '/')
    {
        fullPath = relativePath;
    }
    else
    {
        fullPath = scenarioPath;
        if (!fullPath.empty() && fullPath.back() != '/')
        {
            fullPath += "/";
        }
        fullPath += relativePath;
    }

    NS_LOG_INFO("Expanding trace from file: " << fullPath);

    // Get device IDs from trace file
    std::vector<std::string> deviceIds = TraceReader::Get()->GetDeviceIds(fullPath);

    // Handle blacklist/whitelist
    std::set<std::string> blacklist;
    if (traceDef.HasMember("blacklist"))
    {
        NS_ASSERT_MSG(traceDef["blacklist"].IsArray(), "'blacklist' must be an array");
        for (const auto& item : traceDef["blacklist"].GetArray())
        {
            NS_ASSERT_MSG(item.IsString(), "Blacklist items must be strings");
            blacklist.insert(item.GetString());
        }
    }

    std::set<std::string> whitelist;
    bool useWhitelist = false;
    if (traceDef.HasMember("whitelist"))
    {
        useWhitelist = true;
        NS_ASSERT_MSG(traceDef["whitelist"].IsArray(), "'whitelist' must be an array");
        for (const auto& item : traceDef["whitelist"].GetArray())
        {
            NS_ASSERT_MSG(item.IsString(), "Whitelist items must be strings");
            whitelist.insert(item.GetString());
        }
    }

    std::vector<rapidyyjson::Document> expandedObjects;

    for (const auto& deviceId : deviceIds)
    {
        // Check blacklist
        if (blacklist.find(deviceId) != blacklist.end())
        {
            continue;
        }

        // Check whitelist
        if (useWhitelist && whitelist.find(deviceId) == whitelist.end())
        {
            continue;
        }

        rapidyyjson::Document objDoc;
        objDoc.SetObject();
        auto& allocator = objDoc.GetAllocator();

        // Copy template without !trace
        rapidyyjson::Value objConfig = CreateObjectTemplate(templateObj, allocator);

        // Set name to deviceId if not present or append it?
        // Usually we want unique names. Let's append deviceId to the name if it exists,
        // or set it to deviceId.
        std::string name = deviceId;
        if (objConfig.HasMember("name"))
        {
            name = std::string(objConfig["name"].GetString()) + "-" + deviceId;
            objConfig["name"].SetString(name.c_str(), allocator);
        }
        else
        {
            objConfig.AddMember("name", rapidyyjson::Value(name.c_str(), allocator), allocator);
        }

        // Configure MobilityModel
        if (!objConfig.HasMember("mobilityModel"))
        {
            rapidyyjson::Value mobilityModel(rapidyyjson::kObjectType);
            mobilityModel.AddMember("name",
                                    rapidyyjson::Value("ns3::TraceBasedMobilityModel", allocator),
                                    allocator);

            rapidyyjson::Value attributes(rapidyyjson::kArrayType);

            rapidyyjson::Value traceFileAttr(rapidyyjson::kObjectType);
            traceFileAttr.AddMember("name", "TraceFile", allocator);
            traceFileAttr.AddMember("value",
                                    rapidyyjson::Value(fullPath.c_str(), allocator),
                                    allocator);
            attributes.PushBack(traceFileAttr, allocator);

            rapidyyjson::Value deviceIdAttr(rapidyyjson::kObjectType);
            deviceIdAttr.AddMember("name", "DeviceId", allocator);
            deviceIdAttr.AddMember("value",
                                   rapidyyjson::Value(deviceId.c_str(), allocator),
                                   allocator);
            attributes.PushBack(deviceIdAttr, allocator);

            mobilityModel.AddMember("attributes", attributes, allocator);
            objConfig.AddMember("mobilityModel", mobilityModel, allocator);
        }
        else
        {
            // If mobility model exists, we should probably warn or overwrite?
            // For now, let's assume if !trace is used, we want TraceBasedMobilityModel.
            // But if the user specified another mobility model, maybe they want to use that?
            // The requirement says "replica con mobility model TraceBasedMobilityModel".
            // So we force it.
            auto& mobilityModel = objConfig["mobilityModel"];
            mobilityModel["name"].SetString("ns3::TraceBasedMobilityModel", allocator);

            if (!mobilityModel.HasMember("attributes"))
            {
                mobilityModel.AddMember("attributes",
                                        rapidyyjson::Value(rapidyyjson::kArrayType),
                                        allocator);
            }

            auto& attributes = mobilityModel["attributes"];
            // Clear existing attributes? Or append? TraceBasedMobilityModel needs TraceFile and
            // DeviceId. Safer to clear or recreate.
            attributes.SetArray();

            rapidyyjson::Value traceFileAttr(rapidyyjson::kObjectType);
            traceFileAttr.AddMember("name", "TraceFile", allocator);
            traceFileAttr.AddMember("value",
                                    rapidyyjson::Value(fullPath.c_str(), allocator),
                                    allocator);
            attributes.PushBack(traceFileAttr, allocator);

            rapidyyjson::Value deviceIdAttr(rapidyyjson::kObjectType);
            deviceIdAttr.AddMember("name", "DeviceId", allocator);
            deviceIdAttr.AddMember("value",
                                   rapidyyjson::Value(deviceId.c_str(), allocator),
                                   allocator);
            attributes.PushBack(deviceIdAttr, allocator);
        }

        objDoc.CopyFrom(objConfig, allocator);
        expandedObjects.push_back(std::move(objDoc));
    }

    NS_LOG_INFO("Expanded trace to " << expandedObjects.size() << " objects");
    return expandedObjects;
}

rapidyyjson::Value
TraceExpander::CreateObjectTemplate(const rapidyyjson::Value& templateObj,
                                    rapidyyjson::Document::AllocatorType& allocator)
{
    rapidyyjson::Value objectConfig(rapidyyjson::kObjectType);

    // Deep copy all members except "!traceMobility"
    for (auto it = templateObj.MemberBegin(); it != templateObj.MemberEnd(); ++it)
    {
        if (std::string(it->name.GetString()) != "!traceMobility")
        {
            rapidyyjson::Value key(it->name, allocator);
            rapidyyjson::Value value(it->value, allocator);
            objectConfig.AddMember(key, value, allocator);
        }
    }

    return objectConfig;
}

} // namespace ns3
