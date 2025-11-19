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

#ifndef ISL_CHANNEL_H
#define ISL_CHANNEL_H

#include <string>
#include <stdint.h>

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/channel.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/time-data-calculators.h"
#include "ns3/traced-callback.h"
#include "ns3/mobility-module.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/propagation-loss-model.h"
#include "mock-net-device.h"
#include "mock-channel.h"

/**
 * \file
 * \ingroup leo
 * IslMockChannel declaration
 */

namespace ns3 {

class MockNetDevice;

/**
 * \ingroup leo
 * \brief Simplified model of an inter-satellite channel
 *
 * This channel distributes packets to all attached devies to which the sender has a line-of-sight.
 * It is typically used in conjunction with the IslPropagationLossModel and ConstantSpeedPropagationDelay.
 *
 */
class IslMockChannel : public MockChannel
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /// constructor
  IslMockChannel ();
  /// destructor
  virtual ~IslMockChannel ();

  /**
   * \brief Starts a transmission of a packet
   * \return true iff the transmission was successful
   * \param p the packet to transmit
   * \param devId the id of the sender device as an index into the attached
   * devices
   * \param dst the destination address
   * \param txTime the transmission delay of the packet
   */
  bool TransmitStart (Ptr<const Packet> p, uint32_t devId, Address dst, Time txTime);

private:
  std::vector<Ptr<MockNetDevice> > m_link; ///< Attached devices


}; // class MockChannel

} // namespace ns3

#endif /* ISL_CHANNEL_H */
