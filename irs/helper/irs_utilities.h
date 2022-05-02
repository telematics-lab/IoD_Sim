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
#ifndef IRS_UTILITIES_H
#define IRS_UTILITIES_H

#include <vector>
#include <cmath>
#include <ns3/vector.h>
#include <ns3/pointer.h>
#include <ns3/mobility-model.h>

namespace ns3 {

class IrsUtil
{
public:
  static std::vector<double>
  GetPlaneCoefficients (Vector P1, Vector P2, Vector P3)
  {
    double a = ((P2.y - P1.y) * (P3.z - P1.z) - (P3.y - P1.y) * (P2.z - P1.z));
    double b = ((P3.x - P1.x) * (P2.z - P1.z) - (P2.x - P1.x) * (P3.z - P1.z));
    double c = ((P2.x - P1.x) * (P3.y - P1.y) - (P2.y - P1.y) * (P3.x - P1.x));
    double d = (-a * P1.x - b * P1.y - c * P1.z);
    std::vector<double> coeff = {a, b, c, d}; //ax+by+cz+d=0
    return coeff;
  }

  static Vector
  Calc2DCoordinate (int m, int n, int M_columns, int N_rows, double x_Side, double y_Side)
  {
    double x = x_Side * (m - (M_columns + 1) / 2);
    double y = y_Side * (n - (N_rows + 1) / 2);
    double z = 0;

    return Vector (x, y, z);
  }

  static Vector
  Rotate (Vector P, char axis, double angle)
  {
    double x, y, z;
    switch (axis)
      {
      case 'x': //X=[1 0 0; 0 cos(angle) -sin(angle); 0 sin(angle) cos(angle)];
        x = P.x;
        y = P.y * cos (angle) - P.z * (sin (angle));
        z = P.y * sin (angle) + P.z * (cos (angle));
        break;
      case 'y': //Y=[cos(angle) 0 sin(angle); 0 1 0; -sin(angle) 0 cos(angle)];
        x = P.x * cos (angle) + P.z * (sin (angle));
        y = P.y;
        z = P.z * (cos (angle)) - P.x * sin (angle);
        break;
      case 'z': //Z=[cos(angle) -sin(angle) 0; sin(angle) cos(angle) 0; 0 0 1];
        x = P.x * cos (angle) - P.y * (sin (angle));
        y = P.x * sin (angle) + P.y * (cos (angle));
        z = P.z;
        break;
      }
    return Vector (x, y, z);
  }

  static Vector
  Shift (Vector PR, Ptr<MobilityModel> MM)
  {
    Vector position = MM->GetPosition ();
    return Vector (PR.x + position.x, PR.y + position.y, PR.z + position.z);
  }
};

} // namespace ns3

#endif /* IRS_UTILITIES_H */
