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
 */

#include "nr-sinr-distance-handover-algorithm.h"

#include "../scenario.h"
#include "nr-radio-geo-environment-map-helper.h"

#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/node-list.h"
#include "ns3/nr-common.h"
#include "ns3/nr-gnb-net-device.h"
#include "ns3/nr-gnb-rrc.h"
#include "ns3/nr-ue-net-device.h"
#include "ns3/nr-ue-rrc.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"

#include <iostream>
#include <sstream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrSinrDistanceHandoverAlgorithm");

NS_OBJECT_ENSURE_REGISTERED(NrSinrDistanceHandoverAlgorithm);

NrSinrDistanceHandoverAlgorithm::Provider::Provider(NrSinrDistanceHandoverAlgorithm* algorithm)
    : m_algorithm(algorithm)
{
}

void
NrSinrDistanceHandoverAlgorithm::Provider::ReportUeMeas(uint16_t rnti,
                                                        NrRrcSap::MeasResults measResults)
{
    m_algorithm->DoReportUeMeas(rnti, measResults);
}

TypeId
NrSinrDistanceHandoverAlgorithm::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrSinrDistanceHandoverAlgorithm")
            .SetParent<NrHandoverAlgorithm>()
            .SetGroupName("Nr")
            .AddConstructor<NrSinrDistanceHandoverAlgorithm>()
            .AddAttribute("Threshold",
                          "Hysteresis threshold (in dB) for switching gNBs.",
                          DoubleValue(2.0),
                          MakeDoubleAccessor(&NrSinrDistanceHandoverAlgorithm::m_threshold),
                          MakeDoubleChecker<double>())
            .AddAttribute(
                "DistanceSinrTable",
                "A CSV formatted table: maxDist1:minSinr1|maxDist2:minSinr2",
                StringValue(""),
                MakeStringAccessor(&NrSinrDistanceHandoverAlgorithm::SetDistanceSinrTable,
                                   &NrSinrDistanceHandoverAlgorithm::GetDistanceSinrTable),
                MakeStringChecker())
            .AddAttribute("TimeToTrigger",
                          "Time during which neighbour cell's RSRP must be continuously higher "
                          "than serving cell's RSRP to trigger a handover report.",
                          TimeValue(MilliSeconds(256)),
                          MakeTimeAccessor(&NrSinrDistanceHandoverAlgorithm::m_timeToTrigger),
                          MakeTimeChecker())
            .AddAttribute("HandoverDelay",
                          "Artificial delay added before triggering the handover.",
                          TimeValue(Seconds(0)),
                          MakeTimeAccessor(&NrSinrDistanceHandoverAlgorithm::m_handoverDelay),
                          MakeTimeChecker());
    return tid;
}

NrSinrDistanceHandoverAlgorithm::NrSinrDistanceHandoverAlgorithm()
    : m_handoverManagementSapUser(nullptr),
      m_handoverManagementSapProvider(nullptr),
      m_threshold(2.0),
      m_timeToTrigger(MilliSeconds(256)),
      m_handoverDelay(Seconds(0))
{
    m_handoverManagementSapProvider = new Provider(this);
}

NrSinrDistanceHandoverAlgorithm::~NrSinrDistanceHandoverAlgorithm()
{
}

void
NrSinrDistanceHandoverAlgorithm::DoInitialize()
{
    NS_LOG_FUNCTION(this);

    // Using Event A3 as the trigger for our evaluation to match default handover algorithms.
    // The m_threshold is used here as hysteresis/a3Offset equivalent.
    int8_t a3OffsetIeValue = 0;
    uint8_t hysteresisIeValue = 0;
    if (m_threshold >= 0.0)
    {
        hysteresisIeValue = nr::EutranMeasurementMapping::ActualHysteresis2IeValue(m_threshold);
    }
    else
    {
        a3OffsetIeValue = nr::EutranMeasurementMapping::ActualA3Offset2IeValue(m_threshold);
    }

    NS_LOG_LOGIC(this << " requesting Event A3 measurements"
                      << " (hysteresis=" << (uint16_t)hysteresisIeValue
                      << " a3Offset=" << (int16_t)a3OffsetIeValue << ")"
                      << " (ttt=" << m_timeToTrigger.As(Time::MS) << ")");

    NrRrcSap::ReportConfigEutra reportConfig;
    reportConfig.eventId = NrRrcSap::ReportConfigEutra::EVENT_A3;
    reportConfig.a3Offset = a3OffsetIeValue;
    reportConfig.hysteresis = hysteresisIeValue;
    reportConfig.timeToTrigger = m_timeToTrigger.GetMilliSeconds();
    reportConfig.reportOnLeave = false;
    reportConfig.triggerQuantity = NrRrcSap::ReportConfigEutra::RSRP;
    reportConfig.reportInterval = NrRrcSap::ReportConfigEutra::MS1024;
    m_measIds = m_handoverManagementSapUser->AddUeMeasReportConfigForHandover(reportConfig);

    NrHandoverAlgorithm::DoInitialize();
}

