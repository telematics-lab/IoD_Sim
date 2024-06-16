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
#include "udp-echo-client-application.h"

#include <ns3/seq-ts-header.h>
#include <ns3/simulator.h>
#include <ns3/socket.h>
#include <ns3/uinteger.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("IoDSim::UdpEchoClientApplication");

NS_OBJECT_ENSURE_REGISTERED(UdpEchoClientApplication);

TypeId
UdpEchoClientApplication::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::UdpEchoClientApplication")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<UdpEchoClientApplication>()
            .AddAttribute("Interval",
                          "The time to wait between packets",
                          TimeValue(Seconds(1.0)),
                          MakeTimeAccessor(&UdpEchoClientApplication::m_interval),
                          MakeTimeChecker())
            .AddAttribute("RemoteAddress",
                          "The destination Address of the outbound packets",
                          Ipv4AddressValue(),
                          MakeIpv4AddressAccessor(&UdpEchoClientApplication::m_peerAddress),
                          MakeIpv4AddressChecker())
            .AddAttribute("RemotePort",
                          "The destination port of the outbound packets",
                          UintegerValue(100),
                          MakeUintegerAccessor(&UdpEchoClientApplication::m_peerPort),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("PacketSize",
                          "Size of packets generated. The minimum packet size is 12 bytes which is "
                          "the size of the header carrying the sequence number and the time stamp.",
                          UintegerValue(1024),
                          MakeUintegerAccessor(&UdpEchoClientApplication::m_size),
                          MakeUintegerChecker<uint32_t>(12, 65507));
    return tid;
}

UdpEchoClientApplication::UdpEchoClientApplication()
    : m_sent{0},
      m_totalTx{0},
      m_socket{0},
      m_sendEvent{EventId()}
{
}

void
UdpEchoClientApplication::SetRemote(Ipv4Address ip, uint16_t port)
{
    NS_LOG_FUNCTION(this << ip << port);
    m_peerAddress = ip;
    m_peerPort = port;
}

void
UdpEchoClientApplication::SetRemote(Ipv4Address addr)
{
    NS_LOG_FUNCTION(this << addr);
    m_peerAddress = addr;
}

void
UdpEchoClientApplication::DoDispose(void)
{
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
}

void
UdpEchoClientApplication::StartApplication(void)
{
    NS_LOG_FUNCTION(this);

    if (!m_socket)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);

        if (m_socket->Bind() == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
        m_socket->Connect(InetSocketAddress(m_peerAddress, m_peerPort));
    }

    m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    m_socket->SetAllowBroadcast(true);
    m_sendEvent = Simulator::Schedule(Seconds(0.0), &UdpEchoClientApplication::Send, this);
}

void
UdpEchoClientApplication::StopApplication(void)
{
    NS_LOG_FUNCTION(this);
    Simulator::Cancel(m_sendEvent);
}

void
UdpEchoClientApplication::Send(void)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_sendEvent.IsExpired());
    SeqTsHeader seqTs;
    seqTs.SetSeq(m_sent);
    Ptr<Packet> p = Create<Packet>(m_size - 8 + 4); // 8+4 : the size of the seqTs header
    p->AddHeader(seqTs);

    if ((m_socket->Send(p)) >= 0)
    {
        ++m_sent;
        m_totalTx += p->GetSize();

        NS_LOG_INFO("TX " << m_size << " bytes to " << m_peerAddress << " "
                          << "Uid: " << p->GetUid() << " "
                          << "Time: " << Simulator::Now().As(Time::S) << " "
                          << "Packet: " << p->ToString());
    }
    else
    {
        NS_LOG_INFO("Error while sending " << m_size << " bytes to " << m_peerAddress);
    }

    m_sendEvent = Simulator::Schedule(m_interval, &UdpEchoClientApplication::Send, this);
}

uint64_t
UdpEchoClientApplication::GetTotalTx() const
{
    return m_totalTx;
}

} // Namespace ns3
