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
 *
 * Authors: Giovanni Iacovelli <giovanni.iacovelli@poliba.it>
 */
#include "report-world.h"
#include <ns3/building.h>
#include <ns3/building-list.h>

#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ReportWorld");

ReportWorld::ReportWorld ()
{
  NS_LOG_FUNCTION (this);
}

ReportWorld::~ReportWorld ()
{
  NS_LOG_FUNCTION (this);
}

void
ReportWorld::Write (xmlTextWriterPtr h) const
{
  std::stringstream roistr;
  NS_LOG_FUNCTION (h);
  if (h == nullptr)
    {
      NS_LOG_WARN ("Passed handler is not valid: " << h << ". "
                   "Data will be discarded.");
      return;
    }

  int rc;

  rc = xmlTextWriterStartElement(h, BAD_CAST "World");
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterStartElement(h, BAD_CAST "Buildings");
  NS_ASSERT (rc >= 0);

  for (auto bit = BuildingList::Begin (); bit != BuildingList::End (); ++bit)
  {
    rc = xmlTextWriterStartElement(h, BAD_CAST "Building");
    NS_ASSERT (rc >= 0);

    xmlTextWriterWriteAttribute (h, BAD_CAST "id", BAD_CAST std::to_string((*bit)->GetId ()).c_str ());

    rc = xmlTextWriterWriteElement (h, BAD_CAST "Boundaries", BAD_CAST
    ("["+std::to_string((*bit)->GetBoundaries ().xMin)+", "
    + std::to_string((*bit)->GetBoundaries ().xMax)+", "
    +std::to_string((*bit)->GetBoundaries ().yMin)+", "
    + std::to_string((*bit)->GetBoundaries ().yMax)+", "
    +std::to_string((*bit)->GetBoundaries ().zMin)+", "
    + std::to_string((*bit)->GetBoundaries ().zMax)+"]").c_str ());
    NS_ASSERT (rc >= 0);

    rc = xmlTextWriterWriteElement (h, BAD_CAST "BuildingType", BAD_CAST std::to_string((*bit)->GetBuildingType ()).c_str ());
    NS_ASSERT (rc >= 0);

    rc = xmlTextWriterWriteElement (h, BAD_CAST "WallsType", BAD_CAST std::to_string((*bit)->GetExtWallsType ()).c_str ());
    NS_ASSERT (rc >= 0);

    rc = xmlTextWriterWriteElement (h, BAD_CAST "Floors", BAD_CAST std::to_string((*bit)->GetNFloors ()).c_str ());
    NS_ASSERT (rc >= 0);

    rc = xmlTextWriterWriteElement (h, BAD_CAST "Rooms", BAD_CAST
    ("["+std::to_string((*bit)->GetNRoomsX ())+","+ std::to_string((*bit)->GetNRoomsX())+"]").c_str ());
    NS_ASSERT (rc >= 0);

    rc = xmlTextWriterEndElement(h);
    NS_ASSERT (rc >= 0);
  }

  rc = xmlTextWriterEndElement(h);
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterStartElement(h, BAD_CAST "InterestRegions");
  NS_ASSERT (rc >= 0);

  for (auto ir = irc->Begin (); ir != irc->End (); ir++)
  {
    xmlTextWriterWriteAttribute (h, BAD_CAST "id", BAD_CAST std::to_string(ir-irc->Begin ()).c_str ());

    std::string irstr = "";
    DoubleVector coords = (*ir)->GetCoordinates ();

    for (uint32_t c = 0; c < coords.GetN(); c++) irstr+=std::to_string(coords.Get()[c])+", ";
    irstr.pop_back();
    irstr.pop_back();

    rc = xmlTextWriterWriteElement (h, BAD_CAST "Boundaries", BAD_CAST ("["+irstr+"]").c_str ());
    NS_ASSERT (rc >= 0);
  }

  rc = xmlTextWriterEndElement(h);
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterEndElement(h);
  NS_ASSERT (rc >= 0);
}

} // namespace ns3
