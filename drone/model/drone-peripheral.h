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
#ifndef DRONE_PERIPHERAL_H
#define DRONE_PERIPHERAL_H

#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/double.h>
#include "drone.h"

namespace ns3 {

class Drone;

/**
 * \brief Base class describing a general-purpose peripheral mounted on a drone.
 *
 * A peripheral is characterized by a constant power consumption.
 */
class DronePeripheral : public Object
{
public:
  /**
   * \brief Get the type ID.
   *
   * \returns the object TypeId
   */
  static TypeId GetTypeId(void);

  DronePeripheral();

  /**
   * \brief Sets the pointer of the drone.
   *
   * \param drone Pointer of the drone.
   */
  void SetDrone(Ptr<Drone> drone);

  /**
   * \brief Returns the pointer of the drone.
   *
   * \returns Pointer of the drone.
   */
  Ptr<Drone> GetDrone(void);

  /**
   * \brief Returns the power consumption of the peripheral.
   *
   * \returns Power consumption in Watt.
   */
  double GetPowerConsumption(void);

protected:
  virtual void DoDispose (void);
  virtual void DoInitialize (void);

private:
  Ptr<Drone> m_drone;         //!< Pointer to the drone.
  double m_powerConsumption;  //!< Constant power consumption in Watt.
};


} // namespace ns3

#endif /* DRONE_PERIPHERAL_H */