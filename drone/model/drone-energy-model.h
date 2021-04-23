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
#ifndef DRONE_ENERGY_MODEL_H
#define DRONE_ENERGY_MODEL_H

#include <ns3/device-energy-model.h>
#include <ns3/li-ion-energy-source.h>
#include <ns3/simulator.h>
#include "drone.h"

namespace ns3 {

/**
 * \ingroup energy
 *
 * \brief Defines the power consumption of a drone together with its attached peripherals.
 */
class DroneEnergyModel : public DeviceEnergyModel
{
public:
  /**
   * \brief Get the type ID.
   *
   * \returns the object TypeId
   */
  static TypeId GetTypeId (void);

  DroneEnergyModel ();

  ~DroneEnergyModel ()
  {
  }

  /**
   * \brief Sets the pointer to energy source installed on drone.
   *
   * \param source Pointer to energy source installed on drone.
   */
  virtual void SetEnergySource (Ptr<EnergySource> source);

  /**
   * \brief Sets the pointer of the drone.
   *
   * \param drone Pointer of the drone.
   */
  void SetDrone (Ptr<Drone> drone);

  /**
   * \brief Returns the pointer of the drone.
   *
   * \returns Pointer of the drone.
   */
  Ptr<Drone> GetDrone () const;

  /**
   * \brief Implements DeviceEnergyModel::GetTotalEnergyConsumption.
   *
   * Not implemented.
   *
   * \returns Total energy consumption of the device. (Not implemented!)
   */
  virtual double GetTotalEnergyConsumption (void) const
  {
    return 0;
  }

  /**
   * \brief Calculates the mechanical power consumption of the drone.
   *
   * This method implements the power consumption model employed in
   *
   * Sun, Y., Xu, D., Ng, D. W. K., Dai, L., & Schober, R. (2019).
   * Optimal 3D-trajectory design and resource allocation for solar-powered UAV communication systems.
   * IEEE Transactions on Communications, 67(6), 4281-4298.
   *
   * \returns Mechanical power consumption in Watt.
   */
  double GetPower (void) const;

  /**
   * \brief Calculates the power consumption of the drone peripherals.
   *
   * \returns Peripherals power consumption in Watt.
   */
  double GetPeripheralsPowerConsumption (void) const;

  /**
   * \brief Notifies the low battery threshold being crossed.
   *
   * This method logs the time at which the remaining energy in the EnergySource
   * crosses the low battery threshold.
   */
  virtual void HandleEnergyDepletion (void);

  /**
   * \param newState New state the device is in.
   *
   * Not implemented
   */
  virtual void
  ChangeState (int newState)
  {
  }

  /**
   * \brief Handles energy recharged.
   *
   * Not implemented
   */
  virtual void
  HandleEnergyRecharged (void)
  {
  }

  /**
   * \brief Handles energy changed.
   *
   * Not implemented
   */
  virtual void
  HandleEnergyChanged (void)
  {
  }

private:
  /**
   * \brief Computes the total current draw.
   *
   * \returns Current draw of the drone and its peripherals in Ampere.
   */
  double DoGetCurrentA (void) const;

  Ptr<EnergySource> m_source;
  Ptr<Drone> m_drone;
  TracedValue<double> m_totalEnergyConsumption;
};

} // namespace ns3

#endif /* DRONE_ENERGY_MODEL_H */
