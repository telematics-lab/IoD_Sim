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
 * Authors: Giovanni Iacovelli <giovanni.iacovelli@poliba.it>
 */
#include "lte-ue-phy-layer.h"

#include <ns3/log.h>
#include <ns3/string.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("LteUEPhyLayer");
NS_OBJECT_ENSURE_REGISTERED(LteUEPhyLayer);

TypeId
LteUEPhyLayer::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::LteUEPhyLayer")
            .AddConstructor<LteUEPhyLayer>()
            .SetParent<ProtocolLayer>()
            .AddAttribute("IMSI",
                          "IMSI",
                          StringValue(),
                          MakeStringAccessor(&LteUEPhyLayer::m_imsi),
                          MakeStringChecker())
            .AddAttribute("PropagationLossModelDl",
                          "The propagation loss model in downlink",
                          StringValue(),
                          MakeStringAccessor(&LteUEPhyLayer::m_pldl),
                          MakeStringChecker())
            .AddAttribute("DlEarfcn",
                          "Downlink E-UTRA Absolute Radio Frequency Channel Number",
                          StringValue(),
                          MakeStringAccessor(&LteUEPhyLayer::m_dlearfnc),
                          MakeStringChecker())
            .AddAttribute(
                "DlBandwidth",
                "Downlink Transmission Bandwidth Configuration in number of Resource Blocks",
                StringValue(),
                MakeStringAccessor(&LteUEPhyLayer::m_dlband),
                MakeStringChecker())
            .AddAttribute("PropagationLossModelUl",
                          "The propagation loss model in uplink",
                          StringValue(),
                          MakeStringAccessor(&LteUEPhyLayer::m_plul),
                          MakeStringChecker())
            .AddAttribute("UlEarfcn",
                          "Uplink E-UTRA Absolute Radio Frequency Channel Number",
                          StringValue(),
                          MakeStringAccessor(&LteUEPhyLayer::m_ulearfnc),
                          MakeStringChecker())
            .AddAttribute(
                "UlBandwidth",
                "Uplink Transmission Bandwidth Configuration in number of Resource Blocks",
                StringValue(),
                MakeStringAccessor(&LteUEPhyLayer::m_ulband),
                MakeStringChecker());

    return tid;
}

LteUEPhyLayer::LteUEPhyLayer()
{
    NS_LOG_FUNCTION(this);
}

LteUEPhyLayer::~LteUEPhyLayer()
{
    NS_LOG_FUNCTION(this);
}

void
LteUEPhyLayer::Write(xmlTextWriterPtr h)
{
    NS_LOG_FUNCTION(h);
    if (!h)
    {
        NS_LOG_WARN("Passed handler is not valid: " << h
                                                    << ". "
                                                       "Data will be discarded.");
        return;
    }

    int rc;

    rc = xmlTextWriterStartElement(h, BAD_CAST "phy");
    NS_ASSERT(rc >= 0);

    rc = xmlTextWriterWriteElement(h, BAD_CAST "IMSI", BAD_CAST m_imsi.c_str());
    NS_ASSERT(rc >= 0);

    rc = xmlTextWriterWriteElement(h, BAD_CAST "PropagationLossModelDl", BAD_CAST m_pldl.c_str());
    NS_ASSERT(rc >= 0);

    rc = xmlTextWriterWriteElement(h, BAD_CAST "DlEarfcn", BAD_CAST m_dlearfnc.c_str());
    NS_ASSERT(rc >= 0);

    rc = xmlTextWriterWriteElement(h, BAD_CAST "DlBandwidth", BAD_CAST m_dlband.c_str());
    NS_ASSERT(rc >= 0);

    rc = xmlTextWriterWriteElement(h, BAD_CAST "PropagationLossModelUl", BAD_CAST m_plul.c_str());
    NS_ASSERT(rc >= 0);

    rc = xmlTextWriterWriteElement(h, BAD_CAST "UlEarfcn", BAD_CAST m_ulearfnc.c_str());
    NS_ASSERT(rc >= 0);

    rc = xmlTextWriterWriteElement(h, BAD_CAST "UlBandwidth", BAD_CAST m_ulband.c_str());

    rc = xmlTextWriterEndElement(h);
    NS_ASSERT(rc >= 0);
}

} // namespace ns3
