// Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
// Copyright (c) 2013 Budiarto Herman
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Original work authors (from lte-enb-rrc.cc):
//   Nicola Baldo <nbaldo@cttc.es>
//   Marco Miozzo <mmiozzo@cttc.es>
//   Manuel Requena <manuel.requena@cttc.es>
//
// Converted to handover algorithm interface by:
//   Budiarto Herman <budiarto.herman@magister.fi>

#include "nr-sinr-distance-handover-algorithm.h"

#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device-container.h"
#include "ns3/node-list.h"
#include "ns3/nr-gnb-net-device.h"
#include "ns3/nr-gnb-phy.h"
#include "ns3/nr-gnb-rrc.h"
#include "ns3/nr-phy-layer-configuration.h"
#include "ns3/nr-radio-geo-environment-map-helper.h"
#include "ns3/nr-ue-net-device.h"
#include "ns3/nr-ue-rrc.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>

#define SINR_DISTANCE_PRINT_DEBUG

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrSinrDistanceHandoverAlgorithm");

NS_OBJECT_ENSURE_REGISTERED(NrSinrDistanceHandoverAlgorithm);

///////////////////////////////////////////
// Handover Management SAP forwarder
///////////////////////////////////////////

NrSinrDistanceHandoverAlgorithm::NrSinrDistanceHandoverAlgorithm()
    : m_handoverManagementSapUser(nullptr)
{
    NS_LOG_FUNCTION(this);
    m_handoverManagementSapProvider =
        new MemberNrHandoverManagementSapProvider<NrSinrDistanceHandoverAlgorithm>(this);
}

NrSinrDistanceHandoverAlgorithm::~NrSinrDistanceHandoverAlgorithm()
{
    NS_LOG_FUNCTION(this);
}

TypeId
NrSinrDistanceHandoverAlgorithm::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrSinrDistanceHandoverAlgorithm")
            .SetParent<NrHandoverAlgorithm>()
            .SetGroupName("Nr")
            .AddConstructor<NrSinrDistanceHandoverAlgorithm>()
            .AddAttribute("EvaluationPrecision",
                          "The precision interval at which to evaluate handovers.",
                          TimeValue(MilliSeconds(10)),
                          MakeTimeAccessor(&NrSinrDistanceHandoverAlgorithm::m_evaluationPrecision),
                          MakeTimeChecker())
            .AddAttribute(
                "SinrDistanceTable",
                "Comma separated list of distance:sinr thresholds, e.g. '100:2.0,500:0.5'",
                StringValue(""),
                MakeStringAccessor(&NrSinrDistanceHandoverAlgorithm::SetSinrDistanceTableString,
                                   &NrSinrDistanceHandoverAlgorithm::GetSinrDistanceTableString),
                MakeStringChecker())
            .AddAttribute("HandoverMargin",
                          "The hysteresis margin (in dB) required to trigger a handover.",
                          DoubleValue(0.0),
                          MakeDoubleAccessor(&NrSinrDistanceHandoverAlgorithm::m_handoverMargin),
                          MakeDoubleChecker<double>());
    return tid;
}

void
NrSinrDistanceHandoverAlgorithm::SetEvaluationPrecision(Time precision)
{
    NS_LOG_FUNCTION(this << precision);
    m_evaluationPrecision = precision;
}

void
NrSinrDistanceHandoverAlgorithm::SetSinrDistanceTable(
    const std::vector<SinrDistanceTableEntry>& table)
{
    NS_LOG_FUNCTION(this);
    m_sinrDistanceTable = table;
}

void
NrSinrDistanceHandoverAlgorithm::SetSinrDistanceTableString(std::string tableStr)
{
    NS_LOG_FUNCTION(this << tableStr);
    m_sinrDistanceTable.clear();

    std::stringstream ss(tableStr);
    std::string entryStr;

    while (std::getline(ss, entryStr, ','))
    {
        if (entryStr.empty())
        {
            continue;
        }
        size_t colonPos = entryStr.find(':');
        if (colonPos != std::string::npos)
        {
            double maxDist = std::stod(entryStr.substr(0, colonPos));
            double minSinr = std::stod(entryStr.substr(colonPos + 1));
            m_sinrDistanceTable.push_back({maxDist, minSinr});
        }
    }
}

std::string
NrSinrDistanceHandoverAlgorithm::GetSinrDistanceTableString() const
{
    std::stringstream ss;
    for (size_t i = 0; i < m_sinrDistanceTable.size(); ++i)
    {
        ss << m_sinrDistanceTable[i].maxDistance << ":" << m_sinrDistanceTable[i].minSinr;
        if (i < m_sinrDistanceTable.size() - 1)
        {
            ss << ",";
        }
    }
    return ss.str();
}

