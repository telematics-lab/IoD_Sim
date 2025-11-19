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

#ifndef LEO_MOCK_NET_DEVICE_H_
#define LEO_MOCK_NET_DEVICE_H_

#include "mock-net-device.h"

/**
 * \file
 * \ingroup leo
 */

namespace ns3
{
/**
 * \ingroup leo
 * \brief A satellite-ground communication device with a type
 */
class LeoMockNetDevice : public MockNetDevice
{
public:
  /**
   * \brief Type of the device. Can either be on ground or in space.
   */
  enum DeviceType
  {
    /// Device is on the ground
    GND,
    /// Device is in space
    SAT
  };

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /// constructor
  LeoMockNetDevice ();

  /// destructor
  virtual ~LeoMockNetDevice ()
  {
  }

  /**
   * \brief Get the type of this device
   * \return device type
   */
  DeviceType GetDeviceType () const;

  /**
   * \brief Set the type of this device
   * \return device type
   */
  void SetDeviceType (DeviceType deviceType);

protected:
  virtual double DoCalcRxPower (double rxPower) const;

private:
  /// Device type
  DeviceType m_deviceType;
  /// Receiver loss
  double m_rxLoss;
  /// Receiver gain
  double m_rxGain;
};

};

#endif /* LEO_MOCK_NET_DEVICE_H_ */
