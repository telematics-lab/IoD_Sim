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

#include "app-statistics-helper.h"

#include <ns3/flow-monitor.h>
#include <ns3/inet-socket-address.h>
#include <ns3/ipv4.h>
#include <ns3/log.h>
#include <ns3/names.h>
#include <ns3/node-list.h>
#include <ns3/packet-sink.h>
#include <ns3/udp-client.h>
#include <ns3/udp-server.h>
#include <ns3/uinteger.h>

#include <iomanip>
#include <sstream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("AppStatisticsHelper");
NS_OBJECT_ENSURE_REGISTERED(AppStatisticsHelper);

int32_t
AppStatisticsHelper::GetNodeIdFromIpAddress(Ipv4Address ipAddress)
{
    for (auto iter = NodeList::Begin(); iter != NodeList::End(); ++iter)
    {
        Ptr<Node> node = *iter;
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
        if (ipv4)
        {
            for (uint32_t i = 0; i < ipv4->GetNInterfaces(); ++i)
            {
                for (uint32_t j = 0; j < ipv4->GetNAddresses(i); ++j)
                {
                    if (ipv4->GetAddress(i, j).GetLocal() == ipAddress)
                    {
                        return node->GetId();
                    }
                }
            }
        }
    }
    return -1; // Not found
}

TypeId
AppStatisticsHelper::GetTypeId()
{
    static TypeId tid = TypeId("ns3::AppStatisticsHelper")
                            .SetParent<Object>()
                            .SetGroupName("Applications")
                            .AddConstructor<AppStatisticsHelper>();
    return tid;
}

AppStatisticsHelper::AppStatisticsHelper()
    : m_outputPath("app-statistics.txt"),
      m_reportInterval(Seconds(1.0)),
      m_started(false),
      m_lastReportTime(Seconds(0)),
      m_flowMonitor(nullptr)
{
    NS_LOG_FUNCTION(this);
}

AppStatisticsHelper::~AppStatisticsHelper()
{
    NS_LOG_FUNCTION(this);
    if (m_periodicFile.is_open())
    {
        m_periodicFile.close();
    }
}

void
AppStatisticsHelper::SetOutputPath(const std::string& path)
{
    NS_LOG_FUNCTION(this << path);
    m_outputPath = path;
}

void
AppStatisticsHelper::SetReportingInterval(Time interval)
{
    NS_LOG_FUNCTION(this << interval);
    m_reportInterval = interval;
}

void
AppStatisticsHelper::InstallFlowMonitor(NodeContainer nodes)
{
    NS_LOG_FUNCTION(this);
    m_flowMonitor = m_flowMonitorHelper.InstallAll();
    if (!m_flowMonitor)
    {
        NS_LOG_WARN("FlowMonitorHelper::InstallAll() returned null (likely no IP stack).");
        m_flowMonitor = CreateObject<FlowMonitor>();
    }
    NS_LOG_INFO("FlowMonitor installed on all nodes");
}

void
AppStatisticsHelper::Start()
{
    NS_LOG_FUNCTION(this);

    if (m_started)
    {
        return;
    }

    if (!m_flowMonitor)
    {
        NS_ABORT_MSG("FlowMonitor not installed. Call InstallFlowMonitor() before Start()");
    }

    std::string basePath = m_outputPath.substr(0, m_outputPath.find_last_of("."));

    // Open periodic statistics file
    std::string periodicFileName = basePath + "-periodic.txt";
    m_periodicFile.open(periodicFileName, std::ios::out);
    m_periodicFile.setf(std::ios_base::fixed);
    NS_ABORT_MSG_IF(!m_periodicFile.is_open(), "Cannot open file " << periodicFileName);
    m_periodicFile << "Time_s\tFlowID\tSrcNodeID\tSrcIP:Port\tDstNodeID\tDstIP:Port\tProto\t"
                   << "IntervalTxPkts\tIntervalRxPkts\tIntervalLostPkts\t"
                   << "TotalTxPkts\tTotalRxPkts\tTotalLostPkts\t"
                   << "IntervalTxBytes\tIntervalRxBytes\t"
                   << "Throughput_Mbps\tDelay_ms\tJitter_ms\tPLR_%\n";

    // Schedule periodic reports
    m_lastReportTime = Simulator::Now();
    m_reportEvent =
        Simulator::Schedule(m_reportInterval, &AppStatisticsHelper::GeneratePeriodicReport, this);

    m_started = true;
    NS_LOG_INFO("AppStatisticsHelper started with periodic reporting every "
                << m_reportInterval.GetSeconds() << " seconds");
}

void
AppStatisticsHelper::Stop()
{
    NS_LOG_FUNCTION(this);

    if (!m_started)
    {
        return;
    }

    // Cancel periodic reporting
    Simulator::Cancel(m_reportEvent);

    WriteFlowMonitorStats();

    // Close periodic file
    if (m_periodicFile.is_open())
    {
        m_periodicFile.close();
    }

    m_started = false;
    NS_LOG_INFO("AppStatisticsHelper stopped");
}