void
NrSinrDistanceHandoverAlgorithm::SetHandoverMargin(double threshold)
{
    NS_LOG_FUNCTION(this << threshold);
    m_handoverMargin = threshold;
}

void
NrSinrDistanceHandoverAlgorithm::SetNrHandoverManagementSapUser(NrHandoverManagementSapUser* s)
{
    NS_LOG_FUNCTION(this << s);
    m_handoverManagementSapUser = s;
}

NrHandoverManagementSapProvider*
NrSinrDistanceHandoverAlgorithm::GetNrHandoverManagementSapProvider()
{
    NS_LOG_FUNCTION(this);
    return m_handoverManagementSapProvider;
}

void
NrSinrDistanceHandoverAlgorithm::DoInitialize()
{
    NS_LOG_FUNCTION(this);
    NrHandoverAlgorithm::DoInitialize();

    if (m_evaluationPrecision.IsStrictlyPositive())
    {
        m_evaluationEvent =
            Simulator::Schedule(m_evaluationPrecision,
                                &NrSinrDistanceHandoverAlgorithm::EvaluateSinrDistanceAttachment,
                                this);
    }
}

void
NrSinrDistanceHandoverAlgorithm::DoDispose()
{
    NS_LOG_FUNCTION(this);
    Simulator::Cancel(m_evaluationEvent);
    delete m_handoverManagementSapProvider;
}

void
NrSinrDistanceHandoverAlgorithm::DoReportUeMeas(uint16_t rnti, NrRrcSap::MeasResults measResults)
{
    // A2/A4 packets are skipped in this algorithm
    NS_LOG_FUNCTION(this << rnti << (uint16_t)measResults.measId);
}

