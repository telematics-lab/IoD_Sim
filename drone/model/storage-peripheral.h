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
#ifndef STORAGE_PERIPHERAL_H
#define STORAGE_PERIPHERAL_H

#include <ns3/traced-value.h>
#include "drone-peripheral.h"

namespace ns3 {
/**
 * \brief This class describes a generic storage peripheral.
 */
class StoragePeripheral : public DronePeripheral
{
public:
  /**
  * \brief Multipliers to ease the conversion among units of measure.
  */
  enum unit {
    bit = 1,
    kbit = 1024,
    mbit = 1024 * 1024,
    byte = 8,
    kbyte = 8 * 1024,
    mbyte = 8 * 1024 * 1024
  };

  /**
   * \brief Get the type ID.
   *
   * \returns the object TypeId
   */
  static TypeId GetTypeId (void);

  StoragePeripheral ();

  /**
   * \brief Sets the capacity of the drive.
   *
   * \param cap Capacity of the drive in bits.
   */
  void SetCapacity (uint64_t cap);

  /**
   * \brief Returns the capacity of the drive.
   *
   * \returns Capacity of the drive in bits.
   */
  uint64_t GetCapacity (void) const;

  /**
   * \brief Allocates a certain amount of data to the drive.
   *
   * \param amount Amount of data.
   * \param amountUnit Unit of measure.
   */
  bool Alloc (uint64_t amount, unit amountUnit);

  /**
   * \brief Frees a certain amount of data from the drive.
   *
   * \param amount Amount of data.
   * \param amountUnit Unit of measure.
   */
  bool Free (uint64_t amount, unit amountUnit);

protected:
  void DoInitialize (void);
  void DoDispose (void);

private:
  uint64_t m_capacity;
  TracedValue<uint64_t> m_remainingCapacity;
};

} // namespace ns3

#endif /* STORAGE_PERIPHERAL_H */
