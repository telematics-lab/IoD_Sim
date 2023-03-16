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
#include "drone-peripheral.h"

#include <ns3/integer.h>

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
          .AddAttribute ("PowerConsumption", "The power consumption [J/s] of the peripheral, in OFF|IDLE|ON states",
                   DoubleVectorValue (),
                   MakeDoubleVectorAccessor (&DronePeripheral::SetPowerConsumptionStates),
                   MakeDoubleVectorChecker ())
          .AddAttribute ("RoITrigger", "Indexes of Regions of Interest",
                   IntVectorValue (),
                   MakeIntVectorAccessor (&DronePeripheral::SetRegionsOfInterest),
                   MakeIntVectorChecker ())
          ;
  return tid;
}

DronePeripheral::DronePeripheral ()
{
  m_state = OFF;
  m_powerConsumption = 0;
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
  SetState(IDLE);
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
void
DronePeripheral::SetPowerConsumption(double pc)
{
  NS_LOG_FUNCTION (this);
  m_powerConsumption = pc;
}

DronePeripheral::PeripheralState
DronePeripheral::GetState(void)
{
  return m_state;
}

void
DronePeripheral::SetState(PeripheralState s)
{
  m_state = s;
  switch (s)
    {
      case OFF:
        SetPowerConsumption(GetPowerConsumptionStates()[0]);
      break;
      case IDLE:
        SetPowerConsumption(GetPowerConsumptionStates()[1]);
      break;
      case ON:
        SetPowerConsumption(GetPowerConsumptionStates()[2]);
      break;
    }
  if (GetDrone () != NULL) NS_LOG_DEBUG ("DronePeripheral on Drone #" << GetDrone ()->GetId ()<< ": changing peripheral state to "<<GetState());
  OnChangeState(s);
}

void
DronePeripheral::OnChangeState(PeripheralState ocs)
{

}

void
DronePeripheral::SetPowerConsumptionStates (const DoubleVector &a)
{
  for (auto c = a.Begin (); c != a.End (); c++)
    {
      m_powerConsumptionStates.push_back(*c);
    }
}

std::vector<double>
DronePeripheral::GetPowerConsumptionStates (void)
{
  return m_powerConsumptionStates;
}

void
DronePeripheral::SetRegionsOfInterest (const IntVector &a)
{
  for (auto c = a.Begin (); c != a.End (); c++)
    {
      m_roi.push_back(*c);
    }
}

std::vector<int>
DronePeripheral::GetRegionsOfInterest (void)
{
  return m_roi;
}

int
DronePeripheral::GetNRoI (void)
{
  return m_roi.size();
}

} //namespace ns3