void
NrSinrDistanceHandoverAlgorithm::EvaluateSinrDistanceAttachment()
{
    NS_LOG_FUNCTION(this);

    // This algorithm implements a centralized handover decision logic, similar to a simulation
    // script. It iterates through all UEs and gNBs to determine the best attachment point based on
    // SINR and distance.

    uint16_t servingGnbCellId = 0;
    Ptr<NrGnbNetDevice> servingGnbDevice = nullptr;
    Ptr<Node> servingGnbNode = nullptr;

    NetDeviceContainer allGnbDevices;

    for (uint32_t i = 0; i < NodeList::GetNNodes(); ++i)
    {
        Ptr<Node> node = NodeList::GetNode(i);
        Ptr<NrGnbNetDevice> dev = node->GetObject<NrGnbNetDevice>();
        if (dev != nullptr)
        {
            allGnbDevices.Add(dev);

            // Look for the specific gNB matching our SAP User
            if (servingGnbDevice == nullptr && dev->GetRrc() != nullptr &&
                dev->GetRrc()->GetNrHandoverManagementSapUser() == m_handoverManagementSapUser)
            {
                servingGnbDevice = dev;
                servingGnbNode = node;
                servingGnbCellId = dev->GetCellId();
            }
        }
    }

    if (servingGnbDevice == nullptr || servingGnbNode == nullptr)
    {
        NS_LOG_WARN(
            "Could not find the serving gNB associated with this handover algorithm instance");
        return;
    }

    // Prepare REM Helper for precise SINR calculations
    Ptr<NrRadioGeoEnvironmentMapHelper> remHelper = CreateObject<NrRadioGeoEnvironmentMapHelper>();
    remHelper->SetInterferers(allGnbDevices, 0); // Configure all gNBs as interferers

    // Iterate through all UEs in the simulation
    for (uint32_t i = 0; i < NodeList::GetNNodes(); ++i)
    {
        Ptr<Node> ueNode = NodeList::GetNode(i);
        Ptr<NrUeNetDevice> ueDevice = ueNode->GetObject<NrUeNetDevice>();

        if (ueDevice == nullptr)
        {
            continue; // Not a UE node
        }

        // Check if this UE is currently attached to *this* gNB (the one this algorithm instance
        // serves)
        if (ueDevice->GetCellId() != servingGnbCellId)
        {
            continue; // This UE is not served by this gNB, so this gNB cannot initiate handover for
                      // it.
        }

        // Get UE's RNTI
        Ptr<NrUeRrc> ueRrc = ueDevice->GetRrc();
        if (ueRrc == nullptr)
        {
            continue;
        }
        uint16_t rnti = ueRrc->GetRnti();

        // Get UE's mobility model
        Ptr<MobilityModel> ueMobility = ueNode->GetObject<MobilityModel>();
        if (ueMobility == nullptr)
        {
            NS_LOG_WARN("UE " << ueNode->GetId() << " has no mobility model. Skipping.");
            continue;
        }

        double bestSinr = -std::numeric_limits<double>::infinity();
        uint16_t bestNeighbourCellId = 0;
        Ptr<NrGnbNetDevice> bestNeighbourGnbDevice = nullptr;

        // Iterate through all gNBs to find the best candidate for handover
        for (uint32_t j = 0; j < allGnbDevices.GetN(); ++j)
        {
            Ptr<NrGnbNetDevice> gnbDevice = DynamicCast<NrGnbNetDevice>(allGnbDevices.Get(j));
            if (gnbDevice == nullptr)
            {
                continue;
            }

            // Skip the serving gNB itself
            if (gnbDevice->GetCellId() == servingGnbCellId)
            {
                continue;
            }

            Ptr<Node> gnbNode = gnbDevice->GetNode();
            Ptr<MobilityModel> gnbMobility = gnbNode->GetObject<MobilityModel>();
            if (gnbMobility == nullptr)
            {
                NS_LOG_WARN("gNB " << gnbNode->GetId() << " has no mobility model. Skipping.");
                continue;
            }

            // Calculate distance between UE and gNB
            double distance = ueMobility->GetDistanceFrom(gnbMobility);

#ifdef SINR_DISTANCE_PRINT_DEBUG
            std::cout << "UE " << ueNode->GetId() << " distance to gNB " << gnbNode->GetId() << ": "
                      << distance / 1000 << " km" << std::endl;
#endif

            // Check if this gNB's (distance, SNR) pairs correspond to a better target.
            double minSinrRequired = std::numeric_limits<double>::max();
            const SinrDistanceTableEntry* bestEntry = nullptr;
            double rangeDiff = std::numeric_limits<double>::max();

            if (!m_sinrDistanceTable.empty())
            {
                for (const auto& entry : m_sinrDistanceTable)
                {
                    if (distance <= entry.maxDistance)
                    {
                        if (entry.maxDistance < rangeDiff)
                        {
                            rangeDiff = entry.maxDistance;
                            bestEntry = &entry;
                        }
                    }
                }
                if (bestEntry != nullptr)
                {
                    minSinrRequired = bestEntry->minSinr;
                }
            }

            // Evaluate SNR via real world ns-3 REM helper instead of simple pathloss formula
            double sinrDb = remHelper->GetSinr(ueDevice, gnbDevice, 0, true);

#ifdef SINR_DISTANCE_PRINT_DEBUG
            std::cout << "UE " << ueNode->GetId() << " evaluated SNR for gNB " << gnbNode->GetId()
                      << ": " << sinrDb << " dB (Required: "
                      << (bestEntry ? minSinrRequired : std::numeric_limits<double>::max())
                      << " dB)" << std::endl;
#endif

            NS_LOG_DEBUG("UE " << ueNode->GetId() << " to gNB " << gnbNode->GetId()
                               << " (CellId: " << gnbDevice->GetCellId()
                               << "): Distance=" << distance << "m, SINR=" << sinrDb << "dB");

            // We only consider handovers to gNBs meeting the minimum configured Sinr.
            if ((bestEntry == nullptr && m_sinrDistanceTable.empty() && sinrDb > bestSinr) ||
                (bestEntry != nullptr && sinrDb >= minSinrRequired && sinrDb > bestSinr))
            {
                bestSinr = sinrDb;
                bestNeighbourCellId = gnbDevice->GetCellId();
                bestNeighbourGnbDevice = gnbDevice;
            }
        }

        // After checking all gNBs, decide if a handover is needed
        // Get current serving gNB's SINR for the UE
        Ptr<MobilityModel> servingGnbMobility = servingGnbNode->GetObject<MobilityModel>();
        double currentDistance = ueMobility->GetDistanceFrom(servingGnbMobility);

        // Ensure accurate current SNR Evaluation without formulaic guessing
        double currentSinrDb = remHelper->GetSinr(ueDevice, servingGnbDevice, 0, true);

        // Does the current target meet the distance/sinr threshold?
        bool currentGnbValid = true;

        if (!m_sinrDistanceTable.empty())
        {
            currentGnbValid = false;
            double rangeDiff = std::numeric_limits<double>::max();
            const SinrDistanceTableEntry* bestEntry = nullptr;

            for (const auto& entry : m_sinrDistanceTable)
            {
                if (currentDistance <= entry.maxDistance)
                {
                    if (entry.maxDistance < rangeDiff)
                    {
                        rangeDiff = entry.maxDistance;
                        bestEntry = &entry;
                    }
                }
            }

            if (bestEntry != nullptr && currentSinrDb >= bestEntry->minSinr)
            {
                currentGnbValid = true;
            }
        }

        NS_LOG_DEBUG("UE " << ueNode->GetId() << " current serving gNB " << servingGnbCellId
                           << ": Distance=" << currentDistance << "m, SINR=" << currentSinrDb
                           << "dB");

        // Handover decision logic:
        double handoverMarginDb = m_handoverMargin; // Margin
        bool shouldHandover = true;

        if (currentGnbValid)
        {
            if (bestSinr < (currentSinrDb + handoverMarginDb))
            {
                shouldHandover = false;

#ifdef SINR_DISTANCE_PRINT_DEBUG
                if (bestSinr > currentSinrDb)
                {
                    std::cout << "UE " << ueDevice->GetImsi()
                              << " HANDOVER PREVENTED by threshold (" << handoverMarginDb << " dB)"
                              << " from gNB " << servingGnbNode->GetId()
                              << " (SNR: " << currentSinrDb << " dB)"
                              << " to gNB " << bestNeighbourGnbDevice->GetNode()->GetId()
                              << " (SNR: " << bestSinr << " dB)"
                              << " Delta: " << bestSinr - currentSinrDb << " dB" << std::endl;
                }
#endif
            }
        }

        if (bestNeighbourGnbDevice != nullptr && bestNeighbourCellId != servingGnbCellId &&
            shouldHandover)
        {
#ifdef SINR_DISTANCE_PRINT_DEBUG
            std::cout << "UE " << ueDevice->GetImsi() << " HANDOVER from gNB "
                      << servingGnbNode->GetId() << " (SNR: " << currentSinrDb << " dB)"
                      << " to gNB " << bestNeighbourGnbDevice->GetNode()->GetId()
                      << " (SNR: " << bestSinr << " dB)"
                      << " Threshold: " << handoverMarginDb << " dB" << std::endl;
#endif

            NS_LOG_INFO("Handover triggered for UE "
                        << rnti << " from gNB " << servingGnbCellId << " to gNB "
                        << bestNeighbourCellId << " (Current SINR: " << currentSinrDb
                        << "dB, Best Neighbour SINR: " << bestSinr << "dB)");
            m_handoverManagementSapUser->TriggerHandover(rnti, bestNeighbourCellId);
        }
    }

    // Reschedule for the next evaluation
    if (m_evaluationPrecision.IsStrictlyPositive())
    {
        m_evaluationEvent =
            Simulator::Schedule(m_evaluationPrecision,
                                &NrSinrDistanceHandoverAlgorithm::EvaluateSinrDistanceAttachment,
                                this);
    }
}

