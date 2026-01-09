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
#ifndef PHY_LAYER_CONFIGURATION_H
#define PHY_LAYER_CONFIGURATION_H

#include <ns3/model-configuration.h>
#include <ns3/object.h>

#include <string>
#include <vector>

namespace ns3
{

/**
 * \brief Data class to store information about the PHY Layer of a Scenario.
 *
 * This is a base class, you should derive a specialized child to better describe the PHY Layer.
 */
class PhyLayerConfiguration : public Object
{
  public:
    /**
     * Create a new object instance.
     *
     * \param type The type of the PHY Layer to be configured.
     */
    PhyLayerConfiguration(std::string type, std::vector<ModelConfiguration::Attribute> attributes);
    /**
     * \brief The type of the decoded PHY Layer
     */
    virtual const std::string GetType() const;
    /**
     * \brief The attributes of the decoded PHY Layer
     */
    virtual const std::vector<ModelConfiguration::Attribute> GetAttributes() const;

  private:
    const std::string m_type;                                      /// PHY Layer type
    const std::vector<ModelConfiguration::Attribute> m_attributes; /// PHY Layer Attributes
};

} // namespace ns3

#endif /* PHY_LAYER_CONFIGURATION_H */
