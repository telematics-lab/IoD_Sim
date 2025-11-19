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

#ifndef MOCK_CHANNEL_H
#define MOCK_CHANNEL_H

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

/**
 * \file
 * \ingroup leo
 * Declaration of class MockNetDevice
 */

namespace ns3 {

class MockNetDevice;

/**
 * \ingroup leo
 * \brief Base class for LeoMockChannel and IslMockChannel
 */
class MockChannel : public Channel
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /// constructor
  MockChannel ();
  /// destructor
  virtual ~MockChannel ();

  /**
   * \brief Get device at index i
   * \param i index
   * \return pointer to device
   */
  Ptr<NetDevice> GetDevice (std::size_t i) const;

  /**
   * \brief Attach a device to the channel.
   * \param device Device to attach to the channel
   * \return Index of the device inside the devices list
   */
  virtual int32_t Attach (Ptr<MockNetDevice> device);

  /**
   * \brief Detach a given netdevice from this channel
   * \param device pointer to the netdevice to detach from the channel
   * \return true on success, false on failure
   */
  virtual bool Detach (uint32_t deviceId);

  /**
   * \brief Get number of devices in channel
   * \return number of devices
   */
  std::size_t GetNDevices (void) const;

  /**
   * \brief Start to transmit a packet
   *
   * Subclasses must implement this.
   */
  virtual bool TransmitStart (Ptr<const Packet> p, uint32_t devId, Address dst, Time txTime) = 0;

  /**
   * \brief Get the propagation loss model
   * \return propagation loss in dBm
   */
  Ptr<PropagationLossModel> GetPropagationLoss (void) const;

  /**
   * \brief Set the propagation loss model
   * \param model propagation loss
   */
  void SetPropagationLoss (Ptr<PropagationLossModel> model);

  /**
   * \brief Get the propagation delay model
   * \return propagation delay
   */
  Ptr<PropagationDelayModel> GetPropagationDelay (void) const;

  /**
   * \brief Set the propagation delay model
   * \param delay propagation delay
   */
  void SetPropagationDelay (Ptr<PropagationDelayModel> delay);

protected:
  TracedCallback<Ptr<const Packet>,     // Packet being transmitted
                 Ptr<NetDevice>,  // Transmitting NetDevice
                 Ptr<NetDevice>,  // Receiving NetDevice
                 Time,                  // Amount of time to transmit the pkt
                 Time                   // Last bit receive time (relative to now)
                 > m_txrxMock;

  /**
   * \brief Get the propagation delay associated with this channel
   * \param first mobility model
   * \param second mobility model
   * \param txTime time of the transmission
   * \returns Propagation time delay
   */
  Time GetPropagationDelay (Ptr<MobilityModel> first, Ptr<MobilityModel> second, Time txTime) const;

  /**
   * \brief Get a device by it's address
   * \param addr address of the device
   * \return pointer to the device if it is attached to the channel, null otherwise
   */
  Ptr<MockNetDevice> GetDevice (Address &addr) const;

  /**
   * \brief Deliver a packet to a destination
   * \param p packet
   * \param src source of a packet
   * \param dst destination of a packet
   * \param txTime transmission time of the packet
   * \return true iff the transmission has been successful
   */
  bool Deliver ( Ptr<const Packet> p, Ptr<MockNetDevice> src, Ptr<MockNetDevice> dst, Time txTime);

private:

  /// All devices that are attached to the channel
  std::vector<Ptr<MockNetDevice> > m_link;

  /// Propagation delay model to be used with this channel
  Ptr<PropagationDelayModel> m_propagationDelay;

  /// Propagation loss model to be used with this channel
  Ptr<PropagationLossModel> m_propagationLoss;

}; // class MockChannel

} // namespace ns3

#endif /* MOCK_CHANNEL_H */
