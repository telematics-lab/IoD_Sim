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

#include "geo-sgp4-mobility.h"

#include <libsgp4/DateTime.h>
#include <libsgp4/SGP4.h>
#include <libsgp4/Tle.h>
#include "leo-orbit.h"

#include "ns3/geographic-positions.h"
#include "ns3/simulator.h"
#include "ns3/string.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("GeoSGP4Mobility");

NS_OBJECT_ENSURE_REGISTERED(GeoSGP4Mobility);

TypeId
GeoSGP4Mobility::GetTypeId()
{
    return TypeId("ns3::GeoSGP4Mobility")
        .SetParent<GeocentricMobilityModel>()
        .SetGroupName("Leo")
        .AddConstructor<GeoSGP4Mobility>()
        .AddAttribute("Precision",
                      "The time precision with which to compute position updates. 0 means "
                      "arbitrary precision",
                      TimeValue(Seconds(1)),
                      MakeTimeAccessor(&GeoSGP4Mobility::SetPrecision),
                      MakeTimeChecker())
        .AddAttribute(
            "TleLine1",
            "TLE Line 1",
            StringValue(""),
            MakeStringAccessor(&GeoSGP4Mobility::SetTleLine1, &GeoSGP4Mobility::GetTleLine1),
            MakeStringChecker())
        .AddAttribute(
            "TleLine2",
            "TLE Line 2",
            StringValue(""),
            MakeStringAccessor(&GeoSGP4Mobility::SetTleLine2, &GeoSGP4Mobility::GetTleLine2),
            MakeStringChecker())
        .AddAttribute("TleStartTime",
                      "The start time for TLE propagation (format: ISO 8601)",
                      StringValue(""),
                      MakeStringAccessor(&GeoSGP4Mobility::SetTleStartTime,
                                         &GeoSGP4Mobility::GetTleStartTime),
                      MakeStringChecker());
}


GeoSGP4Mobility::GeoSGP4Mobility()
    : GeocentricMobilityModel()
{
    NS_LOG_FUNCTION_NOARGS();
}

GeoSGP4Mobility::GeoSGP4Mobility(const GeoSGP4Mobility& other)
    : GeocentricMobilityModel(other),
      m_position(other.m_position),
      m_precision(other.m_precision),
      m_tleLine1(other.m_tleLine1),
      m_tleLine2(other.m_tleLine2),
      m_sgp4StartTime(other.m_sgp4StartTime),
      m_sgp4StartTimeString(other.m_sgp4StartTimeString),
      m_sgp4InitAttempted(false) // Reset checking
{
    NS_LOG_FUNCTION(this << &other);
    // If SGP4 was initialized in the other object, we should try to initialize it here too
    if (other.m_sgp4)
    {
        Update();
    }
}

GeoSGP4Mobility::~GeoSGP4Mobility()
{
}

Ptr<MobilityModel>
GeoSGP4Mobility::Copy() const
{
    return CreateObject<GeoSGP4Mobility>(*this);
}

void
GeoSGP4Mobility::SetPrecision(Time precision)
{
    NS_LOG_FUNCTION(this << precision);
    m_precision = precision;
    Update();
}

void
GeoSGP4Mobility::SetTleLine1(std::string tle1)
{
    NS_LOG_FUNCTION(this << tle1);
    m_tleLine1 = tle1;
    NS_LOG_INFO("TLE Line 1 set to: '" << m_tleLine1 << "'");
    Update();
}

std::string
GeoSGP4Mobility::GetTleLine1() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_tleLine1;
}

void
GeoSGP4Mobility::SetTleLine2(std::string tle2)
{
    NS_LOG_FUNCTION(this << tle2);
    m_tleLine2 = tle2;
    NS_LOG_INFO("TLE Line 2 set to: '" << m_tleLine2 << "'");
    Update();
}

std::string
GeoSGP4Mobility::GetTleLine2() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_tleLine2;
}

