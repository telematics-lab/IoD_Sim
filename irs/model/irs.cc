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
#include <irs.h>
#include <ns3/irs_utilities.h>
#include <vector>
#include <cmath>
#include <ns3/vector.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Irs"); //??

NS_OBJECT_ENSURE_REGISTERED (Irs); //??

TypeId
Irs::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Irs")
          .SetParent<Object> ()
          .SetGroupName ("Peripheral") //??
          .AddConstructor<Irs> ()
       /*   .AddAttribute ("PowerConsumption", "The power consumption [J/s] of the peripheral, in OFF|IDLE|ON states",
                   DoubleVectorValue (),
                   MakeDoubleVectorAccessor (&DronePeripheral::SetPowerConsumptionStates),
                   MakeDoubleVectorChecker ()*/)
  return tid;
}

Irs::Irs () //constr
{
}

void
Irs::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void
Irs::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  Object::DoInitialize ();
}

void
Irs::SetRows (int r)
{
  NS_LOG_FUNCTION (this);
  m_rows = r;
}

int
Irs::GetRows () const
{
  return m_rows;
}

void
Irs::SetColumns (int c)
{
  NS_LOG_FUNCTION (this);
  m_columns = c;
}

int
Irs::GetColumns () const
{
  return m_columns;
}

void
Irs::SetPruX (double d)
{
  NS_LOG_FUNCTION (this);
  m_pruX = d;
}

double
Irs::GetPruX () const
{
  return m_pruX;
}

void
Irs::SetPruY (double d)
{
  NS_LOG_FUNCTION (this);
  m_pruY = d;
}

double
Irs::GetPruY () const
{
  return m_pruY;
}

void
Irs::SetRotoAxis (double a)
{
  m_rotoAxis = a;
}

std::vector<char>
Irs::GetRotoAxis () const
{
  return m_rotoAxis;
}

void
Irs::SetRotoAnglesRadians (std::vector<double> a)
{
  m_rotoAngles = a;
}

void
Irs::SetRotoAnglesDegrees (std::vector<double> a)
{
  for (double angle : a)
    {
      m_rotoAngles.push_back (angle / 180 * M_PI);
    }
}

std::vector<double>
Irs::GetRotoAngles () const
{
  return m_rotoAngles;
}

Vector
Irs::CalcPruPosition (int m, int n, Ptr<MobilityModel> MM) const
{
  Vector position = IrsUtil.Calc2DCoordinate (m, n, m_columns, m_rows, m_pruX, m_pruY);
  int i = 0;
  for (char axis : m_rotoAxis)
    {
      position = IrsUtil.Rotate (position, axis, m_rotoAngles[i]);
      i++;
    }
  position = IrsUtil.Shift (position, MM);
}

} //namespace ns3
