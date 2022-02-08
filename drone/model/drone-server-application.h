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
 */
#ifndef DRONE_SERVER_H
#define DRONE_SERVER_H

#include <ns3/application.h>
#include <ns3/socket.h>
#include <ns3/stats-module.h>

namespace ns3 {

enum
ServerState
  {
   SERVER_CLOSED,
   SERVER_LISTEN
  };

class DroneServerApplication : public Application
{
public:
  static TypeId GetTypeId ();

  DroneServerApplication ();
  virtual ~DroneServerApplication ();

protected:
  virtual void DoDispose ();

private:
  virtual void StartApplication ();
  virtual void StopApplication ();

  void ReceivePacket (const Ptr<Socket> socket) const;
  void SendHelloAck (const Ptr<Socket> socket,
                     const Ipv4Address senderAddr,
                     const uint32_t senderPort) const;
  void SendUpdateAck (const Ptr<Socket> socket,
                      const Ipv4Address senderAddr,
                      const uint32_t senderPort) const;
  void SendUpdateBroadcast () const;

  Ptr<Socket> m_socket;
  Ipv4Address m_address;
  Ipv4Mask    m_subnetMask;
  uint32_t    m_port;
  ServerState m_state;

  mutable EventId m_sendEvent;
  TracedCallback<Ptr<const Packet>> m_txTrace;

  mutable int32_t m_sequenceNumber; // correlated with the node, not the connection
  bool m_storage = false;
};

} // namespace ns3

#endif /* DRONE_SERVER_H */
