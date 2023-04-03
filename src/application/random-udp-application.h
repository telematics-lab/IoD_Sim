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
#ifndef RANDOM_UDP_APPLICATION_H
#define RANDOM_UDP_APPLICATION_H

#include <ns3/application.h>
#include <ns3/socket.h>
#include <ns3/traced-callback.h>

namespace ns3
{

/**
 * Application that sends random packets with Seq.Num. and Timestamp to a remote server
 */
class RandomUdpApplication : public Application
{
  public:
    /**
     * \brief Register this application as a type in ns-3 TypeId System
     */
    static TypeId GetTypeId();
    /**
     * \brief Default constructor
     */
    RandomUdpApplication();
    /**
     * \brief Default destructor
     */
    virtual ~RandomUdpApplication();

  protected:
    /**
     * \brief Initialize the Object registered as TypeId
     */
    virtual void DoInitialize();

  private:
    /**
     * \brief Start endpoint to conform with the Application Interface
     */
    virtual void StartApplication();
    /**
     * \brief Stop endpoint to conform with the Application Interface
     */
    virtual void StopApplication();

    /**
     * \brief Send a random packet
     */
    void SendPacket();
    /**
     * \brief Create the payload to be sent
     */
    Ptr<Packet> CreatePacket(uint32_t size) const;

    Ipv4Address
        m_destAddr; /// Remote Server Address - FIXME: Ipv6Address needs an additional parameter
    uint16_t m_destPort; /// UDP Port
    uint32_t m_txFreq;   /// Transmission frequency
    uint16_t
        m_packetSize; /// Packet size in bytes - 16 bits are given as limit due to UDP header field
    TracedCallback<Ptr<const Packet>>
        m_txTrace; /// Trace to signal the transmission of packets from application-level

    EventId m_sendEvent;     /// Event of a packet being sent
    Ptr<Socket> m_socket;    /// The socket to be used for communications
    uint32_t m_seqNum;       /// Packet Sequence Number
    uint16_t m_payloadSize;  /// Payload size, excluding L3,4 header sizes
    double m_packetInterval; /// Packet Tx interval, the inverse of m_txFreq, in Seconds
};

} // namespace ns3

#endif /* RANDOM_UDP_APPLICATION_H */