bool
NrSinrDistanceHandoverAlgorithm::IsValidNeighbour(uint16_t cellId)
{
    NS_LOG_FUNCTION(this << cellId);
    return true;
}

// Useless in this case
void
NrSinrDistanceHandoverAlgorithm::UpdateNeighbourMeasurements(uint16_t rnti,
                                                             uint16_t cellId,
                                                             uint8_t rsrq)
{
    NS_LOG_FUNCTION(this << rnti << cellId << (uint16_t)rsrq);
    auto it1 = m_neighbourCellMeasures.find(rnti);

    if (it1 == m_neighbourCellMeasures.end())
    {
        MeasurementRow_t row;
        auto ret = m_neighbourCellMeasures.insert(std::pair<uint16_t, MeasurementRow_t>(rnti, row));
        NS_ASSERT(ret.second);
        it1 = ret.first;
    }

    NS_ASSERT(it1 != m_neighbourCellMeasures.end());
    Ptr<UeMeasure> neighbourCellMeasures;
    auto it2 = it1->second.find(cellId);

    if (it2 != it1->second.end())
    {
        neighbourCellMeasures = it2->second;
        neighbourCellMeasures->m_cellId = cellId;
        neighbourCellMeasures->m_rsrp = 0;
        neighbourCellMeasures->m_rsrq = rsrq;
    }
    else
    {
        neighbourCellMeasures = Create<UeMeasure>();
        neighbourCellMeasures->m_cellId = cellId;
        neighbourCellMeasures->m_rsrp = 0;
        neighbourCellMeasures->m_rsrq = rsrq;
        it1->second[cellId] = neighbourCellMeasures;
    }
}

} // end of namespace ns3
