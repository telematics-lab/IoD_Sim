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

#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/waypoint-mobility-model.h"

#include "satellite-node-helper.h"

using namespace std;

namespace ns3
{
NS_LOG_COMPONENT_DEFINE ("LeoSatNodeHelper");

LeoSatNodeHelper::LeoSatNodeHelper ()
{
  m_satNodeFactory.SetTypeId ("ns3::Node");
}

LeoSatNodeHelper::~LeoSatNodeHelper ()
{
}

void
LeoSatNodeHelper::SetAttribute (string name, const AttributeValue &value)
{
  m_satNodeFactory.Set (name, value);
}

NodeContainer
LeoSatNodeHelper::Install (vector<string> &wpFiles)
{
  NS_LOG_FUNCTION (this);

  NodeContainer nodes;
  for (size_t i = 0; i < wpFiles.size (); i ++)
    {
      Ptr<WaypointMobilityModel> mob = CreateObject<WaypointMobilityModel> ();
      string fileName = wpFiles[i];
      m_fileStreamContainer.SetFile (fileName);
      Waypoint wp;
      while (m_fileStreamContainer.GetNextSample (wp))
      	{
      	  mob->AddWaypoint (wp);
          NS_LOG_DEBUG ("Added waypoint " << wp);
      	}
      Ptr<Node> node = m_satNodeFactory.Create<Node> ();
      node->AggregateObject (mob);

      nodes.Add (node);
      NS_LOG_INFO ("Added satellite node " << node->GetId ());
    }

  return nodes;
}

}; // namespace ns3
