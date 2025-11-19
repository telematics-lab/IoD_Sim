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

#include "math.h"

#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/waypoint.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/mobility-helper.h"
#include "ns3/geographic-positions.h"
#include "leo-ground-node-helper.h"

using namespace std;

namespace ns3
{
NS_LOG_COMPONENT_DEFINE ("LeoGndNodeHelper");

LeoGndNodeHelper::LeoGndNodeHelper ()
{
  m_gndNodeFactory.SetTypeId ("ns3::Node");
}

LeoGndNodeHelper::~LeoGndNodeHelper ()
{
}

void
LeoGndNodeHelper::SetAttribute (string name, const AttributeValue &value)
{
  m_gndNodeFactory.Set (name, value);
}

NodeContainer
LeoGndNodeHelper::Install (const std::string &file)
{
  NS_LOG_FUNCTION (this << file);

  NodeContainer nodes;
  ifstream stream;
  stream.open (file, ifstream::in);
  LeoLatLong loc;
  while ((stream >> loc))
    {
      Ptr<ConstantPositionMobilityModel> mob = CreateObject<ConstantPositionMobilityModel> ();
      Vector pos = GetEarthPosition (loc);
      mob->SetPosition (pos);
      Ptr<Node> node = m_gndNodeFactory.Create<Node> ();
      node->AggregateObject (mob);
      nodes.Add (node);
      NS_LOG_INFO ("Added ground node at " << pos);
    }
  stream.close ();

  NS_LOG_INFO ("Added " << nodes.GetN () << " ground nodes");

  return nodes;
}

NodeContainer
LeoGndNodeHelper::Install (uint32_t latNodes, uint32_t lonNodes)
{
  NodeContainer nodes;
  for (uint64_t i = 0; i < lonNodes * latNodes; i++)
    {
      nodes.Add (m_gndNodeFactory.Create<Node> ());
    }
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator ("ns3::LeoPolarPositionAllocator",
  				 "LatNum", UintegerValue (latNodes),
  				 "LonNum", UintegerValue (lonNodes));

  mobility.Install (nodes);

  return nodes;
}

Vector3D
LeoGndNodeHelper::GetEarthPosition (const LeoLatLong &loc)
{
  double lat = loc.latitude * (M_PI / 90);
  double lon = loc.longitude * (M_PI / 180);
  Vector3D pos = Vector3D (GeographicPositions::EARTH_SPHERE_RADIUS * sin (lat) * cos (lon),
  			   GeographicPositions::EARTH_SPHERE_RADIUS * sin (lat) * sin (lon),
  			   GeographicPositions::EARTH_SPHERE_RADIUS * cos (lat));
  return pos;
}


NodeContainer
LeoGndNodeHelper::Install (const LeoLatLong &location)
{
  NS_LOG_FUNCTION (this << location);

  NodeContainer nodes;
  Vector pos = GetEarthPosition (location);
  Ptr<ConstantPositionMobilityModel> mob = CreateObject<ConstantPositionMobilityModel> ();
  mob->SetPosition (pos);
  Ptr<Node> node = m_gndNodeFactory.Create<Node> ();
  node->AggregateObject (mob);
  nodes.Add (node);
  NS_LOG_INFO ("Added ground node at " << pos);

  return nodes;
}

NodeContainer
LeoGndNodeHelper::Install (const LeoLatLong &location1,
  	 		   const LeoLatLong &location2)
{
  NS_LOG_FUNCTION (this << location1 << location2);

  NodeContainer nodes;

  for (const LeoLatLong& loc : { location1, location2 }) {
    nodes.Add(this->Install(loc));
  }

  NS_LOG_INFO ("Added " << nodes.GetN () << " ground nodes");

  return nodes;
}



}; // namespace ns3
