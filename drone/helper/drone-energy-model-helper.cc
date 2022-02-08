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
#include "drone-energy-model-helper.h"
#include <ns3/drone-energy-model.h>

namespace ns3 {

DroneEnergyModelHelper::DroneEnergyModelHelper ()
{
  m_droneEnergyModel.SetTypeId ("ns3::DroneEnergyModel");
}

void
DroneEnergyModelHelper::Set (std::string name, const AttributeValue &v)
{
  m_droneEnergyModel.Set (name, v);
}

Ptr<DeviceEnergyModel>
DroneEnergyModelHelper::Install (Ptr<Drone> drone, Ptr<EnergySource> source)
{
  Ptr<DroneEnergyModel> model = m_droneEnergyModel.Create<DroneEnergyModel> ();
  model->SetDrone (drone);
  model->SetEnergySource (source);
  source->AppendDeviceEnergyModel (model);
  source->SetNode (drone);
  return model;
}

Ptr<DeviceEnergyModel>
DroneEnergyModelHelper::DoInstall (Ptr<NetDevice> device, Ptr<EnergySource> source) const
{
  Ptr<DroneEnergyModel> model = m_droneEnergyModel.Create<DroneEnergyModel> ();
  return model;
}

} // namespace ns3
