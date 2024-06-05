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
 */
#ifndef TCP_CLIENT_SERVER_APPLICATION_H
#define TCP_CLIENT_SERVER_APPLICATION_H

#include <ns3/application.h>
#include <ns3/socket.h>
#include <ns3/traced-callback.h>

namespace ns3
{

/**
 * Base application for reliable TCP-based client-server communications.
 */
class TcpClientServerApplication : public Application
{
  public:
    static TypeId GetTypeId();
    TcpClientServerApplication();
    virtual ~TcpClientServerApplication();

  protected:
    virtual void StartApplication();
    virtual void StopApplication();
    virtual void Listen();
    virtual bool Connect();
    virtual void ReceivedDataCallback(Ptr<Socket> s);

    const Ipv4Address& GetAddress() const;
    const uint16_t& GetPort() const;
    Ptr<Socket> GetSocket();

  private:
    bool ConnectionRequestCallback(Ptr<Socket> s, const Address& a);
    void ConnectionEstablishedCallback(Ptr<Socket> s, const Address& a);
    void ConnectionSucceededCallback(Ptr<Socket> s);
    void ConnectionFailedCallback(Ptr<Socket> s);
    void NormalCloseCallback(Ptr<Socket> s);
    void ErrorCloseCallback(Ptr<Socket> s);

    Ipv4Address m_addr;
    uint16_t m_port;
    Ptr<Socket> m_socket;

    TracedCallback<Ptr<const TcpClientServerApplication>, Ptr<Socket>> m_connectionEstablishedTrace;
    TracedCallback<Ptr<const TcpClientServerApplication>, Ptr<Socket>> m_connectionClosedTrace;
};

} // namespace ns3

#endif /* TCP_CLIENT_SERVER_APPLICATION_H */
