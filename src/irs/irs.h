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
#ifndef IRS_H
#define IRS_H

#include "irs-patch.h"

#include <ns3/drone-peripheral.h>
#include <ns3/drone.h>
#include <ns3/model-configuration-vector.h>
#include <ns3/pointer.h>
#include <ns3/str-vec.h>
#include <ns3/vector.h>

#include <vector>

namespace ns3
{

enum class RotoAxisType
{
    X_AXIS = 0,
    Y_AXIS,
    Z_AXIS
};

/**
 * \brief Base class describing an Intelligent Reflective Surface aggregable to a node like a
 * peripheral.
 */
class Irs : public DronePeripheral
{
  public:
    typedef std::vector<Ptr<IrsPatch>>::iterator PatchIterator;

    /**
     * \brief Get the type ID.
     *
     * \returns the object TypeId
     */
    static TypeId GetTypeId(void);

    /**
     * \brief Returns the X-side dimension of a single Passive Reflective Unit.
     *
     * \returns X-side dimension of the PRU.
     */
    double GetPruX() const;

    /**
     * \brief Returns the Y-side dimension of a single Passive Reflective Unit.
     *
     * \returns Y-side dimension of the PRU.
     */
    double GetPruY() const;
    /**
     * \brief TODO
     *
     * \param a TODO
     */
    void SetRotoAxis(const StrVec& v);

    /**
     * \brief TODO
     *
     * \returns TODO
     */
    const std::vector<RotoAxisType>& GetRotoAxis() const;

    /**
     * \brief TODO
     *
     * \param a TODO
     */
    void SetRotoAnglesDegrees(const DoubleVector& a);

    /**
     * \brief TODO
     *
     * \returns TODO
     */
    std::vector<double> GetRotoAngles() const;

    /**
     * \returns Retrieve the current state of patches.
     */
    std::vector<Ptr<IrsPatch>> GetPatchVector() const;

    /**
     * \returns Initialize the IRS with a new set of Patches from a Model description.
     */
    void SetPatchVector(ModelConfigurationVector patchConfigurations);

    /**
     * \brief Add a new patch to an irs. In order to guarantee consistency an overlap check is done.
     *
     * \param p Patch that needs to be added
     * \returns Result of the overlap check
     */
    bool AddPatch(Ptr<IrsPatch> p);

    uint32_t GetPatchN() const; // Not implemented

    Ptr<IrsPatch> GetPatch(uint32_t i) const; // Not implemented

    PatchIterator BeginPatch(); // Not implemented

    PatchIterator EndPatch(); // Not implemented

  protected:
    virtual void DoDispose(void);
    virtual void DoInitialize(void);

  private:
    static const bool IsOverlapped(const uint32_t aStart,
                                   const uint32_t aEnd,
                                   const uint32_t bStart,
                                   const uint32_t bEnd);
    static const bool IsOverlapped(const IrsPatch::Size& a, const IrsPatch::Size& b);

    int m_rows;    // N, the number of rows
    int m_columns; // M, the number of columns
    double m_pruX; // X-side dimension of the Passive Reflective Unit (in meter)
    double m_pruY; // Y-side dimension of the Passive Reflective Unit (in meter)
    std::vector<RotoAxisType> m_rotoAxis; // Well-ordered list of axis used to set IRS rotation (or
                                          // to BackRotate other points)
    std::vector<double>
        m_rotoAngles; // List of angles (in radians) used to rotate the IRS, the n-th angles refers
                      // to n-th axis defined in m_rotoAxis (or to BackRotate other points)
    std::vector<Ptr<IrsPatch>> m_patches; // List of patches
};

} // namespace ns3

#endif /* IRS_H */