void
NrSinrDistanceHandoverAlgorithm::DoDispose()
{
    delete m_handoverManagementSapProvider;
    m_handoverManagementSapProvider = nullptr;
    m_handoverManagementSapUser = nullptr;
}

void
NrSinrDistanceHandoverAlgorithm::SetNrHandoverManagementSapUser(NrHandoverManagementSapUser* s)
{
    m_handoverManagementSapUser = s;
}

NrHandoverManagementSapProvider*
NrSinrDistanceHandoverAlgorithm::GetNrHandoverManagementSapProvider()
{
    NS_LOG_FUNCTION(this);
    return m_handoverManagementSapProvider;
}

void
NrSinrDistanceHandoverAlgorithm::SetDistanceSinrTable(std::string tableStr)
{
    m_tableStr = tableStr;
    ParseTableString(tableStr);
}

std::string
NrSinrDistanceHandoverAlgorithm::GetDistanceSinrTable() const
{
    return m_tableStr;
}

void
NrSinrDistanceHandoverAlgorithm::ParseTableString(const std::string& tableStr)
{
    m_table.clear();
    if (tableStr.empty())
        return;

    std::stringstream ss(tableStr);
    std::string token;
    while (std::getline(ss, token, '|'))
    {
        auto colonPos = token.find(':');
        if (colonPos != std::string::npos)
        {
            double dist = std::stod(token.substr(0, colonPos));
            double sinr = std::stod(token.substr(colonPos + 1));
            m_table.push_back({dist, sinr});
        }
    }
}

