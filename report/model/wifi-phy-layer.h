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
 *
 * Authors: Giovanni Grieco <giovanni.grieco@poliba.it>
 */
#ifndef WIFI_PHY_LAYER_H
#define WIFI_PHY_LAYER_H

#include <libxml/xmlwriter.h>

#include <ns3/packet.h>
#include <ns3/mac48-address.h>
#include <ns3/wifi-phy.h>
#include <ns3/wifi-tx-vector.h>

#include "protocol-layer.h"

namespace ns3 {

class WifiPhyLayer : public ProtocolLayer
{
public:
  /**
   * Register the type using ns-3 TypeId System.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /**
   * Default constructor
   */
  WifiPhyLayer ();
  /**
   * Default destructor
   */
  ~WifiPhyLayer ();

  /**
   * Write Protocol Stack report data to a XML file with a given handler
   *
   * \param handle the XML handler to write data on
   */
  virtual void Write (xmlTextWriterPtr handle);

protected:
  /**
   * Private initialization of the object
   */
  virtual void DoInitialize ();

private:
  /**
   * Initialize RSSI monitor in order to build a history of the RSSI for
   * the current entity with the current network device
   */
  void DoInitializeRssiMonitor ();

  /**
   * Handle the arrival of new RSSI data
   */
  void DoMonitorRssi (std::string context,
                      Ptr<const Packet> packet,
                      uint16_t channelFreqMhz,
                      WifiTxVector txVector,
                      MpduInfo aMpdu,
                      SignalNoiseDbm signalNoise,
                      uint16_t staId);

  uint32_t m_droneReference;            /// ID of the drone
  uint32_t m_netdevReference;           /// ID of the network device

  /// RSSI history of the device
  std::vector<std::tuple<Time, Mac48Address, double, uint16_t>> m_rssi;

  double m_frequency;                   /// the carrier frequency set
  std::string m_standard;               /// the wifi standard used
  std::string m_propagationDelayModel;  /// the prop. delay model set
  std::string m_propagationLossModel;   /// the prop. loss model set
};

} // namespace ns3

#endif /* WIFI_PHY_LAYER_H */
