/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2024 The IoD_Sim Authors.
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
 * Authors: Domingo Dirutigliano <me@domy.sh>
 */
#ifndef LEO_SAT_LIST_H
#define LEO_SAT_LIST_H

#include <ns3/ptr.h>

#include <vector>

namespace ns3
{

class Node;
class CallbackBase;

/**
 * \ingroup iodsim
 *
 * \brief the list of simulated LeoSats
 *
 * Every LeoSat created is manually added to this list.
 */
class LeoSatList
{
  public:
    /// LeoSat container iterator
    typedef std::vector<Ptr<Node>>::const_iterator Iterator;

    /**
     * \param leoSat LeoSat to add
     * \returns index of LeoSat in list
     *
     * This method must be called automatically in order to register
     * a node as a LeoSat
     */
    static uint32_t Add(Ptr<Node> leoSat);

    /**
     * \returns a C++ iterator located at the beginning of this list
     */
    static Iterator Begin();

    /**
     * \returns a C++ iterator located at the end of this list
     */
    static Iterator End();

    /**
     * \param n the index of requested node
     * \returns the LeoSat (Node) associated to index n
     */
    static Ptr<Node> GetLeoSat(uint32_t n);

    /**
     * \returns the number of LeoSats currently in the list
     */
    static uint32_t GetNLeoSats();
};

} // namespace ns3

#endif /* LEO_SAT_LIST_H */