void
GeoSGP4Mobility::SetTleStartTime(std::string startTime)
{
    NS_LOG_FUNCTION(this << startTime);
    if (startTime.empty())
    {
        m_sgp4StartTime = Seconds(0);
        m_sgp4StartTimeString = "";
        return;
    }

    // Parse ISO 8601 format: YYYY-MM-DD HH:MM:SS or YYYY-MM-DDTHH:MM:SS or YYYY-MM-DDTHH:MM:SSZ
    int year, month, day, hour, minute, second;
    int parsed = sscanf(startTime.c_str(),
                        "%d-%d-%d %d:%d:%d",
                        &year,
                        &month,
                        &day,
                        &hour,
                        &minute,
                        &second);

    if (parsed != 6)
    {
        // Try with 'T' separator
        parsed = sscanf(startTime.c_str(),
                        "%d-%d-%dT%d:%d:%d",
                        &year,
                        &month,
                        &day,
                        &hour,
                        &minute,
                        &second);
    }

    if (parsed == 6)
    {
        // Create DateTime for the start time
        libsgp4::DateTime startDt(year, month, day, hour, minute, second);

        // Calculate offset from TLE epoch (we need TLE to be loaded first)
        if (m_tle)
        {
            libsgp4::DateTime tleEpoch = m_tle->Epoch();
            double offsetSeconds = (startDt - tleEpoch).TotalSeconds();
            m_sgp4StartTime = Seconds(offsetSeconds);
            m_sgp4StartTimeString = startTime;
            NS_LOG_INFO("SGP4 start time set: " << offsetSeconds << "s from TLE epoch");
        }
        else
        {
            // TLE not loaded yet, just save the string and calculate offset later
            m_sgp4StartTimeString = startTime;
            NS_LOG_WARN("TLE not loaded yet, will calculate offset when TLE is set");
        }
    }
    else
    {
        NS_LOG_WARN("Invalid Sgp4StartTime format: '"
                    << startTime << "'. Expected ISO 8601 (YYYY-MM-DD HH:MM:SS). Using TLE epoch.");
        m_sgp4StartTime = Seconds(0);
        m_sgp4StartTimeString = "";
    }
    Update();
}

std::string
GeoSGP4Mobility::GetTleStartTime() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_sgp4StartTimeString;
}

void
GeoSGP4Mobility::Update()
{
    // Only try to initialize SGP4 once to avoid repeated errors
    if (!m_sgp4 && !m_tleLine1.empty() && !m_tleLine2.empty() && !m_sgp4InitAttempted)
    {
        m_sgp4InitAttempted = true;
        try
        {
            NS_LOG_INFO("Attempting SGP4 initialization...");
            NS_LOG_INFO("  TLE Line 1: '" << m_tleLine1 << "'");
            NS_LOG_INFO("  TLE Line 2: '" << m_tleLine2 << "'");
            m_tle = std::make_unique<libsgp4::Tle>(m_tleLine1, m_tleLine2);
            m_sgp4 = std::make_unique<libsgp4::SGP4>(*m_tle);
            NS_LOG_INFO("SGP4 initialized successfully for satellite");

            // Recalculate start time offset if string is present (now that TLE is loaded)
            if (!m_sgp4StartTimeString.empty())
            {
                SetTleStartTime(m_sgp4StartTimeString);
            }
        }
        catch (const std::exception& e)
        {
            NS_LOG_ERROR("Failed to initialize SGP4: " << e.what());
            NS_LOG_ERROR("  TLE Line 1 length: " << m_tleLine1.length());
            NS_LOG_ERROR("  TLE Line 2 length: " << m_tleLine2.length());
            m_sgp4.reset();
            m_tle.reset();
        }
    }

    if (m_sgp4)
    {
        try
        {
            m_position = CalcSgp4Position(Simulator::Now() + m_sgp4StartTime);
            NS_LOG_INFO("Updated SGP4 position at t=" << Simulator::Now().GetSeconds() << "s");
        }
        catch (const std::exception& e)
        {
            NS_LOG_ERROR("Error updating SGP4 position: " << e.what());
        }
    }

    NotifyCourseChange();
    if (m_updateEvent.IsPending())
    {
        Simulator::Cancel(m_updateEvent);
        m_updateEvent = EventId(); // Reset the event
    }
    if (m_precision > Seconds(0))
    {
        m_updateEvent = Simulator::Schedule(m_precision, &GeoSGP4Mobility::Update, this);
    }
}

