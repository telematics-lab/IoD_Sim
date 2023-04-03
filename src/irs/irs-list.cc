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
 */
#include "irs-list.h"

#include <ns3/config.h>
#include <ns3/log.h>
#include <ns3/object-vector.h>
#include <ns3/object.h>
#include <ns3/simulator.h>

#include <ns3/irs.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("IrsList");

/**
 * \ingroup Irs
 * \brief private implementation detail of the IrsList API
 */
class IrsListPriv : public Object
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId.
   */
  static TypeId GetTypeId ();

  IrsListPriv ();
  ~IrsListPriv ();

  /**
   * \param irs Irs to add
   * \returns index of Irs in list
   *
   * This method should be called automatically from Irs::Irs at this time.
   */
  uint32_t Add (Ptr<Irs> irs);

  /**
   * \returns a C++ iterator located at the beginning of this list.
   */
  IrsList::Iterator Begin () const;

  /**
   * \returns a C++ iterator located at the end of this list.
   */
  IrsList::Iterator End () const;

  /**
   * \param n index of requested Irs.
   * \returns the Irs (Node) associated to index n.
   */
    Ptr<Irs> Get(uint32_t n);

  /**
   * \returns the number of Irs currently in the list.
   */
    uint32_t GetN();

  /**
   * \brief Get the Irs list object
   * \returns the Irs list
   */
  static Ptr<IrsListPriv> Get ();

private:
  /**
   * \brief Get the irs list object
   * \returns the irs list
   */
  static Ptr<IrsListPriv> *DoGet ();

  /**
   * \brief Delete the Irs list object
   */
  static void Delete ();

  /**
   * \brief Dispose the Irs in the list
   */
  virtual void DoDispose ();

  std::vector<Ptr<Irs>> m_irss; //!< Irs objects container
};

TypeId
IrsListPriv::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::IrsListPriv")
    .SetParent<Object> ()
    .SetGroupName("Irs")
    .AddAttribute ("IrsList", "The list of all Irs created during the simulation.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&IrsListPriv::m_irss),
                   MakeObjectVectorChecker<Irs> ())
    ;

  return tid;
}

Ptr<IrsListPriv>
IrsListPriv::Get ()
{
  NS_LOG_FUNCTION_NOARGS ();

  return *DoGet ();
}

Ptr<IrsListPriv> *
IrsListPriv::DoGet ()
{
  NS_LOG_FUNCTION_NOARGS ();

  static Ptr<IrsListPriv> ptr = 0;
  if (ptr == nullptr)
    {
      ptr = CreateObject<IrsListPriv> ();
      Config::RegisterRootNamespaceObject (ptr);
      Simulator::ScheduleDestroy (&IrsListPriv::Delete);
    }

  return &ptr;
}

void
IrsListPriv::Delete ()
{
  NS_LOG_FUNCTION_NOARGS ();

  Config::UnregisterRootNamespaceObject (Get ());
  (*DoGet ()) = 0;
}

IrsListPriv::IrsListPriv ()
{
  NS_LOG_FUNCTION (this);
}

IrsListPriv::~IrsListPriv ()
{
  NS_LOG_FUNCTION (this);
}

void
IrsListPriv::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  for (auto irs = m_irss.begin ();
       irs != m_irss.end ();
       irs++)
    {
      (*irs)->Dispose ();
    }

  m_irss.erase (m_irss.begin (), m_irss.end ());
  Object::DoDispose ();
}

uint32_t
IrsListPriv::Add (Ptr<Irs> irs)
{
  NS_LOG_FUNCTION (this << irs);

  uint32_t index = m_irss.size ();

  m_irss.push_back (irs);
  Simulator::ScheduleWithContext (index, TimeStep (0), &Irs::Initialize, irs);

  return index;
}

IrsList::Iterator
IrsListPriv::Begin () const
{
  NS_LOG_FUNCTION (this);

  return m_irss.begin ();
}

IrsList::Iterator
IrsListPriv::End () const
{
  NS_LOG_FUNCTION (this);

  return m_irss.end ();
}

uint32_t
IrsListPriv::GetN()
{
  NS_LOG_FUNCTION (this);

  return m_irss.size ();
}

Ptr<Irs>
IrsListPriv::Get(uint32_t n)
{
  NS_LOG_FUNCTION (this << n);

  NS_ASSERT_MSG (n < m_irss.size (), "Irs index " << n <<
                 " is out of range (only have " << m_irss.size () << " irss).");

  return m_irss[n];
}

} // namespace ns3

/**
 * The implementation of the public static-based API,
 * which calls into the private implementation though
 * the simulation of a singleton.
 */
namespace ns3 {

uint32_t
IrsList::Add (Ptr<Irs> irs)
{
  NS_LOG_FUNCTION (irs);

  return IrsListPriv::Get ()->Add (irs);
}

IrsList::Iterator
IrsList::Begin ()
{
  return IrsListPriv::Get ()->Begin ();
}

IrsList::Iterator
IrsList::begin()
{
  return IrsListPriv::Get()->Begin();
}

IrsList::Iterator
IrsList::End ()
{
  return IrsListPriv::Get ()->End ();
}

IrsList::Iterator
IrsList::end()
{
  return IrsListPriv::Get()->End();
}

Ptr<Irs>
IrsList::Get (uint32_t n)
{
  NS_LOG_FUNCTION (n);

  return IrsListPriv::Get ()->Get (n);
}

uint32_t
IrsList::GetN ()
{
  NS_LOG_FUNCTION_NOARGS ();

  return IrsListPriv::Get ()->GetN ();
}

} // namespace ns3
