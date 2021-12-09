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
#ifndef TCP_STUB_SERVER_APPLICATION_H
#define TCP_STUB_SERVER_APPLICATION_H

#include <ns3/application.h>
#include <ns3/socket.h>
#include <ns3/traced-callback.h>

namespace ns3 {

/**
 * Application that sends random packets with Seq.Num. and Timestamp to a remote server.
 */
class TcpStubServerApplication : public Application
{
public:
  /**
   * \brief Register this application as a type in ns-3 TypeId System.
   */
  static TypeId GetTypeId ();
  /**
   * \brief Default constructor.
   */
  TcpStubServerApplication ();
  /**
   * \brief Default destructor.
   */
  virtual ~TcpStubServerApplication ();

private:
  /**
   * \brief Start endpoint to conform with the Application Interface.
   */
  virtual void StartApplication ();
  /**
   * \brief Stop endpoint to conform with the Application Interface.
   */
  virtual void StopApplication ();

  /**
   * \brief Callback for connection requests.
   */
  bool OnConnectionRequest (Ptr<Socket> s, const Address& a);
  /**
   * \brief Callback for new connections.
   */
  void OnNewConnectionCreated (Ptr<Socket> s, const Address& a);
  /**
   * \brief Callback for connections closed normally.
   */
  void OnNormalClose (Ptr<Socket> s);
  /**
   * \brief Callback for connections closed with errors.
   */
  void OnErrorClose (Ptr<Socket> s);
  /**
   * \brief Callback for reception of packets.
   */
  void OnReceivedData (Ptr<Socket> s);

  /// Listening Address.
  Ipv4Address m_addr;
  /// Listening port.
  uint16_t m_port;
  /// Trace new connections being made.
  TracedCallback<Ptr<const TcpStubServerApplication>, Ptr<Socket> > m_connectionEstablishedTrace;
  /// Trace to signal the reception of packets.
  TracedCallback<Ptr<const Packet>, const Address& > m_rxTrace;

  /// The socket to be used for communications.
  Ptr<Socket> m_socket;
};

} // namespace ns3

#endif /* TCP_STUB_SERVER_APPLICATION_H */
