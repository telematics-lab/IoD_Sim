/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2022 The IoD_Sim Authors.
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
#include "report-peripheral.h"

#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ReportPeripheral");

ReportPeripheral::ReportPeripheral (std::string tid, std::vector<double> powerConsumptionStates, std::vector<int> roi) :
  m_tid {tid},
  m_powerConsumptionStates {powerConsumptionStates},
  m_roi {roi}
{
  NS_LOG_FUNCTION (this << tid);
}

ReportPeripheral::~ReportPeripheral ()
{
  NS_LOG_FUNCTION (this);
}

void
ReportPeripheral::AddAttribute(std::tuple<std::string, std::string> attr)
{
  m_otherAttributes.push_back(attr);
}

void
ReportPeripheral::Write (xmlTextWriterPtr h)
{
  std::stringstream roistr;
  NS_LOG_FUNCTION (h);
  if (h == nullptr)
    {
      NS_LOG_WARN ("Passed handler is not valid: " << h << ". "
                   "Data will be discarded.");
      return;
    }

  int rc;

  rc = xmlTextWriterStartElement(h, BAD_CAST "peripheral");
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteElement (h, BAD_CAST "typeID", BAD_CAST m_tid.c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterStartElement(h, BAD_CAST "PowerConsumptionStates");
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterWriteElement (h, BAD_CAST "OFF", BAD_CAST std::to_string(m_powerConsumptionStates[0]).c_str ());
  NS_ASSERT (rc >= 0);
  rc = xmlTextWriterWriteElement (h, BAD_CAST "IDLE", BAD_CAST std::to_string(m_powerConsumptionStates[1]).c_str ());
  NS_ASSERT (rc >= 0);
  rc = xmlTextWriterWriteElement (h, BAD_CAST "ON", BAD_CAST std::to_string(m_powerConsumptionStates[2]).c_str ());
  NS_ASSERT (rc >= 0);

  rc = xmlTextWriterEndElement(h);
  NS_ASSERT (rc >= 0);

  if (m_roi.size () > 0)
    {
      for (auto r : m_roi)
        roistr << std::to_string (r) << ", ";

      std::string roiser = roistr.str ();
      roiser.pop_back ();

      rc = xmlTextWriterWriteElement (h, BAD_CAST "RoITrigger", BAD_CAST roiser.c_str ());
      NS_ASSERT (rc >= 0);
    }

  for (auto attr : m_otherAttributes)
    {
      rc = xmlTextWriterWriteElement (h, BAD_CAST std::get<0> (attr).c_str (), BAD_CAST std::get<1> (attr).c_str ());
      NS_ASSERT (rc >= 0);
    }

  rc = xmlTextWriterEndElement (h);
  NS_ASSERT (rc >= 0);
}

} // namespace ns3
