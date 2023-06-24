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
 *
 * Authors: Giovanni Grieco <giovanni.grieco@poliba.it>
 */
#include "wifi-phy-layer.h"

#include <ns3/config.h>
#include <ns3/integer.h>
#include <ns3/ipv4-header.h>
#include <ns3/log.h>
#include <ns3/node-list.h>
#include <ns3/node.h>
#include <ns3/simulator.h>
#include <ns3/string.h>
#include <ns3/wifi-mac-header.h>

#include <algorithm>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("WifiPhyLayer");
NS_OBJECT_ENSURE_REGISTERED(WifiPhyLayer);

TypeId
WifiPhyLayer::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::WifiPhyLayer")
            .AddConstructor<WifiPhyLayer>()
            .SetParent<ProtocolLayer>()
            .AddAttribute("Frequency",
                          "Wifi Carrier Frequency in use, in MHz",
                          IntegerValue(0),
                          MakeIntegerAccessor(&WifiPhyLayer::m_frequency),
                          MakeIntegerChecker<uint16_t>())
            .AddAttribute("Standard",
                          "Wifi Standard in use",
                          StringValue(),
                          MakeStringAccessor(&WifiPhyLayer::m_standard),
                          MakeStringChecker())
            .AddAttribute("PropagationDelayModel",
                          "The propagation delay model used",
                          StringValue(),
                          MakeStringAccessor(&WifiPhyLayer::m_propagationDelayModel),
                          MakeStringChecker())
            .AddAttribute("PropagationLossModel",
                          "The propagation loss model used",
                          StringValue(),
                          MakeStringAccessor(&WifiPhyLayer::m_propagationLossModel),
                          MakeStringChecker())
            .AddAttribute("NodeId",
                          "Global unique identifier of the node",
                          IntegerValue(),
                          MakeIntegerAccessor(&WifiPhyLayer::m_droneReference),
                          MakeIntegerChecker<uint32_t>())
            .AddAttribute("NetdevId",
                          "ID of the Network Device of the given node",
                          IntegerValue(),
                          MakeIntegerAccessor(&WifiPhyLayer::m_netdevReference),
                          MakeIntegerChecker<uint32_t>());

    return tid;
}

WifiPhyLayer::WifiPhyLayer()
{
    NS_LOG_FUNCTION(this);
}

WifiPhyLayer::~WifiPhyLayer()
{
    NS_LOG_FUNCTION(this);
}

void
WifiPhyLayer::Write(xmlTextWriterPtr h)
{
    NS_LOG_FUNCTION(h);
    if (h == nullptr)
    {
        NS_LOG_WARN("Passed handler is not valid: " << h
                                                    << ". "
                                                       "Data will be discarded.");
        return;
    }

    int rc;

    rc = xmlTextWriterStartElement(h, BAD_CAST "phy");
    NS_ASSERT(rc >= 0);

    /* Nested Elements */
    rc = xmlTextWriterWriteElement(h, BAD_CAST "standard", BAD_CAST m_standard.c_str());
    NS_ASSERT(rc >= 0);

    std::stringstream bFrequency;
    bFrequency << m_frequency;
    rc = xmlTextWriterWriteElement(h, BAD_CAST "frequency", BAD_CAST bFrequency.str().c_str());
    NS_ASSERT(rc >= 0);

    std::stringstream bAddress;
    bAddress << m_address;
    rc = xmlTextWriterWriteElement(h, BAD_CAST "address", BAD_CAST bAddress.str().c_str());
    NS_ASSERT(rc >= 0);

    rc = xmlTextWriterWriteElement(h,
                                   BAD_CAST "propagationDelayModel",
                                   BAD_CAST m_propagationDelayModel.c_str());
    NS_ASSERT(rc >= 0);

    rc = xmlTextWriterWriteElement(h,
                                   BAD_CAST "propagationLossModel",
                                   BAD_CAST m_propagationLossModel.c_str());
    NS_ASSERT(rc >= 0);

    rc = xmlTextWriterStartElement(h, BAD_CAST "signal");
    NS_ASSERT(rc >= 0);

    for (auto& sig : m_rssi)
    {
        std::ostringstream bSig, bTx, bTime, bStaId;

        rc = xmlTextWriterStartElement(h, BAD_CAST "rssi");
        NS_ASSERT(rc >= 0);

        bTime << std::get<0>(sig);
        rc = xmlTextWriterWriteAttribute(h, BAD_CAST "time", BAD_CAST bTime.str().c_str());
        NS_ASSERT(rc >= 0);

        bTx << std::get<1>(sig);
        rc = xmlTextWriterWriteAttribute(h, BAD_CAST "from", BAD_CAST bTx.str().c_str());
        NS_ASSERT(rc >= 0);

        bSig << std::get<2>(sig);
        rc = xmlTextWriterWriteAttribute(h, BAD_CAST "value", BAD_CAST bSig.str().c_str());
        NS_ASSERT(rc >= 0);

        rc = xmlTextWriterEndElement(h);
        NS_ASSERT(rc >= 0);
    }

    rc = xmlTextWriterEndElement(h); // signal
    NS_ASSERT(rc >= 0);

    rc = xmlTextWriterEndElement(h); // phy
    NS_ASSERT(rc >= 0);
}

