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

#ifndef LEO_INPUT_FSTREAM_CONTAINER
#define LEO_INPUT_FSTREAM_CONTAINER

#include <fstream>
#include "ns3/object.h"
#include "ns3/waypoint.h"

/**
 * \file
 * \ingroup leo
 * Declares LeoWaypointInputFileStreamContainer
 */

using namespace std;

namespace ns3
{

/**
 * \ingroup leo
 * \brief Wrapper around a stream of Waypoint
 */
class LeoWaypointInputFileStreamContainer : public Object
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /// constructor
  LeoWaypointInputFileStreamContainer ();
  /// destructor
  virtual ~LeoWaypointInputFileStreamContainer ();

  /**
   * constructor
   *
   * \param filePath path to waypoints file
   * \param lastTime the time until which to read waypoints
   */
  LeoWaypointInputFileStreamContainer (string filePath, Time lastTime);

  /**
   * \brief Get next waypoint
   * \param [out] next waypoint
   * \return true iff there are more waypoints
   */
  bool GetNextSample (Waypoint &sample);

  /**
   * \brief Set the path to the waypoint file
   * \param path path to the waypoint file
   */
  void SetFile (const string path);

  /**
   * \brief Get the path to the waypoint file
   * \return path to the waypoint file
   */
  string GetFile () const;

  /**
   * \brief Set the path to the last time slot
   * \param lastTime last time slot
   */
  void SetLastTime (const Time lastTime);

  /**
   * \brief Get the last time slot
   * \return last time slot
   */
  Time GetLastTime () const;

private:
  /// Path to the waypoints file
  string m_filePath;

  /// Time of the last timestamp
  Time m_lastTime;

  /// Waypoint file stream
  ifstream m_input;

};

};

#endif
