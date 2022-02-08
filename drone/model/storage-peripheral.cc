/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2022 The IoD_Sim Authors.
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
#include "storage-peripheral.h"
#include <ns3/integer.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("StoragePeripheral");

NS_OBJECT_ENSURE_REGISTERED (StoragePeripheral);

TypeId
StoragePeripheral::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::StoragePeripheral")
          .SetParent<DronePeripheral> ()
          .SetGroupName ("Peripheral")
          .AddConstructor<StoragePeripheral> ()
          .AddAttribute ("Capacity", "The capacity of the disk in bit",
                         UintegerValue (8000000), //1MByte
                         MakeUintegerAccessor (&StoragePeripheral::SetCapacity,
                                               &StoragePeripheral::GetCapacity),
                         MakeUintegerChecker<uint64_t> ())
          .AddAttribute ("InitialRemainingCapacity", "The initial capacity of the disk in bit",
                         UintegerValue (8000000), //1MByte
                         MakeUintegerAccessor (&StoragePeripheral::m_remainingCapacity),
                         MakeUintegerChecker<uint64_t> ())
          .AddTraceSource ("RemainingCapacity", "Remaining Capacity at Storage Peripheral.",
                           MakeTraceSourceAccessor (&StoragePeripheral::m_remainingCapacity),
                           "ns3::TracedValueCallback::Uint64")
          ;
  return tid;
}

StoragePeripheral::StoragePeripheral ()
{
}

void
StoragePeripheral::SetCapacity (uint64_t cap)
{
  m_capacity = cap;
  m_remainingCapacity = m_capacity;
}

uint64_t
StoragePeripheral::GetCapacity () const
{
  return m_capacity;
}

void
StoragePeripheral::DoInitialize (void)
{
  DronePeripheral::DoInitialize ();
}

void
StoragePeripheral::DoDispose ()
{
  DronePeripheral::DoDispose ();
}

bool
StoragePeripheral::Alloc (uint64_t amount, unit amountUnit)
{
  if (GetState() != ON) {NS_LOG_DEBUG ("StoragePeripheral: Alloc opeation not possible."); return false;}
  NS_LOG_FUNCTION (this << amount * amountUnit);
  if (amount * amountUnit <= m_remainingCapacity)
    {
      m_remainingCapacity -= amount * amountUnit;
      NS_LOG_DEBUG ("StoragePeripheral:Stored memory on Drone #"
                    << GetDrone ()->GetId () << ": " << m_capacity - m_remainingCapacity << " bits");
      return true;
    }
  else
    {
      NS_LOG_INFO ("StoragePeripheral:Not enough memory on Drone #" << GetDrone ()->GetId ());
      return false;
    }
}

bool
StoragePeripheral::Free (uint64_t amount, unit amountUnit)
{
  if (GetState() != ON) {NS_LOG_DEBUG ("StoragePeripheral: Free opeation not possible."); return false;}
  NS_LOG_FUNCTION (this << amount * amountUnit);
  if (amount * amountUnit <= m_capacity - m_remainingCapacity)
  {
    m_remainingCapacity += amount * amountUnit;
    NS_LOG_DEBUG ("StoragePeripheral:Stored memory on Drone #"
                    << GetDrone ()->GetId () << ": " << m_capacity - m_remainingCapacity << " bits");
    return true;
  } else {
    NS_LOG_INFO ("StoragePeripheral:Impossible to free more memory than occupied on Drone #"
                 << GetDrone ()->GetId ());
    return false;
  }
}

} // namespace ns3
