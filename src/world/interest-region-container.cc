/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (C) 2018-2023 The IoD_Sim Authors.
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

#include "interest-region-container.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("InterestRegionContainer");

InterestRegionContainer::InterestRegionContainer()
{
}

InterestRegionContainer::~InterestRegionContainer()
{
    for (std::vector<ns3::Ptr<ns3::InterestRegion>>::iterator i = m_interestRegions.begin();
         i != m_interestRegions.end();
         i++)
    {
        Ptr<InterestRegion> region = *i;
        region->Dispose();
        *i = 0;
    }
    m_interestRegions.clear();
}

const Ptr<InterestRegion>
InterestRegionContainer::Create(const DoubleVector& coords)
{
    auto region =
        CreateObjectWithAttributes<InterestRegion>("Coordinates", DoubleVectorValue(coords));
    m_interestRegions.push_back(region);
    return region;
}

uint32_t
InterestRegionContainer::GetN(void) const
{
    return m_interestRegions.size();
}

Ptr<InterestRegion>
InterestRegionContainer::GetRoI(uint32_t i)
{
    return m_interestRegions[i];
}

InterestRegionContainer::Iterator
InterestRegionContainer::Begin(void) const
{
    return m_interestRegions.begin();
}

InterestRegionContainer::Iterator
InterestRegionContainer::End(void) const
{
    return m_interestRegions.end();
}

int
InterestRegionContainer::IsInRegions(std::vector<int> indexes, Vector& position)
{
    if (m_interestRegions.size() == 0)
        return -2;
    for (auto index : indexes)
    {
        if (m_interestRegions[index]->IsInside(position))
            return index;
    }
    return -1;
}

int
InterestRegionContainer::IsInRegions(Vector& position)
{
    if (m_interestRegions.size() == 0)
        return -2;
    for (int index = 0; index < (int)m_interestRegions.size(); index++)
    {
        if (m_interestRegions[index]->IsInside(position))
            return index;
    }
    return -1;
}
} // namespace ns3
