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
#ifndef TCP_STORAGE_CLIENT_APPLICATION
#define TCP_STORAGE_CLIENT_APPLICATION

#include <ns3/storage-peripheral.h>

#include "tcp-client-server-application.h"

namespace ns3 {

/**
 * TCP client that transmits data to a remote server to free as much memory as possible on the storage peripheral
 * attached to the same node.
 */
class TcpStorageClientApplication : public TcpClientServerApplication
{
public:
  static TypeId GetTypeId ();
  TcpStorageClientApplication ();
  virtual ~TcpStorageClientApplication ();

protected:
  virtual void DoInitialize ();
  virtual void StartApplication ();
  /// \brief Send a random packet of a given size.
  virtual bool DoSendPacket (const uint16_t payloadSize);

  const uint16_t GetPayloadSize ();

private:
  /// \brief Send a packet and, if successful, free storage memory.
  void SendPacket (const uint16_t payloadSize);
  /// \brief Create the payload to be sent.
  Ptr<Packet> CreatePacket (uint32_t size) const;
  /// \brief Find storage in drone.
  Ptr<StoragePeripheral> FindStorage () const;
  /// \brief Callback when the underlying drone storage receives data updates.
  void StorageUpdateCallback (const uint64_t oldvalue, const uint64_t newvalue);

  uint16_t m_payloadSize;           /// Payload size in bytes.
  uint32_t m_seqNum;                /// Packet Sequence Number.
  Ptr<StoragePeripheral> m_storage; /// Reference to drone storage peripheral.

  /// Trace to signal the transmission of packets from application-level.
  TracedCallback<Ptr<const Packet> > m_txTrace;
};

} // namespace ns3

#endif /* TCP_STORAGE_CLIENT_APPLICATION */
