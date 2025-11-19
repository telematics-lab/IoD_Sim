/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 *
 * Author: Tim Schubert <ns-3-leo@timschubert.net>
 */

#include "ns3/log.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"
#include "ns3/llc-snap-header.h"
#include "ns3/error-model.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/ethernet-header.h"
#include "ns3/ethernet-trailer.h"
#include "ns3/double.h"
#include "mock-channel.h"
#include "mock-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MockNetDevice");

NS_OBJECT_ENSURE_REGISTERED (MockNetDevice);

TypeId
MockNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MockNetDevice")
    .SetParent<NetDevice> ()
    .SetGroupName ("Leo")
    .AddConstructor<MockNetDevice> ()
    .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                   UintegerValue (DEFAULT_MTU),
                   MakeUintegerAccessor (&MockNetDevice::SetMtu,
                                         &MockNetDevice::GetMtu),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("Address",
                   "The MAC address of this device.",
                   Mac48AddressValue (Mac48Address ("ff:ff:ff:ff:ff:ff")),
                   MakeMac48AddressAccessor (&MockNetDevice::m_address),
                   MakeMac48AddressChecker ())
    .AddAttribute ("DataRate",
                   "The default data rate for point to point links",
                   DataRateValue (DataRate ("32768b/s")),
                   MakeDataRateAccessor (&MockNetDevice::m_bps),
                   MakeDataRateChecker ())
    .AddAttribute ("ReceiveErrorModel",
                   "The receiver error model used to simulate packet loss",
                   PointerValue (),
                   MakePointerAccessor (&MockNetDevice::m_receiveErrorModel),
                   MakePointerChecker<ErrorModel> ())
    .AddAttribute ("InterframeGap",
                   "The time to wait between packet (frame) transmissions",
                   TimeValue (Seconds (0.0)),
                   MakeTimeAccessor (&MockNetDevice::m_tInterframeGap),
                   MakeTimeChecker ())
    .AddAttribute ("RxThreshold",
                   "Receive threshold in dBm",
                   DoubleValue (-1000.0),
                   MakeDoubleAccessor (&MockNetDevice::m_rxThreshold),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPower",
                   "Transmit power in dBm",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&MockNetDevice::m_txPower),
                   MakeDoubleChecker<double> ())

    //
    // Transmit queueing discipline for the device which includes its own set
    // of trace hooks.
    //
    .AddAttribute ("TxQueue",
                   "A queue to use as the transmit queue in the device.",
                   PointerValue (),
                   MakePointerAccessor (&MockNetDevice::m_queue),
                   MakePointerChecker<Queue<Packet> > ())
    //
    // Trace sources at the "top" of the net device, where packets transition
    // to/from higher layers.
    //
    .AddTraceSource ("MacTx",
                     "Trace source indicating a packet has arrived "
                     "for transmission by this device",
                     MakeTraceSourceAccessor (&MockNetDevice::m_macTxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacTxDrop",
                     "Trace source indicating a packet has been dropped "
                     "by the device before transmission",
                     MakeTraceSourceAccessor (&MockNetDevice::m_macTxDropTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacPromiscRx",
                     "A packet has been received by this device, "
                     "has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  "
                     "This is a promiscuous trace,",
                     MakeTraceSourceAccessor (&MockNetDevice::m_macPromiscRxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacRx",
                     "A packet has been received by this device, "
                     "has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  "
                     "This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&MockNetDevice::m_macRxTrace),
                     "ns3::Packet::TracedCallback")
    // Not currently implemented for this device
    .AddTraceSource ("MacRxDrop",
                     "Trace source indicating a packet was dropped "
                     "before being forwarded up the stack",
                     MakeTraceSourceAccessor (&MockNetDevice::m_macRxDropTrace),
                     "ns3::Packet::TracedCallback")
    //
    // Trace sources at the "bottom" of the net device, where packets transition
    // to/from the channel.
    //
    .AddTraceSource ("PhyTxBegin",
                     "Trace source indicating a packet has begun "
                     "transmitting over the channel",
                     MakeTraceSourceAccessor (&MockNetDevice::m_phyTxBeginTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyTxEnd",
                     "Trace source indicating a packet has been "
                     "completely transmitted over the channel",
                     MakeTraceSourceAccessor (&MockNetDevice::m_phyTxEndTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyTxDrop",
                     "Trace source indicating a packet has been "
                     "dropped by the device during transmission",
                     MakeTraceSourceAccessor (&MockNetDevice::m_phyTxDropTrace),
                     "ns3::Packet::TracedCallback")
#if 0
    // Not currently implemented for this device
    .AddTraceSource ("PhyRxBegin",
                     "Trace source indicating a packet has begun "
                     "being received by the device",
                     MakeTraceSourceAccessor (&MockNetDevice::m_phyRxBeginTrace),
                     "ns3::Packet::TracedCallback")
#endif
    .AddTraceSource ("PhyRxEnd",
                     "Trace source indicating a packet has been "
                     "completely received by the device",
                     MakeTraceSourceAccessor (&MockNetDevice::m_phyRxEndTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyRxDrop",
                     "Trace source indicating a packet has been "
                     "dropped by the device during reception",
                     MakeTraceSourceAccessor (&MockNetDevice::m_phyRxDropTrace),
                     "ns3::Packet::TracedCallback")

    //
    // Trace sources designed to simulate a packet sniffer facility (tcpdump).
    //
    .AddTraceSource ("Sniffer",
                    "Trace source simulating a non-promiscuous packet sniffer "
                     "attached to the device",
                     MakeTraceSourceAccessor (&MockNetDevice::m_snifferTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PromiscSniffer",
                     "Trace source simulating a promiscuous packet sniffer "
                     "attached to the device",
                     MakeTraceSourceAccessor (&MockNetDevice::m_promiscSnifferTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

MockNetDevice::MockNetDevice ()
  :
    m_txMachineState (READY),
    m_channel (0),
    m_linkUp (false),
    m_currentPkt (0)
{
  NS_LOG_FUNCTION (this);
}

MockNetDevice::~MockNetDevice ()
{
  NS_LOG_FUNCTION (this);
}

void
MockNetDevice::AddHeader (Ptr<Packet> p,
			  Address src,
			  Address dst,
			  uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << p << protocolNumber);
  EthernetHeader header (false);
  header.SetSource (Mac48Address::ConvertFrom (src));
  header.SetDestination (Mac48Address::ConvertFrom (dst));

  EthernetTrailer trailer;

  NS_LOG_LOGIC ("p->GetSize () = " << p->GetSize ());
  NS_LOG_LOGIC ("m_mtu = " << m_mtu);

  uint16_t lengthType = 0;

  NS_LOG_LOGIC ("Encapsulating packet as LLC (length interpretation)");

  LlcSnapHeader llc;
  llc.SetType (protocolNumber);
  p->AddHeader (llc);

  //
  // This corresponds to the length interpretation of the lengthType
  // field but with an LLC/SNAP header added to the payload as in
  // IEEE 802.2
  //
  lengthType = p->GetSize ();

  //
  // All Ethernet frames must carry a minimum payload of 46 bytes.  The
  // LLC SNAP header counts as part of this payload.  We need to padd out
  // if we don't have enough bytes.  These must be real bytes since they
  // will be written to pcap files and compared in regression trace files.
  //
  if (p->GetSize () < 46)
    {
      uint8_t buffer[46];
      memset (buffer, 0, 46);
      Ptr<Packet> padd = Create<Packet> (buffer, 46 - p->GetSize ());
      p->AddAtEnd (padd);
    }

  NS_LOG_LOGIC ("header.SetLengthType (" << lengthType << ")");
  header.SetLengthType (lengthType);
  p->AddHeader (header);

  if (Node::ChecksumEnabled ())
    {
      trailer.EnableFcs (true);
    }
  trailer.CalcFcs (p);
  p->AddTrailer (trailer);
}

void
MockNetDevice::DoInitialize (void)
{
  if (m_queueInterface)
    {
      NS_ASSERT_MSG (m_queue != nullptr, "A Queue object has not been attached to the device");

      // connect the traced callbacks of m_queue to the static methods provided by
      // the NetDeviceQueue class to support flow control and dynamic queue limits.
      // This could not be done in NotifyNewAggregate because at that time we are
      // not guaranteed that a queue has been attached to the netdevice
      m_queueInterface->GetTxQueue (0)->ConnectQueueTraces (m_queue);
    }

  NetDevice::DoInitialize ();
}

void
MockNetDevice::NotifyNewAggregate (void)
{
  NS_LOG_FUNCTION (this);
  if (m_queueInterface == nullptr)
    {
      Ptr<NetDeviceQueueInterface> ndqi = this->GetObject<NetDeviceQueueInterface> ();
      //verify that it's a valid netdevice queue interface and that
      //the netdevice queue interface was not set before
      if (ndqi != nullptr)
        {
          m_queueInterface = ndqi;
        }
    }
  NetDevice::NotifyNewAggregate ();
}

void
MockNetDevice::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_node = 0;
  m_channel = 0;
  m_receiveErrorModel = 0;
  m_currentPkt = 0;
  m_queue = 0;
  m_queueInterface = 0;
  NetDevice::DoDispose ();
}

void
MockNetDevice::SetDataRate (DataRate bps)
{
  NS_LOG_FUNCTION (this);
  m_bps = bps;
}

void
MockNetDevice::SetInterframeGap (Time t)
{
  NS_LOG_FUNCTION (this << t.GetSeconds ());
  m_tInterframeGap = t;
}

bool
MockNetDevice::TransmitStart (Ptr<Packet> p, const Address &dest)
{
  NS_LOG_FUNCTION (this << p);
  NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");

  //
  // This function is called to start the process of transmitting a packet.
  // We need to tell the channel that we've started wiggling the wire and
  // schedule an event that will be executed when the transmission is complete.
  //
  NS_ASSERT_MSG (m_txMachineState == READY, "Must be READY to transmit");
  m_txMachineState = BUSY;
  m_currentPkt = p;
  m_phyTxBeginTrace (m_currentPkt);

  Time txTime = m_bps.CalculateBytesTxTime (p->GetSize ());
  Time txCompleteTime = txTime + m_tInterframeGap;

  NS_LOG_LOGIC ("Schedule TransmitCompleteEvent in " << txCompleteTime.GetNanoSeconds () << " nsec");
  Simulator::Schedule (txCompleteTime, &MockNetDevice::TransmitComplete, this, dest);

  bool result = m_channel->TransmitStart (p, m_channelDevId, dest, txTime);
  if (result == false)
    {
      m_phyTxDropTrace (p);
    }
  else
    {
      NS_LOG_INFO ("[node " << m_node->GetId () << "] send packet on " << m_ifIndex << " to " << dest);
    }
  return result;
}

void
MockNetDevice::TransmitComplete (const Address &dest)
{
  NS_LOG_FUNCTION (this);

  //
  // This function is called to when we're all done transmitting a packet.
  // We try and pull another packet off of the transmit queue.  If the queue
  // is empty, we are done, otherwise we need to start transmitting the
  // next packet.
  //
  NS_ASSERT_MSG (m_txMachineState == BUSY, "Must be BUSY if transmitting");
  m_txMachineState = READY;

  NS_ASSERT_MSG (m_currentPkt != nullptr, "MockNetDevice::TransmitComplete(): m_currentPkt zero");

  m_phyTxEndTrace (m_currentPkt);
  m_currentPkt = nullptr;

  Ptr<Packet> p = m_queue->Dequeue ();
  if (p == nullptr)
    {
      NS_LOG_LOGIC ("No pending packets in device queue after tx complete");
      return;
    }

  //
  // Got another packet off of the queue, so start the transmit process again.
  //
  m_snifferTrace (p);
  m_promiscSnifferTrace (p);
  TransmitStart (p, dest);
}

bool
MockNetDevice::Attach (Ptr<MockChannel> ch)
{
  NS_LOG_FUNCTION (this << &ch);

  m_channel = ch;

  m_channelDevId = m_channel->Attach (this);

  //
  // This device is up whenever it is attached to a channel.  A better plan
  // would be to have the link come up when both devices are attached, but this
  // is not done for now.
  //
  NotifyLinkUp ();
  return true;
}

void
MockNetDevice::SetQueue (Ptr<Queue<Packet> > q)
{
  NS_LOG_FUNCTION (this << q);
  m_queue = q;
}

void
MockNetDevice::SetReceiveErrorModel (Ptr<ErrorModel> em)
{
  NS_LOG_FUNCTION (this << em);
  m_receiveErrorModel = em;
}

double
MockNetDevice::DoCalcRxPower (double rxPower) const
{
  return rxPower;
}

void
MockNetDevice::Receive (Ptr<Packet> packet,
			Ptr<MockNetDevice> senderDevice,
			double rxPower)
{
  NS_LOG_FUNCTION (this << packet << senderDevice << rxPower);

  if (senderDevice == this)
    {
      m_macRxDropTrace (packet);
      return;
    }

  m_phyRxEndTrace (packet);

  rxPower = DoCalcRxPower (rxPower);

  if (rxPower < m_rxThreshold)
    {
      // Received power is below threshold
      m_phyRxDropTrace (packet);
      return;
    }

  if (m_receiveErrorModel && m_receiveErrorModel->IsCorrupt (packet) )
    {
      //
      // If we have an error model and it indicates that it is time to lose a
      // corrupted packet, don't forward this packet up, let it go.
      //
      m_phyRxDropTrace (packet);

      return;
    }

  //
  // Trace sinks will expect complete packets, not packets without some of the
  // headers.
  //
  Ptr<Packet> originalPacket = packet->Copy ();

  EthernetTrailer trailer;
  packet->RemoveTrailer (trailer);
  if (Node::ChecksumEnabled ())
    {
      trailer.EnableFcs (true);
    }

  bool crcGood = trailer.CheckFcs (packet);
  if (!crcGood)
    {
      NS_LOG_INFO ("CRC error on Packet " << packet);
      m_phyRxDropTrace (packet);
      return;
    }

  EthernetHeader header;
  packet->RemoveHeader (header);

  uint16_t protocol;

  if (header.GetLengthType () <= 1500)
    {
      NS_ASSERT (packet->GetSize () >= header.GetLengthType ());
      uint32_t padlen = packet->GetSize () - header.GetLengthType ();
      NS_ASSERT (padlen <= 46);
      if (padlen > 0)
        {
          packet->RemoveAtEnd (padlen);
        }

      LlcSnapHeader llc;
      packet->RemoveHeader (llc);
      protocol = llc.GetType ();
    }
  else
    {
      protocol = header.GetLengthType ();
    }

  PacketType packetType;
  if (header.GetDestination ().IsBroadcast ())
    {
      packetType = PACKET_BROADCAST;
    }
  else if (header.GetDestination () == m_address)
    {
      packetType = PACKET_HOST;
    }
  else if (header.GetDestination ().IsGroup ())
    {
      packetType = PACKET_MULTICAST;
    }
  else
    {
      packetType = PACKET_OTHERHOST;
    }

  m_promiscSnifferTrace (originalPacket);
  if (!m_promiscCallback.IsNull ())
    {
      m_macPromiscRxTrace (originalPacket);
      m_promiscCallback (this, packet, protocol, header.GetSource (), header.GetDestination (), packetType);
    }

  if (packetType != PACKET_OTHERHOST) {
      NS_LOG_INFO ("[node " << m_node->GetId () << "] received packet on " << m_ifIndex << " from " << header.GetSource () << " for " << header.GetDestination ());
      m_macRxTrace (originalPacket);
      m_rxCallback (this, packet, protocol, header.GetSource ());
  }
}

Ptr<Queue<Packet> >
MockNetDevice::GetQueue (void) const
{
  NS_LOG_FUNCTION (this);
  return m_queue;
}

void
MockNetDevice::NotifyLinkUp (void)
{
  NS_LOG_FUNCTION (this);
  m_linkUp = true;
  m_linkChangeCallbacks ();
}

void
MockNetDevice::NotifyLinkDown (void)
{
  NS_LOG_FUNCTION (this);
  m_linkUp = false;
  m_linkChangeCallbacks ();
}

void
MockNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this);
  m_ifIndex = index;
}

uint32_t
MockNetDevice::GetIfIndex (void) const
{
  return m_ifIndex;
}

Ptr<Channel>
MockNetDevice::GetChannel (void) const
{
  return m_channel;
}

//
// This is a point-to-point device, so we really don't need any kind of address
// information.  However, the base class NetDevice wants us to define the
// methods to get and set the address.  Rather than be rude and assert, we let
// clients get and set the address, but simply ignore them.

void
MockNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_address = Mac48Address::ConvertFrom (address);
}

Address
MockNetDevice::GetAddress (void) const
{
  return m_address;
}

bool
MockNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);
  return m_linkUp;
}

void
MockNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (this);
  m_linkChangeCallbacks.ConnectWithoutContext (callback);
}

bool
MockNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

//
// We don't really need any addressing information since this is a
// point-to-point device.  The base class NetDevice wants us to return a
// broadcast address, so we make up something reasonable.
//
Address
MockNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}

