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

#include "drone-peripheral-container.h"
#include "drone-peripheral.h"
#include "storage-peripheral.h"
#include "input-peripheral.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DronePeripheralContainer");

NS_OBJECT_ENSURE_REGISTERED (DronePeripheralContainer);

TypeId
DronePeripheralContainer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DronePeripheralContainer")
    .SetParent<Object> ()
    .SetGroupName("Network")
    .AddConstructor<DronePeripheralContainer> ()
    ;
  return tid;
}

void
DronePeripheralContainer::Add (const std::string typeId)
{
  m_dronePeripheralFactory.SetTypeId(typeId);
}

void
DronePeripheralContainer::Set (std::string name, const AttributeValue &v)
{
  m_dronePeripheralFactory.Set (name, v);
}

void
DronePeripheralContainer::SetDrone (Ptr<Drone> drone)
{
  NS_LOG_FUNCTION (this << drone);
  NS_ASSERT (drone != NULL);
  m_drone = drone;
}

Ptr<DronePeripheral>
DronePeripheralContainer::Create()
{
  auto peripheral = m_dronePeripheralFactory.Create<DronePeripheral>();
  //if there are no trigger regions, this peripheral is always ON
  if (peripheral->GetNRoI() == 0) peripheral->SetState(DronePeripheral::PeripheralState::ON);
  peripheral->SetDrone(m_drone);
  m_dronePeripherals.push_back(peripheral);
  return peripheral;
}

uint32_t
DronePeripheralContainer::GetN (void) const
{
  return m_dronePeripherals.size ();
}

Ptr<DronePeripheral>
DronePeripheralContainer::Get (uint32_t i) const
{
  return m_dronePeripherals[i];
}

DronePeripheralContainer::Iterator
DronePeripheralContainer::Begin (void) const
{
  return m_dronePeripherals.begin ();
}

DronePeripheralContainer::Iterator
DronePeripheralContainer::End (void) const
{
  return m_dronePeripherals.end ();
}

void
DronePeripheralContainer::DoInitialize (void)
{
  Object::DoInitialize();
}

void
DronePeripheralContainer::DoDispose (void)
{
  for (std::vector<ns3::Ptr<ns3::DronePeripheral>>::iterator i = m_dronePeripherals.begin (); i != m_dronePeripherals.end (); i++)
    {
      Ptr<DronePeripheral> peripheral = *i;
      peripheral->Dispose ();
      *i = 0;
    }
  m_dronePeripherals.clear();
  Object::DoDispose();
}

void
DronePeripheralContainer::InstallAll(Ptr<Drone> drone)
{
  PinStorage();
  for (auto peripheral: m_dronePeripherals)
    {
      //peripheral->SetDrone(drone);
      auto value = BooleanValue(false);
      if (peripheral->GetAttributeFailSafe("HasStorage", value) && value)
        {
          auto newperipheral = StaticCast<InputPeripheral, DronePeripheral>(peripheral);
          auto storageperipheral = StaticCast<StoragePeripheral, DronePeripheral>(m_dronePeripherals[0]);
          newperipheral->SetStorage(storageperipheral);
        }
      NS_LOG_DEBUG("DronePeripheralContainer: Peripheral Installed on Drone #"<<drone->GetId()<<" with State "<< peripheral->GetState());
    }
}

void
DronePeripheralContainer::PinStorage(void)
{
  for (uint32_t i=0; i<GetN(); i++)
  {
    if (m_dronePeripherals[i]->GetInstanceTypeId() == StoragePeripheral::GetTypeId())
    {
      std::swap(m_dronePeripherals[0], m_dronePeripherals[i]);
      break;
    }
  }
}

} // namespace ns3