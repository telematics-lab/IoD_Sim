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
#include <ns3/drone.h>
#include <ns3/seq-ts-header.h>
#include <ns3/simulator.h>
#include <ns3/socket-factory.h>
#include <ns3/tcp-socket-factory.h>
#include <ns3/uinteger.h>

#include "tcp-stub-client-application.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpStubClientApplication");
NS_OBJECT_ENSURE_REGISTERED (TcpStubClientApplication);

constexpr uint16_t IPv4_HDR_SZ = 24;  /// Header size of IPv4 Header, in bytes.
constexpr uint16_t TCP_HDR_SZ = 28;   /// Max Header size of TCP Header, in bytes.
constexpr uint16_t SEQTS_HDR_SZ = 12; /// Header size of SeqTsHeader, in bytes.
constexpr uint16_t HDR_SZ = IPv4_HDR_SZ + TCP_HDR_SZ + SEQTS_HDR_SZ;

TypeId
TcpStubClientApplication::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::TcpStubClientApplication")
    .SetParent<Application> ()
    .SetGroupName ("Applications")
    .AddConstructor<TcpStubClientApplication> ()
    .AddAttribute ("RemoteAddress", "IP Address of the server.",
                   Ipv4AddressValue (Ipv4Address::GetBroadcast ()),
                   MakeIpv4AddressAccessor (&TcpStubClientApplication::m_destAddr),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("RemotePort", "Destination application port.",
                   UintegerValue (80),
                   MakeUintegerAccessor (&TcpStubClientApplication::m_destPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("MaxPayloadSize", "Maximum size of the payload, in bytes.",
                   UintegerValue (UINT16_MAX - HDR_SZ - 1),
                   MakeUintegerAccessor (&TcpStubClientApplication::m_maxPayloadSize),
                   MakeUintegerChecker<uint16_t> (1, std::numeric_limits<uint16_t>::max () - HDR_SZ - 1))
    .AddTraceSource ("Connection", "A new connection has been established.",
                     MakeTraceSourceAccessor (&TcpStubClientApplication::m_connectionEstablishedTrace),
                     "ns3::TcpStubClientApplication::OnConnectionSucceeded")
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&TcpStubClientApplication::m_txTrace),
                     "ns3::Packet::TracedCallback")
    ;

  return tid;
}

TcpStubClientApplication::TcpStubClientApplication () :
  m_seqNum {0}
{
  NS_LOG_FUNCTION (this);
}

TcpStubClientApplication::~TcpStubClientApplication ()
{
  NS_LOG_FUNCTION (this);
}

void
TcpStubClientApplication::DoInitialize ()
{
  NS_LOG_FUNCTION (this);

  m_payloadSize = m_maxPayloadSize - HDR_SZ;
  m_storage = FindStorage ();

  // Forward initialization event to parent objects
  Application::DoInitialize ();
}

void
TcpStubClientApplication::StartApplication ()
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_IF (m_socket != NULL, "Application was called for startup, but the socket has been already initialized!");

  Ptr<SocketFactory> socketFactory = GetNode ()->GetObject<SocketFactory> (TcpSocketFactory::GetTypeId ());
  m_socket = socketFactory->CreateSocket ();
  int ret = 0;

  ret = m_socket->Bind ();
  NS_ABORT_MSG_IF (ret < 0, "Failed to bind IPv4-based Socket.");

  ret = m_socket->Connect (InetSocketAddress (m_destAddr, m_destPort));
  NS_ABORT_MSG_IF (ret < 0, "Failed to connect to " << m_destAddr << ":" << m_destPort
                            << " ErrNo=" << m_socket->GetErrno ());

  m_socket->SetConnectCallback (MakeCallback (&TcpStubClientApplication::OnConnectionSucceeded, this),
                                MakeCallback (&TcpStubClientApplication::OnConnectionFailed, this));
  m_socket->SetCloseCallbacks  (MakeCallback (&TcpStubClientApplication::OnNormalClose, this),
                                MakeCallback (&TcpStubClientApplication::OnErrorClose, this));
  m_socket->SetRecvCallback    (MakeCallback (&TcpStubClientApplication::OnReceivedData, this));

  m_storage->TraceConnectWithoutContext ("RemainingCapacity",
                                         MakeCallback (&TcpStubClientApplication::OnStorageUpdate, this));
}

void
TcpStubClientApplication::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != NULL)
    m_socket->Close ();

  m_socket = nullptr;
}

