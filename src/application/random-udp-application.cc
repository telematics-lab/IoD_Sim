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
#include "random-udp-application.h"

#include <ns3/seq-ts-header.h>
#include <ns3/simulator.h>
#include <ns3/socket-factory.h>
#include <ns3/udp-socket-factory.h>
#include <ns3/uinteger.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("RandomUdpApplication");
NS_OBJECT_ENSURE_REGISTERED(RandomUdpApplication);

constexpr uint16_t IPv4_HDR_SZ = 24;  /// Header size of IPv4 Header, in bytes.
constexpr uint16_t UDP_HDR_SZ = 4;    /// Header size of UDP Header, in bytes.
constexpr uint16_t SEQTS_HDR_SZ = 12; /// Header size of SeqTsHeader, in bytes.
constexpr uint16_t HDR_SZ = IPv4_HDR_SZ + UDP_HDR_SZ + SEQTS_HDR_SZ;

TypeId
RandomUdpApplication::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::RandomUdpApplication")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<RandomUdpApplication>()
            .AddAttribute("RemoteAddress",
                          "IP Address of the server.",
                          Ipv4AddressValue(Ipv4Address::GetBroadcast()),
                          MakeIpv4AddressAccessor(&RandomUdpApplication::m_destAddr),
                          MakeIpv4AddressChecker())
            .AddAttribute("RemotePort",
                          "Destination application port.",
                          UintegerValue(80),
                          MakeUintegerAccessor(&RandomUdpApplication::m_destPort),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("TransmissionRate",
                          "Rate of the tranmission, in Hz.",
                          UintegerValue(1),
                          MakeUintegerAccessor(&RandomUdpApplication::m_txFreq),
                          MakeUintegerChecker<uint32_t>(1))
            .AddAttribute(
                "PacketSize",
                "Size of the packet, in bytes, comprehensive of L3,4 header sizes.",
                UintegerValue(UINT16_MAX),
                MakeUintegerAccessor(&RandomUdpApplication::m_packetSize),
                MakeUintegerChecker<uint16_t>(HDR_SZ + 1, std::numeric_limits<uint16_t>::max()))
            .AddTraceSource("Tx",
                            "A new packet is created and is sent",
                            MakeTraceSourceAccessor(&RandomUdpApplication::m_txTrace),
                            "ns3::Packet::TracedCallback");

    return tid;
}

RandomUdpApplication::RandomUdpApplication()
    : m_sendEvent{EventId()},
      m_seqNum{0}
{
    NS_LOG_FUNCTION(this);
}

RandomUdpApplication::~RandomUdpApplication()
{
    NS_LOG_FUNCTION(this);
}

void
RandomUdpApplication::DoInitialize()
{
    NS_LOG_FUNCTION(this);

    m_packetInterval = 1 / m_txFreq;
    m_payloadSize = m_packetSize - IPv4_HDR_SZ - UDP_HDR_SZ - SEQTS_HDR_SZ;

    // Forward initialization event to parent objects
    Application::DoInitialize();
}

void
RandomUdpApplication::StartApplication()
{
    NS_LOG_FUNCTION(this);

    if (m_socket)
    {
        Ptr<SocketFactory> socketFactory =
            GetNode()->GetObject<SocketFactory>(UdpSocketFactory::GetTypeId());
        m_socket = socketFactory->CreateSocket();

        NS_ABORT_MSG_IF(m_socket->Bind() == -1, "Failed to bind IPv4-based Socket.");
        m_socket->Connect(InetSocketAddress(m_destAddr, m_destPort));
    }

    // As this socket will be used for Tx only, make dummy callback if it Rx something
    m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    m_socket->SetAllowBroadcast(true); // handle special cases where the remote address is broadcast
    m_sendEvent = Simulator::ScheduleNow(&RandomUdpApplication::SendPacket, this);
}

void
RandomUdpApplication::StopApplication()
{
    NS_LOG_FUNCTION(this);

    Simulator::Cancel(m_sendEvent);
    if (!m_socket)
        m_socket->Close();
}

void
RandomUdpApplication::SendPacket()
{
    NS_LOG_FUNCTION(this);

    SeqTsHeader seqTs;
    auto p = CreatePacket(m_payloadSize);

    seqTs.SetSeq(m_seqNum++);
    p->AddHeader(seqTs);

    if (m_socket->Send(p) >= 0)
    {
        m_txTrace(p);
    }
    else
    {
        std::stringstream addrStr;

        if (Ipv4Address::IsMatchingType(m_destAddr))
            addrStr << Ipv4Address::ConvertFrom(m_destAddr);
        else if (Ipv6Address::IsMatchingType(m_destAddr))
            addrStr << Ipv6Address::ConvertFrom(m_destAddr);
        else
            addrStr << m_destAddr;

        NS_LOG_WARN("Error while sending " << m_packetSize << " bytes to " << addrStr.str());
    }

    m_sendEvent =
        Simulator::Schedule(Seconds(m_packetInterval), &RandomUdpApplication::SendPacket, this);
}

Ptr<Packet>
RandomUdpApplication::CreatePacket(uint32_t size) const
{
    Ptr<Packet> p;
    uint8_t* buf = new uint8_t[size];
    uint16_t* buf16 = (uint16_t*)(buf);

    for (uint32_t i = 0; i < size / 2; i++)
        buf16[i] = i;

    p = Create<Packet>(buf, size);

    delete[] buf;
    return p;
}

} // namespace ns3
