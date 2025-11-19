/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Tim Schubert <ns-3-leo@timschubert.net>
 */

#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/double.h"
#include "math.h"
#include "ns3/geographic-positions.h"
#include "isl-propagation-loss-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("IslPropagationLossModel");

NS_OBJECT_ENSURE_REGISTERED (IslPropagationLossModel);

TypeId
IslPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::IslPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .SetGroupName ("Leo")
    .AddConstructor<IslPropagationLossModel> ()
  ;
  return tid;
}

IslPropagationLossModel::IslPropagationLossModel ()
{
}

IslPropagationLossModel::~IslPropagationLossModel ()
{
}

bool
IslPropagationLossModel::GetLos (Ptr<MobilityModel> moda, Ptr<MobilityModel> modb)
{
  // origin of LOS
  Vector3D apos = moda->GetPosition ();
  Vector3D bpos = modb->GetPosition ();

  // select upper satellite as origin
  Vector oc = apos.GetLength () > bpos.GetLength () ? apos : bpos;
  Vector bp = apos.GetLength () > bpos.GetLength () ? bpos : apos;

  Vector3D u = bp - oc;
  // distance from s1 to s2
  double s2 = u.GetLength ();
  // direction unit vector
  u = Vector3D (u.x / s2, u.y / s2, u.z / s2);

  double a = u.x*u.x + u.y*u.y + u.z*u.z;
  double b = 2.0 * (oc.x*u.x + oc.y*u.y + oc.z*u.z);
  double c = (oc.x*oc.x + oc.y*oc.y + oc.z*oc.z) - (GeographicPositions::EARTH_SPHERE_RADIUS*GeographicPositions::EARTH_SPHERE_RADIUS);
  double discriminant = b*b - 4*a*c;

  NS_LOG_DEBUG ("a_pos="<<moda->GetPosition ()<<";b_pos"<<modb->GetPosition ()
  		<<";u="<<u
  		<<";a="<<a
  		<<";b="<<b
  		<<";c="<<c
  		<<";disc="<<discriminant);

  if (discriminant < 0)
    {
      return true;
    }
  else
    {
      // distances from oc (first sat)
      double t1 = (-b - sqrt (discriminant)) / (2.0 * a);
      double t2 = (-b + sqrt (discriminant)) / (2.0 * a);

      NS_LOG_DEBUG ("t1="<<t1
      		    <<"t2="<<t2
      		    <<"s2="<<s2);

      // check if second sat is behind earth
      return (s2 < abs (t1) && s2 < abs (t2));
    }
}

double
IslPropagationLossModel::DoCalcRxPower (double txPowerDbm,
                                        Ptr<MobilityModel> a,
                                        Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this << txPowerDbm << a << b);
  if (!GetLos (a, b))
    {
      return -1000.0;
    }

  return txPowerDbm;
}

int64_t
IslPropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}

};
