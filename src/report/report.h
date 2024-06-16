/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2024 The IoD_Sim Authors.
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
#ifndef REPORT_H
#define REPORT_H

#include "report-simulation.h"

#include <ns3/singleton.h>

#include <libxml/xmlwriter.h>
#include <string>

namespace ns3
{

/**
 * The Report module is made to agglomerate useful data and output all of it in
 * a XML file. Report should be initialized with all the needed information
 * prior to Simulation start.
 * The XML file is also referred to as the summary file.
 *
 * Data is gathered during simulation and written to file at the end, during
 * object destruction.
 *
 * Currently, Report supports only IoD_Sim scenarios.
 */
class Report : public Singleton<Report>
{
  public:
    /**
     * Initialize Reporting Infrastructure.
     * This method should be called only once. It has guards for it and it prints
     * a warning message if it is called multiple times.
     *
     * \param scenarioName Name of the scenario
     * \param executedAt   Datetime of scenario execution
     * \param resultsPath  Base path on which the XML file is saved
     */
    void Initialize(const std::string scenarioName,
                    const std::string executedAt,
                    const std::string resultsPath);

    /**
     * Save all data to the summary file.
     */
    void Save();

  private:
    /**
     * Open the summary file.
     */
    void Open();

    /**
     * Close the summary file.
     */
    void Close();

    /**
     * Explore the tree and write on the summary file.
     */
    void Write() const;

    /**
     * Generate the destination file name summary.xml
     *
     * \return Destination file name.
     */
    const std::string GetFilename() const;

    std::string m_resultsPath;            /// Results directory path
    xmlTextWriterPtr m_writer;            /// XML file handler
    Ptr<ReportSimulation> m_dataTreeRoot; /// Root of accumulated data
};

} // namespace ns3

#endif /* REPORT_H */
