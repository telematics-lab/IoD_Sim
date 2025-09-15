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
#include "nr-phy-simulation-helper.h"

#include "scenario-configuration-helper.h"

#include <ns3/boolean.h>
#include <ns3/string.h>

namespace ns3
{

class NrPhySimulationHelperPriv
{
  public:
    static std::string GetS1uLinkPcapPrefix(const size_t stackId)
    {
        std::stringstream prefixBuilder;
        prefixBuilder << CONFIGURATOR->GetResultsPath() << "nr-" << stackId << "-s1u";

        return prefixBuilder.str();
    }

    static std::string GetX2LinkPcapPrefix(const size_t stackId)
    {
        std::stringstream prefixBuilder;
        prefixBuilder << CONFIGURATOR->GetResultsPath() << "nr-" << stackId << "-x2";

        return prefixBuilder.str();
    }
};

NrPhySimulationHelper::NrPhySimulationHelper(const size_t stackId)
    : m_nr{CreateObject<NrHelper>()},
      m_nr_epc{CreateObjectWithAttributes<NrPointToPointEpcHelper>(
          "S1uLinkEnablePcap",
          BooleanValue(CONFIGURATOR->GetLogOnFile()),
          "S1uLinkPcapPrefix",
          StringValue(NrPhySimulationHelperPriv::GetS1uLinkPcapPrefix(stackId)),
          "X2LinkEnablePcap",
          BooleanValue(CONFIGURATOR->GetLogOnFile()),
          "X2LinkPcapPrefix",
          StringValue(NrPhySimulationHelperPriv::GetX2LinkPcapPrefix(stackId)))},
      m_ideal_beam{CreateObject<IdealBeamformingHelper>()}
{
    m_nr->SetEpcHelper(m_nr_epc);
    m_nr->SetBeamformingHelper(m_ideal_beam);
}

NrPhySimulationHelper::~NrPhySimulationHelper()
{
}

Ptr<NrHelper>
NrPhySimulationHelper::GetNrHelper()
{
    return m_nr;
}

Ptr<NrPointToPointEpcHelper>
NrPhySimulationHelper::GetNrEpcHelper()
{
    return m_nr_epc;
}

Ptr<IdealBeamformingHelper>
NrPhySimulationHelper::GetIdealBeamformingHelper()
{
    return m_ideal_beam;
}

} // namespace ns3
