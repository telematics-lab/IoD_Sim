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
#include "report-protocol-stack.h"

#include <ns3/log.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ReportProtocolStack");

ReportProtocolStack::Iterator
ReportProtocolStack::Begin() const
{
    NS_LOG_FUNCTION_NOARGS();

    return m_layers.begin();
}

ReportProtocolStack::Iterator
ReportProtocolStack::End() const
{
    NS_LOG_FUNCTION_NOARGS();

    return m_layers.end();
}

uint32_t
ReportProtocolStack::GetN() const
{
    NS_LOG_FUNCTION_NOARGS();

    return m_layers.size();
}

Ptr<ProtocolLayer>
ReportProtocolStack::Get(const uint32_t i) const
{
    NS_LOG_FUNCTION(i);

    return m_layers[i];
}

void
ReportProtocolStack::Add(Ptr<ProtocolLayer> layer)
{
    NS_LOG_FUNCTION(layer);

    m_layers.push_back(layer);
}

void
ReportProtocolStack::Write(xmlTextWriterPtr h)
{
    NS_LOG_FUNCTION(h);
    if (!h)
    {
        NS_LOG_WARN("Passed handler is not valid: " << h
                                                    << ". "
                                                       "Data will be discarded.");
        return;
    }

    /* Nested Elements */
    for (auto& layer : m_layers)
        layer->Write(h);
}

} // namespace ns3
