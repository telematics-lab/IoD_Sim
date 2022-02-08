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
#ifndef REPORT_WORLD_H
#define REPORT_WORLD_H
#include <ns3/vector.h>

#include <libxml/xmlwriter.h>
#include <ns3/interest-region-container.h>

namespace ns3 {

class ReportWorld
{
public:
    /**
     * Initialize a world report
     */
    ReportWorld ();
    /**
     * Default destructor
     */
    ~ReportWorld ();
    /**
     * Write Zsp report data to a XML file with a given handler
     *
     * \param handle the XML handler to write data on
     */
    void Write (xmlTextWriterPtr handle) const;
};

} // namespace ns3

#endif /* REPORT_WORLD_H */
