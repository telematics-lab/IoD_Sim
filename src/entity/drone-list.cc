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
 *
 * Authors: Giovanni Grieco <giovanni.grieco@poliba.it>
 */
#include "drone-list.h"

#include "drone.h"

#include <ns3/config.h>
#include <ns3/log.h>
#include <ns3/object-vector.h>
#include <ns3/object.h>
#include <ns3/simulator.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("DroneList");

/**
 * \ingroup network
 * \brief private implementation detail of the DroneList API
 */
class DroneListPriv : public Object
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId.
     */
    static TypeId GetTypeId();

    DroneListPriv();
    ~DroneListPriv();

    /**
     * \param drone drone to add
     * \returns index of drone in list
     *
     * This method should be called automatically from Drone::Drone at this time.
     */
    uint32_t Add(Ptr<Drone> drone);

    /**
     * \returns a C++ iterator located at the beginning of this list.
     */
    DroneList::Iterator Begin() const;

    /**
     * \returns a C++ iterator located at the end of this list.
     */
    DroneList::Iterator End() const;

    /**
     * \param n index of requested Drone.
     * \returns the Drone (Node) associated to index n.
     */
    Ptr<Drone> GetDrone(uint32_t n);

    /**
     * \returns the number of drones currently in the list.
     */
    uint32_t GetNDrones();

    /**
     * \brief Get the drone list object
     * \returns the drone list
     */
    static Ptr<DroneListPriv> Get();

  private:
    /**
     * \brief Get the drone list object
     * \returns the drone list
     */
    static Ptr<DroneListPriv>* DoGet();

    /**
     * \brief Delete the drones list object
     */
    static void Delete();

    /**
     * \brief Dispose the drones in the list
     */
    virtual void DoDispose();

    std::vector<Ptr<Drone>> m_drones; //!< drone objects container
};

TypeId
DroneListPriv::GetTypeId()
{
    static TypeId tid = TypeId("ns3::DroneListPriv")
                            .SetParent<Object>()
                            .SetGroupName("Swarm")
                            .AddAttribute("DroneList",
                                          "The list of all drones created during the simulation.",
                                          ObjectVectorValue(),
                                          MakeObjectVectorAccessor(&DroneListPriv::m_drones),
                                          MakeObjectVectorChecker<Drone>());

    return tid;
}

Ptr<DroneListPriv>
DroneListPriv::Get()
{
    NS_LOG_FUNCTION_NOARGS();

    return *DoGet();
}

Ptr<DroneListPriv>*
DroneListPriv::DoGet()
{
    NS_LOG_FUNCTION_NOARGS();

    static Ptr<DroneListPriv> ptr = 0;
    if (!ptr)
    {
        ptr = CreateObject<DroneListPriv>();
        Config::RegisterRootNamespaceObject(ptr);
        Simulator::ScheduleDestroy(&DroneListPriv::Delete);
    }

    return &ptr;
}

void
DroneListPriv::Delete()
{
    NS_LOG_FUNCTION_NOARGS();

    Config::UnregisterRootNamespaceObject(Get());
    (*DoGet()) = 0;
}

DroneListPriv::DroneListPriv()
{
    NS_LOG_FUNCTION(this);
}

DroneListPriv::~DroneListPriv()
{
    NS_LOG_FUNCTION(this);
}

void
DroneListPriv::DoDispose()
{
    NS_LOG_FUNCTION(this);

    for (auto drone = m_drones.begin(); drone != m_drones.end(); drone++)
    {
        (*drone)->Dispose();
    }

    m_drones.erase(m_drones.begin(), m_drones.end());
    Object::DoDispose();
}

uint32_t
DroneListPriv::Add(Ptr<Drone> drone)
{
    NS_LOG_FUNCTION(this << drone);

    uint32_t index = m_drones.size();

    m_drones.push_back(drone);
    Simulator::ScheduleWithContext(index, TimeStep(0), &Drone::Initialize, drone);

    return index;
}

DroneList::Iterator
DroneListPriv::Begin() const
{
    NS_LOG_FUNCTION(this);

    return m_drones.begin();
}

DroneList::Iterator
DroneListPriv::End() const
{
    NS_LOG_FUNCTION(this);

    return m_drones.end();
}

uint32_t
DroneListPriv::GetNDrones()
{
    NS_LOG_FUNCTION(this);

    return m_drones.size();
}

Ptr<Drone>
DroneListPriv::GetDrone(uint32_t n)
{
    NS_LOG_FUNCTION(this << n);

    NS_ASSERT_MSG(n < m_drones.size(),
                  "Drone index " << n << " is out of range (only have " << m_drones.size()
                                 << " drones).");

    return m_drones[n];
}

} // namespace ns3

/**
 * The implementation of the public static-based API,
 * which calls into the private implementation though
 * the simulation of a singleton.
 */
namespace ns3
{

uint32_t
DroneList::Add(Ptr<Drone> drone)
{
    NS_LOG_FUNCTION(drone);

    return DroneListPriv::Get()->Add(drone);
}

DroneList::Iterator
DroneList::Begin()
{
    NS_LOG_FUNCTION_NOARGS();

    return DroneListPriv::Get()->Begin();
}

DroneList::Iterator
DroneList::End()
{
    NS_LOG_FUNCTION_NOARGS();

    return DroneListPriv::Get()->End();
}

Ptr<Drone>
DroneList::GetDrone(uint32_t n)
{
    NS_LOG_FUNCTION(n);

    return DroneListPriv::Get()->GetDrone(n);
}

uint32_t
DroneList::GetNDrones()
{
    NS_LOG_FUNCTION_NOARGS();

    return DroneListPriv::Get()->GetNDrones();
}

} // namespace ns3
