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
 *
 * Authors: Giovanni Iacovelli <giovanni.iacovelli@poliba.it>
 */
#ifndef REPORT_PERIPHERAL_H
#define REPORT_PERIPHERAL_H
#include <ns3/vector.h>

#include <libxml/xmlwriter.h>

namespace ns3
{

/**
 * \ingroup report
 *
 * \brief Report module for a peripheral.
 *
 */
class ReportPeripheral
{
  public:
    /**
     * Initialize a report peripheral with a given power consumptions and regions of interest
     */
    ReportPeripheral(std::string, std::vector<double>, std::vector<int>);
    /**
     * \param a tuple containing the name and the value (string) of an additional attribute
     */
    void AddAttribute(std::tuple<std::string, std::string>);
    /**
     * Write Zsp report data to a XML file with a given handler
     *
     * \param handle the XML handler to write data on
     */
    void Write(xmlTextWriterPtr handle);

  private:
    std::string m_tid;
    std::vector<double> m_powerConsumptionStates; //!< Power consumptions for each state
    std::vector<int> m_roi;                       //!< Regions of interest indexes
    std::vector<std::tuple<std::string, std::string>> m_otherAttributes;
};

} // namespace ns3

#endif /* REPORT_PERIPHERAL_H */
