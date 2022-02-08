/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2022 The IoD_Sim Authors.
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
#include "parametric-speed-flight.h"

#include <cmath>
#include <iomanip>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_roots.h>
#include <ns3/log.h>
#include <ns3/simulator.h>

namespace ns3 {

class PrivFuncParams
{
public:
  PrivFuncParams (std::vector<double> coeffs,
                  std::vector<double> coeffsDeriv);
  PrivFuncParams (std::vector<double> coeffs);
  ~PrivFuncParams ();

  std::vector<double> GetCoeffs ();
  std::vector<double> GetCoeffsDeriv ();

private:
  std::vector<double> m_coeffs;
  std::vector<double> m_coeffsDeriv;
};

NS_LOG_COMPONENT_DEFINE ("ParametricSpeedFlight");

ParametricSpeedFlight::ParametricSpeedFlight (FlightPlan flightPlan,
                                              ParametricSpeedParam speedParams,
                                              double step) :
  Curve (flightPlan, step),
  m_speedParams {speedParams.GetSpeedCoefficients ()},
  m_isHovering {flightPlan.GetN () == 1}
{
  NS_LOG_FUNCTION_NOARGS ();

  Generate ();
}

ParametricSpeedFlight::~ParametricSpeedFlight ()
{
}

void
ParametricSpeedFlight::Generate ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_isHovering)
    {
      // this is a dummy flight plan to hover on a specific point.
      m_length = 0;
      m_currentVelocity = Vector3D ();
      m_time = m_knots.GetBack ()->GetRestTime ();
    }
  else
    {
      m_length = Curve::Generate ();
      m_currentPositionPtr = m_curve.begin ();
      m_time = Seconds (FindTime ());

      NS_LOG_LOGIC ("Drone will take " << m_time << " to traverse the path.");
    }

    m_currentPosition = CurvePoint (m_knots.GetFront ()->GetPosition ());
}

void
ParametricSpeedFlight::Update (const double &t) const
{
  NS_LOG_FUNCTION (t);

  if (!m_isHovering)
    {
      UpdateSpeed (t);
      UpdateDistance (t);
      UpdatePosition ();
      UpdateVelocity ();

      NS_LOG_LOGIC ("t: " << t << " speed: " << m_currentSpeed
                    << " dist: " << m_currentDistance);
    }
}

Time
ParametricSpeedFlight::GetTime () const
{
  return m_time;
}

Vector
ParametricSpeedFlight::GetPosition () const
{
  return m_currentPosition.GetPosition ();
}

Vector
ParametricSpeedFlight::GetVelocity () const
{
  return m_currentVelocity;
}

void
ParametricSpeedFlight::UpdateSpeed (const double &t) const
{
  double speed = 0.0;
  const uint32_t order = m_speedParams.size ();

  for (uint32_t i = 0; i < order; i++)
    speed += m_speedParams[i] * std::pow (t, order - i - 1);

  m_currentSpeed = speed;
}

void
ParametricSpeedFlight::UpdateDistance (const double &t) const
{
  // analytic polynomial integration
  double distance = 0.0;
  const uint32_t orderp1 = m_speedParams.size (); // order of the polynomial, plus 1

  for (uint32_t i = 0; i < orderp1; i++)
      distance += m_speedParams[i] / (i + 1) * std::pow (t, i + 1);

  m_currentDistance = distance;
}

void
ParametricSpeedFlight::UpdatePosition () const
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC ("old pos vector: " << m_currentPosition.GetPosition ());
  m_pastPosition = m_currentPosition;

  for (auto i = m_curve.begin (); i != m_curve.end (); i++)
    {
      if ((*i).GetAbsoluteDistance () > m_currentDistance)
        {
          m_currentPosition = (*i);
          m_currentPositionPtr = i;
          break;
        }
    }

  NS_LOG_LOGIC ("new pos vector: " << m_currentPosition.GetPosition ());
}

