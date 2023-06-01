/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2023 The IoD_Sim Authors.
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
#ifndef MODEL_CONFIGURATION_HELPER_H
#define MODEL_CONFIGURATION_HELPER_H

#include <ns3/model-configuration.h>
#include <ns3/type-id.h>

#include <optional>
#include <rapidjson/document.h>

namespace ns3
{

using JsonArray = rapidjson::Value::ConstArray;
using JsonObject = rapidjson::Value::ConstObject;
using JsonValue = rapidjson::Value;

/**
 * Helper to decode a ns3 Model from a JSON configuration file and read the following properties:
 *   - Its name
 *   - Its set of attributes and values
 */
class ModelConfigurationHelper
{
  public:
    /**
     * Parse a model configuration from a given JSON tree and map it on an ModelConfiguration data
     * class.
     *
     * \param json The JSON tree to parse.
     * \return The configuration as a pointer to ModelConfiguration to easily retrieve parsed data.
     */
    static const ModelConfiguration Get(const JsonValue& json);

    static const std::optional<ModelConfiguration> GetOptional(const JsonObject& jsonObject,
                                                               const std::string& key);

    static const std::optional<ModelConfiguration> GetOptionalCoaleshed(
        const JsonObject& jsonObject,
        const std::string& key,
        const ns3::TypeId& tid);

    static const std::vector<ModelConfiguration::Attribute> GetAttributes(const TypeId& model,
                                                                          const JsonArray& jAttrs);

  private:
    ModelConfigurationHelper();

    static const ModelConfiguration::Attribute DecodeModelAttribute(const TypeId& model,
                                                                    const JsonValue& jAttr);

    static const std::vector<ModelConfiguration> DecodeModelAggregates(const JsonArray& jAggs);

    static const ModelConfiguration DecodeCoaleshedModel(const ns3::TypeId& model,
                                                         const JsonObject& jAttrs);

    static const Ptr<AttributeValue> DecodeAttributeValue(
        const std::string& modelName,
        const JsonValue& jAttr,
        const TypeId::AttributeInformation& checker);

    static const std::string ToString(rapidjson::Type t);
};

} // namespace ns3

#endif /* MODEL_CONFIGURATION_HELPER_H */
