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
#include "lte-phy-simulation-helper.h"

#include <ns3/boolean.h>
#include <ns3/string.h>

#include <ns3/scenario-configuration-helper.h>

namespace ns3 {

class LtePhySimulationHelperPriv
{
public:
  static std::string GetS1uLinkPcapPrefix (const size_t stackId)
  {
    std::stringstream prefixBuilder;
    prefixBuilder << CONFIGURATOR->GetResultsPath () << "lte-" << stackId << "-s1u";

    return prefixBuilder.str ();
  }

  static std::string GetX2LinkPcapPrefix (const size_t stackId)
  {
    std::stringstream prefixBuilder;
    prefixBuilder << CONFIGURATOR->GetResultsPath () << "lte-" << stackId << "-x2";

    return prefixBuilder.str ();
  }
};

LtePhySimulationHelper::LtePhySimulationHelper (const size_t stackId) :
  m_lte {CreateObject<LteHelper> ()},
  m_epc {CreateObjectWithAttributes<PointToPointEpcHelper> ("S1uLinkEnablePcap", BooleanValue (true),
                                                            "S1uLinkPcapPrefix", StringValue (LtePhySimulationHelperPriv::GetS1uLinkPcapPrefix (stackId)),
                                                            "X2LinkEnablePcap", BooleanValue (true),
                                                            "X2LinkPcapPrefix" , StringValue (LtePhySimulationHelperPriv::GetX2LinkPcapPrefix (stackId)))}
{
  m_lte->SetEpcHelper(m_epc);
}

LtePhySimulationHelper::~LtePhySimulationHelper ()
{

}

Ptr<LteHelper>
LtePhySimulationHelper::GetLteHelper ()
{
  return m_lte;
}

Ptr<PointToPointEpcHelper>
LtePhySimulationHelper::GetEpcHelper ()
{
  return m_epc;
}

} // namespace ns3
