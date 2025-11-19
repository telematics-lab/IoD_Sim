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

#ifndef ISL_PROPAGATION_LOSS_MODEL_H
#define ISL_PROPAGATION_LOSS_MODEL_H

#include <ns3/object.h>
#include <ns3/propagation-loss-model.h>

/**
 * \file
 * \ingroup leo
 *
 * Declaration of IslPropagationLossModel
 */

namespace ns3 {

/**
 * \ingroup leo
 * \brief An approximate model for the propagation loss between any two low
 * earth orbit satellites using the line-of-sight
 *
 */
class IslPropagationLossModel : public PropagationLossModel
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /// constructor
  IslPropagationLossModel ();
  /// destructor
  virtual ~IslPropagationLossModel ();

  /**
   * \brief Check if there is a direc line-of-sight between the two points
   *
   * Assumes earth is spherical.
   *
   * \param a first point
   * \param b second point
   * \return true iff there is a line-of-sight between the points
   */
  static bool GetLos (Ptr<MobilityModel> a, Ptr<MobilityModel> b);
private:
  /**
   * Returns the Rx Power taking into account only the particular
   * PropagationLossModel.
   *
   * \param txPowerDbm current transmission power (in dBm)
   * \param a the mobility model of the source
   * \param b the mobility model of the destination
   * \returns the reception power after adding/multiplying propagation loss (in dBm)
   */
  virtual double DoCalcRxPower (double txPowerDbm,
                                Ptr<MobilityModel> a,
                                Ptr<MobilityModel> b) const;

  virtual int64_t DoAssignStreams (int64_t stream);
};

}

#endif /* SATELLITE_ISL_PROPAGATION_LOSS_MODEL_H */
