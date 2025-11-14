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
#include <ns3/ipv4-address.h>
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
     * \brief Install callbacks on all applications in the simulation
     * This method automatically finds all client and server applications
     * in all nodes and installs trace callbacks to collect statistics.
     */
    void InstallAll();

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
     * \brief Structure to hold statistics for a single flow
     */
    struct FlowStats
    {
        uint32_t clientNodeId;    ///< Client node ID
        uint32_t serverNodeId;    ///< Server node ID
        uint32_t packetsSent;     ///< Total packets sent
        uint32_t packetsReceived; ///< Total packets received
        uint32_t bytesReceived;   ///< Total bytes received
        Time firstTxTime;         ///< Time of first packet transmission
        Time lastRxTime;          ///< Time of last packet reception
    };

    /**
     * \brief Callback for packet transmission
     * \param flowId Flow identifier
     * \param packet The transmitted packet
     */
    void TxCallback(std::string flowId, Ptr<const Packet> packet);

    /**
     * \brief Callback for packet reception
     * \param flowId Flow identifier
     * \param packet The received packet
     */
    void RxCallback(std::string flowId, Ptr<const Packet> packet);

    /**
     * \brief Generate periodic statistics report
     */
    void GenerateReport();

    /**
     * \brief Write header to output file
     */
    void WriteHeader();

    std::map<std::string, FlowStats> m_flowStats; ///< Statistics per flow
    std::ofstream m_outputFile;                   ///< Output file stream
    std::string m_outputPath;                     ///< Output file path
    Time m_reportInterval;                        ///< Reporting interval
    EventId m_reportEvent;                        ///< Event for periodic reporting
    bool m_started;                               ///< Whether collection has started
};

} // namespace ns3

#endif /* APP_STATISTICS_HELPER_H */
