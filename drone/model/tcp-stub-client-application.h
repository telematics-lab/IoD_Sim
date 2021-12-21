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
 */
#ifndef TCP_STUB_CLIENT_APPLICATION_H
#define TCP_STUB_CLIENT_APPLICATION_H

#include <ns3/application.h>
#include <ns3/socket.h>
#include <ns3/storage-peripheral.h>
#include <ns3/traced-callback.h>

namespace ns3 {

/**
 * Application that sends random packets with Seq.Num. and Timestamp to a remote server
 */
class TcpStubClientApplication : public Application
{
public:
  /**
   * \brief Register this application as a type in ns-3 TypeId System
   */
  static TypeId GetTypeId ();
  /**
   * \brief Default constructor
   */
  TcpStubClientApplication ();
  /**
   * \brief Default destructor
   */
  virtual ~TcpStubClientApplication ();

protected:
  /**
   * \brief Initialize the Object registered as TypeId
   */
  virtual void DoInitialize ();

private:
  /**
   * \brief Start endpoint to conform with the Application Interface
   */
  virtual void StartApplication ();
  /**
   * \brief Stop endpoint to conform with the Application Interface
   */
  virtual void StopApplication ();

  /**
   * \brief Connection succeeded callback
   */
  void OnConnectionSucceeded (Ptr<Socket> s);
  /**
   * \brief Connection failed callback
   */
  void OnConnectionFailed (Ptr<Socket> s);
  /**
   * \brief Connection closed normally
   */
  void OnNormalClose (Ptr<Socket> s);
  /**
   * \brief Connection closed with errors
   */
  void OnErrorClose (Ptr<Socket> s);
  /**
   * \brief Received Data
   */
  void OnReceivedData (Ptr<Socket> s);

  /**
   * \brief Send a random packet of a given size
   */
  bool SendPacket (const uint16_t payloadSize);
  /**
   * \brief Create the payload to be sent
   */
  Ptr<Packet> CreatePacket (uint32_t size) const;
  /**
   * \brief Find storage in drone
   */
  Ptr<StoragePeripheral> FindStorage () const;
  /**
   * \brief Callback when the underlying drone storage receives data updates
   */
  void OnStorageUpdate (const uint64_t oldvalue, const uint64_t newvalue);


  Ipv4Address m_destAddr;      /// Remote Server Address
  uint16_t m_destPort;         /// UDP Port
  uint16_t m_maxPayloadSize;   /// Payload size in bytes
  /// Trace new connections being made
  TracedCallback<Ptr<const TcpStubClientApplication>, Ptr<Socket> > m_connectionEstablishedTrace;
  /// Trace to signal the transmission of packets from application-level
  TracedCallback<Ptr<const Packet> > m_txTrace;

  Ptr<Socket> m_socket;        /// The socket to be used for communications
  uint32_t m_seqNum;           /// Packet Sequence Number
  uint16_t m_payloadSize;      /// Payload size, excluding L3,4 header sizes
  Ptr<StoragePeripheral> m_storage; /// Reference to drone storage peripheral.
};

} // namespace ns3

#endif /* TCP_STUB_CLIENT_APPLICATION_H */
