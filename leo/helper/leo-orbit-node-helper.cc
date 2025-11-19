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

#include "leo-orbit-node-helper.h"

#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/integer.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/waypoint.h"

#include <fstream>
#include <vector>

using namespace std;

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("LeoOrbitNodeHelper");

LeoOrbitNodeHelper::LeoOrbitNodeHelper()
{
    m_nodeFactory.SetTypeId("ns3::Node");
}

LeoOrbitNodeHelper::~LeoOrbitNodeHelper()
{
}

void
LeoOrbitNodeHelper::SetAttribute(string name, const AttributeValue& value)
{
    m_nodeFactory.Set(name, value);
}

NodeContainer
LeoOrbitNodeHelper::Install(const LeoOrbit& orbit)
{
    NS_LOG_FUNCTION(this << orbit);

    NodeContainer c;
    c.Create(orbit.sats * orbit.planes);

    // Calculate spacing between orbital planes and satellites
    double longitudeSpacing = 360.0 / orbit.planes; // degrees
    double offsetSpacing = 360.0 / orbit.sats;      // degrees

    uint32_t nodeIndex = 0;
    for (uint32_t plane = 0; plane < orbit.planes; plane++)
    {
        // Calculate longitude for this orbital plane
        double longitude = plane * longitudeSpacing;

        // Normalize longitude to [-180, 180] range as required by GeoLeoOrbitMobility
        if (longitude > 180.0)
        {
            longitude -= 360.0;
        }

        for (uint32_t sat = 0; sat < orbit.sats; sat++)
        {
            // Calculate offset for this satellite within the plane
            double offset = sat * offsetSpacing;

            // Use MobilityHelper to configure each satellite individually
            MobilityHelper mobility;
            mobility.SetMobilityModel("ns3::GeoLeoOrbitMobility",
                                      "Precision",
                                      TimeValue(m_precision),
                                      "Altitude",
                                      DoubleValue(orbit.alt),
                                      "Inclination",
                                      DoubleValue(orbit.inc),
                                      "Longitude",
                                      DoubleValue(longitude),
                                      "Offset",
                                      DoubleValue(offset),
                                      "RetrogradeOrbit",
                                      BooleanValue(false),
                                      "EarthSpheroidType",
                                      EnumValue(GeographicPositions::EarthSpheroidType::SPHERE));

            // Install mobility on this specific node
            NodeContainer singleNode;
            singleNode.Add(c.Get(nodeIndex));
            mobility.Install(singleNode);

            nodeIndex++;
        }
    }

    return c;
}

NodeContainer
LeoOrbitNodeHelper::Install(const double& altitude,
                            const double& inclination,
                            const double& longitude,
                            const double& offset,
                            const bool& retrograde)
{
    NS_LOG_FUNCTION(this << altitude << inclination << longitude << offset);

    NodeContainer c;
    c.Create(1);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::GeoLeoOrbitMobility",
                              "Precision",
                              TimeValue(m_precision),
                              "Altitude",
                              DoubleValue(altitude),
                              "Inclination",
                              DoubleValue(inclination),
                              "Longitude",
                              DoubleValue(longitude),
                              "Offset",
                              DoubleValue(offset),
                              "RetrogradeOrbit",
                              BooleanValue(retrograde),
                              "EarthSpheroidType",
                              EnumValue(GeographicPositions::EarthSpheroidType::SPHERE));

    mobility.Install(c);
    return c;
}

Time
LeoOrbitNodeHelper::GetPrecision() const
{
    return m_precision;
}

void
LeoOrbitNodeHelper::SetPrecision(Time precision)
{
    m_precision = precision;
}

NodeContainer
LeoOrbitNodeHelper::Install(const std::string& orbitFile)
{
    NS_LOG_FUNCTION(this << orbitFile);

    NodeContainer nodes;
    ifstream orbits;
    orbits.open(orbitFile, ifstream::in);
    LeoOrbit orbit;
    while ((orbits >> orbit))
    {
        nodes.Add(Install(orbit));
        NS_LOG_DEBUG("Added orbit plane");
    }
    orbits.close();

    NS_LOG_DEBUG("Added " << nodes.GetN() << " nodes");

    return nodes;
}

NodeContainer
LeoOrbitNodeHelper::Install(const vector<LeoOrbit>& orbits)
{
    NS_LOG_FUNCTION(this << orbits);

    NodeContainer nodes;
    for (uint64_t i = 0; i < orbits.size(); i++)
    {
        nodes.Add(Install(orbits[i]));
        NS_LOG_DEBUG("Added orbit plane");
    }

    NS_LOG_DEBUG("Added " << nodes.GetN() << " nodes");

    return nodes;
}

}; // namespace ns3
