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
#include "irs-patch.h"

#include <ns3/boolean.h>
#include <ns3/config.h>
#include <ns3/double-vector.h>
#include <ns3/double.h>
#include <ns3/int-vector.h>
#include <ns3/node-list.h>
#include <ns3/node.h>
#include <ns3/scenario-configuration-helper.h>
#include <ns3/str-vec.h>

#include <vector>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("IrsPatch");
NS_OBJECT_ENSURE_REGISTERED(IrsPatch);

TypeId
IrsPatch::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::IrsPatch")
            .SetParent<Object>()
            .SetGroupName("irs")
            .AddConstructor<IrsPatch>()
            .AddAttribute("Size",
                          "Coordinates of the patch relative to the IRS. (Start Column Index, End "
                          "Column Index, Start Row Index, End Row Index)",
                          IntVectorValue(),
                          MakeIntVectorAccessor(&IrsPatch::SetSizeSerialized),
                          MakeIntVectorChecker())
            .AddAttribute(
                "PhaseX",
                "Phase angle along the X axis in case the IRS is not serving any specific "
                "communication.",
                DoubleValue(),
                MakeDoubleAccessor(&IrsPatch::m_phaseX),
                MakeDoubleChecker<double>())
            .AddAttribute(
                "PhaseY",
                "Phase angle along the Y axis in case the IRS is not serving any specific "
                "communication.",
                DoubleValue(),
                MakeDoubleAccessor(&IrsPatch::m_phaseY),
                MakeDoubleChecker<double>())
            .AddAttribute("ServingNodes",
                          "ns-3 XPath of serving nodes.",
                          StrVecValue(),
                          MakeStrVecAccessor(&IrsPatch::SetServingNodesSerialized),
                          MakeStrVecChecker())
            .AddAttribute("LifeTime",
                          "Life time of the patch in seconds.",
                          DoubleValue(0.0), // Use 0.0 to indicate that the patch will not expire
                          MakeDoubleAccessor(&IrsPatch::SetLifeTime),
                          MakeDoubleChecker<double>());
    return tid;
}

IrsPatch::IrsPatch()
{
    m_isServing = false;
}

IrsPatch::~IrsPatch()
{
}

void
IrsPatch::DoDispose()
{
    NS_LOG_FUNCTION(this);
    Object::DoDispose();
}

void
IrsPatch::DoInitialize(void)
{
    NS_LOG_FUNCTION(this);
    Object::DoInitialize();
}

void
IrsPatch::SetSizeSerialized(const IntVector& a)
{
    NS_ASSERT_MSG(a.GetN() == 4,
                  "RowsRange needs extactly four values. Current input given: " << a);

    m_size.startColIdx = a.Get(0);
    m_size.endColIdx = a.Get(1);
    m_size.startRowIdx = a.Get(2);
    m_size.endRowIdx = a.Get(3);
}

void
IrsPatch::SetSize(const IrsPatch::Size& size)
{
    m_size = size;
}

void
IrsPatch::SetServingNodesSerialized(const StrVec& a)
{
    NS_ASSERT_MSG(a.GetN() == 2 || a.GetN() == 0,
                  "ServingNodes needs exactly two values or none, if dynamically configured. "
                  "Current input given: "
                      << a);
    if (a.GetN() == 2)
        SetServingNodes(a.Get(0), a.Get(1));
}

void
IrsPatch::SetServingNodes(const std::string& nodeXPath1, const std::string& nodeXPath2)
{
    SetServingNodes(LookupNodeByPath(nodeXPath1), LookupNodeByPath(nodeXPath2));
}

Ptr<Node>
IrsPatch::LookupNodeByPath(const std::string& nodePath) const
{
    auto matches = Config::LookupMatches(nodePath);
    NS_ASSERT_MSG(matches.GetN() == 1,
                  "There is ambiguity with the given Node XPath: " << nodePath);
    return StaticCast<Node, Object>(matches.Get(0));
}

void
IrsPatch::SetServingNodes(uint32_t nid1, uint32_t nid2)
{
    SetServingNodes(NodeList::GetNode(nid1), NodeList::GetNode(nid2));
}

void
IrsPatch::SetServingNodes(Ptr<Node> n1, Ptr<Node> n2)
{
    NS_ASSERT_MSG(n1 != nullptr && n2 != nullptr, "ServingNodes must be set with two valid nodes.");
    m_isServing = true;
    m_servingNodes = std::make_pair(n1, n2);
}

void
IrsPatch::SetPhaseX(double phase)
{
    m_phaseX = phase;
}

void
IrsPatch::SetPhaseY(double phase)
{
    m_phaseY = phase;
}

const std::pair<Ptr<Node>, Ptr<Node>>
IrsPatch::GetServedNodes() const
{
    return m_servingNodes;
}

const IrsPatch::Size
IrsPatch::GetSize() const
{
    return m_size;
}

const bool
IrsPatch::IsServing() const
{
    return m_isServing;
}

const double
IrsPatch::GetPhaseX() const
{
    return m_phaseX;
}

const double
IrsPatch::GetPhaseY() const
{
    return m_phaseY;
}

void
IrsPatch::SetLifeTime(const double l)
{
    m_lifetime = (l == 0.0) ? CONFIGURATOR->GetDuration() : l;
}

const double
IrsPatch::GetLifeTime() const
{
    return m_lifetime;
}

std::ostream&
operator<<(std::ostream& os, const IrsPatch::Size& size)
{
    os << size.startColIdx << ";" << size.endColIdx << ";" << size.startRowIdx << ";"
       << size.endRowIdx << ";";
    return os;
}

std::ostream&
operator<<(std::ostream& os, const IrsPatch& patch)
{
    os << patch.GetSize() << ";" << patch.GetPhaseX() << ";" << patch.GetPhaseY() << ";"
       << patch.IsServing() << ";" << patch.GetServedNodes().first->GetId() << ";"
       << patch.GetServedNodes().second->GetId() << ";";
    return os;
}

std::istream&
operator>>(std::istream& is, IrsPatch::Size& size)
{
    uint32_t startColIdx, endColIdx, startRowIdx, endRowIdx;
    is >> startColIdx;
    is.ignore(1);
    is >> endColIdx;
    is.ignore(1);
    is >> startRowIdx;
    is.ignore(1);
    is >> endRowIdx;
    is.ignore(1);

    size.startColIdx = startColIdx;
    size.endColIdx = endColIdx;
    size.startRowIdx = startRowIdx;
    size.endRowIdx = endRowIdx;

    return is;
}

std::istream&
operator>>(std::istream& is, IrsPatch& patch)
{
    IrsPatch::Size size;
    double phaseX, phaseY;
    bool isServing;
    int servingNodeId1, servingNodeId2;

    is >> size;
    is.ignore(1);
    is >> phaseX;
    is.ignore(1);
    is >> phaseY;
    is.ignore(1);
    is >> isServing;
    is.ignore(1);
    is >> servingNodeId1;
    is.ignore(1);
    is >> servingNodeId2;
    is.ignore(1);

    patch.SetSize(size);
    patch.SetPhaseX(phaseX);
    patch.SetPhaseY(phaseY);

    if (isServing)
        patch.SetServingNodes(servingNodeId1, servingNodeId2);

    return is;
}

} // namespace ns3
