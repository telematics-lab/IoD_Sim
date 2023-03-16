/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2023 The IoD_Sim Authors.
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
#ifndef LTE_PHY_SIMULATION_HELPER_H
#define LTE_PHY_SIMULATION_HELPER_H

#include <ns3/object.h>
#include <ns3/lte-helper.h>
#include <ns3/point-to-point-epc-helper.h>

namespace ns3 {

/**
 * A data class to store information about a WiFi PHY layer configuration for a simulation.
 */
class LtePhySimulationHelper : public Object
{
public:
  /** Default constructor */
  LtePhySimulationHelper (const size_t stackId);
  /** Default destructor */
  ~LtePhySimulationHelper ();

  /**
   * \return The LTE Helper used to configure this layer.
   */
  Ptr<LteHelper> GetLteHelper ();
  /**
   * \return The LTE EPC PHY Helper used to configure this layer.
   */
  Ptr<PointToPointEpcHelper> GetEpcHelper ();

private:
  Ptr<LteHelper> m_lte;
  Ptr<PointToPointEpcHelper> m_epc;
};

} // namespace ns3

#endif /* LTE_PHY_SIMULATION_HELPER_H */