Vector
GeoSGP4Mobility::DoGetPosition(PositionType type) const
{
    Vector geocentricPos;
    if (m_precision == Time(0))
    {
        // Notice: NotifyCourseChange () will not be called
        if (m_sgp4)
        {
            geocentricPos = CalcSgp4Position(Simulator::Now() + m_sgp4StartTime);
        }
        else
        {
            NS_LOG_ERROR("SGP4 not initialized");
            return Vector();
        }
    }
    else
    {
        geocentricPos = m_position;
    }

    switch (type)
    {
    case PositionType::GEOCENTRIC:
        return geocentricPos;
    case PositionType::GEOGRAPHIC:
        return GeographicPositions::CartesianToGeographicCoordinates(geocentricPos,
                                                                     GetEarthSpheroidType());
    case PositionType::TOPOCENTRIC:
        return CartesianToTopocentric(geocentricPos,
                                      GetGeographicReferencePoint(),
                                      GetEarthSpheroidType());
    case PositionType::PROJECTED:
        return GeographicPositions::GeographicToProjectedCoordinates(
            GeographicPositions::CartesianToGeographicCoordinates(geocentricPos,
                                                                  GetEarthSpheroidType()),
            GetEarthSpheroidType());

    default:
        NS_FATAL_ERROR("Unknown/unsupported position type");
        return Vector();
    }
}

void
GeoSGP4Mobility::DoSetPosition(const Vector& position, PositionType type)
{
    // Setting position in a classical way has no sense in this model
    NS_LOG_WARN("Setting position in a classical way has no sense in GeoSGP4Mobility");
}

Vector
GeoSGP4Mobility::CalcSgp4Position(Time t) const
{
    if (!m_sgp4)
    {
        NS_LOG_ERROR("SGP4 not initialized");
        return Vector();
    }

    libsgp4::DateTime current = m_tle->Epoch().AddSeconds(t.GetSeconds());

    libsgp4::Eci eci = m_sgp4->FindPosition(current);
    libsgp4::CoordGeodetic geo = eci.ToGeodetic();
    return GeographicPositions::GeographicToCartesianCoordinates(
        libsgp4::Util::RadiansToDegrees(geo.latitude),
        libsgp4::Util::RadiansToDegrees(geo.longitude),
        geo.altitude * 1000.0,
        GetEarthSpheroidType());
}

Vector
GeoSGP4Mobility::CalcSgp4Velocity(Time t) const
{
    Time dt = Seconds(0.01);
    Vector p1 = CalcSgp4Position(t);
    Vector p2 = CalcSgp4Position(t + dt);
    return (p2 - p1) * (1.0 / dt.GetSeconds());
}

Vector
GeoSGP4Mobility::GetVelocity() const
{
    if (m_sgp4)
    {
        return CalcSgp4Velocity(Simulator::Now() + m_sgp4StartTime);
    }
    return Vector();
}

Vector
GeoSGP4Mobility::GetGeocentricVelocity() const
{
    return GetVelocity();
}

Vector
GeoSGP4Mobility::DoGetVelocity() const
{
    return GetVelocity();
}

Vector
GeoSGP4Mobility::DoGetGeocentricVelocity() const
{
    return GetGeocentricVelocity();
}

} // namespace ns3
