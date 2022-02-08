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
#ifndef DRONE_CLIENT_APPLICATION_H
#define DRONE_CLIENT_APPLICATION_H

#include <ns3/application.h>
#include <ns3/socket.h>
#include <ns3/stats-module.h>
#include <ns3/mobility-module.h>

namespace ns3 {

/**
 * \brief the client states
 */
enum
ClientState
  {
   CLOSED,     /// the client has no sockets open
   HELLO_SENT, /// the client has sent a HELLO packet in broadcast
   CONNECTED   /// the client has received a HELLO_ACK packet from a ZSP
  };

/**
 * \brief the intent of a packet
 */
enum
Intent
  {
   NEW,  /// this packet contains new information
   ACK   /// this packet represents an acknowledgement of a previous one
  };

/**
 * \brief Helper function to translate an Intent to a string, useful
 *        for debugging purposes and serialization.
 */
inline const std::string
ToString (Intent i)
{
  switch (i)
    {
    case NEW:
      return std::string("NEW");
    case ACK:
      return std::string("ACK");
    }

  return std::string("UNKNOWN");
}

/**
 * Application to be installed on each drone that wants to participate in the
 * IoD inter-network using a simple JSON via UDP with ACK protocol.
 */
class DroneClientApplication : public Application
{
public:
  /**
   * \brief Register this application as a type in ns-3 TypeId System.
   */
  static TypeId GetTypeId (void);

  /**
   * \brief default constructor
   */
  DroneClientApplication ();
  /**
   * \brief default destructor
   */
  virtual ~DroneClientApplication ();

protected:
  /**
   * \brief Destructor implementation for the ns-3 TypeId System.
   */
  virtual void DoDispose ();

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
   * \brief Send a packet to a given address in the IoD.
   *
   * \param i the intent of the packet to be sent.
   * \param s the socket to be used.
   * \param a the target address.
   */
  void SendPacket (const Intent i,
                   const Ptr<Socket> s,
                   const Ipv4Address a) const;

  /**
   * \brief Callback to detect a new packet arrival.
   *
   * \param s the socket to be listened.
   */
  void ReceivePacket (const Ptr<Socket> s);

  /**
   * \brief Callback to detect a variation in the mobility model and get
   *        new position and velocity vectors.
   *
   * \param context the context of the simulation
   * \param mobility the mobility model
   */
  void CourseChange (const std::string context,
                     const Ptr<const MobilityModel> mobility) const;

  Ipv4Address m_destAddr;
  uint32_t m_destPort;
  double m_interval;
  bool m_initialHandshakeEnable;

  Ptr<Socket> m_socket; /// socket to be used for communications.
  EventId m_sendEvent; /// event scheduled to send a new packet.

  mutable int32_t m_sequenceNumber;
  mutable ClientState m_state;


  TracedCallback<Ptr<const Packet>> m_txTrace;
  bool m_storage = false;
};

} // namespace ns3

#endif /* DRONE_CLIENT_APPLICATION_H */
