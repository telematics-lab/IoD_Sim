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
#ifndef INPUT_PERIPHERAL_H
#define INPUT_PERIPHERAL_H

#include <ns3/nstime.h>
#include <ns3/event-id.h>
#include "drone-peripheral.h"
#include "storage-peripheral.h"

namespace ns3 {

/**
 * \brief This class describes a generic input peripheral with a constant
 * acquisition data rate.
 * It must be linked to a StoragePeripheral to store the gathered data.
 */
class InputPeripheral : public DronePeripheral
{
public:
  /**
   * \brief Get the type ID.
   *
   * \returns the object TypeId
   */
  static TypeId GetTypeId (void);

  InputPeripheral ();

  /**
   * \brief Simulates the data acquisition.
   *
   * This methods schedules every m_acquisitionTimeInterval an event that allocates
   * m_dataRate * m_acquisitionTimeInterval bits to the linked StoragePeripheral.
   */
  void AcquireData (void);

  /**
   * \brief Sets the pointer to a StoragePeripheral.
   *
   * \param storage Pointer to a StoragePeripheral.
   *
   * Input and storage peripherals have to be on the same drone.
   */
  void SetStorage (Ptr<StoragePeripheral> storage);

  /**
   * \brief Installs an InputPeripheral on a Drone and links it to the StoragePeripheral.
   *
   * This method must be called if the InputPeripheral has the Attribute "HasStorage"
   * set to true.
   *
   * \param storage Pointer to a StoragePeripheral.
   * \param drone Pointer to a Drone.
   */
  void Install (Ptr<StoragePeripheral> storage, Ptr<Drone> drone);

  /**
   * \brief Executes custom operations on state transition.
   *
   * \param ocs new state.
   */
  virtual void OnChangeState(PeripheralState ocs);

  double GetDatarate ();

  Time GetAcquisitionTimeInterval ();

  bool HasStorage ();

protected:
  void DoInitialize (void);
  void DoDispose (void);

private:
  Time m_acquisitionTimeInterval;
  double m_dataRate;
  bool m_hasStorage;
  EventId m_dataAcquisitionEvent;
  Ptr<StoragePeripheral> m_storage;
};

} // namespace ns3

#endif /* INPUT_PERIPHERAL_H */
