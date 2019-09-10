/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 The IoD_Sim Authors.
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
#ifndef REPORT_DATA_STATS_H
#define REPORT_DATA_STATS_H
#include <cstddef>

#include <ns3/drone-communications.h>
#include <ns3/object.h>

#include <libxml/xmlwriter.h>

namespace ns3 {

class ReportDataStats : public Object
{
public:
  /**
   * Register the type using ns-3 TypeId System.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /**
   * Default constructor
   */
  ReportDataStats ();
  /**
   * Default destructor
   */
  ~ReportDataStats ();

  constexpr void Add (const uint32_t bytes) { m_bytes += bytes; }

  /**
   * Write report data statistics to a XML file with a given handler
   *
   * \param handle the handler to communicate data to the opened XML file
   */
  void Write (xmlTextWriterPtr handle) const;
private:
  PacketType m_kind;              /// the kind of data packet that is monitored
  mutable std::uint32_t m_bytes;  /// the amount of bytes transferred
};

} // namespace ns3

#endif /* REPORT_DATA_STATS_H */
