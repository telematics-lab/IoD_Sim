/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2025 The IoD_Sim Authors.
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
      m_started(false)
{
    NS_LOG_FUNCTION(this);
}

AppStatisticsHelper::~AppStatisticsHelper()
{
    NS_LOG_FUNCTION(this);
    if (m_outputFile.is_open())
    {
        m_outputFile.close();
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
AppStatisticsHelper::InstallAll()
{
    NS_LOG_FUNCTION(this);

    // Structure to store server info with IP address and port
    struct ServerInfo
    {
        uint32_t nodeId;
        Ptr<Application> app;
        std::string ipAddress;
        uint16_t port;
    };

    std::vector<ServerInfo> servers;

    // Maps to store client info
    struct ClientInfo
    {
        uint32_t nodeId;
        Ptr<Application> app;
        std::string remoteIp;
        uint16_t remotePort;
    };

    std::vector<ClientInfo> clients;

    // First pass: identify all servers with their IP addresses and ports
    for (auto iter = NodeList::Begin(); iter != NodeList::End(); ++iter)
    {
        Ptr<Node> node = *iter;
        uint32_t nodeId = node->GetId();

        for (uint32_t i = 0; i < node->GetNApplications(); ++i)
        {
            Ptr<Application> app = node->GetApplication(i);

            // Try to get the Port attribute
            UintegerValue portValue;
            if (app->GetAttributeFailSafe("Port", portValue))
            {
                uint16_t port = portValue.Get();

                // Get the IP address of this node
                Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
                std::string ipAddress = "unknown";

                if (ipv4 && ipv4->GetNInterfaces() > 1)
                {
                    // Skip loopback (interface 0), get first real interface
                    Ipv4Address addr = ipv4->GetAddress(1, 0).GetLocal();
                    std::ostringstream oss;
                    oss << addr;
                    ipAddress = oss.str();
                }

                ServerInfo serverInfo;
                serverInfo.nodeId = nodeId;
                serverInfo.app = app;
                serverInfo.ipAddress = ipAddress;
                serverInfo.port = port;
                servers.push_back(serverInfo);

                NS_LOG_INFO("Found UdpServer on node " << nodeId << " at " << ipAddress << ":"
                                                       << port);
            }
        }
    }

    // Second pass: identify all clients and match them with servers
    for (auto iter = NodeList::Begin(); iter != NodeList::End(); ++iter)
    {
        Ptr<Node> node = *iter;
        uint32_t nodeId = node->GetId();

        for (uint32_t i = 0; i < node->GetNApplications(); ++i)
        {
            Ptr<Application> app = node->GetApplication(i);
            std::string typeName = app->GetInstanceTypeId().GetName();

            // Try to get the Remote attribute (contains both address and port)
            AddressValue remoteValue;

            if (app->GetAttributeFailSafe("Remote", remoteValue))
            {
                Address remoteAddr = remoteValue.Get();
                uint16_t remotePort = 0;

                // Convert address to string and extract port
                std::ostringstream oss;
                if (InetSocketAddress::IsMatchingType(remoteAddr))
                {
                    InetSocketAddress inetAddr = InetSocketAddress::ConvertFrom(remoteAddr);
                    oss << inetAddr.GetIpv4();
                    remotePort = inetAddr.GetPort();
                }

                ClientInfo clientInfo;
                clientInfo.nodeId = nodeId;
                clientInfo.app = app;
                clientInfo.remoteIp = oss.str();
                clientInfo.remotePort = remotePort;
                clients.push_back(clientInfo);

                NS_LOG_INFO("Found UdpClient on node " << nodeId << " targeting "
                                                       << clientInfo.remoteIp << ":" << remotePort);
            }
        }
    }

    // Third pass: create flows by matching clients with servers
    for (const auto& clientInfo : clients)
    {
        uint32_t clientNodeId = clientInfo.nodeId;
        std::string targetIp = clientInfo.remoteIp;
        uint16_t targetPort = clientInfo.remotePort;

        // Find the server with matching IP and port (both must match)
        ServerInfo* matchedServer = nullptr;
        for (auto& serverInfo : servers)
        {
            if (serverInfo.ipAddress == targetIp && serverInfo.port == targetPort)
            {
                matchedServer = &serverInfo;
                NS_LOG_INFO("Matched client on node " << clientNodeId << " -> server on node "
                                                      << serverInfo.nodeId << " (" << targetIp
                                                      << ":" << targetPort << ")");
                break;
            }
        }

        if (matchedServer == nullptr)
        {
            NS_LOG_WARN("Client on node " << clientNodeId << " targets " << targetIp << ":"
                                          << targetPort << " but no matching server found");
            continue;
        }

        uint32_t serverNodeId = matchedServer->nodeId;
        Ptr<Application> serverApp = matchedServer->app;

        // Create flow identifier
        std::stringstream flowId;
        flowId << "node" << clientNodeId << "->node" << serverNodeId << ":" << targetPort;
        std::string flowIdStr = flowId.str();

        // Initialize flow statistics
        FlowStats stats;
        stats.clientNodeId = clientNodeId;
        stats.serverNodeId = serverNodeId;
        stats.packetsSent = 0;
        stats.packetsReceived = 0;
        stats.bytesReceived = 0;
        stats.firstTxTime = Seconds(-1); // Not set yet
        stats.lastRxTime = Seconds(0);
        m_flowStats[flowIdStr] = stats;

        // Connect client Tx trace
        bool txConnected = clientInfo.app->TraceConnectWithoutContext(
            "Tx",
            MakeCallback(&AppStatisticsHelper::TxCallback, this).Bind(flowIdStr));

        if (txConnected)
        {
            NS_LOG_INFO("Connected Tx trace: " << flowIdStr);
        }
        else
        {
            NS_LOG_WARN("Failed to connect Tx trace for " << flowIdStr);
        }

        // Connect server Rx trace
        bool rxConnected = serverApp->TraceConnectWithoutContext(
            "Rx",
            MakeCallback(&AppStatisticsHelper::RxCallback, this).Bind(flowIdStr));

        if (rxConnected)
        {
            NS_LOG_INFO("Connected Rx trace: " << flowIdStr);
        }
        else
        {
            NS_LOG_WARN("Failed to connect Rx trace for " << flowIdStr);
        }
    }

    NS_LOG_INFO("Total flows configured: " << m_flowStats.size());
}

void
AppStatisticsHelper::Start()
{
    NS_LOG_FUNCTION(this);

    if (m_started)
    {
        return;
    }

    // Open output file
    m_outputFile.open(m_outputPath, std::ios::out);
    NS_ABORT_MSG_IF(!m_outputFile.is_open(), "Cannot open file " << m_outputPath);

    WriteHeader();

    // Schedule periodic reports
    m_reportEvent =
        Simulator::Schedule(m_reportInterval, &AppStatisticsHelper::GenerateReport, this);

    m_started = true;
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

    // Generate final report
    GenerateReport();

    // Write summary
    m_outputFile << "\n=== FINAL SUMMARY ===\n";
    for (auto& entry : m_flowStats)
    {
        const std::string& flowId = entry.first;
        const FlowStats& stats = entry.second;

        double pdr = 0.0;
        if (stats.packetsSent > 0)
        {
            pdr = (static_cast<double>(stats.packetsReceived) / stats.packetsSent) * 100.0;
        }

        uint32_t packetsLost = stats.packetsSent - stats.packetsReceived;

        // Calculate average data rate based on actual transmission time
        double avgDataRate = 0.0;
        if (!stats.firstTxTime.IsNegative() && stats.lastRxTime > stats.firstTxTime)
        {
            double transmissionTime = (stats.lastRxTime - stats.firstTxTime).GetSeconds();
            if (transmissionTime > 0)
            {
                avgDataRate = (stats.bytesReceived * 8.0) / (transmissionTime * 1e6); // Mbps
            }
        }

        m_outputFile << "Flow " << flowId << " (Node " << stats.clientNodeId << " -> Node "
                     << stats.serverNodeId << "):\n";
        m_outputFile << "  Packets Sent: " << stats.packetsSent << "\n";
        m_outputFile << "  Packets Received: " << stats.packetsReceived << "\n";
        m_outputFile << "  Packets Lost: " << packetsLost << "\n";
        m_outputFile << "  PDR: " << std::fixed << std::setprecision(2) << pdr << "%\n";
        m_outputFile << "  Total Bytes Received: " << stats.bytesReceived << "\n";

        if (!stats.firstTxTime.IsNegative())
        {
            m_outputFile << "  First Tx Time: " << std::setprecision(6)
                         << stats.firstTxTime.GetSeconds() << " s\n";
        }

        if (stats.lastRxTime > Seconds(0))
        {
            m_outputFile << "  Last Rx Time: " << std::setprecision(6)
                         << stats.lastRxTime.GetSeconds() << " s\n";
        }

        if (avgDataRate > 0)
        {
            m_outputFile << "  Average Data Rate: " << std::setprecision(3) << avgDataRate
                         << " Mbps\n";
        }

        m_outputFile << "\n";
    }

    m_outputFile.close();
    m_started = false;
}

void
AppStatisticsHelper::TxCallback(std::string flowId, Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << flowId << packet->GetSize());

    if (m_flowStats.find(flowId) != m_flowStats.end())
    {
        m_flowStats[flowId].packetsSent++;

        // Record time of first transmission
        if (m_flowStats[flowId].firstTxTime.IsNegative())
        {
            m_flowStats[flowId].firstTxTime = Simulator::Now();
            NS_LOG_DEBUG("First packet sent at " << m_flowStats[flowId].firstTxTime.GetSeconds()
                                                 << "s for flow " << flowId);
        }
    }
}

void
AppStatisticsHelper::RxCallback(std::string flowId, Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << flowId << packet->GetSize());

    if (m_flowStats.find(flowId) != m_flowStats.end())
    {
        m_flowStats[flowId].packetsReceived++;
        m_flowStats[flowId].bytesReceived += packet->GetSize();

        // Always update the last reception time
        m_flowStats[flowId].lastRxTime = Simulator::Now();
    }
}