void
TcpStubClientApplication::OnConnectionSucceeded (Ptr<Socket> s)
{
  NS_LOG_FUNCTION (this << s);

  m_connectionEstablishedTrace (this, s);
  s->SetRecvCallback (MakeCallback (&TcpStubClientApplication::OnReceivedData, this));
}

void
TcpStubClientApplication::OnConnectionFailed (Ptr<Socket> s)
{
  NS_LOG_FUNCTION (this << s);
  NS_LOG_ERROR ("Client failed to connect to " << m_destAddr << ":" << m_destPort);
}

void
TcpStubClientApplication::OnNormalClose (Ptr<Socket> s)
{
  NS_LOG_FUNCTION (this << s);

  auto sockErrno = s->GetErrno ();

  if (sockErrno != Socket::ERROR_NOTERROR)
    NS_LOG_ERROR (this << " Connection terminated with error " << sockErrno);

  s->SetCloseCallbacks (MakeNullCallback<void, Ptr<Socket> > (), MakeNullCallback<void, Ptr<Socket> > ());
}

void
TcpStubClientApplication::OnErrorClose (Ptr<Socket> s)
{
  NS_LOG_FUNCTION (this << s);

  auto sockErrno = s->GetErrno ();

  if (sockErrno != Socket::ERROR_NOTERROR)
    NS_LOG_ERROR (this << " Connection terminated with error " << sockErrno);
}

void
TcpStubClientApplication::OnReceivedData (Ptr<Socket> s)
{
  NS_LOG_FUNCTION (this << s);
}

bool
TcpStubClientApplication::SendPacket (const uint16_t payloadSize)
{
  NS_LOG_FUNCTION (this << payloadSize << GetNode ()->GetId ());

  if (m_socket == nullptr)
    return false;

  SeqTsHeader seqTs;
  auto p = CreatePacket (payloadSize);

  seqTs.SetSeq (m_seqNum++);
  p->AddHeader (seqTs);

  if (m_socket->Send (p) >= 0)
    {
      m_txTrace (p);
      Simulator::ScheduleNow (&StoragePeripheral::Free, m_storage,
                              (uint64_t) payloadSize, StoragePeripheral::byte);
      return true;
    }
  else
    {
      std::stringstream addrStr;

      if (Ipv4Address::IsMatchingType (m_destAddr))
        addrStr << Ipv4Address::ConvertFrom (m_destAddr);
      else if (Ipv6Address::IsMatchingType (m_destAddr))
        addrStr << Ipv6Address::ConvertFrom (m_destAddr);
      else
        addrStr << m_destAddr;

      NS_LOG_WARN ("Error while sending " << m_maxPayloadSize << " bytes to " << addrStr.str ()
                   << " ErrNo=" << m_socket->GetErrno ());
      return false;
    }
}

Ptr<Packet>
TcpStubClientApplication::CreatePacket (uint32_t size) const
{
  NS_LOG_FUNCTION (this << size);

  Ptr<Packet> p;
  uint8_t* buf = new uint8_t[size];
  uint16_t* buf16 = (uint16_t*) (buf);

  for (uint32_t i = 0; i < size / 2; i++)
    buf16[i] = i;

  p = Create<Packet> (buf, size);

  delete buf;
  return p;
}

Ptr<StoragePeripheral>
TcpStubClientApplication::FindStorage () const
{
  NS_LOG_FUNCTION (this);

  auto drone = StaticCast <Drone, Node> (GetNode ());
  NS_ASSERT_MSG (drone != nullptr, "This application must be installed on a drone.");

  auto peripherals = drone->getPeripherals ();
  NS_ASSERT_MSG (peripherals->thereIsStorage (), "Drone must be equipped with a storage peripheral");

  // Drone storage is always the first one in the container
  auto storage = StaticCast<StoragePeripheral, DronePeripheral> (peripherals->Get (0));
  NS_ASSERT_MSG (storage != nullptr, "Drone must be equipped with a storage peripheral.");

  return storage;
}

void
TcpStubClientApplication::OnStorageUpdate (const uint64_t oldValBits, const uint64_t newValBits)
{
  NS_LOG_FUNCTION (this << oldValBits << newValBits << Simulator::Now ());

  const uint32_t maxPayloadSizeBits = m_maxPayloadSize * StoragePeripheral::byte;
  uint16_t nextPayloadSize = (newValBits >= maxPayloadSizeBits)
                             ? maxPayloadSizeBits
                             : newValBits / StoragePeripheral::byte;

  Simulator::ScheduleNow (&TcpStubClientApplication::SendPacket, this, nextPayloadSize);
}

} // namespace ns3
