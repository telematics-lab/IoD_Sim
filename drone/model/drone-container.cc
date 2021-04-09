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
#include <ns3/drone-energy-model-helper.h>
#include <ns3/li-ion-energy-source-helper.h>
#include <ns3/double.h>
#include "drone-container.h"
#include "drone-energy-model.h"
#include "input-peripheral.h"

namespace ns3 {

void
DroneContainer::Create (uint32_t n)
{
  for (uint32_t i = 0; i < n; i++)
    {
      auto drone = CreateObjectWithAttributes<Drone>(
        "Mass", DoubleValue(0.750),
        "RotorDiskArea", DoubleValue(0.18),
        "AirDensity", DoubleValue(1.225),
        "DragCoefficient", DoubleValue(0.08)
      );
      m_drones.push_back (drone);
      auto nativenode = StaticCast<Node, Drone>(drone);
      NodeContainer::Add(nativenode);
      InitializeDrone(drone);
    }
}

uint32_t
DroneContainer::GetN (void) const
{
  return m_drones.size ();
}

Ptr<Drone>
DroneContainer::Get (uint32_t i) const
{
  return m_drones[i];
}

DroneContainer::Iterator
DroneContainer::Begin (void) const
{
  return m_drones.begin ();
}

DroneContainer::Iterator
DroneContainer::End (void) const
{
  return m_drones.end ();
}

void
DroneContainer::InitializeDrone(Ptr<Drone> drone)
{
  LiIonEnergySourceHelper liion;
  liion.Set("LiIonEnergySourceInitialEnergyJ", DoubleValue(200.0));
  liion.Set("LiIonEnergyLowBatteryThreshold", DoubleValue(0.2));
  auto sources = liion.Install(drone);

  DroneEnergyModelHelper energyModel;
  energyModel.Install(drone, sources.Get(0));

  Ptr<StoragePeripheral> storage = CreateObjectWithAttributes<StoragePeripheral>(
    "PowerConsumption", DoubleValue(1),
    "Capacity", IntegerValue(10*8*pow(1024,2))
  );
  storage->SetDrone(drone);

  Ptr<InputPeripheral> input = CreateObjectWithAttributes<InputPeripheral>(
    "PowerConsumption", DoubleValue(1),
    "DataRate", DoubleValue(8*pow(1024,2))
  );
  input->Install(storage, drone);
}

} // namespace ns3