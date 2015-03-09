/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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
 *
 * Author: Sami Rantanen <sami.rantanen@magister.fi>
 */

#include <cmath>
#include <sstream>

#include "ns3/fatal-error.h"
#include "ns3/log.h"

#include "satellite-utils.h"
#include "geo-coordinate.h"


NS_LOG_COMPONENT_DEFINE ("geo-coordinate");

namespace ns3 {

ATTRIBUTE_HELPER_CPP (GeoCoordinate);

GeoCoordinate::GeoCoordinate (double latitude, double longitude, double altitude, ReferenceEllipsoid_t refEllipsoid)
  : m_refEllipsoid (refEllipsoid)
{
  NS_LOG_FUNCTION (this << latitude << longitude << altitude);

  Construct (latitude, longitude, altitude);
}

GeoCoordinate::GeoCoordinate (double latitude, double longitude, double altitude)
  : m_refEllipsoid (GeoCoordinate::SPHERE)
{
  NS_LOG_FUNCTION (this << latitude << longitude << altitude);

  Construct (latitude, longitude, altitude);
}

GeoCoordinate::GeoCoordinate (Vector vector)
  : m_refEllipsoid (GeoCoordinate::SPHERE)
{
  NS_LOG_FUNCTION (this << vector);

  ConstructFromVector (vector);
}

GeoCoordinate::GeoCoordinate (Vector vector, ReferenceEllipsoid_t refEllipsoid)
  : m_refEllipsoid (refEllipsoid)
{
  NS_LOG_FUNCTION (this << vector);

  ConstructFromVector (vector);
}

GeoCoordinate::GeoCoordinate ()
  : m_latitude (NAN),
    m_longitude (NAN),
    m_altitude (NAN),
    m_refEllipsoid (GeoCoordinate::SPHERE)
{
  NS_LOG_FUNCTION (this);

  Initialize ();
}

void
GeoCoordinate::Construct (double latitude, double longitude, double altitude)
{
  NS_LOG_FUNCTION (this << latitude << longitude << altitude);

  if ( IsValidLatitude (latitude) == false)
    {
      NS_FATAL_ERROR ("Invalid latitude!!!");
    }

  if (IsValidLongtitude (longitude) == false)
    {
      NS_FATAL_ERROR ("Invalid longitude!!!");
    }

  if (IsValidAltitude (altitude, m_refEllipsoid) == false)
    {
      NS_FATAL_ERROR ("Invalid altitude!!!");
    }

  Initialize ();

  m_latitude = latitude;
  m_longitude = longitude;
  m_altitude = altitude;
}

Vector GeoCoordinate::ToVector ()
{
  NS_LOG_FUNCTION (this);

  Vector cartesian;
  double latRads = SatUtils::DegreesToRadians (m_latitude);
  double lonRads = SatUtils::DegreesToRadians (m_longitude);

  cartesian.x = ( GetRadiusCurvature (latRads) + m_altitude) * std::cos (latRads) * std::cos (lonRads);
  cartesian.y = ( GetRadiusCurvature (latRads) + m_altitude) * std::cos (latRads) * std::sin (lonRads);
  cartesian.z = ( GetRadiusCurvature (latRads) * (1 - m_e2Param) + m_altitude) * std::sin (latRads);

  return cartesian;
}

void
GeoCoordinate::Initialize ()
{
  NS_LOG_FUNCTION (this);

  switch ( m_refEllipsoid )
    {
    case SPHERE:
      m_polarRadius = GeoCoordinate::polarRadius_sphere;
      break;

    case WGS84:
      m_polarRadius = GeoCoordinate::polarRadius_wgs84;
      break;

    case GRS80:
      m_polarRadius = GeoCoordinate::polarRadius_grs80;
      break;

    default:
      NS_FATAL_ERROR ("Invalid Reference Ellipsoid!!!");
      break;
    }

  m_equatorRadius = GeoCoordinate::equatorRadius;
  m_e2Param = ( ( m_equatorRadius * m_equatorRadius ) - ( m_polarRadius * m_polarRadius ) ) / (m_equatorRadius * m_equatorRadius );
}

double GeoCoordinate::GetLongitude () const
{
  NS_LOG_FUNCTION (this);
  return m_longitude;
}

double GeoCoordinate::GetLatitude () const
{
  NS_LOG_FUNCTION (this);
  return m_latitude;
}

double GeoCoordinate::GetAltitude () const
{
  NS_LOG_FUNCTION (this);
  return m_altitude;
}

void GeoCoordinate::SetLongitude (double longitude)
{
  NS_LOG_FUNCTION (this << longitude);

  if ( IsValidLatitude (longitude) == false)
    {
      NS_FATAL_ERROR ("Invalid longitude!!!");
    }

  m_longitude = longitude;
}

void GeoCoordinate::SetLatitude (double latitude)
{
  NS_LOG_FUNCTION (this << latitude);

  if ( IsValidLongtitude (latitude) == false)
    {
      NS_FATAL_ERROR ("Invalid latitude!!!");
    }

  m_latitude = latitude;
}

void GeoCoordinate::SetAltitude (double altitude)
{
  NS_LOG_FUNCTION (this << altitude);

  if ( IsValidAltitude (altitude, m_refEllipsoid) == false)
    {
      NS_FATAL_ERROR ("Invalid altitude!!!");
    }

  m_altitude = altitude;
}

void GeoCoordinate::ConstructFromVector (const Vector &v)
{
  NS_LOG_FUNCTION (this << v);

  Initialize ();

  // distance from the position point (P) to earth center point (origin O)
  double op = std::sqrt ( v.x * v.x + v.y * v.y + v.z * v.z );

  if ( op > 0 )
    {
      // longitude calculation
      double lon = std::atan (v.y / v.x);

      // scale longitude between - PI and PI (-180 and 180 in degrees)
      if ( v.x != 0 || v.y != 0 )
        {
          m_longitude = SatUtils::RadiansToDegrees (std::atan (v.y / v.x ));

          if ( v.x < 0 )
            {
              if ( v.y > 0)
                {
                  m_longitude = 180 + m_longitude;
                  lon = lon - M_PI;
                }
              else
                {
                  m_longitude = -180 + m_longitude;
                  lon = M_PI + lon;
                }
            }
        }

      // Geocentric latitude
      double latG = std::atan (v.z / (std::sqrt ( v.x * v.x + v.y * v.y )));

      // Geocentric latitude (of point Q, Q is intersection point of segment OP and reference ellipsoid)
      double latQ = std::atan (v.z / ( (1 - m_e2Param ) * (std::sqrt ( v.x * v.x + v.y * v.y ))) );

      // calculate radius of the curvature
      double rCurvature = GetRadiusCurvature (latQ);

      // x, y, z of point Q
      double xQ = rCurvature * std::cos (latQ) * std::cos (lon);
      double yQ = rCurvature * std::cos (latQ) * std::sin (lon);
      double zQ = rCurvature * (1 - m_e2Param) * std::sin (latQ);

      // distance OQ
      double oq = std::sqrt ( xQ * xQ + yQ * yQ + zQ * zQ );

      // distance PQ is OP - OQ
      double pq = op - oq;

      // length of the normal segment from point P of line (PO) to point T.
      // T is intersection point of linen the PO normal and ellipsoid normal from point Q.
      double tp = pq * std::sin (latG - latQ);

      m_latitude = SatUtils::RadiansToDegrees (latQ + tp / op * std::cos (latQ - latG));

      m_altitude = pq * std::cos (latQ - latG);
    }
}
double
GeoCoordinate::GetRadiusCurvature (double latitude)
{
  return ( m_equatorRadius / std::sqrt (1 - m_e2Param * std::sin (latitude) * std::sin (latitude)) );
}

bool
GeoCoordinate::IsValidAltitude (double altitude, ReferenceEllipsoid_t refEllipsoid)
{
  double polarRadius = NAN;

  switch ( refEllipsoid )
    {
    case SPHERE:
      polarRadius = GeoCoordinate::polarRadius_sphere;
      break;

    case WGS84:
      polarRadius = GeoCoordinate::polarRadius_wgs84;
      break;

    case GRS80:
      polarRadius = GeoCoordinate::polarRadius_grs80;
      break;

    default:
      NS_FATAL_ERROR ("Invalid polar radius value!!!");
      break;
    }

  return ( (polarRadius + altitude) >= 0 );
}

std::ostream &operator << (std::ostream &os, const GeoCoordinate &coordinate)
{
  double longitude = coordinate.GetLongitude ();
  double latitude = coordinate.GetLatitude ();
  double altitude = coordinate.GetAltitude ();

  os << latitude << "," << longitude << "," << altitude;

  return os;
}

std::istream &operator >> (std::istream &is, GeoCoordinate &coordinate)
{
  double longitude;
  double latitude;
  double altitude;
  char c1;
  char c2;

  is >> latitude >> c1 >> longitude >> c2 >> altitude;

  coordinate.SetLongitude (longitude);
  coordinate.SetLatitude (latitude);
  coordinate.SetAltitude (altitude);

  if (c1 != ',' || c2 != ',')
    {
      is.setstate (std::ios_base::failbit);
    }
  return is;
}

} // namespace ns3
