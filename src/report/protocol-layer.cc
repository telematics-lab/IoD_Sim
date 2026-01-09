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
#include "protocol-layer.h"

#include <ns3/log.h>
#include <ns3/string.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ProtocolLayer");
NS_OBJECT_ENSURE_REGISTERED(ProtocolLayer);

TypeId
ProtocolLayer::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ProtocolLayer")
                            .AddConstructor<ProtocolLayer>()
                            .SetParent<Object>()
                            .AddAttribute("InstanceType",
                                          "The InstanceTypeId",
                                          StringValue(),
                                          MakeStringAccessor(&ProtocolLayer::m_instancetypeid),
                                          MakeStringChecker());

    return tid;
}

void
ProtocolLayer::Write(xmlTextWriterPtr h)
{
    NS_LOG_FUNCTION(h);
    if (!h)
    {
        NS_LOG_WARN("Passed handler is not valid: " << h
                                                    << ". "
                                                       "Data will be discarded.");
        return;
    }

    int rc;

    rc = xmlTextWriterWriteElement(h, BAD_CAST "type", BAD_CAST m_instancetypeid.c_str());
    NS_ASSERT(rc >= 0);
}

} // namespace ns3
