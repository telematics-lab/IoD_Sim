/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2026 The IoD_Sim Authors.
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
#include "drone-container.h"

#include "drone-energy-model.h"

#include <ns3/double.h>
#include <ns3/drone-energy-model-helper.h>
#include <ns3/input-peripheral.h>
#include <ns3/li-ion-energy-source-helper.h>

namespace ns3
{

void
DroneContainer::Create(uint32_t n)
{
    for (uint32_t i = 0; i < n; i++)
    {
        auto drone = CreateObject<Drone>();
        drone->GetPeripherals()->SetDrone(drone);
        m_drones.push_back(drone);
        auto nativenode = StaticCast<Node, Drone>(drone);
        NodeContainer::Add(nativenode);
    }
}

uint32_t
DroneContainer::GetN(void) const
{
    return m_drones.size();
}

Ptr<Drone>
DroneContainer::Get(uint32_t i) const
{
    return m_drones[i];
}

DroneContainer::Iterator
DroneContainer::Begin(void) const
{
    return m_drones.begin();
}

DroneContainer::Iterator
DroneContainer::End(void) const
{
    return m_drones.end();
}

} // namespace ns3
