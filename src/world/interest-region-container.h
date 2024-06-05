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
#ifndef INTEREST_REGION_CONTAINER_H
#define INTEREST_REGION_CONTAINER_H

#include "interest-region.h"

#include <ns3/boolean.h>
#include <ns3/object-factory.h>
#include <ns3/singleton.h>

#include <vector>

#define irc InterestRegionContainer::Get()

namespace ns3
{

/**
 * \brief Keeps track of a set of region of interest pointers.
 */
class InterestRegionContainer : public Singleton<InterestRegionContainer>
{
  public:
    /**
     * \brief Get the type ID.
     *
     * \returns the object TypeId
     */
    // static TypeId GetTypeId (void);

    InterestRegionContainer();
    ~InterestRegionContainer();

    /// Container iterator
    typedef std::vector<Ptr<InterestRegion>>::const_iterator Iterator;

    /**
     * \brief Creates a RoI and append the pointer to the container
     *
     * \param coords 3D coordinates of the desired box
     * \return The RoI pointer created
     */
    const Ptr<InterestRegion> Create(const DoubleVector& coords);

    /**
     * \brief An iterator which refers to the first RoI in the
     * container.
     *
     * \returns an iterator which refers to the first RoI in the container.
     */
    Iterator Begin(void) const;

    /**
     * \brief An iterator which indicates past-the-last RoI in the
     * container.
     *
     * \returns an iterator which indicates an ending condition for a loop.
     */
    Iterator End(void) const;

    /**
     * \brief Gets the number of Ptr<RegionOfInterest> stored in this container.
     *
     * \returns the number of Ptr<RegionOfInterest> stored in this container.
     */
    uint32_t GetN(void) const;

    /**
     * \brief Gets the Ptr<RegionOfInterest> stored in this container at a given
     * index.
     *
     * \param i the index of the requested RoI pointer.
     * \returns the requested RoI pointer.
     */
    Ptr<InterestRegion> GetRoI(uint32_t i);

    /**
     * \brief Verifies that a certain point belongs to a set of regions
     *
     * \param indexes vector of regions' indexes.
     * \param position Vector of 3D coordinates describing the point.
     * \returns -1 If it does not belong to any region
     *          -2 If the region vector is empty
     *          <index> If it does belong to a region
     */
    int IsInRegions(std::vector<int> indexes, Vector& position);
    int IsInRegions(Vector& position);

  private:
    std::vector<Ptr<InterestRegion>> m_interestRegions; //!< Regions smart pointers
};

} // namespace ns3

#endif /* DRONE_CONTAINER_H */
