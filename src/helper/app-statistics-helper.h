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

#ifndef APP_STATISTICS_HELPER_H
#define APP_STATISTICS_HELPER_H

#include <ns3/application-container.h>
#include <ns3/application.h>
#include <ns3/flow-monitor-helper.h>
#include <ns3/ipv4-address.h>
#include <ns3/ipv4-flow-classifier.h>
#include <ns3/node-container.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/simulator.h>

#include <fstream>
#include <map>
#include <string>

namespace ns3
{

/**
 * \brief Helper class to collect and log application-level statistics
 *
 * This helper tracks packets sent and received by applications, calculates packet delivery ratio,
 * packet loss, and data rate for each connection.
 */
class AppStatisticsHelper : public Object
{
  public:
    /**
     * \brief Get the type ID
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * \brief Constructor
     */
    AppStatisticsHelper();

    /**
     * \brief Destructor
     */
    virtual ~AppStatisticsHelper();

    /**
     * \brief Set the output file path for statistics
     * \param path The file path
     */
    void SetOutputPath(const std::string& path);

    /**
     * \brief Set the statistics reporting interval
     * \param interval The interval in seconds
     */
    void SetReportingInterval(Time interval);

    /**
     * \brief Install FlowMonitor on nodes
     * \param nodes The nodes to monitor
     */
    void InstallFlowMonitor(NodeContainer nodes);

    /**
     * \brief Start statistics collection
     */
    void Start();

    /**
     * \brief Stop statistics collection and write final report
     */
    void Stop();

  private:
    /**
     * \brief Structure to hold previous flow statistics for delta calculations
     */
    struct PreviousFlowStats
    {
        uint32_t txPackets;
        uint32_t rxPackets;
        uint64_t txBytes;
        uint64_t rxBytes;
        Time delaySum;
        Time jitterSum;
    };

    /**
     * \brief Generate periodic FlowMonitor report
     */
    void GeneratePeriodicReport();

    /**
     * \brief Write FlowMonitor statistics
     */
    void WriteFlowMonitorStats();

    /**
     * \brief Get NodeID from IP address
     * \param ipAddress The IP address
     * \return The node ID, or -1 if not found
     */
    int32_t GetNodeIdFromIpAddress(Ipv4Address ipAddress);

    std::string m_outputPath; ///< Output file path
    Time m_reportInterval;    ///< Reporting interval
    EventId m_reportEvent;    ///< Event for periodic reporting
    bool m_started;           ///< Whether collection has started
    Time m_lastReportTime;    ///< Time of last periodic report

    // FlowMonitor support
    FlowMonitorHelper m_flowMonitorHelper; ///< FlowMonitor helper
    Ptr<FlowMonitor> m_flowMonitor;        ///< FlowMonitor instance
    std::ofstream m_periodicFile;          ///< Periodic statistics file
    std::map<uint32_t, PreviousFlowStats>
        m_previousStats; ///< Previous flow statistics for delta calculations
};

} // namespace ns3

#endif /* APP_STATISTICS_HELPER_H */
