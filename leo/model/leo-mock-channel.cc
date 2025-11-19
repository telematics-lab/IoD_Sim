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

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/enum.h"

#include "leo-mock-net-device.h"
#include "leo-mock-channel.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LeoMockChannel");

NS_OBJECT_ENSURE_REGISTERED (LeoMockChannel);

TypeId
LeoMockChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LeoMockChannel")
    .SetParent<MockChannel> ()
    .SetGroupName ("Leo")
    .AddConstructor<LeoMockChannel> ()
  ;
  return tid;
}

LeoMockChannel::LeoMockChannel() :
  MockChannel ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

LeoMockChannel::~LeoMockChannel()
{
}

bool
LeoMockChannel::TransmitStart (Ptr<const Packet> p,
                          uint32_t devId,
                          Address dst,
                          Time txTime)
{
  NS_LOG_FUNCTION (this << p << devId << dst << txTime);

  // Find devices joined to channel
  if (devId >= GetNDevices ())
    {
      NS_LOG_ERROR ("Source device unknown");
      return false;
    }

  Ptr<MockNetDevice> srcDev = StaticCast<MockNetDevice> (GetDevice (devId));
  if (srcDev == nullptr)
    {
      NS_LOG_ERROR ("Source device unknown");
      return false;
    }

  bool fromGround = m_groundDevices.find (srcDev->GetAddress ()) != m_groundDevices.end ();
  bool fromSpace = m_satelliteDevices.find (srcDev->GetAddress ()) != m_satelliteDevices.end ();

  NS_ASSERT_MSG (!(fromGround && fromSpace), "Source device can not be both on ground and in space");

  DeviceIndex *dests;
  if (fromGround)
    {
      NS_LOG_LOGIC ("ground to space: " << srcDev->GetAddress () << " to " << dst);
      dests = &m_satelliteDevices;
    }
  else if (fromSpace)
    {
      NS_LOG_LOGIC ("space to ground: " << srcDev->GetAddress () << " to " << dst);
      dests = &m_groundDevices;
    }
  else
    {
      NS_LOG_ERROR ("unable to find source interface " << srcDev);
      return false;
    }

  // make sure to return false if packet has been delivered to *no* device
  bool result = false;
  for (DeviceIndex::iterator it = dests->begin (); it != dests->end(); it ++)
    {
      if (Deliver (p, srcDev, it->second, txTime))
      	{
      	  result = true;
      	}
    }
  return result;
}

int32_t
LeoMockChannel::Attach (Ptr<MockNetDevice> device)
{
  Ptr<LeoMockNetDevice> leodev = StaticCast<LeoMockNetDevice> (device);

  // Add to index
  switch (leodev->GetDeviceType ())
    {
    case LeoMockNetDevice::DeviceType::GND:
      m_groundDevices[leodev->GetAddress ()] = leodev;
      break;
    case LeoMockNetDevice::DeviceType::SAT:
      m_satelliteDevices[leodev->GetAddress ()] = leodev;
      break;
    default:
      break;
    }

  return MockChannel::Attach (device);
}

bool
LeoMockChannel::Detach (uint32_t deviceId)
{
  Ptr<NetDevice> dev = GetDevice (deviceId);
  m_groundDevices.erase (dev->GetAddress ());
  m_satelliteDevices.erase (dev->GetAddress ());

  return MockChannel::Detach (deviceId);
}

}; // namespace ns3