void
AppStatisticsHelper::GenerateReport()
{
    NS_LOG_FUNCTION(this);

    Time now = Simulator::Now();

    for (auto& entry : m_flowStats)
    {
        const std::string& flowId = entry.first;
        FlowStats& stats = entry.second;

        // Calculate total PDR
        double totalPdr = 0.0;
        if (stats.packetsSent > 0)
        {
            totalPdr = (static_cast<double>(stats.packetsReceived) / stats.packetsSent) * 100.0;
        }

        // Calculate total packets lost
        uint32_t packetsLost = stats.packetsSent - stats.packetsReceived;

        // Calculate average data rate (in Mbps) from the beginning
        double dataRate = 0.0;
        if (now.GetSeconds() > 0)
        {
            dataRate = (stats.bytesReceived * 8.0) / (now.GetSeconds() * 1e6);
        }

        // Write to file
        m_outputFile << std::fixed << std::setprecision(6) << now.GetSeconds() << "\t" << flowId
                     << "\t" << stats.clientNodeId << "\t" << stats.serverNodeId << "\t"
                     << stats.packetsSent << "\t" << stats.packetsReceived << "\t" << packetsLost
                     << "\t" << std::setprecision(2) << totalPdr << "\t" << std::setprecision(3)
                     << dataRate << "\n";

        m_outputFile.flush();
    }

    // Schedule next report
    m_reportEvent =
        Simulator::Schedule(m_reportInterval, &AppStatisticsHelper::GenerateReport, this);
}

void
AppStatisticsHelper::WriteHeader()
{
    NS_LOG_FUNCTION(this);

    m_outputFile << "# Application-Level Statistics\n";
    m_outputFile << "# Time: Simulation time (s)\n";
    m_outputFile << "# FlowID: Flow identifier (ClientNode->ServerNode:Port)\n";
    m_outputFile << "# ClientNodeID: ID of the client node\n";
    m_outputFile << "# ServerNodeID: ID of the server node\n";
    m_outputFile << "# TotalPktsSent: Total packets sent from the beginning\n";
    m_outputFile << "# TotalPktsRcvd: Total packets received from the beginning\n";
    m_outputFile << "# TotalPktsLost: Total packets lost from the beginning\n";
    m_outputFile << "# PDR: Packet Delivery Ratio (%)\n";
    m_outputFile << "# AvgDataRate: Average data rate from the beginning (Mbps)\n";
    m_outputFile << "#\n";
    m_outputFile << "Time\tFlowID\tClientNodeID\tServerNodeID\tTotalPktsSent\tTotalPktsRcvd\t"
                    "TotalPktsLost\tPDR(%)\tAvgDataRate(Mbps)\n";
}

} // namespace ns3
