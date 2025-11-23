/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#ifndef GEO_SGP4_MOBILITY_H
#define GEO_SGP4_MOBILITY_H

#include "ns3/geocentric-mobility-model.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/vector.h"

#include <memory>
#include <string>

/**
 * \file
 * \ingroup leo
 *
 * Declaration of GeoSGP4Mobility - Pure SGP4-based satellite mobility model
 */

namespace libsgp4
{
class SGP4;
class Tle;
} // namespace libsgp4

namespace ns3
{

/**
 * \ingroup leo
 * \brief SGP4-based satellite mobility model using TLE data
 *
 * This model uses SGP4 propagation for accurate satellite position
 * and velocity calculation based on Two-Line Element (TLE) data.
 */
class GeoSGP4Mobility : public GeocentricMobilityModel
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);

    /// constructor
    GeoSGP4Mobility();
    /// destructor
    virtual ~GeoSGP4Mobility();

    /**
     * \brief Sets the TLE Line 1
     * \param tle1 TLE Line 1
     */
    void SetTleLine1(std::string tle1);

    /**
     * \brief Gets the TLE Line 1
     * \return TLE Line 1
     */
    std::string GetTleLine1() const;

    /**
     * \brief Sets the TLE Line 2
     * \param tle2 TLE Line 2
     */
    void SetTleLine2(std::string tle2);

    /**
     * \brief Gets the TLE Line 2
     * \return TLE Line 2
     */
    std::string GetTleLine2() const;

    /**
     * \brief Sets the TLE start time (offset from TLE epoch for simulation t=0)
     * \param startTime Start time string in ISO 8601 format
     */
    void SetTleStartTime(std::string startTime);

    /**
     * \brief Gets the TLE start time
     * \return Start time string
     */
    std::string GetTleStartTime() const;

    /**
     * \brief Get velocity
     * \return the current velocity
     */
    virtual Vector GetVelocity() const;

    /**
     * \brief Get geocentric velocity
     * \return the current geocentric velocity
     */
    virtual Vector GetGeocentricVelocity() const;

  private:
    /**
     * Current position
     */
    Vector3D m_position;

    /**
     * Time precision for positions
     */
    Time m_precision;

    std::string m_tleLine1;
    std::string m_tleLine2;
    std::unique_ptr<libsgp4::Tle> m_tle;
    std::unique_ptr<libsgp4::SGP4> m_sgp4;
    Time m_sgp4StartTime = Seconds(0);      ///< Offset from TLE epoch for simulation start
    std::string m_sgp4StartTimeString = ""; ///< Original ISO 8601 string
    bool m_sgp4InitAttempted = false;       ///< Flag to prevent repeated SGP4 init attempts

    EventId m_updateEvent = EventId(); ///< Event for periodic updates

    /**
     * \brief Implementation of DoGetPosition for the GeocentricMobilityModel interface
     * \param type the coordinate type to return
     * \return the current position in the specified coordinate system
     */
    Vector DoGetPosition(PositionType type) const override;

    /**
     * \brief Implementation of DoSetPosition for the GeocentricMobilityModel interface
     * \param position the position to set
     * \param type the coordinate type of the input position
     */
    void DoSetPosition(const Vector& position, PositionType type) override;

    /**
     * \return the current velocity.
     */
    Vector DoGetVelocity(void) const override;

    /**
     * \brief Get geographic velocity
     * \return the current geographic velocity.
     */
    virtual Vector DoGetGeocentricVelocity() const;

    /**
     * \brief Update the internal precision of the mobility model (and schedule next update)
     * \param precision the precision to set
     */
    void SetPrecision(Time precision);

    /**
     * \brief Update the internal position of the mobility model
     */
    void Update();

    /**
     * \brief Calculate the position using SGP4 at time t
     * \param t time
     * \return position at time t
     */
    Vector CalcSgp4Position(Time t) const;

    /**
     * \brief Calculate the velocity using SGP4 at time t
     * \param t time
     * \return velocity at time t
     */
    Vector CalcSgp4Velocity(Time t) const;
};

} // namespace ns3

#endif /* GEO_SGP4_MOBILITY_H */
