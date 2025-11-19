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

#ifndef LEO_MOCK_CHANNEL_H_
#define LEO_MOCK_CHANNEL_H_

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
#include "mock-channel.h"

/**
 * \file
 * \ingroup leo
 *
 * Declaration of LeoMockChannl
 */

namespace ns3 {

/**
 * \ingroup leo
 * \brief Channel between satellite and gateway
 *
 * Delivers packets to all attached devices on opposing site (satellite to
 * gateway and vice-versa)
 *
 * Usually used together with LeoPropagationLossModel and LeoPropagationDelay.
 */
class LeoMockChannel : public MockChannel
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /// constructor
  LeoMockChannel ();
  /// destructor
  virtual ~LeoMockChannel ();

  /**
   * \see MockChannel::TransmitStart
   *
   * \brief A packet is transmitted if the destination is reachable via the beam.
   */
  virtual bool TransmitStart (Ptr<const Packet> p, uint32_t devId, Address dst, Time txTime);

  virtual int32_t Attach (Ptr<MockNetDevice> device);
  virtual bool Detach (uint32_t deviceId);

private:
  /**
   * \brief Ground and satellite devices
   *
   * This channel does not allow for communication between devices of the same
   * type (no sat-sat or ground-ground).
   */
  typedef std::map<Address, Ptr<MockNetDevice> > DeviceIndex;

  /// Devices that are on the ground (gateways)
  DeviceIndex m_groundDevices;

  /// Devices that are in space (satellites)
  DeviceIndex m_satelliteDevices;
}; // class MockChannel

} // namespace ns3

#endif /* LEO_MOCK_CHANNEL_H */
