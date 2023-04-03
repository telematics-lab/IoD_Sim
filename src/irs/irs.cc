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
#include "irs.h"

#include <ns3/str-vec.h>

#include <cmath>
#include <vector>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Irs");

NS_OBJECT_ENSURE_REGISTERED(Irs);

TypeId
Irs::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::Irs")
            .SetParent<DronePeripheral>()
            .SetGroupName("Irs")
            .AddConstructor<Irs>()
            .AddAttribute("Rows",
                          "N, the number of rows",
                          IntegerValue(1),
                          MakeIntegerAccessor(&Irs::m_rows),
                          MakeIntegerChecker<int>())
            .AddAttribute("Columns",
                          "M, the number of columns",
                          IntegerValue(1),
                          MakeIntegerAccessor(&Irs::m_columns),
                          MakeIntegerChecker<int>())
            .AddAttribute("PruX",
                          "X-side dimension of the Passive Reflective Unit (in meter)",
                          DoubleValue(0.01),
                          MakeDoubleAccessor(&Irs::m_pruX),
                          MakeDoubleChecker<double>())
            .AddAttribute("PruY",
                          "Y-side dimension of the Passive Reflective Unit (in meter)",
                          DoubleValue(0.01),
                          MakeDoubleAccessor(&Irs::m_pruY),
                          MakeDoubleChecker<double>())
            .AddAttribute("RotoAxis",
                          "Well-ordered list of axis used to set IRS rotation (or to BackRotate "
                          "other points)",
                          StrVecValue(),
                          MakeStrVecAccessor(&Irs::SetRotoAxis),
                          MakeStrVecChecker())
            .AddAttribute("RotoAngles",
                          "List of angles (in radians) used to rotate the IRS, the n-th angles "
                          "refers to n-th "
                          "axis defined in m_rotoAxis (or to BackRotate other points)",
                          DoubleVectorValue(),
                          MakeDoubleVectorAccessor(&Irs::SetRotoAnglesDegrees),
                          MakeDoubleVectorChecker())
            .AddAttribute("Patches",
                          "List of patches used to build the IRS",
                          ModelConfigurationVectorValue(),
                          MakeModelConfigurationVectorAccessor(&Irs::SetPatchVector),
                          MakeModelConfigurationVectorChecker());
    return tid;
}

Irs::Irs()
{
}

void
Irs::DoDispose()
{
    NS_LOG_FUNCTION(this);
    Object::DoDispose();
}

void
Irs::DoInitialize(void)
{
    NS_LOG_FUNCTION(this);
    Object::DoInitialize();
}

double
Irs::GetPruX() const
{
    return m_pruX;
}

double
Irs::GetPruY() const
{
    return m_pruY;
}

void
Irs::SetRotoAxis(const StrVec& v)
{
    for (auto& x : v)
    {
        if (x == "X_AXIS")
            m_rotoAxis.push_back(RotoAxisType::X_AXIS);
        else if (x == "Y_AXIS")
            m_rotoAxis.push_back(RotoAxisType::Y_AXIS);
        else if (x == "Z_AXIS")
            m_rotoAxis.push_back(RotoAxisType::Z_AXIS);
        else
            NS_ABORT_MSG("Cannot convert input \""
                         << x
                         << "\" to any of the "
                            "following values: [\"X_AXIS\", \"Y_AXIS\", \"Z_AXIS\"]");
    }
}

const std::vector<RotoAxisType>&
Irs::GetRotoAxis() const
{
    return m_rotoAxis;
}

void
Irs::SetRotoAnglesDegrees(const DoubleVector& a)
{
    for (auto c = a.Begin(); c != a.End(); c++)
    {
        m_rotoAngles.push_back(*c / 180 * M_PI);
    }
}

std::vector<double>
Irs::GetRotoAngles() const
{
    return m_rotoAngles;
}

std::vector<Ptr<IrsPatch>>
Irs::GetPatchVector() const
{
    return m_patches;
}

void
Irs::SetPatchVector(ModelConfigurationVector patchConfs)
{
    NS_LOG_FUNCTION(patchConfs);
    ObjectFactory factory;

    for (uint32_t i = 0; i < m_patches.size(); i++)
    {
        m_patches[i]->Dispose();
    }
    m_patches.clear();

    for (auto c = patchConfs.Begin(); c != patchConfs.End(); c++)
    {
        factory = ObjectFactory{"ns3::IrsPatch"};
        for (auto attrIt = c->AttributesBegin(); attrIt != c->AttributesEnd(); attrIt++)
            factory.Set(attrIt->name, *attrIt->value);

        auto patch = factory.Create<IrsPatch>();

        for (auto aggIt = c->AggregatesBegin(); aggIt != c->AggregatesEnd(); aggIt++)
        {
            factory = ObjectFactory{aggIt->GetName()};
            for (auto attrIt = aggIt->AttributesBegin(); attrIt != aggIt->AttributesEnd(); attrIt++)
                factory.Set(attrIt->name, *attrIt->value);

            auto obj = factory.Create<Object>();
            obj->AggregateObject(patch);
            obj->Initialize();
        }

        NS_ABORT_MSG_IF(!AddPatch(patch),
                        "Error while adding patch to the IRS, as there is overlapping.");
    }
}

bool
Irs::AddPatch(Ptr<IrsPatch> p)
{
    for (auto& installedPatch : m_patches)
        if (IsOverlapped(p->GetSize(), installedPatch->GetSize()))
            return false;

    m_patches.push_back(p);
    return true;
}

const bool
Irs::IsOverlapped(const uint32_t aStart,
                  const uint32_t aEnd,
                  const uint32_t bStart,
                  const uint32_t bEnd)
{
    return (aStart <= bStart && aEnd >= bEnd) || (bStart <= aStart && aStart <= bEnd) ||
           (bStart <= aEnd && aEnd <= bEnd);
}

const bool
Irs::IsOverlapped(const IrsPatch::Size& a, const IrsPatch::Size& b)
{
    return IsOverlapped(a.startColIdx, a.endColIdx, b.startColIdx, b.endColIdx) &&
           IsOverlapped(a.startRowIdx, a.endRowIdx, b.startRowIdx, b.endRowIdx);
}

} // namespace ns3
