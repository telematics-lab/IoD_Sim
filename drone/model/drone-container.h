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
 */
#ifndef DRONE_CONTAINER_H
#define DRONE_CONTAINER_H

#include <ns3/node-container.h>
#include "drone.h"

namespace ns3 {

/**
 * \brief Keeps track of a set of drones pointers.
 */
class DroneContainer : public NodeContainer
{
public:
  /// Drone container iterator
  typedef std::vector<Ptr<Drone>>::const_iterator Iterator;

  /**
   * \brief Create n drones and append pointers to them to the end of this
   * DroneContainer.
   *
   * \param n The number of Drones to create
   */
  void Create (uint32_t n);

  /**
   * \brief Get an iterator which refers to the first Drone in the
   * container.
   *
   * \returns an iterator which refers to the first Drone in the container.
   */
  Iterator Begin (void) const;

  /**
   * \brief Get an iterator which indicates past-the-last Drone in the
   * container.
   *
   * \returns an iterator which indicates an ending condition for a loop.
   */
  Iterator End (void) const;

  /**
   * \brief Get the number of Ptr<Drone> stored in this container.
   *
   * \returns the number of Ptr<Drone> stored in this container.
   */
  uint32_t GetN (void) const;

  /**
   * \brief Get the Ptr<Drone> stored in this container at a given
   * index.
   *
   * \param i the index of the requested drone pointer.
   * \returns the requested drone pointer.
   */
  Ptr<Drone> Get (uint32_t i) const;

private:
  std::vector<Ptr<Drone>> m_drones; //!< Drones smart pointers
};

} // namespace ns3

#endif /* DRONE_CONTAINER_H */
