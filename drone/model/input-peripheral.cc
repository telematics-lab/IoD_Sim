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
#include <ns3/integer.h>
#include <ns3/simulator.h>
#include "input-peripheral.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("InputPeripheral");

NS_OBJECT_ENSURE_REGISTERED (InputPeripheral);

TypeId
InputPeripheral::GetTypeId()
{
  static TypeId tid = TypeId ("ns3::InputPeripheral")
    .SetParent<DronePeripheral> ()
    .SetGroupName ("Peripheral")
    .AddConstructor<InputPeripheral> ()
    .AddAttribute("DataRate", "The acquisition data rate of the peripheral in bit",
                  DoubleValue (0),
                  MakeDoubleAccessor (&InputPeripheral::m_dataRate),
                  MakeDoubleChecker<double> (0))
    .AddAttribute("DataAcquisitionTimeInterval", "The time interval occurring between any data acquisition",
                  TimeValue (Seconds(1.0)),
                  MakeTimeAccessor (&InputPeripheral::m_acquisitionTimeInterval),
                  MakeTimeChecker ())
    ;
  return tid;
}

InputPeripheral::InputPeripheral()
{
}

void
InputPeripheral::DoInitialize(void)
{
  DronePeripheral::DoInitialize();
}

void
InputPeripheral::DoDispose(void)
{
  DronePeripheral::DoDispose();
}

void
InputPeripheral::SetStorage(Ptr<StoragePeripheral> storage)
{
  NS_ASSERT (storage != NULL);
  NS_ASSERT(storage->GetDrone() == this->GetDrone());
  m_storage = storage;
}

void
InputPeripheral::Install(Ptr<StoragePeripheral> storage, Ptr<Drone> drone)
{
  SetDrone(drone);
  SetStorage(storage);
  AcquireData();
}

void
InputPeripheral::AcquireData(void)
{
  if (Simulator::IsFinished ()) return;
  m_dataAcquisitionEvent.Cancel();
  if (Simulator::Now().GetMilliSeconds() >= m_acquisitionTimeInterval.GetMilliSeconds())
    {
      m_storage->Alloc(m_dataRate * m_acquisitionTimeInterval.GetMilliSeconds()/1000, StoragePeripheral::bit);
    }
  m_dataAcquisitionEvent = Simulator::Schedule(m_acquisitionTimeInterval, &InputPeripheral::AcquireData, this);
}

} // namespace ns3