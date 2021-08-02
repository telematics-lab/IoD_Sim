/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2021 The IoD_Sim Authors.
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
#ifndef REMOTE_LIST_H
#define REMOTE_LIST_H

#include <vector>
#include <ns3/ptr.h>

namespace ns3 {

class Node;
class CallbackBase;

/**
 * \ingroup iodsim
 *
 * \brief the list of simulated Remotes
 *
 * Every Remote created is manually added to this list.
 */
class RemoteList
{
public:
  /// Remote container iterator
  typedef std::vector<Ptr<Node>>::const_iterator Iterator;

  /**
   * \param remote Remote to add
   * \returns index of Remote in list
   *
   * This method must be called automatically in order to register
   * a node as a Remote
   */
  static uint32_t Add (Ptr<Node> remote);

  /**
   * \returns a C++ iterator located at the beginning of this list
   */
  static Iterator Begin ();

  /**
   * \returns a C++ iterator located at the end of this list
   */
  static Iterator End ();

  /**
   * \param n the index of requested node
   * \returns the Remote (Node) associated to index n
   */
  static Ptr<Node> GetRemote (uint32_t n);

  /**
   * \returns the number of Remotes currently in the list
   */
  static uint32_t GetNRemotes ();
};

} // namespace ns3

#endif /* REMOTE_LIST_H */
