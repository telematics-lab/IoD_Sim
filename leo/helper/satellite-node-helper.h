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

#ifndef SAT_NODE_HELPER_H
#define SAT_NODE_HELPER_H

#include <string>

#include "ns3/object-factory.h"
#include "ns3/node-container.h"

#include "ns3/leo-input-fstream-container.h"

/**
 * \file
 * \ingroup leo
 */

namespace ns3
{

/**
 * \ingroup leo
 * \brief Builds a node container with a waypoint mobility model
 *
 * Adds waypoints from file for each node.
 * The node satId must must correspond to the NORAD id from Celestrack.
 */
class LeoSatNodeHelper
{
public:
  /// constructor
  LeoSatNodeHelper ();
  /// destructor
  virtual ~LeoSatNodeHelper ();

  /**
   * \brief Install the nodes
   * \param nodeIds paths to satellite to waypoint files
   * \returns a node container containing nodes using the specified attributes
   */
  NodeContainer Install (std::vector<std::string> &wpFiles);

  /**
   * \brief Set an attribute for each node
   * \param name name of the attribute
   * \param value value of the attribute
   */
  void SetAttribute (std::string name, const AttributeValue &value);

private:
  /// Satellite nodes
  ObjectFactory m_satNodeFactory;
  /// Stream of waypoints
  LeoWaypointInputFileStreamContainer m_fileStreamContainer;
};

}; // namespace ns3

#endif
