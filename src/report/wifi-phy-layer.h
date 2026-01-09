/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2026 The IoD_Sim Authors.
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

#include "protocol-layer.h"

#include <ns3/mac48-address.h>
#include <ns3/packet.h>
#include <ns3/wifi-phy.h>
#include <ns3/wifi-tx-vector.h>

#include <libxml/xmlwriter.h>

namespace ns3
{

/**
 * \ingroup report
 * \brief Define a Wifi PHY Layer for report generation.
 *
 * Defines an object which contains the Wifi PHY Layer informations.
 */
class WifiPhyLayer : public ProtocolLayer
{
  public:
    typedef std::tuple<double /* Sim Time */, Mac48Address /* Signal From */, double /* dBm */>
        RssiSample;

    /**
     * Register the type using ns-3 TypeId System.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * Write Protocol Stack report data to a XML file with a given handler
     *
     * \param handle the XML handler to write data on
     */
    virtual void Write(xmlTextWriterPtr handle);

  protected:
    /**
     * Private initialization of the object
     */
    virtual void DoInitialize();

  private:
    /**
     * Retrieve device MAC Address.
     */
    Mac48Address GetDeviceAddress();
    /**
     * Initialize RSSI monitor in order to build a history of the RSSI for
     * the current entity with the current network device
     */
    void DoInitializeRssiMonitor();

    /**
     * Handle the arrival of new RSSI data
     */
    void DoMonitorRssi(Ptr<const Packet> packet, RxPowerWattPerChannelBand rxPowersW);

    uint32_t m_droneReference;  /// ID of the drone
    uint32_t m_netdevReference; /// ID of the network device

    std::vector<RssiSample> m_rssi; /// RSSI history of the device

    double m_frequency;                  /// the carrier frequency set
    Mac48Address m_address;              /// Device MAC Address
    std::string m_standard;              /// the wifi standard used
    std::string m_propagationDelayModel; /// the prop. delay model set
    std::string m_propagationLossModel;  /// the prop. loss model set
};

} // namespace ns3

#endif /* WIFI_PHY_LAYER_H */
