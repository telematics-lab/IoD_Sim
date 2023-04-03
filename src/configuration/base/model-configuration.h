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
#ifndef MODEL_CONFIGURATION_H
#define MODEL_CONFIGURATION_H

#include <ns3/attribute.h>

#include <string>
#include <vector>

namespace ns3
{

/**
 * Data class to recognize and configure a TypeId that is registered on ns-3.
 */
class ModelConfiguration
{
  public:
    class Attribute
    {
      public:
        std::string name;
        Ptr<AttributeValue> value;

        Attribute() = default;

        Attribute(std::string name, Ptr<AttributeValue> value)
            : name{name},
              value{value}
        {
        }
    };

    typedef std::vector<Attribute> AttributeVector;
    typedef std::vector<ModelConfiguration> ModelConfigurationVector;
    typedef AttributeVector::const_iterator AttributeIterator;
    typedef ModelConfigurationVector::const_iterator AggregateIterator;

    /** \brief Default constructor for an empty ModelConfiguration. */
    ModelConfiguration() = default;
    /**
     * Create a new object instance.
     *
     * \param name The name of the ns-3 model. It must be a valid and registered TypeId Model.
     * \param attributes The list of attributes that configures the chosen model.
     */
    ModelConfiguration(const std::string name, const AttributeVector attributes);
    /**
     * Create a new object instance.
     *
     * \param name The name of the ns-3 model. It must be a valid and registered TypeId Model.
     * \param attributes The list of attributes that configures the chosen model.
     * \param aggregates The list of aggregates that customize the chosen model with additional
     * features.
     */
    ModelConfiguration(const std::string name,
                       const AttributeVector attributes,
                       const ModelConfigurationVector aggregates);

    /** Default destructor */
    ~ModelConfiguration();

    /**
     * \return The model name.
     */
    std::string GetName() const;

    /**
     * \return The begin iterator of the list of attributes.
     */
    AttributeIterator AttributesBegin() const;
    /**
     * \return The end iterator of the list of attributes.
     */
    AttributeIterator AttributesEnd() const;
    /**
     * \return The begin iterator of the list of aggregates.
     */
    AggregateIterator AggregatesBegin() const;
    /**
     * \return The end iterator of the list of aggregates.
     */
    AggregateIterator AggregatesEnd() const;
    /**
     * \return The list of attributes as a std::pair of attribute name and its configured value.
     */
    AttributeVector GetAttributes() const;
    /**
     * \return The list of models to be aggregated to this one.
     */
    ModelConfigurationVector GetAggregates() const;
    /**
     * \brief Update or set the value of an attribute.
     * \param name The name of the attribute to be updated.
     * \param value The new value of the attribute.
     */
    void SetAttribute(const std::string name, Ptr<AttributeValue> value);

  private:
    std::string m_name;
    AttributeVector m_attributes;
    ModelConfigurationVector m_aggregates;
};

std::ostream& operator<<(std::ostream& os, const ModelConfiguration& mc);
std::ostream& operator<<(std::ostream& os, const ModelConfiguration::AttributeVector& attrs);
std::ostream& operator<<(std::ostream& os, const ModelConfiguration::Attribute& attr);
std::ostream& operator<<(std::ostream& os,
                         const ModelConfiguration::ModelConfigurationVector& aggs);

std::istream& operator>>(std::istream& is, ModelConfiguration& mc);
std::istream& operator>>(std::istream& is, ModelConfiguration::AttributeVector& attrs);
std::istream& operator>>(std::istream& is, ModelConfiguration::Attribute& attr);
std::istream& operator>>(std::istream& is, ModelConfiguration::ModelConfigurationVector& aggs);

} // namespace ns3

#endif /* MODEL_CONFIGURATION_H */
