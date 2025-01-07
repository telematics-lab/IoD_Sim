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
#ifndef DRONE_PERIPHERAL_H
#define DRONE_PERIPHERAL_H

#include <ns3/double-vector.h>
#include <ns3/double.h>
#include <ns3/drone.h>
#include <ns3/int-vector.h>
#include <ns3/object.h>
#include <ns3/ptr.h>

namespace ns3
{

class Drone;

/**
 * \ingroup peripheral
 *
 * \brief Base class describing a general-purpose peripheral mounted on a drone.
 *
 * A peripheral is characterized by a constant power consumption.
 */
class DronePeripheral : public Object
{
  public:
    /**
     * \brief the peripheral states
     */
    enum PeripheralState
    {
        OFF,  /// the peripheral is OFF
        IDLE, /// the peripheral cannot execute operations
        ON    /// the peripheral can perform actions
    };

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

    /**
     * \brief Returns the current peripheral state.
     *
     * \returns Peripheral state.
     */
    PeripheralState GetState(void);

    /**
     * \brief Sets the peripheral state.
     *
     * \param s state.
     */
    void SetState(PeripheralState s);

    /**
     * \brief Executes custom operations on state transition.
     *
     * \param ocs new state.
     */
    virtual void OnChangeState(PeripheralState ocs);

    /**
     * \return Vector of the regions indexes
     */
    std::vector<int> GetRegionsOfInterest(void);

    /**
     * \return Number of regions.
     */
    int GetNRoI(void);

    /**
     * \return Gets the power consumption vector for each state.
     */
    std::vector<double> GetPowerConsumptionStates(void);

  protected:
    virtual void DoDispose(void);
    virtual void DoInitialize(void);

    /**
     * \brief Sets the current power consumption.
     *
     * \param pc power consumption in W.
     */
    void SetPowerConsumption(double pc);

    /**
     * \brief Sets the power consumption vector for each state.
     *
     * \param a Structure describing the power consumption for OFF|IDLE|ON state
     */
    void SetPowerConsumptionStates(const DoubleVector& a);

    /**
     * \brief Sets the trigger regions of interest.
     *
     * \param a Structure containing indexes of regions
     */
    void SetRegionsOfInterest(const IntVector& a);

  private:
    Ptr<Drone> m_drone;                           //!< Pointer to the drone.
    double m_powerConsumption;                    //!< Constant power consumption in Watt.
    PeripheralState m_state;                      //!< Current peripheral state
    std::vector<double> m_powerConsumptionStates; //!< Power consumptions for each state
    std::vector<int> m_roi;                       //!< Regions of interest indexes
};

} // namespace ns3

#endif /* DRONE_PERIPHERAL_H */
