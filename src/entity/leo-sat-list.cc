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
 *
 * Authors: Domingo Dirutigliano <me@domy.sh>
 */
#include "leo-sat-list.h"

#include <ns3/config.h>
#include <ns3/log.h>
#include <ns3/node.h>
#include <ns3/object-vector.h>
#include <ns3/object.h>
#include <ns3/simulator.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("LeoSatList");

/**
 * \ingroup network
 * \brief private implementation detail of the LeoSatList API
 */
class LeoSatListPriv : public Object
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId.
     */
    static TypeId GetTypeId();

    LeoSatListPriv() = default;
    ~LeoSatListPriv() = default;

    /**
     * \param leoSat leoSat to add
     * \returns index of leoSat in list
     *
     * This method should be called automatically from LeoSat::LeoSat at this time.
     */
    uint32_t Add(Ptr<Node> leoSat);

    /**
     * \returns a C++ iterator located at the beginning of this list.
     */
    LeoSatList::Iterator Begin() const;

    /**
     * \returns a C++ iterator located at the end of this list.
     */
    LeoSatList::Iterator End() const;

    /**
     * \param n index of requested LeoSat.
     * \returns the leoSat (Node) associated to index n.
     */
    Ptr<Node> GetLeoSat(uint32_t n);

    /**
     * \returns the number of leoSats currently in the list.
     */
    uint32_t GetNLeoSats();

    /**
     * \brief Get the leoSat list object
     * \returns the leoSat list
     */
    static Ptr<LeoSatListPriv> Get();

  private:
    /**
     * \brief Get the leoSat list object
     * \returns the leoSat list
     */
    static Ptr<LeoSatListPriv>* DoGet();

    /**
     * \brief Delete the leoSats list object
     */
    static void Delete();

    /**
     * \brief Dispose the leoSats in the list
     */
    virtual void DoDispose();

    std::vector<Ptr<Node>> m_leoSats; //!< leoSat objects container
};

TypeId
LeoSatListPriv::GetTypeId()
{
    static TypeId tid = TypeId("ns3::LeoSatListPriv")
                            .SetParent<Object>()
                            .SetGroupName("LeoSats")
                            .AddAttribute("LeoSatList",
                                          "The list of all leoSats created during the simulation.",
                                          ObjectVectorValue(),
                                          MakeObjectVectorAccessor(&LeoSatListPriv::m_leoSats),
                                          MakeObjectVectorChecker<Node>());

    return tid;
}

Ptr<LeoSatListPriv>
LeoSatListPriv::Get()
{
    NS_LOG_FUNCTION_NOARGS();

    return *DoGet();
}

Ptr<LeoSatListPriv>*
LeoSatListPriv::DoGet()
{
    NS_LOG_FUNCTION_NOARGS();

    static Ptr<LeoSatListPriv> ptr = 0;
    if (!ptr)
    {
        ptr = CreateObject<LeoSatListPriv>();
        Config::RegisterRootNamespaceObject(ptr);
        Simulator::ScheduleDestroy(&LeoSatListPriv::Delete);
    }

    return &ptr;
}

void
LeoSatListPriv::Delete()
{
    NS_LOG_FUNCTION_NOARGS();

    Config::UnregisterRootNamespaceObject(Get());
    (*DoGet()) = 0;
}

void
LeoSatListPriv::DoDispose()
{
    NS_LOG_FUNCTION(this);

    for (auto leoSat = m_leoSats.begin(); leoSat != m_leoSats.end(); leoSat++)
    {
        (*leoSat)->Dispose();
    }

    m_leoSats.erase(m_leoSats.begin(), m_leoSats.end());
    Object::DoDispose();
}

uint32_t
LeoSatListPriv::Add(Ptr<Node> leoSat)
{
    NS_LOG_FUNCTION(this << leoSat);

    uint32_t index = m_leoSats.size();

    m_leoSats.push_back(leoSat);
    Simulator::ScheduleWithContext(index, TimeStep(0), &Node::Initialize, leoSat);

    return index;
}

LeoSatList::Iterator
LeoSatListPriv::Begin() const
{
    NS_LOG_FUNCTION(this);

    return m_leoSats.begin();
}

LeoSatList::Iterator
LeoSatListPriv::End() const
{
    NS_LOG_FUNCTION(this);

    return m_leoSats.end();
}

uint32_t
LeoSatListPriv::GetNLeoSats()
{
    NS_LOG_FUNCTION(this);

    return m_leoSats.size();
}

Ptr<Node>
LeoSatListPriv::GetLeoSat(uint32_t n)
{
    NS_LOG_FUNCTION(this << n);

    NS_ASSERT_MSG(n < m_leoSats.size(),
                  "leoSat index " << n << " is out of range (only have " << m_leoSats.size()
                                  << " leoSats).");

    return m_leoSats[n];
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
LeoSatList::Add(Ptr<Node> leoSat)
{
    NS_LOG_FUNCTION(leoSat);

    return LeoSatListPriv::Get()->Add(leoSat);
}

LeoSatList::Iterator
LeoSatList::Begin()
{
    NS_LOG_FUNCTION_NOARGS();

    return LeoSatListPriv::Get()->Begin();
}

LeoSatList::Iterator
LeoSatList::End()
{
    NS_LOG_FUNCTION_NOARGS();

    return LeoSatListPriv::Get()->End();
}

Ptr<Node>
LeoSatList::GetLeoSat(uint32_t n)
{
    NS_LOG_FUNCTION(n);

    return LeoSatListPriv::Get()->GetLeoSat(n);
}

uint32_t
LeoSatList::GetNLeoSats()
{
    NS_LOG_FUNCTION_NOARGS();

    return LeoSatListPriv::Get()->GetNLeoSats();
}

} // namespace ns3
