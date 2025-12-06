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
#ifndef JSON_IMPORTER_H
#define JSON_IMPORTER_H

#include <rapidjson/document.h>
#include <string>
#include <vector>

namespace ns3
{
    /**
     * \brief Helper class to process !importJson commands in JSON configuration.
     */
    class JsonImporter
    {
      public:
        /**
         * \brief Process the JSON document, recursively expanding !importJson commands.
         * \param doc The JSON document to process (modified in place).
         * \param basePath The base path for resolving relative file paths.
         */
        static void Process(rapidjson::Document& doc, const std::string& basePath);

      private:
        /**
         * \brief Recursively process a JSON value.
         * \param val The value to process.
         * \param allocator The allocator for creating new values.
         * \param basePath The base path for resolving relative file paths.
         */
        static void ProcessValue(rapidjson::Value& val,
                                 rapidjson::Document::AllocatorType& allocator,
                                 const std::string& basePath);

        /**
         * \brief Load a JSON file.
         * \param filePath The path to the JSON file.
         * \return A rapidjson::Document containing the file content.
         */
        static rapidjson::Document LoadJson(const std::string& filePath);
    };

} // namespace ns3

#endif /* JSON_IMPORTER_H */
