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
#ifndef REMOTE_CONFIGURATION_H
#define REMOTE_CONFIGURATION_H

#include <ns3/model-configuration.h>
#include <ns3/object.h>

namespace ns3
{

/**
 * \brief Describe the configuration of a remote to be simulated.
 */
class RemoteConfiguration : public Object
{
  public:
    RemoteConfiguration(uint32_t networkLayerId, std::vector<ModelConfiguration> m_applications);

    const uint32_t GetNetworkLayerId() const;
    const std::vector<ModelConfiguration>& GetApplications() const;

  private:
    const uint32_t m_networkLayerId;
    const std::vector<ModelConfiguration> m_applications;
};

} // namespace ns3

#endif /* REMOTE_CONFIGURATION_H */
