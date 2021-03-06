/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2021 The IoD_Sim Authors.
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
#ifndef NETDEVICE_CONFIGURATION_H
#define NETDEVICE_CONFIGURATION_H

#include <ns3/object.h>
#include "model-configuration.h"

namespace ns3 {

class NetdeviceConfiguration : public Object
{
public:
  NetdeviceConfiguration (const std::string type,
                          const ModelConfiguration macLayer,
                          const uint32_t networkLayerId);
  ~NetdeviceConfiguration ();

  const std::string GetType () const;
  const ModelConfiguration GetMacLayer () const;
  const uint32_t GetNetworkLayerId () const;

private:
  const std::string m_type;
  const ModelConfiguration m_macLayer;
  const uint32_t m_networkLayerId;
};

} // namespace ns3

#endif /* NETDEVICE_CONFIGURATION_H */
