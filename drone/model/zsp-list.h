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
#ifndef ZSP_LIST_H
#define ZSP_LIST_H

#include <vector>
#include <ns3/ptr.h>

namespace ns3 {

class Node;
class CallbackBase;

/**
 * \ingroup iodsim
 *
 * \brief the list of simulated Zsps
 *
 * Every Zsp created is manually added to this list.
 */
class ZspList
{
public:
  /// Zsp container iterator
  typedef std::vector<Ptr<Node>>::const_iterator Iterator;

  /**
   * \param zsp Zsp to add
   * \returns index of Zsp in list
   *
   * This method must be called automatically in order to register
   * a node as a Zsp
   */
  static uint32_t Add (Ptr<Node> zsp);

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
   * \returns the Zsp (Node) associated to index n
   */
  static Ptr<Node> GetZsp (uint32_t n);

  /**
   * \returns the number of Zsps currently in the list
   */
  static uint32_t GetNZsps ();
};

} // namespace ns3

#endif /* ZSP_LIST_H */
