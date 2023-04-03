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
 * Authors: Giovanni Grieco <giovanni.grieco@poliba.it>, Giovanni Iacovelli
 * <giovanni.iacovelli@poliba.it>
 */
#include "report-remote.h"

#include "drone-control-layer.h"
#include "ipv4-layer.h"
#include "wifi-inspector.h"
#include "wifi-mac-layer.h"
#include "wifi-phy-layer.h"

#include <ns3/application.h>
#include <ns3/config.h>
#include <ns3/integer.h>
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/log.h>
#include <ns3/mobility-module.h>
#include <ns3/node-list.h>
#include <ns3/simulator.h>
#include <ns3/string.h>
#include <ns3/udp-header.h>

#include <rapidjson/document.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ReportRemote");
NS_OBJECT_ENSURE_REGISTERED(ReportRemote);

TypeId
ReportRemote::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::ReportRemote").AddConstructor<ReportRemote>().SetParent<ReportEntity>();

    return tid;
}

ReportRemote::ReportRemote()
{
    NS_LOG_FUNCTION(this);
}

ReportRemote::~ReportRemote()
{
    NS_LOG_FUNCTION(this);
}

void
ReportRemote::DoWrite(xmlTextWriterPtr h)
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

    rc = xmlTextWriterStartElement(h, BAD_CAST "Remote");
    NS_ASSERT(rc >= 0);

    /* Nested Elements */
    rc = xmlTextWriterStartElement(h, BAD_CAST "NetDevices");
    NS_ASSERT(rc >= 0);

    for (int nid = 0; nid < (int)m_networkStacks.size(); nid++)
    {
        rc = xmlTextWriterStartElement(h, BAD_CAST "NetDevice");
        NS_ASSERT(rc >= 0);

        m_networkStacks[nid].Write(h);

        rc = xmlTextWriterStartElement(h, BAD_CAST "dataTx");
        NS_ASSERT(rc >= 0);

        for (auto rid = m_dataTx.begin(); rid != m_dataTx.end();)
        {
            if ((*rid)->GetIface() == nid)
            {
                (*rid)->Write(h);
                m_dataTx.erase(rid);
            }
            else
            {
                rid++;
            }
        }
        rc = xmlTextWriterEndElement(h);
        NS_ASSERT(rc >= 0);

        rc = xmlTextWriterStartElement(h, BAD_CAST "dataRx");
        NS_ASSERT(rc >= 0);

        for (auto rid = m_dataRx.begin(); rid != m_dataRx.end();)
        {
            if ((*rid)->GetIface() == nid)
            {
                (*rid)->Write(h);
                m_dataRx.erase(rid);
            }
            else
            {
                rid++;
            }
        }

        rc = xmlTextWriterEndElement(h);
        NS_ASSERT(rc >= 0);

        rc = xmlTextWriterEndElement(h);
        NS_ASSERT(rc >= 0);
    }
    rc = xmlTextWriterEndElement(h);
    NS_ASSERT(rc >= 0);

    rc = xmlTextWriterStartElement(h, BAD_CAST "dataTx");
    // broadcast
    NS_ASSERT(rc >= 0);

    for (auto rid = m_dataTx.begin(); rid != m_dataTx.end(); rid++)
        (*rid)->Write(h);

    rc = xmlTextWriterEndElement(h);
    NS_ASSERT(rc >= 0);

    rc = xmlTextWriterStartElement(h, BAD_CAST "dataRx");
    NS_ASSERT(rc >= 0);

    for (auto rid = m_dataRx.begin(); rid != m_dataRx.end(); rid++)
        (*rid)->Write(h);

    rc = xmlTextWriterEndElement(h);
    NS_ASSERT(rc >= 0);
    //////
    rc = xmlTextWriterStartElement(h, BAD_CAST "cumulativeDataTx");
    NS_ASSERT(rc >= 0);

    for (auto& dataStatsTx : m_cumulativeDataTx)
        dataStatsTx->Write(h);

    rc = xmlTextWriterEndElement(h);
    NS_ASSERT(rc >= 0);

    rc = xmlTextWriterStartElement(h, BAD_CAST "cumulativeDataRx");
    NS_ASSERT(rc >= 0);

    for (auto& dataStatsRx : m_cumulativeDataRx)
        dataStatsRx->Write(h);

    rc = xmlTextWriterEndElement(h);
    NS_ASSERT(rc >= 0);

    rc = xmlTextWriterEndElement(h);
    NS_ASSERT(rc >= 0);
}

} // namespace ns3
