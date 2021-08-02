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
 *
 * Authors: Giovanni Iacovelli <giovanni.iacovelli@poliba.it>
 */
#include "remote-list.h"

#include <ns3/config.h>
#include <ns3/log.h>
#include <ns3/node.h>
#include <ns3/object.h>
#include <ns3/object-vector.h>
#include <ns3/simulator.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RemoteList");

/**
 * \ingroup network
 * \brief private implementation detail of the RemoteList API
 */
class RemoteListPriv : public Object
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId.
   */
  static TypeId GetTypeId ();

  RemoteListPriv ();
  ~RemoteListPriv ();

  /**
   * \param remote remote to add
   * \returns index of remote in list
   *
   * This method should be called automatically from Node::Node at this time.
   */
  uint32_t Add (Ptr<Node> remote);

  /**
   * \returns a C++ iterator located at the beginning of this list.
   */
  RemoteList::Iterator Begin () const;

  /**
   * \returns a C++ iterator located at the end of this list.
   */
  RemoteList::Iterator End () const;

  /**
   * \param n index of requested node.
   * \returns the remote (Node) associated to index n.
   */
  Ptr<Node> GetRemote (uint32_t n);

  /**
   * \returns the number of drones currently in the list.
   */
  uint32_t GetNRemotes ();

  /**
   * \brief Get the remote list object
   * \returns the remote list
   */
  static Ptr<RemoteListPriv> Get ();

private:
  /**
   * \brief Get the remote list object
   * \returns the remote list
   */
  static Ptr<RemoteListPriv> *DoGet ();

  /**
   * \brief Delete the drones list object
   */
  static void Delete ();

  /**
   * \brief Dispose the drones in the list
   */
  virtual void DoDispose ();

  std::vector<Ptr<Node>> m_remotes; //!< remote objects container
};

TypeId
RemoteListPriv::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::RemoteListPriv")
    .SetParent<Object> ()
    .SetGroupName("Remotes")
    .AddAttribute ("RemoteList", "The list of all remotes created during the simulation.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&RemoteListPriv::m_remotes),
                   MakeObjectVectorChecker<Node> ())
    ;

  return tid;
}

Ptr<RemoteListPriv>
RemoteListPriv::Get ()
{
  NS_LOG_FUNCTION_NOARGS ();

  return *DoGet ();
}

Ptr<RemoteListPriv> *
RemoteListPriv::DoGet ()
{
  NS_LOG_FUNCTION_NOARGS ();

  static Ptr<RemoteListPriv> ptr = 0;
  if (ptr == 0)
    {
      ptr = CreateObject<RemoteListPriv> ();
      Config::RegisterRootNamespaceObject (ptr);
      Simulator::ScheduleDestroy (&RemoteListPriv::Delete);
    }

  return &ptr;
}

void
RemoteListPriv::Delete ()
{
  NS_LOG_FUNCTION_NOARGS ();

  Config::UnregisterRootNamespaceObject (Get ());
  (*DoGet ()) = 0;
}

RemoteListPriv::RemoteListPriv ()
{
  NS_LOG_FUNCTION (this);
}

RemoteListPriv::~RemoteListPriv ()
{
  NS_LOG_FUNCTION (this);
}

void
RemoteListPriv::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  for (auto remote = m_remotes.begin ();
       remote != m_remotes.end ();
       remote++)
    {
      (*remote)->Dispose ();
    }

  m_remotes.erase (m_remotes.begin (), m_remotes.end ());
  Object::DoDispose ();
}

uint32_t
RemoteListPriv::Add (Ptr<Node> remote)
{
  NS_LOG_FUNCTION (this << remote);

  uint32_t index = m_remotes.size ();

  m_remotes.push_back (remote);
  Simulator::ScheduleWithContext (index, TimeStep (0), &Node::Initialize, remote);

  return index;
}

RemoteList::Iterator
RemoteListPriv::Begin () const
{
  NS_LOG_FUNCTION (this);

  return m_remotes.begin ();
}

RemoteList::Iterator
RemoteListPriv::End () const
{
  NS_LOG_FUNCTION (this);

  return m_remotes.end ();
}

uint32_t
RemoteListPriv::GetNRemotes ()
{
  NS_LOG_FUNCTION (this);

  return m_remotes.size ();
}

Ptr<Node>
RemoteListPriv::GetRemote (uint32_t n)
{
  NS_LOG_FUNCTION (this << n);

  NS_ASSERT_MSG (n < m_remotes.size (), "remote index " << n <<
                 " is out of range (only have " << m_remotes.size () << " drones).");

  return m_remotes[n];
}

} // namespace ns3

/**
 * The implementation of the public static-based API,
 * which calls into the private implementation though
 * the simulation of a singleton.
 */
namespace ns3 {

uint32_t
RemoteList::Add (Ptr<Node> remote)
{
  NS_LOG_FUNCTION (remote);

  return RemoteListPriv::Get ()->Add (remote);
}

RemoteList::Iterator
RemoteList::Begin ()
{
  NS_LOG_FUNCTION_NOARGS ();

  return RemoteListPriv::Get ()->Begin ();
}

RemoteList::Iterator
RemoteList::End ()
{
  NS_LOG_FUNCTION_NOARGS ();

  return RemoteListPriv::Get ()->End ();
}

Ptr<Node>
RemoteList::GetRemote (uint32_t n)
{
  NS_LOG_FUNCTION (n);

  return RemoteListPriv::Get ()->GetRemote (n);
}

uint32_t
RemoteList::GetNRemotes ()
{
  NS_LOG_FUNCTION_NOARGS ();

  return RemoteListPriv::Get ()->GetNRemotes ();
}

} // namespace ns3
