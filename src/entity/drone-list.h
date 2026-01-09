/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2026 The IoD_Sim Authors.
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
#ifndef DRONE_LIST_H
#define DRONE_LIST_H

#include <ns3/ptr.h>

#include <vector>

namespace ns3
{

class Drone;
class CallbackBase;

/**
 * \ingroup list
 *
 * \brief The list of simulated Drones
 *
 * Every drone created is manually added to this list.
 */
class DroneList
{
  public:
    /// Drone container iterator
    typedef std::vector<Ptr<Drone>>::const_iterator Iterator;

    /**
     * \param drone drone to add
     * \returns index of drone in list
     *
     * This method must be called automatically in order to register
     * a node as a Drone
     */
    static uint32_t Add(Ptr<Drone> drone);

    /**
     * \returns a C++ iterator located at the beginning of this list
     */
    static Iterator Begin();

    /**
     * \returns a C++ iterator located at the end of this list
     */
    static Iterator End();

    /**
     * \param n the index of requested Drone
     * \returns the drone (Node) associated to index n
     */
    static Ptr<Drone> GetDrone(uint32_t n);

    /**
     * \returns the number of drones currently in the list
     */
    static uint32_t GetNDrones();
};

} // namespace ns3

#endif /* DRONE_LIST_H */