void
NrSinrDistanceHandoverAlgorithm::DoReportUeMeas(uint16_t rnti, NrRrcSap::MeasResults measResults)
{
    NS_LOG_FUNCTION(this << rnti << (uint16_t)measResults.measId);

    if (std::find(m_measIds.begin(), m_measIds.end(), measResults.measId) == m_measIds.end())
    {
        NS_LOG_WARN("Ignoring measId " << (uint16_t)measResults.measId);
        return;
    }

    Ptr<NrUeNetDevice> ueDevice = nullptr;
    Ptr<NrGnbNetDevice> currentGnb = nullptr;
    NetDeviceContainer allGnbDevices;

    // Search globally for the required devices
    for (NodeList::Iterator i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        Ptr<Node> node = *i;
        for (uint32_t j = 0; j < node->GetNDevices(); ++j)
        {
            Ptr<NetDevice> dev = node->GetDevice(j);
            if (auto gnb = DynamicCast<NrGnbNetDevice>(dev))
            {
                allGnbDevices.Add(gnb);
                PointerValue ptrVal;
                gnb->GetAttribute("NrHandoverAlgorithm", ptrVal);
                if (ptrVal.Get<NrHandoverAlgorithm>().operator->() == this)
                {
                    currentGnb = gnb;
                }
            }
        }
    }

    if (!currentGnb)
    {
        NS_LOG_WARN("Could not find Serving gNB for this HandoverAlgorithm in global NodeList.");
        return;
    }

    uint16_t servingCellId = currentGnb->GetCellId();

    for (NodeList::Iterator i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        Ptr<Node> node = *i;
        for (uint32_t j = 0; j < node->GetNDevices(); ++j)
        {
            Ptr<NetDevice> dev = node->GetDevice(j);
            if (auto ue = DynamicCast<NrUeNetDevice>(dev))
            {
                if (ue->GetRrc() && ue->GetRrc()->GetRnti() == rnti &&
                    ue->GetRrc()->GetCellId() == servingCellId)
                {
                    ueDevice = ue;
                    break;
                }
            }
        }
        if (ueDevice)
            break;
    }

    if (!ueDevice || !currentGnb)
    {
        NS_LOG_WARN("Could not find UE or Serving gNB in global NodeList.");
        return;
    }

    Ptr<MobilityModel> ueMobility = ueDevice->GetNode()->GetObject<MobilityModel>();
    if (!ueMobility)
    {
        return;
    }

    // Best (gNB, bwpId) pair found
    Ptr<NrGnbNetDevice> bestGnb = nullptr;
    double bestSnr = -std::numeric_limits<double>::infinity();

    double currentSnr = -std::numeric_limits<double>::infinity();
    bool currentGnbValid = false;

    for (uint32_t k = 0; k < allGnbDevices.GetN(); ++k)
    {
        Ptr<NrGnbNetDevice> gnbDevice = DynamicCast<NrGnbNetDevice>(allGnbDevices.Get(k));
        Ptr<MobilityModel> gnbMobility = gnbDevice->GetNode()->GetObject<MobilityModel>();
        if (!gnbMobility)
            continue;

        double distance = ueMobility->GetDistanceFrom(gnbMobility);

        const TableEntry* bestEntry = nullptr;
        double rangeDiff = std::numeric_limits<double>::max();
        for (const auto& entry : m_table)
        {
            if (distance <= entry.maxDistance && entry.maxDistance < rangeDiff)
            {
                rangeDiff = entry.maxDistance;
                bestEntry = &entry;
            }
        }

        if (!bestEntry)
            continue;

        double minSinrRequired = bestEntry->minSinr;
        uint32_t gnbBwpCount = gnbDevice->GetCcMapSize(); // Or NrHelper::GetNumberBwp(gnbDevice)

        for (uint32_t bwpId = 0; bwpId < gnbBwpCount; ++bwpId)
        {
            Ptr<NrRadioGeoEnvironmentMapHelper> remHelper =
                CreateObject<NrRadioGeoEnvironmentMapHelper>();
            remHelper->SetInterferers(allGnbDevices, bwpId);

            double estimatedSnr = remHelper->GetSnr(ueDevice, gnbDevice, bwpId, true);

            if (estimatedSnr >= minSinrRequired)
            {
                if (gnbDevice == currentGnb)
                {
                    currentSnr = estimatedSnr;
                    currentGnbValid = true;
                }

                if (estimatedSnr > bestSnr)
                {
                    bestSnr = estimatedSnr;
                    bestGnb = gnbDevice;
                }
            }
        }
    }

    if (bestGnb && bestGnb != currentGnb)
    {
        bool shouldSwitch = true;
        if (currentGnbValid)
        {
            if (bestSnr < currentSnr + m_threshold)
            {
                shouldSwitch = false;
            }
        }

        if (shouldSwitch)
        {
            Ptr<MobilityModel> currentMobility = currentGnb->GetNode()->GetObject<MobilityModel>();
            Ptr<MobilityModel> bestMobility = bestGnb->GetNode()->GetObject<MobilityModel>();

            Time dynamicIslDelay = Seconds(0);
            /*
            if (Scenario::GetInstance())
            {
                // In this example, we are calculating additional delay based on the distance
                // between the current gNB, the best gNB, and the ground station. This is an example
                // model not reflecting real-world conditions. The aim is to show how ISL data
                // calculated by the simulation can be used to influence handover decisions. In a
                // real-world scenario, the delay would depend on the actual ISL network topology
                // and conditions.

                // First let's retrieve the netId for the current gNB to use in distance
                // calculations Every network config has a indipendent ISL configuration
                uint32_t netId = 0;
                auto netIdOpt = Scenario::GetInstance()->GetNetId(currentGnb);
                if (netIdOpt.has_value())
                {
                    netId = netIdOpt.value();
                }
                else
                {
                    NS_LOG_WARN("Could not find netId for currentGnb, defaulting to 0");
                }

                // Getting distances from ISL path
                double distCurrentToGround =
                    Scenario::GetInstance()
                        ->GetISLMinimumDistance(currentGnb->GetNode(), netId)
                        .first;
                double distBestToGround =
                    Scenario::GetInstance()->GetISLMinimumDistance(bestGnb->GetNode(), netId).first;
                double distCurrentToBest =
                    Scenario::GetInstance()->GetISLMinimumDistance(currentGnb->GetNode(),
                                                                   bestGnb->GetNode(),
                                                                   netId);

                // Total handover path distance: current->ground + ground->best + direct
                // current->best
                double handoverTotDistance =
                    distCurrentToGround * 2 + distBestToGround + distCurrentToBest;

                if (handoverTotDistance != std::numeric_limits<double>::infinity())
                {
                    dynamicIslDelay = Seconds(handoverTotDistance / 2.99792458e8);
                }
                // Print all useful details about the impending handover
                std::cout << "Triggering Handover - RNTI: " << rnti << std::endl;
                std::cout << "  Current gNB CellId: " << currentGnb->GetCellId()
                          << " (NodeId: " << currentGnb->GetNode()->GetId() << ")" << std::endl;
                std::cout << "  Target  gNB CellId: " << bestGnb->GetCellId()
                          << " (NodeId: " << bestGnb->GetNode()->GetId() << ")" << std::endl;
                std::cout << "  Handover path distance (m): " << handoverTotDistance << std::endl;
                std::cout << "    - current gNB to ground (m): " << distCurrentToGround
                          << std::endl;
                std::cout << "    - best gNB to ground    (m): " << distBestToGround << std::endl;
                std::cout << "    - direct current->best (m): " << distCurrentToBest << std::endl;
                std::cout << "  Dynamic ISL delay (s): " << dynamicIslDelay.GetSeconds()
                          << std::endl;
                std::cout << "  Static handover delay (s): " << m_handoverDelay.GetSeconds()
                          << std::endl;
                std::cout << "  Total scheduled delay (s): "
                          << (m_handoverDelay + dynamicIslDelay).GetSeconds() << std::endl;
            }
            */
            Time totalDelay = m_handoverDelay + dynamicIslDelay;
            // Schedulating the Trigger of the Handover after the total delay (static + dynamic)
            Simulator::Schedule(totalDelay,
                                &NrHandoverManagementSapUser::TriggerHandover,
                                m_handoverManagementSapUser,
                                rnti,
                                bestGnb->GetCellId());
        }
    }
}

} // namespace ns3
