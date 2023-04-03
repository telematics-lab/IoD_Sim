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
#ifndef FLIGHT_PLAN_H
#define FLIGHT_PLAN_H

#include "proto-point.h"

#include <ns3/attribute-helper.h>
#include <ns3/attribute.h>

#include <istream>
#include <ostream>
#include <vector>

namespace ns3
{

/**
 * \brief keep track of a set of ProtoPoint pointers.
 *
 * This is useful if we want to operate on more than one ProtoPoint at a time,
 * or just keep them organized in a data structure and pass it to a
 * MobilityModel compatible with the IoD proposed strucutres.
 */
class FlightPlan
{
  public:
    /// ProtoPoint Container iterator
    typedef std::vector<Ptr<ProtoPoint>>::const_iterator Iterator;

    /**
     * Create an empty FlightPlan
     */
    FlightPlan();
    /**
     * Create a FlightPlan with exactly one ProtoPoint which has been
     * previously instantiated. This single ProtoPoint is specified by a smart
     * pointer.
     *
     * \param point The Ptr<ProtoPoint> to add to the container.
     */
    FlightPlan(Ptr<ProtoPoint> point);
    /**
     * Create a FlightPlan with exactly one point which has been
     * previously instantiated and assigned a name using the Object Name Service.
     * This ProtoPoint is then specified by its assigned name.
     *
     * \param pointName The name of the ProtoPoint Object to add to the container.
     */
    // FlightPlan (std::string pointName);
    /**
     * Create a FlightPlan which is a concatenation of two input
     * FlightPlans.
     *
     * \param a The first FlightPlan
     * \param b The second FlightPlan
     */
    FlightPlan(const FlightPlan& a, const FlightPlan& b);
    /**
     * Create a FlightPlan which is a concatenation of three input
     * FlightPlans.
     *
     * \param a The first FlightPlan
     * \param b The second FlightPlan
     * \param c The third FlightPlan
     */
    FlightPlan(const FlightPlan& a, const FlightPlan& b, const FlightPlan& c);
    /**
     * Create a FlightPlan which is a concatenation of four input
     * FlightPlans.
     *
     * \param a The first FlightPlan
     * \param b The second FlightPlan
     * \param c The third FlightPlan
     * \param d The fourth FlightPlan
     */
    FlightPlan(const FlightPlan& a, const FlightPlan& b, const FlightPlan& c, const FlightPlan& d);
    /**
     * Create a FlightPlan which is a concatenation of six input
     * FlightPlans.
     *
     * \param a The first FlightPlan
     * \param b The second FlightPlan
     * \param c The third FlightPlan
     * \param d The fourth FlightPlan
     * \param e The fifth FlightPlan
     */
    FlightPlan(const FlightPlan& a,
               const FlightPlan& b,
               const FlightPlan& c,
               const FlightPlan& d,
               const FlightPlan& e);

    /**
     * \brief Get an iterator which refers to the first ProtoPoint in the
     * container.
     *
     * ProtoPoints can be retrieved from the container in two ways. First,
     * directly by an index into the container, and second, using an iterator.
     * This method is used in the iterator method and is typically used in a
     * for-loop to run through the ProtoPoints
     *
     * \code
     *   FlightPlan::Iterator i;
     *   for (i = container.Begin (); i != container.End (); i++)
     *     {
     *       (*i)->method ();  // some ProtoPoint method
     *     }
     * \endcode
     *
     * \return an iterator which refers to the first ProtoPoint in the container.
     */
    Iterator Begin() const;
    /**
     * \brief Get an iterator which refers to the past-the-last ProtoPoint in the
     * container.
     *
     * ProtoPoints can be retrieved from the container in two ways. First,
     * directly by an index into the container, and second, using an iterator.
     * This method is used in the iterator method and is typically used in a
     * for-loop to run through the ProtoPoints
     *
     * \code
     *   FlightPlan::Iterator i;
     *   for (i = container.Begin (); i != container.End (); i++)
     *     {
     *       (*i)->method ();  // some ProtoPoint method
     *     }
     * \endcode
     *
     * \return an iterator which indicates an ending condition for a loop.
     */
    Iterator End() const;

    /**
     * \brief Get the number of Ptr<ProtoPoint> stored in this container.
     *
     * ProtoPoints can be retrieved from the container in two ways. First,
     * directly by and index into the container, and second, using an iterator.
     * This method is used in the direct method and is typically used to define an
     * ending condition in a for-loop that runs through the stored ProtoPoints
     *
     * \code
     *   uint32_t nProtoPoints = container.GetN ();
     *   for (uint32_t i = 0; i < nProtoPoints; ++i)
     *     {
     *       Ptr<ProtoPoint> p = containter.Get (i);
     *       i->method ();  // some ProtoPoint method
     *     }
     * \endcode
     *
     * \return the number of Ptr<ProtoPoint> stored in this container.
     */
    uint32_t GetN() const;

    /**
     * \brief Get the Ptr<ProtoPoint> stored in this container at a given index.
     *
     * ProtoPoints can be retrieved from the container in two ways. First,
     * directly by and index into the container, and second, using an iterator.
     * This method is used in the direct method and is typically used to define an
     * ending condition in a for-loop that runs through the stored ProtoPoints
     *
     * \code
     *   uint32_t nProtoPoints = container.GetN ();
     *   for (uint32_t i = 0; i < nProtoPoints; ++i)
     *     {
     *       Ptr<ProtoPoint> p = containter.Get (i);
     *       i->method ();  // some ProtoPoint method
     *     }
     * \endcode
     *
     * \param i the index of the requested ProtoPoint pointer.
     * \return the requested ProtoPoint pointer.
     */
    Ptr<ProtoPoint> Get(const uint32_t i) const;

    Ptr<ProtoPoint> GetFront() const;
    Ptr<ProtoPoint> GetBack() const;

    /**
     * \brief Append the contents of another FlightPlan to the end of
     * this container.
     *
     * \param other The FlightPlan to append.
     */
    void Add(const FlightPlan other);
    /**
     * \brief Append a single Ptr<ProtoPoint> to this container.
     *
     * \param point the Ptr<ProtoPoint> to append.
     */
    void Add(const Ptr<ProtoPoint> point);
    /**
     * \brief Append to this container the single Ptr<ProtoPoint> referred to via
     * its Object Name Service registered name.
     *
     * \param pointName The name of the ProtoPoint Object to add to the container.
     */
    void Add(const std::string pointName);

  private:
    std::vector<Ptr<ProtoPoint>> m_protoPoints; //!< ProtoPoints smart pointers
};

ATTRIBUTE_HELPER_HEADER(FlightPlan);

std::ostream& operator<<(std::ostream& os, const FlightPlan& prototrajectory);
std::istream& operator>>(std::istream& is, FlightPlan& prototrajectory);

} // namespace ns3

#endif /* FLIGHT_PLAN_H */
