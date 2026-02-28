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
 */
#include "mobility-factory-helper.h"

#include <ns3/building-position-allocator.h>
#include <ns3/constant-acceleration-mobility-model.h>
#include <ns3/constant-velocity-mobility-model.h>
#include <ns3/node.h>

namespace ns3
{

void
MobilityFactoryHelper::SetMobilityModel(MobilityHelper& helper,
                                        const MobilityModelConfiguration& modelConf)
{
    helper.m_mobility.SetTypeId(modelConf.GetName());

    if (modelConf.GetName() == "ns3::GeoConstantVelocityMobility")
    {
        helper.SetPositionAllocator(nullptr);
    }
    else if (modelConf.GetName() == "ns3::RandomWalk2dOutdoorMobilityModel")
    {
        static auto positionAllocator = CreateObject<OutdoorPositionAllocator>();
        helper.SetPositionAllocator(positionAllocator);
    }

    if (modelConf.GetAttributes().size() == 0 && !modelConf.GetInitialPosition())
    {
        return;
    }

    auto positionAllocator = CreateObject<ListPositionAllocator>();
    bool hasPosition = false;

    if (modelConf.GetInitialPosition())
    {
        positionAllocator->Add(modelConf.GetInitialPosition().value());
        hasPosition = true;
    }

    for (auto& attr : modelConf.GetAttributes())
    {
        if (attr.name == "Position")
        {
            Vector3D initialPosition = StaticCast<Vector3DValue, AttributeValue>(attr.value)->Get();
            if (!hasPosition)
            {
                positionAllocator->Add(initialPosition);
                hasPosition = true;
            }
        }
        else if (attr.name == "Velocity" || attr.name == "Acceleration")
        {
            // Handled post-installation by ApplyExtraAttributes
        }
        else
        {
            helper.m_mobility.Set(attr.name, *attr.value);
        }
    }

    if (hasPosition)
    {
        helper.SetPositionAllocator(positionAllocator);
    }
}

void
MobilityFactoryHelper::ApplyExtraAttributes(Ptr<Node> node,
                                            const MobilityModelConfiguration& modelConf)
{
    auto mob = node->GetObject<MobilityModel>();
    if (!mob)
    {
        return;
    }

    for (auto& attr : modelConf.GetAttributes())
    {
        if (attr.name == "Velocity")
        {
            Vector3D velocity = StaticCast<Vector3DValue, AttributeValue>(attr.value)->Get();
            if (auto cv = DynamicCast<ConstantVelocityMobilityModel>(mob))
            {
                cv->SetVelocity(velocity);
            }
            else if (auto ca = DynamicCast<ConstantAccelerationMobilityModel>(mob))
            {
                ca->SetVelocityAndAcceleration(velocity, Vector3D(0.0, 0.0, 0.0));
            }
        }
        else if (attr.name == "Acceleration")
        {
            Vector3D acceleration = StaticCast<Vector3DValue, AttributeValue>(attr.value)->Get();
            if (auto ca = DynamicCast<ConstantAccelerationMobilityModel>(mob))
            {
                ca->SetVelocityAndAcceleration(ca->GetVelocity(), acceleration);
            }
        }
    }
}

} // namespace ns3
