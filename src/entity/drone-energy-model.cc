/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2023 The IoD_Sim Authors.
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
#include "drone-energy-model.h"

#include <ns3/mobility-model.h>
#include <ns3/simulator.h>

#define AIR_DENSITY 1.225

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("DroneEnergyModel");

NS_OBJECT_ENSURE_REGISTERED(DroneEnergyModel);

TypeId
DroneEnergyModel::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::DroneEnergyModel")
                            .SetParent<energy::DeviceEnergyModel>()
                            .SetGroupName("Energy")
                            .AddConstructor<DroneEnergyModel>();
    return tid;
}

DroneEnergyModel::DroneEnergyModel()
{
    NS_LOG_FUNCTION(this);
    m_source = 0;
}

void
DroneEnergyModel::SetEnergySource(Ptr<energy::EnergySource> source)
{
    NS_LOG_FUNCTION(this << source);
    m_source = source;
}

void
DroneEnergyModel::SetDrone(Ptr<Drone> drone)
{
    NS_LOG_FUNCTION(this << drone);
    NS_ASSERT(drone);
    m_drone = drone;
}

Ptr<Drone>
DroneEnergyModel::GetDrone() const
{
    return m_drone;
}

void
DroneEnergyModel::HandleEnergyDepletion()
{
    Time time = Simulator::Now();
    NS_LOG_DEBUG("DroneEnergyModel:LowBatteryThreshold on Drone #"
                 << GetDrone()->GetId() << " crossed at " << time.GetSeconds() << " seconds.");
}

double
DroneEnergyModel::GetPeripheralsPowerConsumption(void) const
{
    double peripheralsPowerConsumption = 0;

    for (auto i = m_drone->GetPeripherals()->Begin(); i != m_drone->GetPeripherals()->End(); i++)
    {
        peripheralsPowerConsumption += (*i)->GetPowerConsumption();
    }

    NS_LOG_DEBUG("DroneEnergyModel:Peripherals Power Consumption on Drone #"
                 << GetDrone()->GetId() << ": " << peripheralsPowerConsumption << " W");
    return peripheralsPowerConsumption;
}

double
DroneEnergyModel::GetPower() const
{
    Ptr<Drone> drone = GetDrone();
    Ptr<Object> obj = StaticCast<Object, Drone>(drone);
    Vector velocity = obj->GetObject<MobilityModel>()->GetVelocity();
    double Phover = sqrt(drone->GetWeight() / (2 * AIR_DENSITY * drone->GetArea()));
    double Plevel =
        ((pow(drone->GetWeight(), 2)) / (sqrt(2) * AIR_DENSITY * drone->GetArea())) *
        (1 /
         sqrt(pow(velocity.x, 2) + pow(velocity.y, 2) +
              sqrt(pow(sqrt(pow(velocity.x, 2) + pow(velocity.y, 2)), 4) + (4 * pow(Phover, 4)))));
    double Pdrag = (1 / 8) * drone->GetDragCoefficient() * AIR_DENSITY * drone->GetArea() *
                   pow(sqrt(pow(velocity.x, 2) + pow(velocity.y, 2)), 3);
    double Pvertical = drone->GetWeight() * velocity.z;

    if (Pvertical < 0.0)
        Pvertical = 0.0;

    double PowerConsumption = Plevel + Pdrag + Pvertical;

    NS_LOG_LOGIC("Plevel for Drone #" << GetDrone()->GetId() << ": " << Plevel << " W");
    NS_LOG_LOGIC("Pdrag for Drone #" << GetDrone()->GetId() << ": " << Pdrag << " W");
    NS_LOG_LOGIC("Pvertical for Drone #" << GetDrone()->GetId() << ": " << Pvertical << " W");
    NS_LOG_LOGIC("Mechanical Power Consumption for Drone #" << GetDrone()->GetId() << ": "
                                                            << PowerConsumption << " W");

    return PowerConsumption;
}

double
DroneEnergyModel::DoGetCurrentA(void) const
{
    if (Simulator::Now() <= Time())
        return 0;

    double PowerConsumption = GetPower() + GetPeripheralsPowerConsumption();
    double VoltageV = m_source->GetSupplyVoltage();
    double CurrentA = (PowerConsumption / VoltageV);

    NS_LOG_LOGIC("Total Power Consumption for Drone #" << GetDrone()->GetId() << ": "
                                                       << PowerConsumption << " W");
    NS_LOG_LOGIC("Current draw for Drone #" << GetDrone()->GetId() << ": " << CurrentA << " A");

    return CurrentA;
}

} // namespace ns3
