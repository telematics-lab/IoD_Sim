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
 */
#ifndef IRS_PATCH_H
#define IRS_PATCH_H

#include <ns3/double-vector.h>
#include <ns3/int-vector.h>
#include <ns3/node.h>
#include <ns3/object.h>
#include <ns3/str-vec.h>

#include <vector>

namespace ns3
{

/**
 * \ingroup irs
 *
 * \brief Model the Patch of an Intelligent Reflective Surface (IRS).
 */
class IrsPatch : public Object
{
  public:
    /** Helper class to define the dimensions of the Patch relative to the IRS. */
    class Size
    {
      public:
        Size()
            : startColIdx{0},
              endColIdx{0},
              startRowIdx{0},
              endRowIdx{0}
        {
        }

        Size(uint32_t startColIdx, uint32_t endColIdx, uint32_t startRowIdx, uint32_t endRowIdx)
            : startColIdx{startColIdx},
              endColIdx{endColIdx},
              startRowIdx{startRowIdx},
              endRowIdx{endRowIdx}
        {
        }

        const uint32_t GetRowSize() const
        {
            return endRowIdx - startRowIdx + 1;
        }

        const uint32_t GetColSize() const
        {
            return endColIdx - startColIdx + 1;
        }

        uint32_t startColIdx;
        uint32_t endColIdx;
        uint32_t startRowIdx;
        uint32_t endRowIdx;
    };

    static TypeId GetTypeId();

    IrsPatch();

    /** Set the size of the patch.
     * \param serializedSize The coordinates of the patch specified through a
     *                       serialized vector with the following sequence of ints:
     *                        0. Start Column Index
     *                        1. End Column Index
     *                        2. Start Row Index
     *                        3. End Row Index
     */
    void SetSizeSerialized(const IntVector& serializedSize);
    /** Set the size of the patch given a IrsPatch::Size object. */
    void SetSize(const IrsPatch::Size& size);
    /** Set the nodes served by this Patch given a vector of two Node XPaths. */
    void SetServingNodesSerialized(const StrVec& nodeIds);
    /** Set the nodes served by this Patch given their ns-3 XPath. */
    void SetServingNodes(const std::string& nodeXPath1, const std::string& nodeXPath2);
    /** Lookup and retrieve Node reference given its ns-3 XPath. */
    Ptr<Node> LookupNodeByPath(const std::string& nodePath) const;
    /** Set the nodes served by this Patch given two Node IDs. */
    void SetServingNodes(const uint32_t nodeId1, const uint32_t nodeId2);
    /** Set the nodes served by this Patch given two Node pointers. */
    void SetServingNodes(const Ptr<Node> node1, const Ptr<Node> node2);
    /** Set PhaseX of this Patch. */
    void SetPhaseX(const double phaseX);
    /** Set PhaseY of this Patch. */
    void SetPhaseY(const double phaseY);

    const Size GetSize() const;
    /** Get the served nodes. */
    const std::pair<Ptr<Node>, Ptr<Node>> GetServedNodes() const;
    /** Determine whether the IRS Patch is serving specific Nodes or not. */
    const bool IsServing() const;
    /** Retrieve the phase shift along the X-axis. */
    const double GetPhaseX() const;
    /** Retrieve the phase shift along the Y-axis. */
    const double GetPhaseY() const;
    /**Set life periods from JSON using TypeId**/
    void SetLifeTime(const double l);
    /**Get life periods**/
    const double GetLifeTime() const;

  protected:
    void DoInitialize(void);
    void DoDispose(void);

  private:
    Size m_size;
    bool m_isServing; /// Indicate if this patch is serving any Node
    double m_phaseX; /// Phase shift on the X-axis used if the patch is not serving any PhyLayer (or
                     /// Node)
    double m_phaseY; /// Phase shift on the Y-axis used if the patch is not serving any PhyLayer (or
                     /// Node)
    std::pair<Ptr<Node>, Ptr<Node>> m_servingNodes; /// The Nodes served by this Patch
    double m_lifetime;                              /// The life time of this Patch
};

ATTRIBUTE_HELPER_HEADER(IrsPatch);

std::ostream& operator<<(std::ostream& os, const IrsPatch::Size& s);
std::ostream& operator<<(std::ostream& os, const IrsPatch& p);
std::istream& operator>>(std::istream& is, IrsPatch::Size& s);
std::istream& operator>>(std::istream& is, IrsPatch& p);

} // namespace ns3

#endif /* IRS_PATCH_H */
