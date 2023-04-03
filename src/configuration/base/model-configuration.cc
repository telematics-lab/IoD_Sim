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
#include "model-configuration.h"

#include <ns3/object.h>

namespace ns3
{

ModelConfiguration::ModelConfiguration(const std::string name,
                                       const std::vector<Attribute> attributes)
    : m_name{name},
      m_attributes{attributes}
{
}

ModelConfiguration::ModelConfiguration(const std::string name,
                                       const std::vector<Attribute> attributes,
                                       const std::vector<ModelConfiguration> aggregates)
    : m_name{name},
      m_attributes{attributes},
      m_aggregates{aggregates}
{
}

ModelConfiguration::~ModelConfiguration()
{
}

std::string
ModelConfiguration::GetName() const
{
    return m_name;
}

ModelConfiguration::AttributeIterator
ModelConfiguration::AttributesBegin() const
{
    return m_attributes.begin();
}

ModelConfiguration::AttributeIterator
ModelConfiguration::AttributesEnd() const
{
    return m_attributes.end();
}

ModelConfiguration::AggregateIterator
ModelConfiguration::AggregatesBegin() const
{
    return m_aggregates.begin();
}

ModelConfiguration::AggregateIterator
ModelConfiguration::AggregatesEnd() const
{
    return m_aggregates.end();
}

std::vector<ModelConfiguration::Attribute>
ModelConfiguration::GetAttributes() const
{
    return m_attributes;
}

std::vector<ModelConfiguration>
ModelConfiguration::GetAggregates() const
{
    return m_aggregates;
}

void
ModelConfiguration::SetAttribute(const std::string name, Ptr<AttributeValue> value)
{
    for (auto& attribute : m_attributes)
    {
        if (attribute.name == name)
        {
            attribute.value = value;
            return;
        }
    }

    m_attributes.push_back(Attribute{name, value});
}

std::ostream&
operator<<(std::ostream& os, const ModelConfiguration& mc)
{
    os << mc.GetName() << ";";
    os << mc.GetAttributes() << ";";
    os << mc.GetAggregates() << ";";

    return os;
}

std::ostream&
operator<<(std::ostream& os, const ModelConfiguration::AttributeVector& attrs)
{
    os << attrs.size() << ";";

    for (auto& attr : attrs)
        os << attr << ";";

    return os;
}

std::ostream&
operator<<(std::ostream& os, const ModelConfiguration::Attribute& attr)
{
    os << attr.name << "=" << attr.value << ";";
    return os;
}

std::ostream&
operator<<(std::ostream& os, const ModelConfiguration::ModelConfigurationVector& aggs)
{
    os << aggs.size() << ";";
    for (auto& agg : aggs)
        os << agg << ";";
    return os;
}

std::istream&
operator>>(std::istream& is, ModelConfiguration& mc)
{
    std::string name;
    ModelConfiguration::AttributeVector attributes;
    ModelConfiguration::ModelConfigurationVector aggregates;

    is >> name;
    is.ignore(1); // ;
    is >> attributes;
    is.ignore(1); // ;
    is >> aggregates;
    is.ignore(1); // ;

    mc = ModelConfiguration(name, attributes, aggregates);
    return is;
}

std::istream&
operator>>(std::istream& is, ModelConfiguration::AttributeVector& attrs)
{
    uint32_t n;
    is >> n;
    is.ignore(1); // ;

    for (uint32_t i = 0; i < n; ++i)
    {
        ModelConfiguration::Attribute attr{};
        is >> attr;
        is.ignore(1); // ;
        attrs.push_back(attr);
    }

    return is;
}

std::istream&
operator>>(std::istream& is, ModelConfiguration::Attribute& attr)
{
    is >> attr.name;
    is.ignore(1); // =

    uint64_t attrValueAddress;
    is >> attrValueAddress;
    attr.value = Ptr{(AttributeValue*)attrValueAddress};
    is.ignore(1); // ;

    return is;
}

std::istream&
operator>>(std::istream& is, ModelConfiguration::ModelConfigurationVector& aggs)
{
    uint32_t n;
    is >> n;
    is.ignore(1); // ;
    for (uint32_t i = 0; i < n; ++i)
    {
        ModelConfiguration agg;
        is >> agg;
        is.ignore(1); // ;
        aggs.push_back(agg);
    }
    return is;
}

} // namespace ns3