bool
MockNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

Address
MockNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address ("01:00:5e:00:00:00");
}

Address
MockNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this << addr);
  return Mac48Address ("33:33:00:00:00:00");
}

bool
MockNetDevice::IsMock (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

bool
MockNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool
MockNetDevice::Send (Ptr<Packet> packet,
  		     const Address &dest,
  		     uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);
  NS_LOG_LOGIC ("p=" << packet << ", dest=" << &dest);
  NS_LOG_LOGIC ("UID is " << packet->GetUid ());

  //
  // If IsLinkUp() is false it means there is no channel to send any packet
  // over so we just hit the drop trace on the packet and return an error.
  //
  if (IsLinkUp () == false)
    {
      m_macTxDropTrace (packet);
      return false;
    }

  Mac48Address destination = Mac48Address::ConvertFrom (dest);
  Mac48Address source = Mac48Address::ConvertFrom (m_address);
  AddHeader (packet, source, destination, protocolNumber);

  m_macTxTrace (packet);

  //
  // We should enqueue and dequeue the packet to hit the tracing hooks.
  //
  if (m_queue->Enqueue (packet))
    {
      //
      // If the channel is ready for transition we send the packet right now
      //
      if (m_txMachineState == READY)
        {
          packet = m_queue->Dequeue ();
          m_promiscSnifferTrace (packet);
          m_snifferTrace (packet);
          TransmitStart (packet, dest);
        }
      else
      	{
      	  NS_LOG_LOGIC (this << "channel not ready, postponing transmit start: " << m_queue->GetCurrentSize () << "/" << m_queue->GetMaxSize ());
      	}
      return true;
    }

  // Enqueue may fail (overflow)
  NS_LOG_WARN ("queue overflowed: " << m_queue->GetCurrentSize () << "/" << m_queue->GetMaxSize ());

  m_macTxDropTrace (packet);
  return false;
}

