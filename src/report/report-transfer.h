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
 * Authors: Giovanni Grieco <giovanni.grieco@poliba.it>
 */
#ifndef REPORT_TRANSFER_H
#define REPORT_TRANSFER_H
#include "transfer-direction.h"

#include <ns3/drone-communications.h>
#include <ns3/nstime.h>
#include <ns3/object.h>

#include <cstddef>
#include <libxml/xmlwriter.h>
#include <string>

namespace ns3
{

class ReportTransfer : public Object
{
  public:
    /**
     * Register the type using ns-3 TypeId System.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * Default constructor
     */
    ReportTransfer();
    /**
     * Default destructor
     */
    ~ReportTransfer();

    int32_t GetIface();
    /**
     * Write Transfer report data to a XML file with a given handler
     *
     * \param handle the XML handler to write data on
     */
    void Write(xmlTextWriterPtr handle);

  private:
    /// entityid
    uint32_t m_entityid;
    /// netdevice id
    int32_t m_iface;
    /// the type of transfer
    PacketType m_type;
    /// the direction related to the monitored entity (Tx or Rx?)
    TransferDirection m_direction;
    /// time of transfer
    Time m_time;
    /// the sender of the packet
    std::string m_sourceAddress;
    /// the receiver of the packet
    std::string m_destinationAddress;
    /// the length of the payload
    uint32_t m_length;
    /// the sequence number of DCL payload
    uint32_t m_sequenceNumber;
    /// the full JSON, DCL payload, decoded in ASCII
    std::string m_payload;
};

} // namespace ns3

#endif /* REPORT_TRANSFER_H */
