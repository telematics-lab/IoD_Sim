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
    .AddAttribute ("TransmissionRate", "Rate of the tranmission, in Hz.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&TcpStubClientApplication::m_txFreq),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("PacketSize", "Size of the packet, in bytes, comprehensive of L3,4 header sizes.",
                   UintegerValue (UINT16_MAX),
                   MakeUintegerAccessor (&TcpStubClientApplication::m_packetSize),
                   MakeUintegerChecker<uint16_t> (HDR_SZ + 1, std::numeric_limits<uint16_t>::max ()))
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
  m_sendEvent {EventId ()},
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

  m_packetInterval = 1 / m_txFreq;
  m_payloadSize = m_packetSize - HDR_SZ;

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
}

void
TcpStubClientApplication::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  Simulator::Cancel (m_sendEvent);
  if (m_socket != NULL)
    m_socket->Close ();
}

void
TcpStubClientApplication::OnConnectionSucceeded (Ptr<Socket> s)
{
  NS_LOG_FUNCTION (this << s);

  m_connectionEstablishedTrace (this, s);
  s->SetRecvCallback (MakeCallback (&TcpStubClientApplication::OnReceivedData, this));

  m_sendEvent = Simulator::ScheduleNow (&TcpStubClientApplication::SendPacket, this);
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

  Simulator::Cancel (m_sendEvent);

  if (sockErrno != Socket::ERROR_NOTERROR)
    NS_LOG_ERROR (this << " Connection terminated with error " << sockErrno);

  s->SetCloseCallbacks (MakeNullCallback<void, Ptr<Socket> > (), MakeNullCallback<void, Ptr<Socket> > ());
}

void
TcpStubClientApplication::OnErrorClose (Ptr<Socket> s)
{
  NS_LOG_FUNCTION (this << s);

  auto sockErrno = s->GetErrno ();

  Simulator::Cancel (m_sendEvent);

  if (sockErrno != Socket::ERROR_NOTERROR)
    NS_LOG_ERROR (this << " Connection terminated with error " << sockErrno);
}

void
TcpStubClientApplication::OnReceivedData (Ptr<Socket> s)
{
  NS_LOG_FUNCTION (this << s);
}

void
TcpStubClientApplication::SendPacket ()
{
  NS_LOG_FUNCTION (this);

  SeqTsHeader seqTs;
  auto p = CreatePacket (m_payloadSize);

  seqTs.SetSeq (m_seqNum++);
  p->AddHeader (seqTs);

  if (m_socket->Send (p) >= 0)
    {
      m_txTrace (p);
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

      NS_LOG_WARN ("Error while sending " << m_packetSize << " bytes to " << addrStr.str ()
                   << " ErrNo=" << m_socket->GetErrno ());
    }

  m_sendEvent = Simulator::Schedule (Seconds (m_packetInterval), &TcpStubClientApplication::SendPacket, this);
}

Ptr<Packet>
TcpStubClientApplication::CreatePacket (uint32_t size) const
{
  Ptr<Packet> p;
  uint8_t* buf = new uint8_t[size];
  uint16_t* buf16 = (uint16_t*) (buf);

  for (uint32_t i = 0; i < size / 2; i++)
    buf16[i] = i;

  p = Create<Packet> (buf, size);

  delete buf;
  return p;
}

} // namespace ns3
