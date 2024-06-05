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
#include "tcp-echo-server-application.h"

#include <ns3/tcp-socket-factory.h>
#include <ns3/uinteger.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TcpEchoServerApplication");
NS_OBJECT_ENSURE_REGISTERED(TcpEchoServerApplication);

constexpr uint16_t IPv4_HDR_SZ = 24;  /// Header size of IPv4 Header, in bytes.
constexpr uint16_t TCP_HDR_SZ = 28;   /// Max Header size of TCP Header, in bytes.
constexpr uint16_t SEQTS_HDR_SZ = 12; /// Header size of SeqTsHeader, in bytes.
constexpr uint16_t HDR_SZ = IPv4_HDR_SZ + TCP_HDR_SZ + SEQTS_HDR_SZ;

TypeId
TcpEchoServerApplication::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::TcpEchoServerApplication")
            .SetParent<TcpClientServerApplication>()
            .SetGroupName("Applications")
            .AddConstructor<TcpEchoServerApplication>()
            .AddTraceSource("Rx",
                            "A new packet has been received.",
                            MakeTraceSourceAccessor(&TcpEchoServerApplication::m_rxTrace),
                            "ns3::Packet::PacketAddressTracedCallback");

    return tid;
}

TcpEchoServerApplication::TcpEchoServerApplication()
{
    NS_LOG_FUNCTION(this);
}

TcpEchoServerApplication::~TcpEchoServerApplication()
{
    NS_LOG_FUNCTION(this);
}

void
TcpEchoServerApplication::StartApplication()
{
    NS_LOG_FUNCTION(this);
    TcpClientServerApplication::StartApplication();
    Listen();
}

void
TcpEchoServerApplication::ReceivedDataCallback(Ptr<Socket> s)
{
    NS_LOG_FUNCTION(this << s);
    Ptr<Packet> p;
    Address from;

    while ((p = s->RecvFrom(from)))
    {
        if (InetSocketAddress::IsMatchingType(from))
        {
            auto sockAddr = InetSocketAddress::ConvertFrom(from);
            NS_LOG_INFO(this << " Rx Packet (Size=" << p->GetSize() << ","
                             << "Sender=" << sockAddr.GetIpv4() << ":" << sockAddr.GetPort()
                             << ")");
        }
        else if (Inet6SocketAddress::IsMatchingType(from))
        {
            auto sockAddr = Inet6SocketAddress::ConvertFrom(from);
            NS_LOG_INFO(this << " Rx Packet (Size=" << p->GetSize() << ","
                             << "Sender=" << sockAddr.GetIpv6() << ":" << sockAddr.GetPort()
                             << ")");
        }
        else
        {
            NS_LOG_INFO(this << " Rx Packet (Size=" << p->GetSize() << ","
                             << "Sender=" << from << ")");
        }

        m_rxTrace(p, from);
    }
}

} // namespace ns3
