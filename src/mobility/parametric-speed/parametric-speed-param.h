/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2024 The IoD_Sim Authors.
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
#ifndef PARAMETRIC_SPEED_PARAM_H
#define PARAMETRIC_SPEED_PARAM_H

#include <vector>

namespace ns3
{

class ParametricSpeedParam
{
  public:
    ParametricSpeedParam(std::vector<double> speedCoefficients);

    const std::vector<double> GetSpeedCoefficients() const;

    void Add(double coefficient);

  private:
    std::vector<double> m_speedCoefficients;
};

} // namespace ns3

#endif /* PARAMETRIC_SPEED_PARAM_H */
