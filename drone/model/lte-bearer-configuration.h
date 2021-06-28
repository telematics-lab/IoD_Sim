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
#ifndef LTE_BEARER_CONFIGURATION_H
#define LTE_BEARER_CONFIGURATION_H

#include <ns3/object.h>
#include <ns3/epc-tft.h>
#include <ns3/eps-bearer.h>

namespace ns3 {

/**
 * Data class to store parameters useful at LTE Bearer configuration.
 */
class LteBearerConfiguration : public Object
{
public:
  /**
   * Create a new object instance.
   *
   * \param type The type of the network device (e.g., "wifi" to use the underlying WiFi Protocol Stack).
   * \param macLayer The configuration of the MAC Layer to be simulated for this network device.
   * \param networkLayerId The identifier for the Network Layer that has been defined for this simulation.
   *                       It must be compatible with the given type and macLayer.
   */
  LteBearerConfiguration (const std::string type,
                          const uint64_t gbrDl,
                          const uint64_t gbrUl,
                          const uint64_t mbrDl,
                          const uint64_t mbrUl);
  /** Default destructor */
  ~LteBearerConfiguration ();

  /**
   * \return The type of the Network Device.
   */
  const EpsBearer::Qci GetType () const;
  /**
   * \return The reference network layer identifier.
   */
  const GbrQosInformation GetQos () const;

private:
  const EpsBearer::Qci m_type;
  const GbrQosInformation m_qos;
};

} // namespace ns3

#endif /* LTE_BEARER_CONFIGURATION_H */