void
ParametricSpeedFlight::UpdateVelocity () const
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC ("old vel: " << m_currentVelocity);

  const Vector relativeDistance = m_currentPosition.GetRelativeDistanceVector (m_pastPosition);
  const double relativeDistScalar = m_currentPosition.GetRelativeDistance (m_pastPosition);

  if (relativeDistScalar == 0.0)
    return;

  const double velX = (relativeDistance.x != 0)
                      ? (relativeDistance.x / relativeDistScalar) * m_currentSpeed
                      : 0;
  const double velY = (relativeDistance.y != 0)
                      ? (relativeDistance.y / relativeDistScalar) * m_currentSpeed
                      : 0;
  const double velZ = (relativeDistance.z != 0)
                      ? (relativeDistance.z / relativeDistScalar) * m_currentSpeed
                      : 0;

  NS_LOG_LOGIC ("relative distance: " << relativeDistance
                << "; current speed: " << m_currentSpeed);

  m_currentVelocity = {velX, velY, velZ};

  NS_LOG_LOGIC ("new vel: " << m_currentVelocity);
}

double
Quadratic (double x, void *params)
{
  std::vector<double> coeffs = ((PrivFuncParams *) params)->GetCoeffs ();
  uint32_t order = coeffs.size () - 1;
  double y = 0;

  for (uint32_t i = 0; i < coeffs.size (); i++)
    y += coeffs[i] * std::pow (x, order - i);

  return y;
}

double
QuadraticDeriv (double x, void *params)
{
  std::vector<double> coeffs = ((PrivFuncParams *) params)->GetCoeffsDeriv ();
  uint32_t order = coeffs.size () - 1;
  double y = 0;

  for (uint32_t i = 0; i < coeffs.size (); i++)
    y += coeffs[i] * std::pow (x, order - i);

  return y;
}

void
QuadraticFdf (double x, void *params, double *y, double *dy)
{
  *y = Quadratic (x, params);
  *dy = QuadraticDeriv (x, params);
}

const double
ParametricSpeedFlight::FindTime () const
{
  NS_LOG_FUNCTION_NOARGS ();

  // skip a rather heavy elaboration if length is zero
  if (m_length == 0.0)
    return 0.0;

  uint32_t order = m_speedParams.size ();

  // analytic polynomial integration
  std::vector<double> coeffs = m_speedParams;
  for (uint32_t i = 0; i < order; i++)
    {
      coeffs[i] /= order - i;
    }
  coeffs.push_back (-m_length);

  NS_LOG_LOGIC ("Length: " << m_length);

  /* setup GSL */
  int status;
  int iter = 0, max_iter = 1000;
  const gsl_root_fdfsolver_type *T;
  gsl_root_fdfsolver *s;
  double x0, x = m_length; // x will be our root
  gsl_function_fdf FDF;
  PrivFuncParams params (coeffs, m_speedParams);

  FDF.f = &Quadratic;
  FDF.df = &QuadraticDeriv;
  FDF.fdf = &QuadraticFdf;
  FDF.params = &params;

  T = gsl_root_fdfsolver_newton;
  s = gsl_root_fdfsolver_alloc (T);
  gsl_root_fdfsolver_set (s, &FDF, x);

  NS_LOG_LOGIC ("Using " << gsl_root_fdfsolver_name (s) << " method.");
  NS_LOG_LOGIC ("iter\t root");

  do
    {
      iter++;
      status = gsl_root_fdfsolver_iterate (s);
      if (status != GSL_SUCCESS)
        NS_LOG_ERROR ("GSL Encountered and error when applying the Newton-Raphson method. Error code: " << status);

      x0 = x;
      x = gsl_root_fdfsolver_root (s);
      status = gsl_root_test_delta (x, x0, 0, 1e-3);

      if (status == GSL_SUCCESS)
        NS_LOG_LOGIC ("Converged:");

      NS_LOG_LOGIC (std::fixed << std::setw (4) << iter << "\t"
                    << std::fixed << std::setw (10) << std::setprecision (7) << x);
    }
  while (status == GSL_CONTINUE && iter < max_iter);

  gsl_root_fdfsolver_free (s);

  return std::abs(x);
}

PrivFuncParams::PrivFuncParams (std::vector<double> coeffs,
                                std::vector<double> coeffsDeriv) :
  m_coeffs {coeffs},
  m_coeffsDeriv {coeffsDeriv}
{
}


PrivFuncParams::PrivFuncParams (std::vector<double> coeffs) :
  m_coeffs {coeffs},
  m_coeffsDeriv {}
{
}

PrivFuncParams::~PrivFuncParams ()
{
}

std::vector<double>
PrivFuncParams::GetCoeffs ()
{
  return m_coeffs;
}

std::vector<double>
PrivFuncParams::GetCoeffsDeriv ()
{
  return m_coeffsDeriv;
}

} // namespace ns3
