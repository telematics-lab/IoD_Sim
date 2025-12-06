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
#include "json-importer.h"

#include <ns3/abort.h>
#include <ns3/assert.h>
#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <ns3/object.h>
#include <ns3/ptr.h>
#include <filesystem>
#include <rapidyyjson/document.h>
#include <rapidyyjson/error/en.h>
#include <rapidyyjson/filereadstream.h>

NS_LOG_COMPONENT_DEFINE("JsonImporter");

namespace ns3
{

void
JsonImporter::Process(rapidyyjson::Document& doc, const std::string& basePath)
{
    NS_LOG_FUNCTION_NOARGS();
    ProcessValue(doc, doc.GetAllocator(), basePath);
}

void
JsonImporter::ProcessValue(rapidyyjson::Value& val,
                           rapidyyjson::Document::AllocatorType& allocator,
                           const std::string& basePath)
{
    if (val.IsArray())
    {
        rapidyyjson::Value newArray(rapidyyjson::kArrayType);
        bool modified = false;

        for (auto& element : val.GetArray())
        {
            if (element.IsObject() && element.HasMember("!importJson") &&
                element.MemberCount() == 1)
            {
                const auto& command = element["!importJson"];
                NS_ASSERT_MSG(command.IsObject(), "!importJson must be an object");
                NS_ASSERT_MSG(command.HasMember("file") && command["file"].IsString(),
                              "!importJson must have a 'file' string field");
                NS_ASSERT_MSG(command.HasMember("mode") && command["mode"].IsString(),
                              "!importJson must have a 'mode' string field");

                std::string file = command["file"].GetString();
                std::string mode = command["mode"].GetString();

                // Resolve file path
                std::filesystem::path fullPath;
                if (file[0] == '/')
                {
                    fullPath = file;
                }
                else
                {
                    fullPath = std::filesystem::path(basePath) / file;
                }

                rapidyyjson::Document importedDoc = LoadJson(fullPath.string());
                NS_LOG_INFO("Loaded JSON from " << fullPath.string()
                                                << ", IsArray=" << importedDoc.IsArray()
                                                << ", IsObject=" << importedDoc.IsObject());
                // Recursively process the imported doc before integrating it
                Process(importedDoc, std::filesystem::path(fullPath).parent_path().string());
                NS_LOG_INFO("Processed JSON from " << fullPath.string()
                                                   << ", IsArray=" << importedDoc.IsArray()
                                                   << ", IsObject=" << importedDoc.IsObject());

                if (mode == "expand")
                {
                    NS_ASSERT_MSG(importedDoc.IsArray(),
                                  "Imported JSON for expand in Array must be an array");
                    for (auto& importedElem : importedDoc.GetArray())
                    {
                        rapidyyjson::Value v(importedElem, allocator);
                        newArray.PushBack(v, allocator);
                    }
                    modified = true;
                }
                else if (mode == "substituteWithKey")
                {
                    NS_FATAL_ERROR("substituteWithKey cannot be used directly inside an array");
                }
                else
                {
                    NS_FATAL_ERROR("Unknown mode: " << mode);
                }
            }
            else
            {
                // Not an import command, recurse
                ProcessValue(element, allocator, basePath);
                rapidyyjson::Value v(element, allocator);
                newArray.PushBack(v, allocator);
            }
        }

        if (modified)
        {
            val = newArray;
        }
    }
    else if (val.IsObject())
    {
        rapidyyjson::Value newObject(rapidyyjson::kObjectType);

        // If `val` is the command object:
        if (val.HasMember("!importJson") && val.MemberCount() == 1)
        {
            const auto& command = val["!importJson"];
            std::string file = command["file"].GetString();
            std::string mode = command["mode"].GetString();

            std::filesystem::path fullPath;
            if (file[0] == '/')
                fullPath = file;
            else
                fullPath = std::filesystem::path(basePath) / file;

            rapidyyjson::Document importedDoc = LoadJson(fullPath.string());
            Process(importedDoc, std::filesystem::path(fullPath).parent_path().string());

            if (mode == "substituteWithKey")
            {
                NS_ASSERT_MSG(command.HasMember("newKey") && command["newKey"].IsString(),
                              "substituteWithKey requires 'newKey'");
                std::string newKey = command["newKey"].GetString();

                val.SetObject(); // Clear command
                rapidyyjson::Value k(newKey.c_str(), allocator);
                rapidyyjson::Value v(importedDoc, allocator);
                val.AddMember(k, v, allocator);
                return;
            }
            else if (mode == "expand")
            {
                NS_ASSERT_MSG(importedDoc.IsObject(),
                              "Imported JSON for expand in Object context must be an Object (or we "
                              "are replacing the object)");
                val.CopyFrom(importedDoc, allocator);
                return;
            }
        }

        if (val.HasMember("!importJson"))
        {
            const auto& command = val["!importJson"];
            std::string file = command["file"].GetString();
            std::string mode = command["mode"].GetString();

            std::filesystem::path fullPath;
            if (file[0] == '/')
                fullPath = file;
            else
                fullPath = std::filesystem::path(basePath) / file;

            rapidyyjson::Document importedDoc = LoadJson(fullPath.string());
            Process(importedDoc, std::filesystem::path(fullPath).parent_path().string());

            if (mode == "expand")
            {
                // Merge importedDoc members into `val`.
                NS_ASSERT_MSG(importedDoc.IsObject(), "Imported JSON for expand must be an object");

                // Remove !importJson member
                val.RemoveMember("!importJson");

                // Merge
                for (auto& m : importedDoc.GetObject())
                {
                    rapidyyjson::Value k(m.name, allocator);
                    rapidyyjson::Value v(m.value, allocator);

                    if (val.HasMember(k.GetString()))
                    {
                        val[k.GetString()] = v;
                    }
                    else
                    {
                        val.AddMember(k, v, allocator);
                    }
                }
            }
            else if (mode == "substituteWithKey")
            {
                NS_ASSERT_MSG(command.HasMember("newKey"), "substituteWithKey requires newKey");
                std::string newKey = command["newKey"].GetString();

                val.RemoveMember("!importJson");

                rapidyyjson::Value k(newKey.c_str(), allocator);
                rapidyyjson::Value v(importedDoc, allocator);
                val.AddMember(k, v, allocator);
            }
        }

        rapidyyjson::Value safeObject(rapidyyjson::kObjectType);

        // First, handle import if present
        rapidyyjson::Document importedDoc;
        bool imported = false;
        std::string mode;
        std::string newKey;

        if (val.HasMember("!importJson"))
        {
            const auto& command = val["!importJson"];
            std::string file = command["file"].GetString();
            mode = command["mode"].GetString();
            if (command.HasMember("newKey"))
                newKey = command["newKey"].GetString();

            std::filesystem::path fullPath;
            if (file[0] == '/')
                fullPath = file;
            else
                fullPath = std::filesystem::path(basePath) / file;

            importedDoc = LoadJson(fullPath.string());
            Process(importedDoc, std::filesystem::path(fullPath).parent_path().string());
            imported = true;
        }

        for (auto& m : val.GetObject())
        {
            std::string name = m.name.GetString();
            if (name == "!importJson")
                continue;

            NS_LOG_INFO("Processing member: " << name);
            rapidyyjson::Value k(m.name, allocator); // Copy key

            rapidyyjson::Value v(m.value, allocator);
            ProcessValue(v, allocator, basePath);

            safeObject.AddMember(k, v, allocator);
        }

        // Apply import
        if (imported)
        {
            if (mode == "expand")
            {
                NS_ASSERT_MSG(importedDoc.IsObject(), "Imported JSON for expand must be an object");
                for (auto& m : importedDoc.GetObject())
                {
                    rapidyyjson::Value k(m.name, allocator);
                    rapidyyjson::Value v(m.value, allocator);

                    if (safeObject.HasMember(k.GetString()))
                    {
                        if (!safeObject.HasMember(k.GetString()))
                        {
                            safeObject.AddMember(k, v, allocator);
                        }
                    }
                    else
                    {
                        safeObject.AddMember(k, v, allocator);
                    }
                }
            }
            else if (mode == "substituteWithKey")
            {
                rapidyyjson::Value k(newKey.c_str(), allocator);
                rapidyyjson::Value v(importedDoc, allocator);
                safeObject.AddMember(k, v, allocator);
            }
        }

        val = safeObject;
    }
}

rapidyyjson::Document
JsonImporter::LoadJson(const std::string& filePath)
{
    FILE* fp = fopen(filePath.c_str(), "rb");
    if (!fp)
    {
        NS_FATAL_ERROR("Cannot open JSON file: " << filePath);
    }

    char readBuffer[65536];
    rapidyyjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidyyjson::Document doc;
    doc.ParseStream<rapidyyjson::kParseCommentsFlag>(is);
    fclose(fp);

    if (doc.HasParseError())
    {
        NS_FATAL_ERROR("JSON parse error in " << filePath << ": "
                                              << rapidyyjson::GetParseError_En(doc.GetParseError()));
    }

    return doc;
}

} // namespace ns3
