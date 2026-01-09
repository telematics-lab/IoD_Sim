/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2026 The IoD_Sim Authors.
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
#include "tcp-storage-client-application.h"

#include <ns3/drone.h>
#include <ns3/nstime.h>
#include <ns3/seq-ts-header.h>
#include <ns3/simulator.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TcpStorageClientApplication");
NS_OBJECT_ENSURE_REGISTERED(TcpStorageClientApplication);

constexpr uint16_t IPv4_HDR_SZ = 24;  /// Header size of IPv4 Header, in bytes.
constexpr uint16_t TCP_HDR_SZ = 28;   /// Max Header size of TCP Header, in bytes.
constexpr uint16_t SEQTS_HDR_SZ = 12; /// Header size of SeqTsHeader, in bytes.
constexpr uint16_t HDR_SZ = IPv4_HDR_SZ + TCP_HDR_SZ + SEQTS_HDR_SZ;

TypeId
TcpStorageClientApplication::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::TcpStorageClientApplication")
            .SetParent<TcpClientServerApplication>()
            .SetGroupName("Applications")
            .AddConstructor<TcpStorageClientApplication>()
            .AddAttribute(
                "PayloadSize",
                "Size of the payload, in bytes.",
                UintegerValue(std::numeric_limits<uint16_t>::max() - HDR_SZ - 1),
                MakeUintegerAccessor(&TcpStorageClientApplication::m_payloadSize),
                MakeUintegerChecker<uint16_t>(1, std::numeric_limits<uint16_t>::max() - HDR_SZ - 1))
            .AddTraceSource("Tx",
                            "A new packet is created and sent",
                            MakeTraceSourceAccessor(&TcpStorageClientApplication::m_txTrace),
                            "ns3::Packet::TracedCallback");

    return tid;
}

TcpStorageClientApplication::TcpStorageClientApplication()
    : m_seqNum{0}
{
}

void
TcpStorageClientApplication::DoInitialize()
{
    NS_LOG_FUNCTION(this);
    TcpClientServerApplication::DoInitialize();

    m_storage = FindStorage();
}

void
TcpStorageClientApplication::StartApplication()
{
    NS_LOG_FUNCTION(this);
    TcpClientServerApplication::StartApplication();
    Connect();

    m_storage->TraceConnectWithoutContext(
        "RemainingCapacity",
        MakeCallback(&TcpStorageClientApplication::StorageUpdateCallback, this));
}

bool
TcpStorageClientApplication::DoSendPacket(const uint16_t payloadSize)
{
    NS_LOG_FUNCTION(this << payloadSize);
    const auto sock = GetSocket();

    if (!sock)
        return false;

    SeqTsHeader seqTs;
    auto p = CreatePacket(payloadSize);

    seqTs.SetSeq(m_seqNum++);
    p->AddHeader(seqTs);

    if (sock->Send(p) >= 0)
    {
        m_txTrace(p);
        return true;
    }
    else
    {
        std::stringstream addrStr;
        const auto addr = GetAddress();

        if (Ipv4Address::IsMatchingType(addr))
            addrStr << Ipv4Address::ConvertFrom(addr);
        else if (Ipv6Address::IsMatchingType(addr))
            addrStr << Ipv6Address::ConvertFrom(addr);
        else
            addrStr << addr;

        NS_LOG_WARN("Error while sending " << m_payloadSize << " bytes to " << addrStr.str() << "."
                                           << " ErrNo=" << sock->GetErrno());

        return false;
    }
}

const uint16_t
TcpStorageClientApplication::GetPayloadSize()
{
    NS_LOG_FUNCTION(this);
    return m_payloadSize;
}

void
TcpStorageClientApplication::SendPacket(const uint16_t payloadSize)
{
    NS_LOG_FUNCTION(this << payloadSize << GetNode()->GetId());

    if (DoSendPacket(payloadSize))
        Simulator::ScheduleNow(&StoragePeripheral::Free,
                               m_storage,
                               (uint64_t)payloadSize,
                               StoragePeripheral::byte);
}

Ptr<Packet>
TcpStorageClientApplication::CreatePacket(uint32_t size) const
{
    NS_LOG_FUNCTION(this << size);

    Ptr<Packet> p;
    uint8_t* buf = new uint8_t[size];
    uint16_t* buf16 = (uint16_t*)(buf);

    for (uint32_t i = 0; i < size / 2; i++)
        buf16[i] = i;

    p = Create<Packet>(buf, size);

    delete[] buf;
    return p;
}

Ptr<StoragePeripheral>
TcpStorageClientApplication::FindStorage() const
{
    NS_LOG_FUNCTION(this);

    auto drone = StaticCast<Drone, Node>(GetNode());
    NS_ASSERT_MSG(drone, "This application must be installed on a drone.");

    auto peripherals = drone->GetPeripherals();
    NS_ASSERT_MSG(peripherals->ThereIsStorage(),
                  "Drone must be equipped with a storage peripheral");

    // Drone storage is always the first one in the container
    auto storage = StaticCast<StoragePeripheral, DronePeripheral>(peripherals->Get(0));
    NS_ASSERT_MSG(storage, "Drone must be equipped with a storage peripheral.");

    return storage;
}

void
TcpStorageClientApplication::StorageUpdateCallback(const uint64_t oldValBits,
                                                   const uint64_t newValBits)
{
    NS_LOG_FUNCTION(this << oldValBits << newValBits);

    const uint64_t maxPayloadSizeBits = m_payloadSize * StoragePeripheral::byte;
    const uint64_t occupiedStorageBits = m_storage->GetCapacity() - newValBits;

    if (occupiedStorageBits == 0)
        return;

    NS_LOG_LOGIC("Occupied memory in bits: " << occupiedStorageBits << " | MaxPayloadSize bits "
                                             << maxPayloadSizeBits);

    uint16_t nextPayloadSizeBytes = (occupiedStorageBits >= maxPayloadSizeBits)
                                        ? m_payloadSize
                                        : occupiedStorageBits / StoragePeripheral::byte;

    Simulator::Schedule(Seconds(0.1),
                        &TcpStorageClientApplication::SendPacket,
                        this,
                        nextPayloadSizeBytes);
}

} // namespace ns3