bool
MockNetDevice::SendFrom (Ptr<Packet> packet,
                                 const Address &source,
                                 const Address &dest,
                                 uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);
  return false;
}

Ptr<Node>
MockNetDevice::GetNode (void) const
{
  return m_node;
}

void
MockNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);
  m_node = node;
}

bool
MockNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

void
MockNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  m_rxCallback = cb;
}

void
MockNetDevice::SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb)
{
  m_promiscCallback = cb;
}

bool
MockNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

void
MockNetDevice::DoMpiReceive (Ptr<Packet> p, Ptr<MockNetDevice> senderDevice, double txPower)
{
  NS_LOG_FUNCTION (this << p);
  Receive (p, senderDevice, txPower);
}

Address
MockNetDevice::GetRemote (Ptr<MockNetDevice> senderDevice) const
{
  NS_LOG_FUNCTION (this);
  return senderDevice->GetAddress ();
}

bool
MockNetDevice::SetMtu (uint16_t mtu)
{
  NS_LOG_FUNCTION (this << mtu);
  m_mtu = mtu;
  return true;
}

uint16_t
MockNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mtu;
}

bool
MockNetDevice::IsPointToPoint() const
{
  return false;
}

double
MockNetDevice::GetTxPower () const
{
  return m_txPower;
}

void
MockNetDevice::SetTxPower (double txPower)
{
  m_txPower = txPower;
}

double
MockNetDevice::GetRxTreshold () const
{
  return m_rxThreshold;
}

void
MockNetDevice::SetRxThreshold (double rxThresh)
{
  m_rxThreshold = rxThresh;
}

} // namespace ns3
