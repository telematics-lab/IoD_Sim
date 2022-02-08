/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2022 The IoD_Sim Authors.
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
#include <ns3/node.h>
#include <ns3/node-list.h>
#include <ns3/net-device.h>
#include <ns3/application.h>
#include <ns3/packet.h>
#include <ns3/simulator.h>
#include <ns3/object-vector.h>
#include <ns3/uinteger.h>
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/assert.h>
#include <ns3/global-value.h>
#include <ns3/boolean.h>
#include <ns3/mobility-model.h>
#include "drone.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Drone");

NS_OBJECT_ENSURE_REGISTERED (Drone);

TypeId
Drone::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Drone")
          .SetParent<Node> ()
          .SetGroupName ("Network")
          .AddConstructor<Drone> ()
          .AddAttribute ("Mass", "The mass of this Drone.",
                         DoubleValue (0),
                         MakeDoubleAccessor (&Drone::setMass, &Drone::getMass),
                         MakeDoubleChecker<double> ())
          .AddAttribute ("RotorDiskArea", "The area of the rotor disk of this Drone.",
                         DoubleValue (0),
                         MakeDoubleAccessor (&Drone::setArea, &Drone::getArea),
                         MakeDoubleChecker<double> ())
          .AddAttribute ("DragCoefficient", "Drag Coefficient relative to this Drone.",
                         DoubleValue (0),
                         MakeDoubleAccessor (&Drone::getDragCoefficient, &Drone::setDragCoefficient),
                         MakeDoubleChecker<double> ())

          ;
  return tid;
}

Drone::Drone ()
{
  m_peripheralContainer = CreateObject<DronePeripheralContainer> ();
}

void
Drone::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Node::DoDispose ();
}

void
Drone::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  Node::DoInitialize ();
}

Ptr<DronePeripheralContainer>
Drone::getPeripherals ()
{
  //NS_LOG_FUNCTION (this);
  return m_peripheralContainer;
}

void
Drone::setMass (double mass)
{
  m_mass = mass;
  m_weightForce = m_mass * GRAVITY;
}

void
Drone::setArea (double area)
{
  m_diskArea = area;
}

void
Drone::setDragCoefficient (double coefficient)
{
  m_dragCoefficient = coefficient;
}

double
Drone::getMass () const
{
  return m_mass;
}

double
Drone::getWeight () const
{
  return m_weightForce;
}

double
Drone::getArea () const
{
  return m_diskArea;
}

double
Drone::getDragCoefficient () const
{
  return m_dragCoefficient;
}

} // namespace ns3
