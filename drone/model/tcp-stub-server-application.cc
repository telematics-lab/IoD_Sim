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

#include "tcp-stub-server-application.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpStubServerApplication");
NS_OBJECT_ENSURE_REGISTERED (TcpStubServerApplication);

constexpr uint16_t IPv4_HDR_SZ = 24;  /// Header size of IPv4 Header, in bytes.
constexpr uint16_t TCP_HDR_SZ = 28;   /// Max Header size of TCP Header, in bytes.
constexpr uint16_t SEQTS_HDR_SZ = 12; /// Header size of SeqTsHeader, in bytes.
constexpr uint16_t HDR_SZ = IPv4_HDR_SZ + TCP_HDR_SZ + SEQTS_HDR_SZ;

TypeId
TcpStubServerApplication::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::TcpStubServerApplication")
    .SetParent<Application> ()
    .SetGroupName ("Applications")
    .AddConstructor<TcpStubServerApplication> ()
    .AddAttribute ("ListenAddress", "IP Address to bind the socket.",
                   Ipv4AddressValue (Ipv4Address::GetAny ()),
                   MakeIpv4AddressAccessor (&TcpStubServerApplication::m_addr),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("ListenPort", "Port to use.",
                   UintegerValue (80),
                   MakeUintegerAccessor (&TcpStubServerApplication::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddTraceSource ("Connection", "A new connection has been established.",
                     MakeTraceSourceAccessor (&TcpStubServerApplication::m_connectionEstablishedTrace),
                     "ns3::TcpStubServerApplication::OnConnectionEstablished")
    .AddTraceSource ("Rx", "A new packet has been received.",
                     MakeTraceSourceAccessor (&TcpStubServerApplication::m_rxTrace),
                     "ns3::Packet::PacketAddressTracedCallback")
    ;

  return tid;
}

TcpStubServerApplication::TcpStubServerApplication ()
{
  NS_LOG_FUNCTION (this);
}

TcpStubServerApplication::~TcpStubServerApplication ()
{
  NS_LOG_FUNCTION (this);
}

void
TcpStubServerApplication::StartApplication ()
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_IF (m_socket != NULL, "Application was called for startup, but the socket has been already initialized!");

  Ptr<SocketFactory> socketFactory = GetNode ()->GetObject<SocketFactory> (TcpSocketFactory::GetTypeId ());
  m_socket = socketFactory->CreateSocket ();
  int ret = 0;

  ret = m_socket->Bind (InetSocketAddress (m_addr, m_port));
  NS_ABORT_MSG_IF (ret < 0, "Failed to bind IPv4-based Socket to " << m_addr << ":" << m_port);

  ret = m_socket->Listen ();
  NS_ABORT_MSG_IF (ret < 0, "Failed to listen to " << m_addr << ":" << m_port << " ErrNo=" << m_socket->GetErrno ());

  m_socket->SetAcceptCallback (MakeCallback (&TcpStubServerApplication::OnConnectionRequest, this),
                               MakeCallback (&TcpStubServerApplication::OnNewConnectionCreated, this));
  m_socket->SetCloseCallbacks (MakeCallback (&TcpStubServerApplication::OnNormalClose, this),
                               MakeCallback (&TcpStubServerApplication::OnErrorClose, this));
  m_socket->SetRecvCallback   (MakeCallback (&TcpStubServerApplication::OnReceivedData, this));

  // As this socket will be used for Tx only, make dummy callback if it Rx something
  m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
}

void
TcpStubServerApplication::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != NULL)
    {
      m_socket->Close ();

      m_socket->SetAcceptCallback (MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                                   MakeNullCallback<void, Ptr<Socket>, const Address &> ());
      m_socket->SetCloseCallbacks (MakeNullCallback<void, Ptr<Socket>> (),
                                   MakeNullCallback<void, Ptr<Socket>> ());
      m_socket->SetRecvCallback   (MakeNullCallback<void, Ptr<Socket>> ());
    }
}

bool
TcpStubServerApplication::OnConnectionRequest (Ptr<Socket> s, const Address& from)
{
  if (InetSocketAddress::IsMatchingType (from))
    NS_LOG_FUNCTION (this << s << InetSocketAddress::ConvertFrom (from));
  else if (Inet6SocketAddress::IsMatchingType (from))
    NS_LOG_FUNCTION (this << s << Inet6SocketAddress::ConvertFrom (from));
  else if (Ipv4Address::IsMatchingType (from))
    NS_LOG_FUNCTION (this << s << Ipv4Address::ConvertFrom (from));
  else if (Ipv6Address::IsMatchingType (from))
    NS_LOG_FUNCTION (this << s << Ipv6Address::ConvertFrom (from));
  else
    NS_LOG_FUNCTION (this << s << from);

  return true; // Unconditionally accept new connection requests
}

void
TcpStubServerApplication::OnNewConnectionCreated (Ptr<Socket> s, const Address& from)
{
  if (InetSocketAddress::IsMatchingType (from))
    NS_LOG_FUNCTION (this << s << InetSocketAddress::ConvertFrom (from));
  else if (Inet6SocketAddress::IsMatchingType (from))
    NS_LOG_FUNCTION (this << s << Inet6SocketAddress::ConvertFrom (from));
  else if (Ipv4Address::IsMatchingType (from))
    NS_LOG_FUNCTION (this << s << Ipv4Address::ConvertFrom (from));
  else if (Ipv6Address::IsMatchingType (from))
    NS_LOG_FUNCTION (this << s << Ipv6Address::ConvertFrom (from));
  else
    NS_LOG_FUNCTION (this << s << from);

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
  m_connectionEstablishedTrace (this, s);
  s->SetRecvCallback (MakeCallback (&TcpStubServerApplication::OnReceivedData, this));
}

void
TcpStubServerApplication::OnNormalClose (Ptr<Socket> s)
{
  NS_LOG_FUNCTION (this << s);
  s->ShutdownSend ();
}

void
TcpStubServerApplication::OnErrorClose (Ptr<Socket> s)
{
  NS_LOG_FUNCTION (this << s);
  s->Close ();
}

void
TcpStubServerApplication::OnReceivedData (Ptr<Socket> s)
{
  NS_LOG_FUNCTION (this << s);

  Ptr<Packet> p;
  Address from;

  while ((p = s->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          auto sockAddr = InetSocketAddress::ConvertFrom (from);
          NS_LOG_INFO (this << " Rx Packet (Size=" << p->GetSize () << ","
                            << "Sender=" << sockAddr.GetIpv4 () << ":" << sockAddr.GetPort () << ")");
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          auto sockAddr = Inet6SocketAddress::ConvertFrom (from);
          NS_LOG_INFO (this << " Rx Packet (Size=" << p->GetSize () << ","
                            << "Sender=" << sockAddr.GetIpv6 () << ":" << sockAddr.GetPort () << ")");
        }
      else
        {
          NS_LOG_INFO (this << " Rx Packet (Size=" << p->GetSize () << ","
                            << "Sender=" << from << ")");
        }

      m_rxTrace (p, from);
    }
}

} // namespace ns3
