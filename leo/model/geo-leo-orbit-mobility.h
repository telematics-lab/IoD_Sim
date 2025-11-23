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
 *
 * Author: Tim Schubert <ns-3-leo@timschubert.net>
 */

#ifndef LEO_CIRCULAR_ORBIT_MOBILITY_MODEL_H
#define LEO_CIRCULAR_ORBIT_MOBILITY_MODEL_H

#include "ns3/geocentric-mobility-model.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/vector.h"

#include <memory>
#include <string>

/**
 * \file
 * \ingroup leo
 *
 * Declaration of GeoLeoOrbitMobility
 */

#define LEO_EARTH_GM_KM_E10 39.8600436

#define LEO_EARTH_GM_KM_E10 39.8600436

namespace libsgp4
{
class SGP4;
class Tle;
class DateTime;
} // namespace libsgp4

namespace ns3
{

/**
 * \ingroup leo
 * \brief Keep track of the orbital postion and velocity of a satellite.
 *
 * This uses simple circular orbits based on the inclination of the orbital
 * plane and the height of the satellite.
 */
class GeoLeoOrbitMobility : public GeocentricMobilityModel
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId(void);
    /// constructor
    GeoLeoOrbitMobility();
    /// destructor
    virtual ~GeoLeoOrbitMobility();

    /**
     * \brief Gets the speed of the node
     * \return the speed in m/s
     */
    double GetSpeed() const;

    /**
     * \brief Gets the altitude
     * \return the altitude
     */
    double GetAltitude() const;
    /**
     * \brief Sets the altitude
     * \param the altitude
     */
    void SetAltitude(double h);

    /**
     * \brief Gets the inclination
     * \return the inclination
     */
    double GetInclination() const;

    /**
     * \brief Sets the inclination
     * \param the inclination
     */
    void SetInclination(double incl);

    /**
     * \brief Sets the retrograde orbit flag
     * \param the retrograde orbit flag
     */
    void SetRetrogradeOrbit(bool retrograde);

    /**
     * \brief Gets the retrograde orbit flag
     * \return the retrograde orbit flag
     */
    bool GetRetrogradeOrbit() const;

    /**
     * \brief Sets the longitude offset
     * \param the longitude offset
     */
    void SetLongitudeOffset(double longitude);

    /**
     * \brief Gets the current longitude
     * \return the current longitude in deg
     */
    double GetLongitudeOffset() const;

    /**
     * \brief Sets the orbital offset
     * \param the orbital offset
     */
    void SetOffset(double offset);

    /**
     * \brief Gets the current offset
     * \return the current offset in deg
     */
    double GetOffset() const;

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
     * Orbit height from the center of the Earth in km
     */
    double m_orbitHeight;

    /**
     * Inclination in rad
     */
    double m_inclination;

    /**
     * Longitudinal offset in rad
     */
    double m_longitude;

    /**
     * Offset on the orbital plane in rad
     */
    double m_offset;

    /**
     * Retrograde orbit flag
     */
    bool m_retrogradeOrbit = false;

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
    Time m_tleStartTime = Seconds(0);      ///< Offset from TLE epoch for simulation start
    std::string m_tleStartTimeString = ""; ///< Original ISO 8601 string

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
    Vector DoGetVelocity() const override;

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
     * \brief Get the normal vector of the orbital plane
     */
    Vector3D PlaneNorm() const;

    /**
     * \brief Gets the distance the satellite has progressed from its original
     * position at time t in rad
     *
     * \param t a point in time
     * \return distance in rad
     */
    double GetProgress(Time t) const;

    /**
     * \brief Advances a satellite by a degrees inside the orbital plane
     * \param a angle by which to rotate
     * \param x vector to rotate
     * \return rotated vector
     */
    Vector3D RotatePlane(double a, const Vector3D& x) const;

    /**
     * \brief Calculate the position at time t
     * \param t time
     * \return position at time t
     */
    Vector CalcPosition(Time t) const;

    /**
     * \brief Calc the latitude depending on simulation time inside ITRF coordinate
     * system
     *
     * \return latitude
     */
    double CalcLatitude() const;

    /**
     * \brief Update the internal position of the mobility model
     */
    void Update();

    /**
     * \brief Initialize classical orbital parameters from TLE data
     * Uses SGP4 once to get position at start time, then extracts orbital parameters
     */
    void InitializeFromTLE();
};

} // namespace ns3

#endif
