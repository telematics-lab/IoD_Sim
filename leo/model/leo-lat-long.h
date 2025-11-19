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

#ifndef LEO_LAT_LONG_H
#define LEO_LAT_LONG_H

#include <iostream>

/**
 * \file
 * \ingroup leo
 *
 * Declaration of LeoLatLong
 */

namespace ns3 {

/**
 * \brief Wrapper around a pair of latitude and longitude
 */
class LeoLatLong
{
public:
  /// constructor
  LeoLatLong ();
  /// destructor
  LeoLatLong (double latitude, double longitude);
  virtual ~LeoLatLong();

  /// Latitude
  double latitude;
  /// Longitude
  double longitude;
};

/**
 * \brief Serialize the LeoLatLong instance using a colon as a separator
 * \param os stream to serialize into
 * \param latLong to serialize
 * \return returns the stream
 */
std::ostream &operator << (std::ostream &os, const LeoLatLong &latLong);

/**
 * \brief Deserialize the LeoLatLong instance using a colon as a separator
 * \param is stream to deserialize from
 * \param latLong to deserialize into
 * \return returns the stream
 */
std::istream &operator >> (std::istream &is, LeoLatLong &latLong);

};

#endif
