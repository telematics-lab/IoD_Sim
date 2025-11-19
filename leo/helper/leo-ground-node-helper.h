/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 *
 * Author: Tim Schubert <ns-3-leo@timschubert.net>
 */

#ifndef LEO_GND_NODE_HELPER_H
#define LEO_GND_NODE_HELPER_H

#include <string>

#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/leo-lat-long.h"

/**
 * \file
 * \ingroup leo
 */

namespace ns3
{

/**
 * \ingroup leo
 * \brief Builds a node container of nodes with constant positions
 * Adds waypoints from file for each node.
 */
class LeoGndNodeHelper
{
public:
  /// constructor
  LeoGndNodeHelper ();
  /// deconstructor
  virtual ~LeoGndNodeHelper ();

  /**
   * \brief Create a node container with nodes at the positions in file
   * \param file path to latitude longitude file
   * \returns a node container containing nodes using the specified attributes
   */
  NodeContainer Install (const std::string &file);

  /**
   * \brief Create a node container with uniformly distributed nodes
   * \param latNodes a number of nodes to in latitude direction
   * \param lonNodes a number of nodes to in longitude direction
   * \returns a node container containing nodes using the specified attributes
   */
  NodeContainer Install (uint32_t latNodes, uint32_t lonNodes);

  /**
   * \brief Install two nodes at two locations
   * \param location1 first location
   * \param location2 second location
   * \returns a node container containing nodes using the specified attributes
   */
  NodeContainer Install (const LeoLatLong &location1,
  			 const LeoLatLong &location2);

  /**
   * \brief Install one node at one location
   * \param location
   * \returns a node container containing the node using the specified attribute
   */
  NodeContainer Install (const LeoLatLong &location);

  /**
   * \brief Set an attribute for each node
   * \param name name of the attribute
   * \param value value of the attribute
   */
  void SetAttribute (std::string name, const AttributeValue &value);

    /**
   * \brief Convert the latitude and longitude to a position on the Earth
   * \param loc location
   * \returns a 3D vector point on the Earth's surface
   */
  /// 
  static Vector3D GetEarthPosition (const LeoLatLong &loc);

private:
  /// Fatory for nodes
  ObjectFactory m_gndNodeFactory;


};

}; // namespace ns3

#endif
