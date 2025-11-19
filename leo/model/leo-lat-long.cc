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

#include "leo-lat-long.h"

namespace ns3 {

std::ostream &operator << (std::ostream &os, const LeoLatLong &l)
{
  os << l.latitude << ":" << l.longitude;
  return os;
}

std::istream &operator >> (std::istream &is, LeoLatLong &l)
{
  char c1;
  is >> l.latitude >> c1 >> l.longitude;
  if (c1 != ':')
    {
      is.setstate (std::ios_base::failbit);
    }
  return is;
}

LeoLatLong::LeoLatLong () : latitude (0), longitude (0) {}
LeoLatLong::LeoLatLong (double la, double lo) : latitude (la), longitude (lo) {}
LeoLatLong::~LeoLatLong () {}

};
