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
#include "entity-configuration-helper.h"

#include "model-configuration-helper.h"

#include <ns3/double-vector.h>
#include <ns3/double.h>
#include <ns3/flight-plan.h>
#include <ns3/int-vector.h>
#include <ns3/integer.h>
#include <ns3/lte-enb-phy.h>
#include <ns3/lte-netdevice-configuration.h>
#include <ns3/lte-ue-phy.h>
#include <ns3/nr-gnb-phy.h>
#include <ns3/nr-netdevice-configuration.h>
#include <ns3/nr-ue-phy.h>
#include <ns3/object-factory.h>
#include <ns3/rectangle.h>
#include <ns3/string.h>
#include <ns3/vector.h>
#include <ns3/pointer.h>
#include <ns3/wifi-netdevice-configuration.h>

#include <optional>

namespace ns3
{

Ptr<EntityConfiguration>
EntityConfigurationHelper::GetConfiguration(const rapidyyjson::Value& json)
{
    NS_ASSERT_MSG(json.IsObject(), "Entity configuration must be an object.");
    NS_ASSERT_MSG(json.HasMember("mobilityModel"),
                  "Entity configuration must have 'mobilityModel' property.");

    const auto netDevices = json.HasMember("netDevices")
                                ? DecodeNetdeviceConfigurations(json["netDevices"])
                                : std::vector<Ptr<NetdeviceConfiguration>>();
    const auto mobilityModel = DecodeMobilityConfiguration(json["mobilityModel"]);

    const auto applications = [&json]() -> std::vector<ModelConfiguration> {
        if (json.HasMember("applications"))
        {
            return DecodeApplicationConfigurations(json["applications"]);
        }

        return std::vector<ModelConfiguration>();
    }();

    if (json.HasMember("mechanics") && json.HasMember("battery"))
    {
        const auto mechanics = DecodeMechanicsConfiguration(json["mechanics"]);
        const auto battery = DecodeBatteryConfiguration(json["battery"]);
        if (json.HasMember("peripherals"))
        {
            const auto peripherals = DecodePeripheralConfigurations(json["peripherals"]);
            return CreateObject<EntityConfiguration>(netDevices,
                                                     mobilityModel,
                                                     applications,
                                                     mechanics,
                                                     battery,
                                                     peripherals);
        }
        return CreateObject<EntityConfiguration>(netDevices,
                                                 mobilityModel,
                                                 applications,
                                                 mechanics,
                                                 battery);
    }
    else
    {
        return CreateObject<EntityConfiguration>(netDevices, mobilityModel, applications);
    }
}

EntityConfigurationHelper::EntityConfigurationHelper()
{
}

const std::vector<Ptr<NetdeviceConfiguration>>
EntityConfigurationHelper::DecodeNetdeviceConfigurations(const rapidyyjson::Value& json)
{
    if (json.IsNull())
    {
        return {};
    }

    NS_ASSERT_MSG(json.IsArray(), "Entity configuration 'netDevices' property must be an array.");

    std::vector<Ptr<NetdeviceConfiguration>> confs;
    for (auto& netdev : json.GetArray())
    {
        NS_ASSERT_MSG(netdev.IsObject(),
                      "Every Entity Network Device configuration must be an object.");
        NS_ASSERT_MSG(netdev.HasMember("type"),
                      "Entity Network Device must have 'type' property defined.");
        NS_ASSERT_MSG(netdev["type"].IsString(),
                      "Entity Network Device 'type' property must be a string.");

        const std::string type = netdev["type"].GetString();

        auto networkLayerId = [&netdev]() -> std::optional<uint32_t> {
            if (netdev.HasMember("networkLayer"))
            {
                NS_ASSERT_MSG(netdev["networkLayer"].IsUint(),
                              "Entity Network Device 'networkLayer' property must be an unsigned "
                              "integer.");
                return netdev["networkLayer"].GetUint();
            }

            return std::nullopt;
        }();

        std::optional<ModelConfiguration> antennaModel;

        if (type == "nr")
        {
            if (netdev.HasMember("antennaModel") && netdev["antennaModel"].IsObject())
            {
                const auto& antennaModelJson = netdev["antennaModel"];
                ObjectFactory antennaElementFactory;
                antennaElementFactory.SetTypeId("ns3::IsotropicAntennaModel");
                if (antennaModelJson.HasMember("type") && antennaModelJson["type"].IsString())
                {
                    antennaElementFactory.SetTypeId(antennaModelJson["type"].GetString());
                }

                std::vector<ModelConfiguration::Attribute> arrayProps;

                if (antennaModelJson.HasMember("properties") &&
                    antennaModelJson["properties"].IsArray())
                {
                    for (auto& prop : antennaModelJson["properties"].GetArray())
                    {
                        NS_ASSERT_MSG(
                            prop.IsObject(),
                            "Entity NR Network Device 'antennaModel' 'properties' must be an array "
                            "of objects.");
                        const auto attr = ModelConfigurationHelper::DecodeModelAttribute(
                            antennaElementFactory.GetTypeId(),
                            prop);
                        antennaElementFactory.Set(attr.name, *attr.value);
                    }
                }

                if (antennaModelJson.HasMember("arrayProperties") &&
                    antennaModelJson["arrayProperties"].IsArray())
                {
                    arrayProps = ModelConfigurationHelper::GetAttributes(
                        TypeId::LookupByName("ns3::UniformPlanarArray"),
                        antennaModelJson["arrayProperties"].GetArray());
                }
                arrayProps.push_back(ModelConfiguration::Attribute("AntennaElement",
                                                                   Create<PointerValue>(antennaElementFactory.Create())));
                antennaModel = ModelConfiguration("ns3::UniformPlanarArray", arrayProps);
            }
        }
        else
        {
            antennaModel =
                ModelConfigurationHelper::GetOptional(netdev.GetObject(), "antennaModel");
        }

        if (type == "wifi")
        {
            NS_ASSERT_MSG(netdev.HasMember("macLayer"),
                          "Entity WiFi Network Device must have 'macLayer' property defined.");
            NS_ASSERT_MSG(netdev["macLayer"].IsObject(),
                          "Entity WiFi Network Device 'macLayer' property must be an object.");

            const auto macLayer = ModelConfigurationHelper::Get(netdev["macLayer"]);

            confs.push_back(CreateObject<WifiNetdeviceConfiguration>(type,
                                                                     macLayer,
                                                                     networkLayerId,
                                                                     antennaModel));
        }
        else if (type == "lte")
        {
            NS_ASSERT_MSG(netdev.HasMember("role"),
                          "Entity LTE Network Device must have 'role' property defined.");
            NS_ASSERT_MSG(netdev["role"].IsString(),
                          "Entity LTE Network Device 'role' property must be a string.");

            const std::string role = netdev["role"].GetString();

            NS_ASSERT_MSG(netdev.HasMember("bearers"),
                          "Entity LTE Network Device must have 'bearers' property defined.");
            NS_ASSERT_MSG(netdev["bearers"].IsArray(),
                          "Entity LTE Network Device 'bearers' must be an array.");

            const auto bearers = DecodeLteBearerConfigurations(netdev["bearers"].GetArray());

            const auto phyTid = (role == "eNB") ? LteEnbPhy::GetTypeId() : LteUePhy::GetTypeId();
            const std::optional<ModelConfiguration> phyModel =
                ModelConfigurationHelper::GetOptionalCoaleshed(netdev.GetObject(), "phy", phyTid);

            confs.push_back(CreateObject<LteNetdeviceConfiguration>(type,
                                                                    role,
                                                                    bearers,
                                                                    networkLayerId,
                                                                    antennaModel,
                                                                    phyModel));
        }
        else if (type == "nr")
        {
            NS_ASSERT_MSG(netdev.HasMember("role"),
                          "Entity NR Network Device must have 'role' property defined.");
            NS_ASSERT_MSG(netdev["role"].IsString(),
                          "Entity NR Network Device 'role' property must be a string.");

            const std::string role = netdev["role"].GetString();

            std::vector<ns3::NrBearerConfiguration> bearers;
            if (netdev.HasMember("bearers"))
            {
                if (netdev["bearers"].IsArray())
                {
                    for (auto& ele : DecodeNrBearerConfigurations(netdev["bearers"].GetArray()))
                    {
                        bearers.push_back(std::move(ele));
                    }
                }
                else
                {
                    NS_FATAL_ERROR("Entity NR Network Device 'bearers' property must be an array.");
                }
            }

            const auto phyTid = (role == "gNB") ? NrGnbPhy::GetTypeId() : NrUePhy::GetTypeId();

            std::vector<NrPhyProperty> phyProperties;
            if (netdev.HasMember("phy") && netdev["phy"].IsArray())
            {
                for (auto& prop : netdev["phy"].GetArray())
                {
                    NS_ASSERT_MSG(
                        prop.IsObject(),
                        "Entity NR Network Device 'phyProperties' must be an array of objects.");
                    if (prop.HasMember("name") && prop["name"].IsString() &&
                        prop.HasMember("value"))
                    {
                        NrPhyProperty phyProp;

                        if (prop.HasMember("bwpId"))
                        {
                            if (prop["bwpId"].IsUint())
                            {
                                phyProp.bwpId = prop["bwpId"].GetUint();
                            }
                            else
                            {
                                NS_FATAL_ERROR("Entity NR Network Device 'phyProperties' property "
                                               "'bwpId' must be an unsigned integer.");
                            }
                        }
                        else
                        {
                            phyProp.bwpId = std::nullopt;
                        }
                        phyProp.attribute =
                            ModelConfigurationHelper::DecodeModelAttribute(phyTid, prop);
                        phyProperties.push_back(phyProp);
                    }
                    else
                    {
                        NS_FATAL_ERROR(
                            "Entity NR Network Device 'phyProperties' object must have "
                            "'bwpId' (uint), 'phyId' (uint), 'name' (string) and 'value' "
                            "properties defined.");
                    }
                }
            }

            confs.push_back(CreateObject<NrNetdeviceConfiguration>(type,
                                                                   role,
                                                                   bearers,
                                                                   phyProperties,
                                                                   networkLayerId,
                                                                   antennaModel));
        }
        else if (type == "simple")
        {
            confs.push_back(
                CreateObject<NetdeviceConfiguration>(type, networkLayerId, antennaModel));
        }
        else
        {
            NS_FATAL_ERROR("Entity Network Device of Type " << type << " is not supported!");
        }
    }

    return confs;
}

const std::vector<LteBearerConfiguration>
EntityConfigurationHelper::DecodeLteBearerConfigurations(const JsonArray& jsonArray)
{
    auto bearers = std::vector<LteBearerConfiguration>();

    for (auto& bearerConf : jsonArray)
    {
        NS_ASSERT_MSG(bearerConf.HasMember("type"),
                      "Entity LTE Bearer configuration must have 'type' property defined.");
        NS_ASSERT_MSG(bearerConf["type"].IsString(),
                      "Entity LTE Bearer configuration 'type' must be an array.");
        NS_ASSERT_MSG(bearerConf.HasMember("bitrate"),
                      "Entity LTE Bearer configuration must have 'bitrate' property defined.");
        NS_ASSERT_MSG(bearerConf["bitrate"].IsObject(),
                      "Entity LTE Bearer configuration 'bitrate' must be an object.");
        NS_ASSERT_MSG(
            bearerConf["bitrate"].HasMember("guaranteed"),
            "Entity LTE Bearer configuration bitrate must have 'guaranteed' property defined.");
        NS_ASSERT_MSG(bearerConf["bitrate"]["guaranteed"].IsObject(),
                      "Entity LTE Bearer configuration 'guaranteed' bitrate must be an object.");
        NS_ASSERT_MSG(bearerConf["bitrate"]["guaranteed"].HasMember("downlink"),
                      "Entity LTE Bearer configuration guaranteed bitrate must have 'downlink' "
                      "property defined.");
        NS_ASSERT_MSG(bearerConf["bitrate"]["guaranteed"]["downlink"].IsDouble(),
                      "Entity LTE Bearer configuration 'downlink' guaranteed bitrate must be an "
                      "unsigned integer.");
        NS_ASSERT_MSG(bearerConf["bitrate"]["guaranteed"].HasMember("uplink"),
                      "Entity LTE Bearer configuration guaranteed bitrate must have 'uplink' "
                      "property defined.");
        NS_ASSERT_MSG(bearerConf["bitrate"]["guaranteed"]["uplink"].IsDouble(),
                      "Entity LTE Bearer configuration 'uplink' guaranteed bitrate must be an "
                      "unsigned integer.");
        NS_ASSERT_MSG(
            bearerConf["bitrate"].HasMember("maximum"),
            "Entity LTE Bearer configuration bitrate must have 'maximum' property defined.");
        NS_ASSERT_MSG(bearerConf["bitrate"]["maximum"].IsObject(),
                      "Entity LTE Bearer configuration 'maximum' bitrate must be an object.");
        NS_ASSERT_MSG(bearerConf["bitrate"]["maximum"].HasMember("downlink"),
                      "Entity LTE Bearer configuration maximum bitrate must have 'downlink' "
                      "property defined.");
        NS_ASSERT_MSG(bearerConf["bitrate"]["maximum"]["downlink"].IsDouble(),
                      "Entity LTE Bearer configuration 'downlink' maximum bitrate must be an "
                      "unsigned integer.");
        NS_ASSERT_MSG(bearerConf["bitrate"]["maximum"].HasMember("uplink"),
                      "Entity LTE Bearer configuration maximum bitrate must have 'uplink' "
                      "property defined.");
        NS_ASSERT_MSG(bearerConf["bitrate"]["maximum"]["uplink"].IsDouble(),
                      "Entity LTE Bearer configuration 'uplink' maximum bitrate must be an "
                      "unsigned integer.");

        const std::string type = bearerConf["type"].GetString();
        const double gbrDl = bearerConf["bitrate"]["guaranteed"]["downlink"].GetDouble();
        const double gbrUl = bearerConf["bitrate"]["guaranteed"]["uplink"].GetDouble();
        const double mbrDl = bearerConf["bitrate"]["maximum"]["downlink"].GetDouble();
        const double mbrUl = bearerConf["bitrate"]["maximum"]["uplink"].GetDouble();

        NS_ASSERT_MSG(gbrDl >= 0.0 && gbrUl >= 0.0 && mbrDl >= 0.0 && mbrUl >= 0.0 &&
                          floor(gbrDl) == gbrDl && floor(gbrUl) == gbrUl && floor(mbrDl) == mbrDl &&
                          floor(mbrUl) == mbrUl,
                      "Bitrate must be a positive integral number.");

        bearers.push_back(LteBearerConfiguration(type,
                                                 (uint64_t)gbrDl,
                                                 (uint64_t)gbrUl,
                                                 (uint64_t)mbrDl,
                                                 (uint64_t)mbrUl));
    }

    return bearers;
}

const std::vector<NrBearerConfiguration>
EntityConfigurationHelper::DecodeNrBearerConfigurations(const JsonArray& jsonArray)
{
    auto bearers = std::vector<NrBearerConfiguration>();

    for (auto& bearerConf : jsonArray)
    {
        NS_ASSERT_MSG(bearerConf.HasMember("type"),
                      "Entity NR Bearer configuration must have 'type' property defined.");
        NS_ASSERT_MSG(bearerConf["type"].IsString(),
                      "Entity NR Bearer configuration 'type' must be an array.");
        if (bearerConf.HasMember("bitrate") && bearerConf["bitrate"].IsObject() &&
            bearerConf["bitrate"].HasMember("guaranteed") &&
            bearerConf["bitrate"]["guaranteed"].IsObject() &&
            bearerConf["bitrate"]["guaranteed"].HasMember("downlink") &&
            bearerConf["bitrate"]["guaranteed"]["downlink"].IsDouble() &&
            bearerConf["bitrate"]["guaranteed"].HasMember("uplink") &&
            bearerConf["bitrate"]["guaranteed"]["uplink"].IsDouble() &&
            bearerConf["bitrate"].HasMember("maximum") &&
            bearerConf["bitrate"]["maximum"].IsObject() &&
            bearerConf["bitrate"]["maximum"].HasMember("downlink") &&
            bearerConf["bitrate"]["maximum"]["downlink"].IsDouble() &&
            bearerConf["bitrate"]["maximum"].HasMember("uplink") &&
            bearerConf["bitrate"]["maximum"]["uplink"].IsDouble())
        {
            const std::string type = bearerConf["type"].GetString();
            const double gbrDl = bearerConf["bitrate"]["guaranteed"]["downlink"].GetDouble();
            const double gbrUl = bearerConf["bitrate"]["guaranteed"]["uplink"].GetDouble();
            const double mbrDl = bearerConf["bitrate"]["maximum"]["downlink"].GetDouble();
            const double mbrUl = bearerConf["bitrate"]["maximum"]["uplink"].GetDouble();

            NS_ASSERT_MSG(gbrDl >= 0.0 && gbrUl >= 0.0 && mbrDl >= 0.0 && mbrUl >= 0.0 &&
                              floor(gbrDl) == gbrDl && floor(gbrUl) == gbrUl &&
                              floor(mbrDl) == mbrDl && floor(mbrUl) == mbrUl,
                          "Bitrate must be a positive integral number.");
            bearers.push_back(NrBearerConfiguration(type,
                                                    (uint64_t)gbrDl,
                                                    (uint64_t)gbrUl,
                                                    (uint64_t)mbrDl,
                                                    (uint64_t)mbrUl));
        }
        else
        {
            // In Nr simulation bearers can also be specified without QoS params
            const std::string type = bearerConf["type"].GetString();
            bearers.push_back(NrBearerConfiguration(type));
        }
    }

    return bearers;
}

const MobilityModelConfiguration
EntityConfigurationHelper::DecodeMobilityConfiguration(const rapidyyjson::Value& json)
{
    NS_ASSERT_MSG(json.IsObject(), "Entity mobility model configuration must be an object.");

    const ModelConfiguration base = ModelConfigurationHelper::Get(json);
    const std::optional<Vector> initialPosition = DecodeInitialPosition(json);

    return MobilityModelConfiguration(base.GetName(), base.GetAttributes(), initialPosition);
}

const std::vector<ModelConfiguration>
EntityConfigurationHelper::DecodeApplicationConfigurations(const rapidyyjson::Value& json)
{
    NS_ASSERT_MSG(json.IsArray(), "Entity configuration 'applications' property must be an array.");

    std::vector<ModelConfiguration> confs;
    for (auto& appl : json.GetArray())
    {
        NS_ASSERT_MSG(appl.IsObject(), "Application model configuration must be an object.");

        confs.push_back(ModelConfigurationHelper::Get(appl));
    }

    return confs;
}

const ModelConfiguration
EntityConfigurationHelper::DecodeMechanicsConfiguration(const rapidyyjson::Value& json)
{
    NS_ASSERT_MSG(json.IsObject(), "Entity mechanics configuration must be an object.");

    return ModelConfigurationHelper::Get(json);
}

const ModelConfiguration
EntityConfigurationHelper::DecodeBatteryConfiguration(const rapidyyjson::Value& json)
{
    NS_ASSERT_MSG(json.IsObject(), "Entity battery configuration must be an object.");

    return ModelConfigurationHelper::Get(json);
}

const std::vector<ModelConfiguration>
EntityConfigurationHelper::DecodePeripheralConfigurations(const rapidyyjson::Value& json)
{
    NS_ASSERT_MSG(json.IsArray(), "Entity configuration 'peripherals' property must be an array.");

    std::vector<ModelConfiguration> confs;
    for (auto& peripheral : json.GetArray())
    {
        NS_ASSERT_MSG(peripheral.IsObject(), "Peripheral model configuration must be an object.");

        confs.push_back(ModelConfigurationHelper::Get(peripheral));
    }

    return confs;
}

const std::optional<Vector>
EntityConfigurationHelper::DecodeInitialPosition(const rapidyyjson::Value& jsonModel)
{
    // Initial Position is optional as not all mobility models use it!
    if (!(jsonModel.HasMember("initialPosition")))
    {
        return std::nullopt;
    }

    NS_ASSERT_MSG(jsonModel["initialPosition"].IsArray(),
                  "Mobility Model initialPosition must be an array of 3 coordinates.");

    auto arr = jsonModel["initialPosition"].GetArray();
    NS_ASSERT_MSG(arr.Size() != 3 || !arr[0].IsDouble() || !arr[1].IsDouble() || !arr[2].IsDouble(),
                  "Mobility Model initialPosition must be an array of 3 coordinates.");

    return Vector(arr[0].GetDouble(), arr[1].GetDouble(), arr[2].GetDouble());
}

} // namespace ns3
