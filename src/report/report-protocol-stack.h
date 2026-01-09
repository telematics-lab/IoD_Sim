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
#ifndef REPORT_PROTOCOL_STACK_H
#define REPORT_PROTOCOL_STACK_H
#include "protocol-layer.h"

#include <libxml/xmlwriter.h>
#include <vector>

namespace ns3
{

/**
 * \ingroup report
 *
 * \brief Report module for a Protocol Stack.
 *
 */
class ReportProtocolStack
{
  public:
    /// Protocol Layers iterator
    typedef std::vector<Ptr<ProtocolLayer>>::const_iterator Iterator;

    /**
     * Get an iterator which refers to the base Protocol Layer in the stack
     */
    Iterator Begin() const;

    /**
     * Get an iterator which indicates past-the-top Protocol Layer in the stack
     */
    Iterator End() const;

    /**
     * Get the number of Ptr<ProtocolLayer> stored in the stack
     */
    uint32_t GetN() const;

    /**
     * Get the Ptr<ProtocolLayer> stored in the stack at the given index
     */
    Ptr<ProtocolLayer> Get(const uint32_t i) const;

    /**
     * Place a single Ptr<ProtocolLayer> to the top of the stack
     */
    void Add(Ptr<ProtocolLayer> layer);

    /**
     * Write Protocol Stack report data to a XML file with a given handler
     */
    void Write(xmlTextWriterPtr handle);

  private:
    std::vector<Ptr<ProtocolLayer>> m_layers; /// the stack
};

} // namespace ns3

#endif /* REPORT_PROTOCOL_STACK_H */
