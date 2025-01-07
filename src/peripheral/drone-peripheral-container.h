/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2024 The IoD_Sim Authors.
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
#ifndef DRONE_PERIPHERAL_CONTAINER_H
#define DRONE_PERIPHERAL_CONTAINER_H

#include "drone-peripheral.h"

#include <ns3/drone.h>
#include <ns3/object-factory.h>

#include <vector>

namespace ns3
{

class DronePeripheral;
class Drone;

/**
 * \ingroup peripheral
 *
 * \brief Keeps track of a set of drone peripherals pointers.
 */
class DronePeripheralContainer : public Object
{
  public:
    /**
     * \brief Get the type ID.
     *
     * \returns the object TypeId
     */
    static TypeId GetTypeId(void);

    /// Drone peripherals container iterator
    typedef std::vector<Ptr<DronePeripheral>>::const_iterator Iterator;

    /**
     * \brief Sets the Peripheral TypeId in the ObjectFactory
     *
     * \param typeId String containing the Peripheral TypeId
     */
    void Add(const std::string typeId);

    /**
     * \brief Sets Peripheral Attributes in the ObjectFactory.
     *
     * \param name String containing the Attribute name.
     * \param v Value of the Attribute.
     */
    void Set(std::string name, const AttributeValue& v);

    /**
     * \brief Creates the Peripheral Object and adds it to the container.
     *
     * \returns Pointer to the DronePeripheral created.
     */
    Ptr<DronePeripheral> Create();

    /**
     * \brief Gets an iterator which refers to the first DronePeripheral in the
     * container.
     *
     * \returns an iterator which refers to the first DronePeripheral in the container.
     */
    Iterator Begin(void) const;

    /**
     * \brief Gets an iterator which indicates past-the-last DronePeripheral in the
     * container.
     *
     * \returns an iterator which indicates an ending condition for a loop.
     */
    Iterator End(void) const;

    /**
     * \brief Sets the pointer of the drone.
     *
     * \param drone Pointer of the drone.
     */
    void SetDrone(Ptr<Drone> drone);

    /**
     * \brief Get the number of Ptr<DronePeripheral> stored in this container.
     *
     * \returns the number of Ptr<DronePeripheral> stored in this container.
     */
    uint32_t GetN(void) const;

    /**
     * \brief Get the Ptr<DronePeripheral> stored in this container at a given
     * index.
     *
     * \param i the index of the requested drone peripheral pointer.
     * \returns the requested drone peripheral pointer.
     */
    Ptr<DronePeripheral> Get(uint32_t i) const;

    /**
     * \brief Installs drone peripherals on a drone and eventually links
     * input peripherals to storage peripherals.
     *
     * \param drone Pointer to the drone.
     */
    void InstallAll(Ptr<Drone> drone);

    /**
     * \return the presence of storage peripheral
     */
    bool ThereIsStorage();

  protected:
    void virtual DoInitialize(void);
    void virtual DoDispose(void);

  private:
    /**
     * \brief Moves the storage peripheral on the top of the container.
     */
    void PinStorage(void);

    std::vector<Ptr<DronePeripheral>> m_dronePeripherals; //!< Drone peripherals smart pointers
    ObjectFactory m_dronePeripheralFactory;
    Ptr<Drone> m_drone;
    bool m_storage = false;
};

} // namespace ns3

#endif /* DRONE_PERIPHERAL_CONTAINER_H */
