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
#ifndef NR_PHY_SIMULATION_HELPER_H
#define NR_PHY_SIMULATION_HELPER_H

#include <ns3/ideal-beamforming-helper.h>
#include <ns3/nr-helper.h>
#include <ns3/nr-point-to-point-epc-helper.h>
#include <ns3/object.h>

namespace ns3
{

/**
 * A data class to store information about a WiFi PHY layer configuration for a simulation.
 */
class NrPhySimulationHelper : public Object
{
  public:
    /** Default constructor */
    NrPhySimulationHelper(const size_t stackId);
    /** Default destructor */
    ~NrPhySimulationHelper();

    /**
     * \return The NR Helper used to configure this layer.
     */
    Ptr<NrHelper> GetNrHelper();
    /**
     * \return The NR EPC PHY Helper used to configure this layer.
     */
    Ptr<NrPointToPointEpcHelper> GetNrEpcHelper();
    /**
     * \return The Ideal Beamforming Helper used to configure Beamforming.
     */
    Ptr<IdealBeamformingHelper> GetIdealBeamformingHelper();

  private:
    Ptr<NrHelper> m_nr;
    Ptr<NrPointToPointEpcHelper> m_nr_epc;
    Ptr<IdealBeamformingHelper> m_ideal_beam;
};

} // namespace ns3

#endif /* NR_PHY_SIMULATION_HELPER_H */
