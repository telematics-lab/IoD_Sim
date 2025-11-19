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

#include <math.h>

#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/double.h"

#include "leo-propagation-loss-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LeoPropagationLossModel");

NS_OBJECT_ENSURE_REGISTERED (LeoPropagationLossModel);

TypeId
LeoPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LeoPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .SetGroupName ("Leo")
    .AddConstructor<LeoPropagationLossModel> ()
    .AddAttribute ("ElevationAngle",
                   "Cut-off angle for signal propagation",
                   DoubleValue (40.0),
                   MakeDoubleAccessor (&LeoPropagationLossModel::SetElevationAngle,
				       &LeoPropagationLossModel::GetElevationAngle),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("AtmosphericLoss",
                   "Atmospheric loss due to attenuation in dB",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&LeoPropagationLossModel::m_atmosphericLoss),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("FreeSpacePathLoss",
                   "Free space path loss in dB",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&LeoPropagationLossModel::m_freeSpacePathLoss),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("LinkMargin",
                   "Link margin in dB",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&LeoPropagationLossModel::m_linkMargin),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

LeoPropagationLossModel::LeoPropagationLossModel ()
{
}

LeoPropagationLossModel::~LeoPropagationLossModel ()
{
}

void
LeoPropagationLossModel::SetElevationAngle (double angle)
{
  m_elevationAngle = angle * (M_PI/180.0);
}

double
LeoPropagationLossModel::GetCutoffDistance (const Ptr<MobilityModel> sat) const
{
  double angle = m_elevationAngle;
  double hs = sat->GetPosition ().GetLength ();

  double a = 1 + tan (angle) * tan (angle);
  double b = 2.0 * tan (angle) * hs;
  double c = hs*hs - LEO_PROP_EARTH_RAD*LEO_PROP_EARTH_RAD;

  double disc = b*b + 4*a*c;

  NS_LOG_DEBUG ("angle="<<angle<<" hs="<<hs<<" a="<<a<<" b="<<b<<" c="<<c<<" disc="<<disc);

  if (disc < 0)
    {
      // point not on earth surface
      return - 1.0;
    }

  double t1 = (-b - sqrt (disc)) / (2.0 * a);
  double t2 = (-b + sqrt (disc)) / (2.0 * a);

  double num1 = Vector2D (t1, - tan (angle) * t1).GetLength ();
  double num2 = Vector2D (t2, - tan (angle) * t2).GetLength ();

  return fmin (num1, num2);
}

double
LeoPropagationLossModel::GetElevationAngle () const
{
  return m_elevationAngle * (180.0/M_PI);
}

double
LeoPropagationLossModel::DoCalcRxPower (double txPowerDbm,
                                        Ptr<MobilityModel> a,
                                        Ptr<MobilityModel> b) const
{
  Ptr<MobilityModel> sat = a->GetPosition ().GetLength () > b->GetPosition ().GetLength () ? a : b;
  double distance = a->GetDistanceFrom (b);
  double cutOff = GetCutoffDistance (sat);
  if (distance > cutOff)
    {
      NS_LOG_DEBUG ("LEO DROP distance: a=" << a->GetPosition () << " b=" << b->GetPosition ()<<" dist=" << distance<<" cutoff="<<cutOff);

      return -1000.0;
    }

  // txPowerDbm includes tx antenna gain and losses
  // receiver loss and gain added at net device
  // P_{RX} = P_{TX} + G_{TX} - L_{TX} - L_{FS} - L_M + G_{RX} - L_{RX}
  double rxc = txPowerDbm - m_atmosphericLoss - m_freeSpacePathLoss - m_linkMargin;
  NS_LOG_DEBUG ("LEO TRANSMIT distance: a=" << a->GetPosition () << " b=" << b->GetPosition ()<<" dist=" << distance <<" cutoff="<<cutOff<< "rxc=" << rxc);

  return rxc;
}

int64_t
LeoPropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}

};