void
AppStatisticsHelper::GeneratePeriodicReport()
{
    NS_LOG_FUNCTION(this);

    if (!m_flowMonitor)
    {
        NS_LOG_WARN("FlowMonitor not available for periodic report");
        return;
    }

    Time now = Simulator::Now();
    double intervalDuration = (now - m_lastReportTime).GetSeconds();

    if (intervalDuration <= 0)
    {
        intervalDuration = m_reportInterval.GetSeconds();
    }

    m_flowMonitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(m_flowMonitorHelper.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = m_flowMonitor->GetFlowStats();

    for (auto i = stats.begin(); i != stats.end(); ++i)
    {
        uint32_t flowId = i->first;
        const FlowMonitor::FlowStats& currentStats = i->second;
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(flowId);

        // Get NodeIDs from IP addresses
        int32_t srcNodeId = GetNodeIdFromIpAddress(t.sourceAddress);
        int32_t dstNodeId = GetNodeIdFromIpAddress(t.destinationAddress);

        std::stringstream protoStream;
        protoStream << (uint16_t)t.protocol;
        if (t.protocol == 6)
        {
            protoStream.str("TCP");
        }
        else if (t.protocol == 17)
        {
            protoStream.str("UDP");
        }

        // Get previous stats or initialize if first time
        PreviousFlowStats prevStats;
        auto prevIt = m_previousStats.find(flowId);
        if (prevIt != m_previousStats.end())
        {
            prevStats = prevIt->second;
        }
        else
        {
            // First measurement for this flow
            prevStats.txPackets = 0;
            prevStats.rxPackets = 0;
            prevStats.txBytes = 0;
            prevStats.rxBytes = 0;
            prevStats.delaySum = Seconds(0);
            prevStats.jitterSum = Seconds(0);
        }

        // Calculate deltas (incremental values for this interval)
        // Note: deltaRxPackets can be > deltaTxPackets if receiving packets from previous intervals
        uint32_t deltaTxPackets = currentStats.txPackets - prevStats.txPackets;
        uint32_t deltaRxPackets = currentStats.rxPackets - prevStats.rxPackets;

        // Calculate lost packets safely (can be negative if more packets received than sent in this
        // interval)
        int64_t deltaLostPackets =
            static_cast<int64_t>(deltaTxPackets) - static_cast<int64_t>(deltaRxPackets);

        uint64_t deltaTxBytes = currentStats.txBytes - prevStats.txBytes;
        uint64_t deltaRxBytes = currentStats.rxBytes - prevStats.rxBytes;
        Time deltaDelaySum = currentStats.delaySum - prevStats.delaySum;
        Time deltaJitterSum = currentStats.jitterSum - prevStats.jitterSum;

        // Calculate metrics for this interval
        double throughput = 0.0;
        double delay = 0.0;
        double jitter = 0.0;
        double plr = 0.0;

        if (deltaRxPackets > 0)
        {
            throughput = deltaRxBytes * 8.0 / intervalDuration / 1000.0 / 1000.0;
            delay = 1000.0 * deltaDelaySum.GetSeconds() / deltaRxPackets;
            jitter = 1000.0 * deltaJitterSum.GetSeconds() / deltaRxPackets;
        }

        // Calculate PLR based on total statistics to avoid interval artifacts
        // Interval PLR can be misleading when packets arrive delayed
        if (currentStats.txPackets > 0)
        {
            int64_t totalLost = static_cast<int64_t>(currentStats.txPackets) -
                                static_cast<int64_t>(currentStats.rxPackets);
            if (totalLost > 0)
            {
                plr = 100.0 * totalLost / currentStats.txPackets;
            }
            else
            {
                plr = 0.0;
            }
        }

        // Calculate total lost packets (can be negative temporarily)
        int64_t totalLostPackets = static_cast<int64_t>(currentStats.txPackets) -
                                   static_cast<int64_t>(currentStats.rxPackets);

        // Write interval statistics
        m_periodicFile << std::fixed << std::setprecision(6) << now.GetSeconds() << "\t" << flowId
                       << "\t" << srcNodeId << "\t" << t.sourceAddress << ":" << t.sourcePort
                       << "\t" << dstNodeId << "\t" << t.destinationAddress << ":"
                       << t.destinationPort << "\t" << protoStream.str() << "\t" << deltaTxPackets
                       << "\t" << deltaRxPackets << "\t" << deltaLostPackets << "\t"
                       << currentStats.txPackets << "\t" << currentStats.rxPackets << "\t"
                       << totalLostPackets << "\t" << deltaTxBytes << "\t" << deltaRxBytes << "\t"
                       << std::setprecision(3) << throughput << "\t" << std::setprecision(3)
                       << delay << "\t" << std::setprecision(3) << jitter << "\t"
                       << std::setprecision(2) << plr << "\n";

        // Update previous stats for next interval
        m_previousStats[flowId].txPackets = currentStats.txPackets;
        m_previousStats[flowId].rxPackets = currentStats.rxPackets;
        m_previousStats[flowId].txBytes = currentStats.txBytes;
        m_previousStats[flowId].rxBytes = currentStats.rxBytes;
        m_previousStats[flowId].delaySum = currentStats.delaySum;
        m_previousStats[flowId].jitterSum = currentStats.jitterSum;
    }

    m_periodicFile.flush();
    m_lastReportTime = now;

    // Schedule next report
    m_reportEvent =
        Simulator::Schedule(m_reportInterval, &AppStatisticsHelper::GeneratePeriodicReport, this);
}

void
AppStatisticsHelper::WriteFlowMonitorStats()
{
    NS_LOG_FUNCTION(this);

    if (!m_flowMonitor)
    {
        NS_LOG_WARN("FlowMonitor not installed, skipping FlowMonitor statistics");
        return;
    }

    // Check for lost packets

    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(m_flowMonitorHelper.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = m_flowMonitor->GetFlowStats();

    std::string basePath = m_outputPath.substr(0, m_outputPath.find_last_of("."));
    std::string flowMonFileName = basePath + "-final.txt";
    std::ofstream flowMonFile;
    flowMonFile.open(flowMonFileName, std::ios::out | std::ios::trunc);
    flowMonFile.setf(std::ios_base::fixed);

    if (!flowMonFile.is_open())
    {
        NS_LOG_ERROR("Cannot open file " << flowMonFileName);
        return;
    }

    double averageFlowThroughput = 0.0;
    double averageFlowDelay = 0.0;
    double averageFlowJitter = 0.0;
    double averagePacketLoss = 0.0;
    uint32_t flowCount = 0;

    for (auto i = stats.begin(); i != stats.end(); ++i)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);

        // Get NodeIDs from IP addresses
        int32_t srcNodeId = GetNodeIdFromIpAddress(t.sourceAddress);
        int32_t dstNodeId = GetNodeIdFromIpAddress(t.destinationAddress);

        std::stringstream protoStream;
        protoStream << (uint16_t)t.protocol;
        if (t.protocol == 6)
        {
            protoStream.str("TCP");
        }
        if (t.protocol == 17)
        {
            protoStream.str("UDP");
        }

        flowMonFile << "Flow " << i->first << " (Node " << srcNodeId << " [" << t.sourceAddress
                    << ":" << t.sourcePort << "] -> Node " << dstNodeId << " ["
                    << t.destinationAddress << ":" << t.destinationPort << "]) proto "
                    << protoStream.str() << "\n";
        flowMonFile << "  Tx Packets: " << i->second.txPackets << "\n";
        flowMonFile << "  Tx Bytes:   " << i->second.txBytes << "\n";

        double flowDuration = (Simulator::Now() - Seconds(0)).GetSeconds();
        if (flowDuration > 0)
        {
            flowMonFile << "  TxOffered:  "
                        << i->second.txBytes * 8.0 / flowDuration / 1000.0 / 1000.0 << " Mbps\n";
        }

        flowMonFile << "  Rx Bytes:   " << i->second.rxBytes << "\n";
        flowMonFile << "  Rx Packets: " << i->second.rxPackets << "\n";

        // Calculate packet loss
        uint32_t lostPackets = i->second.txPackets - i->second.rxPackets;
        double packetLossRatio = 0.0;
        if (i->second.txPackets > 0)
        {
            packetLossRatio = 100.0 * lostPackets / i->second.txPackets;
        }
        flowMonFile << "  Lost Packets: " << lostPackets << "\n";
        flowMonFile << "  Packet Loss Ratio: " << std::setprecision(2) << packetLossRatio << " %\n";

        if (i->second.rxPackets > 0)
        {
            double throughput = i->second.rxBytes * 8.0 / flowDuration / 1000.0 / 1000.0;
            double delay = 1000.0 * i->second.delaySum.GetSeconds() / i->second.rxPackets;
            double jitter = 1000.0 * i->second.jitterSum.GetSeconds() / i->second.rxPackets;

            averageFlowThroughput += throughput;
            averageFlowDelay += delay;
            averageFlowJitter += jitter;
            averagePacketLoss += packetLossRatio;
            flowCount++;

            flowMonFile << "  Throughput: " << std::setprecision(3) << throughput << " Mbps\n";
            flowMonFile << "  Mean delay:  " << std::setprecision(3) << delay << " ms\n";
            flowMonFile << "  Mean jitter:  " << std::setprecision(3) << jitter << " ms\n";
        }
        else
        {
            flowMonFile << "  Throughput:  0 Mbps\n";
            flowMonFile << "  Mean delay:  0 ms\n";
            flowMonFile << "  Mean jitter: 0 ms\n";
        }
    }

    if (flowCount > 0)
    {
        flowMonFile << "\n\n  Mean flow throughput: " << averageFlowThroughput / flowCount
                    << " Mbps\n";
        flowMonFile << "  Mean flow delay: " << averageFlowDelay / flowCount << " ms\n";
        flowMonFile << "  Mean flow jitter: " << averageFlowJitter / flowCount << " ms\n";
        flowMonFile << "  Mean packet loss: " << std::setprecision(2)
                    << averagePacketLoss / flowCount << " %\n";
    }

    flowMonFile.close();
    NS_LOG_INFO("FlowMonitor statistics written to " << flowMonFileName);
}

} // namespace ns3
