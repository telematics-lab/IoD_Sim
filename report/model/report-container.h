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
 * Authors: Giovanni Grieco <giovanni.grieco@poliba.it>
 */
#ifndef REPORT_CONTAINER_H
#define REPORT_CONTAINER_H

#include <string>
#include <vector>

#include <ns3/ptr.h>

#include <libxml/xmlwriter.h>

#include "report-drone.h"
#include "report-zsp.h"
#include "report-remote.h"

namespace ns3 {

/**
 * A container to easily keep pointers
 */
template<class ReportType>
class ReportContainer
{
public:
  /// Container iterator
  typedef typename std::vector<Ptr<ReportType>>::const_iterator Iterator;

  /**
   * Create an empty ReportContainer
   *
   * \param groupName name of entities group
   */
  ReportContainer (const std::string groupName);

  /**
   * Get an iterator which refers to the first entity in the container
   */
  Iterator Begin () const;

  /**
   * Get an iterator which indicates past-the-last entity in the container
   */
  Iterator End () const;

  /**
   * Get the number of Ptr<ReportType> stored in this container
   */
  uint32_t GetN () const;

  /**
   * Get the Ptr<ReportType> stored in this container at a given index
   */
  Ptr<ReportType> Get (const uint32_t i) const;

  /**
   * Append a single Ptr<ReportType> to this container
   */
  void Add (Ptr<ReportType> entity);

  /**
   * Append a new Ptr<ReportType> to this container initializing with
   * an entity UID.
   *
   * \param entityUid The entity unique identifier
   */
  void Add (uint32_t entityUid);

  /**
   * Write entity report data to a XML file with a given handler
   *
   * \param handle the handler to communicate data to the opened XML file
   */
  void Write (xmlTextWriterPtr handle) const;
private:
  const std::string m_groupName;           /// name of entities group
  std::vector<Ptr<ReportType>> m_entities; /// smart pointers
};

/*
 * Structures that can be embedded in a ReportContainer
 */
template class ReportContainer<ReportDrone>;
template class ReportContainer<ReportZsp>;
template class ReportContainer<ReportRemote>;

} // namespace ns3

#endif /* REPORT_CONTAINER_H */
