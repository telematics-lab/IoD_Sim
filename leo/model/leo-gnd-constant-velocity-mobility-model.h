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

#ifndef LEO_GROUND_MOBILITY_MODEL_H
#define LEO_GROUND_MOBILITY_MODEL_H

#include "ns3/vector.h"
#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/geocentric-constant-position-mobility-model.h"
#include "ns3/nstime.h"

/**
 * \file
 * \ingroup leo
 *
 * Declaration of GndConstantVelocityMobilityModel
 */

namespace ns3 {

/**
 * \ingroup leo
 * \brief Keep track of the position of a ground node with constant velocity.
 *
 * This uses simple circular orbits based on the first position of the node
 * and the direction of the velocity vector.
 * The node will move with constant speed in the direction of the velocity vector
 * around the Earth.
 */
class GndConstantVelocityMobilityModel : public GeocentricConstantPositionMobilityModel
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  
  /// Default constructor  
  GndConstantVelocityMobilityModel ();
  /// destructor
  virtual ~GndConstantVelocityMobilityModel () = default;
 
    /**
     * @brief Get the position using geographic (geodetic) coordinates
     * @return Vector containing (latitude (degree), longitude (degree), altitude (meter))
     */
    virtual Vector GetGeographicPosition() const override;
 
    /**
     * @brief Set the position using geographic coordinates
     *
     * Sets the position, using geographic coordinates and asserting
     * that the provided parameter falls within the appropriate range.
     *
     * @param latLonAlt pointer to a Vector containing (latitude (degree), longitude (degree),
     * altitude (meter)). The values are expected to be in the ranges [-90, 90], [-180, 180], [0,
     * +inf[, respectively. These assumptions are enforced with an assert for the latitude and the
     * altitude, while the longitude is normalized to the expected range.
     */
    virtual void SetGeographicPosition(const Vector& latLonAlt) override;
 
    /**
     * @brief Get the position using Geocentric Cartesian coordinates
     * @return Vector containing (x, y, z) (meter) coordinates
     */
    virtual Vector GetGeocentricPosition() const override;
 
    /**
     * @brief Set the position using Geocentric Cartesian coordinates
     * @param position pointer to a Vector containing (x, y, z) (meter) coordinates
     */
    virtual void SetGeocentricPosition(const Vector& position) override;
 
    /**
     * @brief Set the reference point for coordinate conversion
     * @param refPoint vector containing the geographic reference point (meter)
     */
    virtual void SetCoordinateTranslationReferencePoint(const Vector& refPoint) override;
 
    /**
     * @brief Get the reference point for coordinate conversion
     * @return Vector containing geographic reference point (meter)
     */
    virtual Vector GetCoordinateTranslationReferencePoint() const override;
 
    /**
     * @return the current position
     */
    virtual Vector GetPosition() const override;

      /**
       * \brief Get velocity
       * \return the current velocity
       */
      virtual Vector GetVelocity () const;
      /**
       * \brief Get geocentric velocity
       * \return the current geocentric velocity
       */
      virtual Vector GetGeocentricVelocity () const;
 
    /**
     * @param position the position to set.
     */
    virtual void SetPosition(const Vector& position) override;

private:

  /**
   * Initial latitude in degrees
   */
  double m_initialLatitude;

  /**
   * Initial longitude in degrees
   */
  double m_initialLongitude;

  /** 
   * The altitude of the node in meters above the Earth's surface.
   */
  double m_altitude;

  /**
   * The direction of the velocity vector
   */
  double m_azimuth;

  /**
   * The velocity of the node in m/s
   */
   double m_speed;

  /**
   * Current position
   */
  Vector m_position;

  EventId m_updateEvent = EventId(); ///< Event for periodic updates

  /**
   * Time precision for positions
   */
  Time m_precision;

  Time lastPositionUpdate; ///< Last time the position was updated

  // Override methods from GeocentricConstantPositionMobilityModel
  virtual Vector DoGetPosition() const override;
  virtual void DoSetPosition(const Vector& position) override;
  virtual Vector DoGetGeographicPosition() const override;
  virtual void DoSetGeographicPosition(const Vector& latLonAlt) override;
  virtual Vector DoGetGeocentricPosition() const override;
  virtual void DoSetGeocentricPosition(const Vector& position) override;

  /**
   * \return the current velocity.
   */
  virtual Vector DoGetVelocity (void) const override;
  /**
   * \brief Get geographic velocity
   * \return the current geographic velocity.
   */
  virtual Vector DoGetGeocentricVelocity() const;

  /**
   * \brief Calculate the position at time t
   * \param t time
   * \return position at time t
   */
  Vector CalcPosition (Time t) const;

  /**
   * \brief Update the internal precision of the mobility model (and schedule next update)
   * \param precision the precision to set
   */
  void SetPrecision (Time precision);

  /**
   * \brief Get the azimuth of the velocity vector
   * \return azimuth in degrees
   */
  double GetAzimuth () const;

  /**
   * \brief Set the azimuth of the velocity vector
   * \param azimuth in degrees
   */
  void SetAzimuth (double azimuth);

    /**
   * \brief Get the velocity of the node
   * \return velocity in m/s
   */
  double GetSpeed () const;

  /**
   * \brief Set the velocity of the node
   * \param velocity in m/s
   */
  void SetSpeed(double velocity);

  /**
   * \brief Update the internal position of the mobility model
   * \return position that will be returned upon next call to DoGetPosition
   */
  Vector Update ();
};

}

#endif
