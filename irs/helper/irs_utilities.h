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

namespace ns3 {

//ref: https://www.geeksforgeeks.org/program-to-find-equation-of-a-plane-passing-through-3-points/
class IrsUtil
{
public:
  static std::vector<double> director_coefficients(double x1, double y1,
                    double z1, double x2,
                    double y2, double z2,
                    double x3, double y3, double z3)
  {
    std::vector<double> coeff = {((y2 - y1) * (z3 - z1) - (y3 - y1) * (z2 - z1)), ((x3 - x1) * (z2 - z1) - (x2 - x1) * (z3 - z1)), ((x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1))};
    return coeff;
  }
};

} // namespace ns3

#endif /* IRS_UTILITIES_H */
