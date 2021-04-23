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
#include "drone-peripheral.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DronePeripheral");

NS_OBJECT_ENSURE_REGISTERED (DronePeripheral);

TypeId
DronePeripheral::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::DronePeripheral")
          .SetParent<Object> ()
          .SetGroupName ("Peripheral")
          .AddConstructor<DronePeripheral> ()
          .AddAttribute ("PowerConsumption", "The power consumption of the peripheral in J/s",
                         DoubleValue (0),
                         MakeDoubleAccessor (&DronePeripheral::m_powerConsumption),
                         MakeDoubleChecker<double> ())
          ;
  return tid;
}

DronePeripheral::DronePeripheral ()
{
}

void
DronePeripheral::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void
DronePeripheral::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  Object::DoInitialize ();
}

void
DronePeripheral::SetDrone (Ptr<Drone> drone)
{
  NS_LOG_FUNCTION (this << drone);
  NS_ASSERT (drone != NULL);
  m_drone = drone;
}

Ptr<Drone>
DronePeripheral::GetDrone (void)
{
  return m_drone;
}

double
DronePeripheral::GetPowerConsumption (void)
{
  NS_LOG_FUNCTION (this);
  return m_powerConsumption;
}

} //namespace ns3
