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

#ifndef LEO_ORBIT_NODE_HELPER_H
#define LEO_ORBIT_NODE_HELPER_H

#include "ns3/leo-orbit.h"
#include "ns3/node-container.h"
#include "ns3/nstime.h"
#include "ns3/object-factory.h"

#include <string>

/**
 * \file
 * \ingroup leo
 */

namespace ns3
{

/**
 * \ingroup leo
 * \brief Builds a node container of nodes with LEO positions using a list of
 * orbit definitions.
 *
 * Adds orbits with from a file for each node.
 */
class LeoOrbitNodeHelper
{
  public:
    /// constructor
    LeoOrbitNodeHelper();

    /// destructor
    virtual ~LeoOrbitNodeHelper();

    /**
     *
     * \param orbitFile path to orbit definitions file
     * \returns a node container containing nodes using the specified attributes
     */
    NodeContainer Install(const std::string& orbitFile);

    /**
     *
     * \param orbits orbit definitions
     * \returns a node container containing nodes using the specified attributes
     */
    NodeContainer Install(const std::vector<LeoOrbit>& orbits);

    /**
     *
     * \param orbit orbit definition
     * \returns a node container containing nodes using the specified attributes
     */
    NodeContainer Install(const LeoOrbit& orbit);

    NodeContainer Install(const double& altitude,
                          const double& inclination,
                          const double& longitude,
                          const double& offset,
                          const bool& retrograde = false);

    /**
     * Set an attribute for each node
     *
     * \param name name of the attribute
     * \param value value of the attribute
     */
    void SetAttribute(std::string name, const AttributeValue& value);

    Time GetPrecision() const;

    void SetPrecision(Time precision);

  private:
    /// Factory for nodes
    ObjectFactory m_nodeFactory;
    // Precision of the position allocator
    Time m_precision = Seconds(1.0); // Default precision
};

}; // namespace ns3

#endif
