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
#include "tcp-client-server-application.h"

#include <ns3/log.h>
#include <ns3/tcp-socket-factory.h>
#include <ns3/uinteger.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TcpClientServerApplication");
NS_OBJECT_ENSURE_REGISTERED(TcpClientServerApplication);

TypeId
TcpClientServerApplication::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::TcpClientServerApplication")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<TcpClientServerApplication>()
            .AddAttribute("Address",
                          "Remote/Listening IP Address.",
                          Ipv4AddressValue(Ipv4Address::GetLoopback()),
                          MakeIpv4AddressAccessor(&TcpClientServerApplication::m_addr),
                          MakeIpv4AddressChecker())
            .AddAttribute("Port",
                          "Remote/Listening port.",
                          UintegerValue(4242),
                          MakeUintegerAccessor(&TcpClientServerApplication::m_port),
                          MakeUintegerChecker<uint16_t>())
            .AddTraceSource(
                "ConnectionEstablished",
                "A new connection has been established.",
                MakeTraceSourceAccessor(&TcpClientServerApplication::m_connectionEstablishedTrace),
                "ns3::TcpClientServerApplication::ConnectionSucceededCallback")
            .AddTraceSource(
                "ConnectionClosed",
                "A connection has been closed successfully.",
                MakeTraceSourceAccessor(&TcpClientServerApplication::m_connectionClosedTrace),
                "ns3::TcpClientServerApplication::NormalCloseCallback");

    return tid;
}

void
TcpClientServerApplication::StartApplication()
{
    NS_LOG_FUNCTION(this);
    NS_ABORT_MSG_IF(
        m_socket,
        "Application was called for startup, but the socket has been already initialized!");

    Ptr<SocketFactory> socketFactory =
        GetNode()->GetObject<SocketFactory>(TcpSocketFactory::GetTypeId());
    m_socket = socketFactory->CreateSocket();

    m_socket->SetAcceptCallback(
        MakeCallback(&TcpClientServerApplication::ConnectionRequestCallback, this),
        MakeCallback(&TcpClientServerApplication::ConnectionEstablishedCallback, this));
    m_socket->SetConnectCallback(
        MakeCallback(&TcpClientServerApplication::ConnectionSucceededCallback, this),
        MakeCallback(&TcpClientServerApplication::ConnectionFailedCallback, this));
    m_socket->SetCloseCallbacks(
        MakeCallback(&TcpClientServerApplication::NormalCloseCallback, this),
        MakeCallback(&TcpClientServerApplication::ErrorCloseCallback, this));
    // m_socket->SetRecvCallback    (MakeCallback
    // (&TcpClientServerApplication::ReceivedDataCallback, this));
    m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
}

void
TcpClientServerApplication::StopApplication()
{
    NS_LOG_FUNCTION(this);

    if (m_socket)
    {
        m_socket->Close();

        m_socket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address&>(),
                                    MakeNullCallback<void, Ptr<Socket>, const Address&>());
        m_socket->SetConnectCallback(MakeNullCallback<void, Ptr<Socket>>(),
                                     MakeNullCallback<void, Ptr<Socket>>());
        m_socket->SetCloseCallbacks(MakeNullCallback<void, Ptr<Socket>>(),
                                    MakeNullCallback<void, Ptr<Socket>>());
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }

    m_socket = nullptr;
}

void
TcpClientServerApplication::Listen()
{
    NS_LOG_FUNCTION(this);
    int ret = 0;

    ret = m_socket->Bind(InetSocketAddress(m_addr, m_port));
    NS_ABORT_MSG_IF(ret < 0,
                    "Failed to bind IPv4-based Socket to " << m_addr << ":" << m_port << ".");

    ret = m_socket->Listen();
    NS_ABORT_MSG_IF(ret < 0,
                    "Failed to listen to " << m_addr << ":" << m_port
                                           << ". ErrNo=" << m_socket->GetErrno());
}

bool
TcpClientServerApplication::Connect()
{
    NS_LOG_FUNCTION(this);
    int ret = 0;

    ret = m_socket->Bind();
    NS_ABORT_MSG_IF(ret < 0, "Failed to bind IPv4-based Socket.");

    ret = m_socket->Connect(InetSocketAddress(m_addr, m_port));
    if (ret < 0)
        NS_LOG_LOGIC("Failed to connect to " << m_addr << ":" << m_port
                                             << " ErrNo=" << m_socket->GetErrno());

    return (ret == 0) ? true : false;
}

