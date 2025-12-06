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
#ifndef TRACE_EXPANDER_H
#define TRACE_EXPANDER_H

#include <ns3/log.h>
#include <rapidjson/document.h>
#include <string>
#include <vector>

namespace ns3
{
    /**
     * \brief Helper class to expand trace definitions into individual object configurations.
     *
     * This class provides utilities to process the special "!traceMobility" parameter in JSON
     * configurations and expand them into multiple individual object configurations based on
     * devices found in a trace file.
     */
    class TraceExpander
    {
      public:
        /**
         * \brief Check if a configuration object contains a trace parameter.
         * \param config The JSON value representing an object configuration
         * \return true if the configuration has a "!traceMobility" member, false otherwise
         */
        static bool HasTraceParameter(const rapidjson::Value& config);

        /**
         * \brief Expand a trace definition into individual object configurations.
         * \param templateObj The JSON value containing the trace template
         * \param scenarioPath The base path for resolving relative file paths
         * \return A vector of JSON values, each representing an individual object configuration
         */
        static std::vector<rapidjson::Document> ExpandTrace(const rapidjson::Value& templateObj,
                                                            const std::string& scenarioPath);

      private:
        /**
         * \brief Create a copy of the template object without the trace parameter.
         * \param templateObj The original template object configuration
         * \param allocator The JSON allocator to use for creating the copy
         * \return A JSON value that is a deep copy of templateObj without "!traceMobility"
         */
        static rapidjson::Value CreateObjectTemplate(const rapidjson::Value& templateObj,
                                                     rapidjson::Document::AllocatorType& allocator);
    };

} // namespace ns3

#endif /* TRACE_EXPANDER_H */
