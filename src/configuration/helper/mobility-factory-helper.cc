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
#include "mobility-factory-helper.h"

#include <ns3/building-position-allocator.h>

namespace ns3
{

void
MobilityFactoryHelper::SetMobilityModel(MobilityHelper& helper, const ModelConfiguration& modelConf)
{
    helper.m_mobility.SetTypeId(modelConf.GetName());

    if (modelConf.GetName() == "ns3::ConstantPositionMobilityModel")
    {
        if (modelConf.GetAttributes().size() == 0)
            return;

        auto positionAllocator = CreateObject<ListPositionAllocator>();
        Vector3D initialPosition =
            StaticCast<Vector3DValue, AttributeValue>(modelConf.GetAttributes()[0].value)->Get();
        positionAllocator->Add(initialPosition);
        helper.SetPositionAllocator(positionAllocator);
    }
    else if (modelConf.GetName() == "ns3::RandomWalk2dOutdoorMobilityModel")
    {
        static auto positionAllocator = CreateObject<OutdoorPositionAllocator>();
        helper.SetPositionAllocator(positionAllocator);
    }
    else
    {
        for (auto& attr : modelConf.GetAttributes())
            helper.m_mobility.Set(attr.name, *attr.value);
    }
}

MobilityFactoryHelper::MobilityFactoryHelper()
{
}

} // namespace ns3
