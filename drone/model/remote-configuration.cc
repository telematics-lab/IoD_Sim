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
#include "remote-configuration.h"

namespace ns3 {

RemoteConfiguration::RemoteConfiguration (uint32_t netId,
                                          std::vector<ModelConfiguration> applications) :
  m_networkLayerId {netId},
  m_applications {applications}
{

}

RemoteConfiguration::~RemoteConfiguration ()
{

}

const uint32_t
RemoteConfiguration::GetNetworkLayerId () const
{
  return m_networkLayerId;
}

const std::vector<ModelConfiguration>&
RemoteConfiguration::GetApplications () const
{
  return m_applications;
}

} // namespace ns3
