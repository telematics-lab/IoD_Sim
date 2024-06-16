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
#include "report.h"

#include "report-simulation.h"

#include <ns3/log.h>
#include <ns3/object-factory.h>
#include <ns3/string.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Report");

void
Report::Initialize(const std::string scenarioName,
                   const std::string executedAt,
                   const std::string resultsPath)
{
    NS_LOG_FUNCTION(this);
    if (m_dataTreeRoot)
    {
        NS_LOG_WARN("Report is already being populated at "
                    << m_dataTreeRoot << ". There is no need to reinitialize it.");
        return;
    }

    m_resultsPath = resultsPath;

    m_dataTreeRoot = CreateObjectWithAttributes<ReportSimulation>("Scenario",
                                                                  StringValue(scenarioName),
                                                                  "ExecutedAt",
                                                                  StringValue(executedAt));

    m_dataTreeRoot->Initialize();

    /*
     * File early opening is to check that a new file can be created and
     * written to before starting a simulation that can last minutes, hours,
     * days, whatever.
     * Lazy opening is discouraged here to not lose data and time.
     */
    Open();
}

void
Report::Save()
{
    NS_LOG_FUNCTION_NOARGS();

    Write();
    Close();
}

void
Report::Open()
{
    NS_LOG_FUNCTION_NOARGS();
    if (m_writer)
        return;

    int rc;

    m_writer = xmlNewTextWriterFilename(GetFilename().c_str(), 0);
    NS_ASSERT(m_writer);

    rc = xmlTextWriterSetIndent(m_writer, 4);
    NS_ASSERT(rc == 0);

    rc = xmlTextWriterStartDocument(m_writer, nullptr, "utf-8", nullptr);
    NS_ASSERT(rc >= 0);

    rc = xmlTextWriterWriteComment(m_writer, BAD_CAST "IoD_Sim Report Summary");
    NS_ASSERT(rc >= 0);
}

void
Report::Close()
{
    NS_LOG_FUNCTION_NOARGS();
    if (!m_writer)
        return;

    const int rc = xmlTextWriterEndDocument(m_writer);
    NS_ASSERT(rc >= 0);

    xmlFreeTextWriter(m_writer);
    m_writer = nullptr;
}

void
Report::Write() const
{
    NS_LOG_FUNCTION_NOARGS();
    NS_ASSERT(m_writer);

    m_dataTreeRoot->Write(m_writer);
}

const std::string
Report::GetFilename() const
{
    NS_LOG_FUNCTION_NOARGS();
    std::ostringstream bFilename;

    bFilename << m_resultsPath << "summary.xml";

    return bFilename.str();
}

} // namespace ns3
