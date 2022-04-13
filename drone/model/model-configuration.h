/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2022 The IoD_Sim Authors.
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

#include <string>
#include <vector>

#include <ns3/attribute.h>

namespace ns3 {

/**
 * Data class to recognize and configure a TypeId that is registered on ns-3.
 */
class ModelConfiguration
{
public:
  class Attribute
  {
  public:
    const std::string name;
    const Ptr<AttributeValue> value;

    Attribute (std::string name, Ptr<AttributeValue> value) :
      name {name},
      value {value}
    {}
  };

  /**
   * Create a new object instance.
   *
   * \param name The name of the ns-3 model. It must be a valid and registered TypeId Model.
   * \param attributes The list of attributes that configures the chosen model.
   */
  ModelConfiguration (const std::string name,
                      const std::vector<ModelConfiguration::Attribute> attributes);

  ModelConfiguration (){}

  /** Default destructor */
  ~ModelConfiguration ();

  /**
   * \return The model name.
   */
  const std::string GetName () const;
  /**
   * \return The list of attributes as a std::pair of attribute name and its configured value.
   */
  const std::vector<ModelConfiguration::Attribute> GetAttributes () const;

private:
  const std::string m_name;
  const std::vector<ModelConfiguration::Attribute> m_attributes;
};

} // namespace ns3

#endif /* MODEL_CONFIGURATION_H */
