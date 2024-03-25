/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2023 The IoD_Sim Authors.
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
#ifndef IRS_LIST_H
#define IRS_LIST_H

#include <ns3/ptr.h>

#include <vector>

namespace ns3
{

class Irs;
class CallbackBase;

/**
 * \ingroup Irs
 *
 * \brief the list of Irs.
 *
 * Every Irs created is automatically added to this list.
 */
class IrsList
{
  public:
    /// Irs container iterator
    typedef std::vector<Ptr<Irs>>::const_iterator Iterator;

    /**
     * \param irs drone to add
     * \returns index of the Irs in list
     *
     * This method must be called automatically in order to register
     * a node as a Irs
     */
    static uint32_t Add(Ptr<Irs> irs);

    /**
     * \returns a C++ iterator located at the beginning of this list
     */
    static Iterator Begin();
    static Iterator begin();

    /**
     * \returns a C++ iterator located at the end of this list
     */
    static Iterator End();
    static Iterator end();

    /**
     * \param n the index of requested Irs
     * \returns the Irs (Node) associated to index n
     */
    static Ptr<Irs> Get(uint32_t n);

    /**
     * \returns the number of Irs currently in the list
     */
    static uint32_t GetN();
};

} // namespace ns3

#endif /* IRS_LIST_H */
