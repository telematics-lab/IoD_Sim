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
#include "ns3/enum.h"
#include "ns3/double.h"

#include "leo-mock-net-device.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("LeoMockNetDevice");

NS_OBJECT_ENSURE_REGISTERED (LeoMockNetDevice);

TypeId
LeoMockNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LeoMockNetDevice")
    .SetParent<MockNetDevice> ()
    .SetGroupName ("Leo")
    .AddConstructor<LeoMockNetDevice> ()
    .AddAttribute ("DeviceType", "The type of the mocked device",
                   EnumValue<DeviceType> (DeviceType::GND),
                   MakeEnumAccessor<DeviceType> (&LeoMockNetDevice::m_deviceType),
                   MakeEnumChecker (
                     DeviceType::GND, "ns3::LeoMockNetDevice::NetDeviceType::GND",
                     DeviceType::SAT, "ns3::LeoMockNetDevice::NetDeviceType::SAT"))
    .AddAttribute ("RxGain",
                   "Receiver gain in dBm",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&LeoMockNetDevice::m_rxGain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxLoss",
                   "Receiver loss in dBm",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&LeoMockNetDevice::m_rxLoss),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
};

LeoMockNetDevice::LeoMockNetDevice ()
{
}

LeoMockNetDevice::DeviceType
LeoMockNetDevice::GetDeviceType () const
{
  return m_deviceType;
}

void
LeoMockNetDevice::SetDeviceType (LeoMockNetDevice::DeviceType deviceType)
{
  m_deviceType = deviceType;
}

double
LeoMockNetDevice::DoCalcRxPower (double rxPower) const
{
  return rxPower - m_rxLoss + m_rxGain;
}

}; /* namspace ns3 */
