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

#include <ns3/trace-source-accessor.h>
#include <ns3/packet.h>
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/enum.h>
#include "mock-channel.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MockChannel");

NS_OBJECT_ENSURE_REGISTERED (MockChannel);

TypeId
MockChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MockChannel")
    .SetParent<Channel> ()
    .SetGroupName ("Leo")
    .AddAttribute ("PropagationDelay",
                   "A propagation delay model for the channel.",
                   PointerValue (),
                   MakePointerAccessor (&MockChannel::m_propagationDelay),
                   MakePointerChecker<PropagationDelayModel> ())
    .AddAttribute ("PropagationLoss",
                   "A propagation loss model for the channel.",
                   PointerValue (),
                   MakePointerAccessor (&MockChannel::m_propagationLoss),
                   MakePointerChecker<PropagationLossModel> ())
    .AddTraceSource ("TxRxMockChannel",
                     "Trace source indicating transmission of packet "
                     "from the MockChannel, used by the Animation "
                     "interface.",
                     MakeTraceSourceAccessor (&MockChannel::m_txrxMock),
                     "ns3::MockChannel::TxRxAnimationCallback")
    ;
  return tid;
}

//
// By default, you get a channel that
// has an "infitely" fast transmission speed and zero processing delay.
MockChannel::MockChannel() : Channel (), m_link (0)
{
  NS_LOG_FUNCTION_NOARGS ();
}

MockChannel::~MockChannel()
{
}

bool
MockChannel::Detach (uint32_t deviceId)
{
  NS_LOG_FUNCTION (this << deviceId);
  if (deviceId < m_link.size ())
    {
      if (!m_link[deviceId]->IsLinkUp ())
    	{
      	  NS_LOG_WARN ("MockChannel::Detach(): Device is already detached (" << deviceId << ")");
      	  return false;
    	}

      m_link[deviceId]->NotifyLinkDown ();
    }
  else
    {
      return false;
    }
  return true;
}

int32_t
MockChannel::Attach (Ptr<MockNetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  NS_ASSERT (device != nullptr);
  m_link.push_back(device);
  return  m_link.size() - 1;
}

std::size_t
MockChannel::GetNDevices (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_link.size ();
}

Ptr<NetDevice>
MockChannel::GetDevice (std::size_t i) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_link[i];
}

Time
MockChannel::GetPropagationDelay (Ptr<MobilityModel> srcMob, Ptr<MobilityModel> dstMob, Time txTime) const
{
  NS_LOG_FUNCTION (this << srcMob << dstMob << txTime);

  if (GetPropagationDelay ())
    {
      Time propagationDelay = m_propagationDelay->GetDelay (srcMob, dstMob);
      return propagationDelay;
    }
  else
    {
      return Time (0);
    }
}

// TODO optimize
Ptr<MockNetDevice>
MockChannel::GetDevice (Address &addr) const
{
  for (Ptr<MockNetDevice> dev : m_link)
    {
      if (dev->GetAddress () == addr)
    	{
      	  return dev;
    	}
    }

  return 0;
}

Ptr<PropagationDelayModel>
MockChannel::GetPropagationDelay () const
{
  return m_propagationDelay;
}

Ptr<PropagationLossModel>
MockChannel::GetPropagationLoss () const
{
  return m_propagationLoss;
}

void
MockChannel::SetPropagationLoss (Ptr<PropagationLossModel> model)
{
  m_propagationLoss = model;
}

bool
MockChannel::Deliver (
    		    Ptr<const Packet> p,
    		    Ptr<MockNetDevice> src,
    		    Ptr<MockNetDevice> dst,
    		    Time txTime)
{
  NS_LOG_FUNCTION (this << p << src->GetAddress () << dst->GetAddress () << txTime);
  Time delay = txTime;

  Ptr<MobilityModel> srcMob = src->GetNode ()->GetObject<MobilityModel> ();
  Ptr<MobilityModel> dstMob = dst->GetNode ()->GetObject<MobilityModel> ();

  double txPower = src->GetTxPower ();
  double rxPower = txPower;

  if (srcMob != nullptr && dstMob != nullptr)
    {
      Ptr<PropagationLossModel> pLoss = GetPropagationLoss ();
      if (pLoss != nullptr)
    	{
      	  // check if signal reaches destination
      	  rxPower = pLoss->CalcRxPower (txPower, srcMob, dstMob);
      	  if (rxPower < -900.0)
    	    {
      	      NS_LOG_WARN (this << "unable to reach destination " << dst->GetNode ()->GetId () << " from " << src->GetNode ()->GetId ());
      	      return false;
    	    }
    	}
      delay = GetPropagationDelay (srcMob, dstMob, txTime);
      NS_LOG_DEBUG ("delay = "<<delay);
    }

  Simulator::ScheduleWithContext (dst->GetNode ()->GetId (),
        			  delay,
        			  &MockNetDevice::Receive,
        			  dst,
        			  p->Copy (),
        			  src,
        			  rxPower);

  // Call the tx anim callback on the net device
  m_txrxMock (p, src, dst, txTime, delay);
  return true;
}

void
MockChannel::SetPropagationDelay (Ptr<PropagationDelayModel> delay)
{
  m_propagationDelay = delay;
}

} // namespace ns3