void
WifiPhyLayer::DoInitialize()
{
    NS_LOG_FUNCTION_NOARGS();

    m_address = GetDeviceAddress(); // TODO: it would be usedul to report it in the XML file.
    DoInitializeRssiMonitor();

    ProtocolLayer::DoInitialize();
}

void
WifiPhyLayer::DoInitializeRssiMonitor()
{
    NS_LOG_FUNCTION_NOARGS();
    /* set callback using ns-3 XPath addressing system */
    std::stringstream xPathCallback;

    xPathCallback << "/NodeList/" << m_droneReference << "/DeviceList/" << m_netdevReference
                  << "/$ns3::WifiNetDevice/Phy/PhyRxBegin";
    Config::ConnectWithoutContext(xPathCallback.str(),
                                  MakeCallback(&WifiPhyLayer::DoMonitorRssi, this));
}

Mac48Address
WifiPhyLayer::GetDeviceAddress()
{
    auto n = NodeList::GetNode(m_droneReference);
    auto d = n->GetDevice(m_netdevReference);
    auto a = d->GetAddress();
    return Mac48Address::ConvertFrom(a);
}

void
WifiPhyLayer::DoMonitorRssi(Ptr<const Packet> packet, RxPowerWattPerChannelBand rxPowersW)
{
    // The total RX power corresponds to the maximum over all the bands
    auto it = std::max_element(
        rxPowersW.begin(),
        rxPowersW.end(),
        [](const RxPowerWattPerChannelBand::value_type& p1,
           const RxPowerWattPerChannelBand::value_type& p2) { return p1 < p2; });
    auto rssi = 10 * log(it->second / 1e-3 /* to mW */);

    WifiMacHeader hdr;
    Mac48Address sender;
    packet->PeekHeader(hdr);

    // check if packet is for this net device
    auto da = (hdr.IsToDs()) ? hdr.GetAddr3() : hdr.GetAddr1();
    if (da != m_address)
        return;

    if ((!hdr.IsToDs() && !hdr.IsFromDs()) || (hdr.IsToDs() && !hdr.IsFromDs()))
        sender = hdr.GetAddr2();
    else if (!hdr.IsToDs() && hdr.IsFromDs())
        sender = hdr.GetAddr3();
    else if (hdr.IsToDs() && hdr.IsFromDs())
        sender = hdr.GetAddr4();

    // filter out any sender, which happens in case of CTL ACK frames
    if (sender == Mac48Address())
        return;

    m_rssi.push_back({Simulator::Now().GetSeconds(), sender, rssi});
}

} // namespace ns3
