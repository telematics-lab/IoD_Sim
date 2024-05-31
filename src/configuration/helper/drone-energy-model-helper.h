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
#ifndef DRONE_ENERGY_MODEL_HELPER_H
#define DRONE_ENERGY_MODEL_HELPER_H

#include <ns3/drone.h>
#include <ns3/energy-model-helper.h>

namespace ns3
{

/**
 * \ingroup energy
 * \brief Helper to install an energy model onto a drone and link it to an energy source.
 */
class DroneEnergyModelHelper : public DeviceEnergyModelHelper
{
  public:
    DroneEnergyModelHelper();

    /**
     * \param name Name of attribute to set.
     * \param v Value of the attribute.
     *
     * Sets one of the attributes of underlying DroneEnergyModel.
     */
    virtual void Set(std::string name, const AttributeValue& v);

    /**
     * \param drone Pointer to the Drone to install DroneEnergyModel.
     * \param source The EnergySource the DroneEnergyModel will be using.
     * \returns The resultant DeviceEnergyModel.
     *
     * Installs a DroneEnergyModel with a specified EnergySource onto a
     * Drone.
     */
    Ptr<energy::DeviceEnergyModel> Install(Ptr<Drone> drone, Ptr<energy::EnergySource> source);

  private:
    Ptr<energy::DeviceEnergyModel> DoInstall(Ptr<NetDevice> device,
                                             Ptr<energy::EnergySource> source) const;

    ObjectFactory m_droneEnergyModel;
};

} // namespace ns3

#endif /* DRONE_ENERGY_MODEL_HELPER_H */
