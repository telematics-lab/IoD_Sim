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

#include <fstream>
#include <vector>

#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/waypoint.h"
#include "ns3/mobility-helper.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/integer.h"

#include "leo-orbit-node-helper.h"

using namespace std;

namespace ns3
{
NS_LOG_COMPONENT_DEFINE ("LeoOrbitNodeHelper");

LeoOrbitNodeHelper::LeoOrbitNodeHelper ()
{
  m_nodeFactory.SetTypeId ("ns3::Node");
}

LeoOrbitNodeHelper::~LeoOrbitNodeHelper ()
{
}

void
LeoOrbitNodeHelper::SetAttribute (string name, const AttributeValue &value)
{
  m_nodeFactory.Set (name, value);
}

NodeContainer
LeoOrbitNodeHelper::Install (const LeoOrbit &orbit)
{
  NS_LOG_FUNCTION (this << orbit);

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::LeoCircularOrbitPostionAllocator",
                                 "NumOrbits", IntegerValue (orbit.planes),
                                 "NumSatellites", IntegerValue (orbit.sats));
  mobility.SetMobilityModel ("ns3::LeoCircularOrbitMobilityModel",
             "Precision", TimeValue (m_precision),
  			     "Altitude", DoubleValue (orbit.alt),
  			     "Inclination", DoubleValue (orbit.inc));

  NodeContainer c;
  c.Create (orbit.sats*orbit.planes);
  mobility.Install (c);

  return c;
}

NodeContainer
LeoOrbitNodeHelper::Install (const double &altitude, const double &inclination,
      const double &longitude, const double &offset, const bool &retrograde){
  NS_LOG_FUNCTION (this << altitude << inclination << longitude << offset);


  Ptr<LeoCircularOrbitMobilityModel> mobilityModel = CreateObject<LeoCircularOrbitMobilityModel>();
  mobilityModel->SetAttribute("Precision", TimeValue (m_precision));
  mobilityModel->SetAttribute("Altitude", DoubleValue(altitude));
  mobilityModel->SetAttribute("Inclination", DoubleValue (inclination));
  mobilityModel->SetAttribute("Longitude", DoubleValue (longitude));
  mobilityModel->SetAttribute("Offset", DoubleValue (offset));
  mobilityModel->SetAttribute("RetrogradeOrbit", BooleanValue (retrograde));

  NodeContainer c;
  c.Create (1);
  c.Get(0)->AggregateObject(mobilityModel);
  return c;
}

Time LeoOrbitNodeHelper::GetPrecision () const
{
  return m_precision;
}

void LeoOrbitNodeHelper::SetPrecision (Time precision)
{
  m_precision = precision;
}

NodeContainer
LeoOrbitNodeHelper::Install (const std::string &orbitFile)
{
  NS_LOG_FUNCTION (this << orbitFile);

  NodeContainer nodes;
  ifstream orbits;
  orbits.open (orbitFile, ifstream::in);
  LeoOrbit orbit;
  while ((orbits >> orbit))
    {
      nodes.Add (Install (orbit));
      NS_LOG_DEBUG ("Added orbit plane");
    }
  orbits.close ();

  NS_LOG_DEBUG ("Added " << nodes.GetN () << " nodes");

  return nodes;
}

NodeContainer
LeoOrbitNodeHelper::Install (const vector<LeoOrbit> &orbits)
{
  NS_LOG_FUNCTION (this << orbits);

  NodeContainer nodes;
  for (uint64_t i = 0; i < orbits.size(); i++)
    {
      nodes.Add (Install (orbits[i]));
      NS_LOG_DEBUG ("Added orbit plane");
    }

  NS_LOG_DEBUG ("Added " << nodes.GetN () << " nodes");

  return nodes;
}

}; // namespace ns3
