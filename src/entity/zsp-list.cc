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
#include "zsp-list.h"

#include <ns3/config.h>
#include <ns3/log.h>
#include <ns3/node.h>
#include <ns3/object-vector.h>
#include <ns3/object.h>
#include <ns3/simulator.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ZspList");

/**
 * \ingroup network
 * \brief private implementation detail of the ZspList API
 */
class ZspListPriv : public Object
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId.
     */
    static TypeId GetTypeId();

    ZspListPriv();
    ~ZspListPriv();

    /**
     * \param zsp zsp to add
     * \returns index of zsp in list
     *
     * This method should be called automatically from Node::Node at this time.
     */
    uint32_t Add(Ptr<Node> zsp);

    /**
     * \returns a C++ iterator located at the beginning of this list.
     */
    ZspList::Iterator Begin() const;

    /**
     * \returns a C++ iterator located at the end of this list.
     */
    ZspList::Iterator End() const;

    /**
     * \param n index of requested node.
     * \returns the zsp (Node) associated to index n.
     */
    Ptr<Node> GetZsp(uint32_t n);

    /**
     * \returns the number of drones currently in the list.
     */
    uint32_t GetNZsps();

    /**
     * \brief Get the zsp list object
     * \returns the zsp list
     */
    static Ptr<ZspListPriv> Get();

  private:
    /**
     * \brief Get the zsp list object
     * \returns the zsp list
     */
    static Ptr<ZspListPriv>* DoGet();

    /**
     * \brief Delete the drones list object
     */
    static void Delete();

    /**
     * \brief Dispose the drones in the list
     */
    virtual void DoDispose();

    std::vector<Ptr<Node>> m_zsps; //!< zsp objects container
};

TypeId
ZspListPriv::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ZspListPriv")
                            .SetParent<Object>()
                            .SetGroupName("Zsps")
                            .AddAttribute("ZspList",
                                          "The list of all ZSPs created during the simulation.",
                                          ObjectVectorValue(),
                                          MakeObjectVectorAccessor(&ZspListPriv::m_zsps),
                                          MakeObjectVectorChecker<Node>());

    return tid;
}

Ptr<ZspListPriv>
ZspListPriv::Get()
{
    NS_LOG_FUNCTION_NOARGS();

    return *DoGet();
}

Ptr<ZspListPriv>*
ZspListPriv::DoGet()
{
    NS_LOG_FUNCTION_NOARGS();

    static Ptr<ZspListPriv> ptr = 0;
    if (!ptr)
    {
        ptr = CreateObject<ZspListPriv>();
        Config::RegisterRootNamespaceObject(ptr);
        Simulator::ScheduleDestroy(&ZspListPriv::Delete);
    }

    return &ptr;
}

void
ZspListPriv::Delete()
{
    NS_LOG_FUNCTION_NOARGS();

    Config::UnregisterRootNamespaceObject(Get());
    (*DoGet()) = 0;
}

ZspListPriv::ZspListPriv()
{
    NS_LOG_FUNCTION(this);
}

ZspListPriv::~ZspListPriv()
{
    NS_LOG_FUNCTION(this);
}

void
ZspListPriv::DoDispose()
{
    NS_LOG_FUNCTION(this);

    for (auto zsp = m_zsps.begin(); zsp != m_zsps.end(); zsp++)
    {
        (*zsp)->Dispose();
    }

    m_zsps.erase(m_zsps.begin(), m_zsps.end());
    Object::DoDispose();
}

uint32_t
ZspListPriv::Add(Ptr<Node> zsp)
{
    NS_LOG_FUNCTION(this << zsp);

    uint32_t index = m_zsps.size();

    m_zsps.push_back(zsp);
    Simulator::ScheduleWithContext(index, TimeStep(0), &Node::Initialize, zsp);

    return index;
}

ZspList::Iterator
ZspListPriv::Begin() const
{
    NS_LOG_FUNCTION(this);

    return m_zsps.begin();
}

ZspList::Iterator
ZspListPriv::End() const
{
    NS_LOG_FUNCTION(this);

    return m_zsps.end();
}

uint32_t
ZspListPriv::GetNZsps()
{
    NS_LOG_FUNCTION(this);

    return m_zsps.size();
}

Ptr<Node>
ZspListPriv::GetZsp(uint32_t n)
{
    NS_LOG_FUNCTION(this << n);

    NS_ASSERT_MSG(n < m_zsps.size(),
                  "zsp index " << n << " is out of range (only have " << m_zsps.size()
                               << " drones).");

    return m_zsps[n];
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
ZspList::Add(Ptr<Node> zsp)
{
    NS_LOG_FUNCTION(zsp);

    return ZspListPriv::Get()->Add(zsp);
}

ZspList::Iterator
ZspList::Begin()
{
    NS_LOG_FUNCTION_NOARGS();

    return ZspListPriv::Get()->Begin();
}

ZspList::Iterator
ZspList::End()
{
    NS_LOG_FUNCTION_NOARGS();

    return ZspListPriv::Get()->End();
}

Ptr<Node>
ZspList::GetZsp(uint32_t n)
{
    NS_LOG_FUNCTION(n);

    return ZspListPriv::Get()->GetZsp(n);
}

uint32_t
ZspList::GetNZsps()
{
    NS_LOG_FUNCTION_NOARGS();

    return ZspListPriv::Get()->GetNZsps();
}

} // namespace ns3
