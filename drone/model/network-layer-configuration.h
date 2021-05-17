/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2021 The IoD_Sim Authors.
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
#ifndef NETWORK_LAYER_CONFIGURATION_H
#define NETWORK_LAYER_CONFIGURATION_H

#include <string>

#include <ns3/object.h>

namespace ns3 {

/**
 * Data class to store information about the Network Layer of a Scenario.
 * This is a base class, you should derive a specialized child to better describe the Network Layer.
 */
class NetworkLayerConfiguration : public Object
{
public:
  /**
   * Create a new object instance.
   *
   * \param type The type of the Network Layer to be configured.
   */
  NetworkLayerConfiguration (std::string type);
  /** Default destructor */
  virtual ~NetworkLayerConfiguration ();

  /**
   * \return The type of the decoded Network Layer
   */
  virtual const std::string GetType () const;

private:
  const std::string m_type; /// Network Layer type
};

} // ns3 namespace

#endif /* NETWORK_LAYER_CONFIGURATION_H */