void
TcpClientServerApplication::ReceivedDataCallback(Ptr<Socket> s)
{
    NS_LOG_FUNCTION(this << s);
    // placeholder for child objects lacking receive functionality
}

const Ipv4Address&
TcpClientServerApplication::GetAddress() const
{
    return m_addr;
}

const uint16_t&
TcpClientServerApplication::GetPort() const
{
    return m_port;
}

Ptr<Socket>
TcpClientServerApplication::GetSocket()
{
    return m_socket;
}

bool
TcpClientServerApplication::ConnectionRequestCallback(Ptr<Socket> s, const Address& from)
{
    if (InetSocketAddress::IsMatchingType(from))
        NS_LOG_FUNCTION(this << s << InetSocketAddress::ConvertFrom(from));
    else if (Inet6SocketAddress::IsMatchingType(from))
        NS_LOG_FUNCTION(this << s << Inet6SocketAddress::ConvertFrom(from));
    else if (Ipv4Address::IsMatchingType(from))
        NS_LOG_FUNCTION(this << s << Ipv4Address::ConvertFrom(from));
    else if (Ipv6Address::IsMatchingType(from))
        NS_LOG_FUNCTION(this << s << Ipv6Address::ConvertFrom(from));
    else
        NS_LOG_FUNCTION(this << s << from);

    return true; // Unconditionally accept new connection requests
}

void
TcpClientServerApplication::ConnectionEstablishedCallback(Ptr<Socket> s, const Address& from)
{
    if (InetSocketAddress::IsMatchingType(from))
        NS_LOG_FUNCTION(this << s << InetSocketAddress::ConvertFrom(from));
    else if (Inet6SocketAddress::IsMatchingType(from))
        NS_LOG_FUNCTION(this << s << Inet6SocketAddress::ConvertFrom(from));
    else if (Ipv4Address::IsMatchingType(from))
        NS_LOG_FUNCTION(this << s << Ipv4Address::ConvertFrom(from));
    else if (Ipv6Address::IsMatchingType(from))
        NS_LOG_FUNCTION(this << s << Ipv6Address::ConvertFrom(from));
    else
        NS_LOG_FUNCTION(this << s << from);

    /*
     * A typical connection is established after receiving an empty (i.e., no
     * data) TCP packet with ACK flag. The actual data will follow in a separate
     * packet after that and will be received by ReceivedDataCallback().
     *
     * However, that empty ACK packet might get lost. In this case, we may
     * receive the first data packet right here already, because it also counts
     * as a new connection. The statement below attempts to fetch the data from
     * that packet, if any.
     */
    m_connectionEstablishedTrace(this, s);
    s->SetRecvCallback(MakeCallback(&TcpClientServerApplication::ReceivedDataCallback, this));
}

void
TcpClientServerApplication::ConnectionSucceededCallback(Ptr<Socket> s)
{
    NS_LOG_FUNCTION(this << s);
    m_connectionEstablishedTrace(this, s);
    s->SetRecvCallback(MakeCallback(&TcpClientServerApplication::ReceivedDataCallback, this));
}

void
TcpClientServerApplication::ConnectionFailedCallback(Ptr<Socket> s)
{
    NS_LOG_FUNCTION(this << s);
    NS_LOG_ERROR("Cannot connect to " << m_addr << ":" << m_port);
    s->SetCloseCallbacks(MakeNullCallback<void, Ptr<Socket>>(),
                         MakeNullCallback<void, Ptr<Socket>>());
}

void
TcpClientServerApplication::NormalCloseCallback(Ptr<Socket> s)
{
    NS_LOG_FUNCTION(this << s);
    NS_LOG_LOGIC(this << " Connection terminated.");
    s->ShutdownSend();
    s->SetCloseCallbacks(MakeNullCallback<void, Ptr<Socket>>(),
                         MakeNullCallback<void, Ptr<Socket>>());
}

void
TcpClientServerApplication::ErrorCloseCallback(Ptr<Socket> s)
{
    NS_LOG_FUNCTION(this << s);
    NS_LOG_ERROR(this << " Connection terminated abruptly. ErrNo=" << s->GetErrno());
    s->Close();
    s->SetCloseCallbacks(MakeNullCallback<void, Ptr<Socket>>(),
                         MakeNullCallback<void, Ptr<Socket>>());
}

} // namespace ns3
