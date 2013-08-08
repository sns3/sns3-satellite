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

#include "geo-coordinate.h"


NS_LOG_COMPONENT_DEFINE ("geo-coordinate");

namespace ns3 {

ATTRIBUTE_HELPER_CPP (GeoCoordinate);

GeoCoordinate::GeoCoordinate (double longitude, double latitude, double altitude)
{
  NS_LOG_FUNCTION (this << longitude << latitude << altitude);

  NS_ASSERT(IsValidLongtitude(longitude));
  NS_ASSERT(IsValidLatitude(latitude));
  NS_ASSERT(IsValidAltitude(altitude));

  m_longitude = longitude;
  m_latitude = latitude;
  m_altitude = altitude;
}

GeoCoordinate::GeoCoordinate (Vector vector)
{
  NS_LOG_FUNCTION (this << vector);

  FromVector(vector);
}

GeoCoordinate::GeoCoordinate ()
  : m_longitude (NAN),
    m_latitude (NAN),
    m_altitude (NAN)
{
  NS_LOG_FUNCTION (this);
}

Vector GeoCoordinate::ToVector()
{
  NS_LOG_FUNCTION (this);

  Vector cartesian;

  cartesian.x = (N(DegToRad(m_latitude))+ m_altitude) * std::cos (DegToRad(m_latitude)) * std::cos (DegToRad(m_longitude));
  cartesian.y = (N(DegToRad(m_latitude))+ m_altitude) * std::cos (DegToRad(m_latitude)) * std::sin (DegToRad(m_longitude));
  cartesian.z = (N(DegToRad(m_latitude))*(1 - e2Param) + m_altitude) * std::sin (DegToRad(m_latitude));

  return cartesian;
}

double GeoCoordinate::GetLongitude() const
{
  NS_LOG_FUNCTION (this);
  return m_longitude;
}

double GeoCoordinate::GetLatitude() const
{
  NS_LOG_FUNCTION (this);
  return m_latitude;
}

double GeoCoordinate::GetAltitude() const
{
  NS_LOG_FUNCTION (this);
  return m_altitude;
}

void GeoCoordinate::SetLongitude(double longitude)
{
  NS_LOG_FUNCTION (this);
  m_longitude = longitude;
}

void GeoCoordinate::SetLatitude(double latitude)
{
  NS_LOG_FUNCTION (this);
  m_latitude = latitude;
}

void GeoCoordinate::SetAltitude(double altitude)
{
  NS_LOG_FUNCTION (this);
  m_altitude = altitude;
}

void GeoCoordinate::FromVector(const Vector &v)
{
  NS_LOG_FUNCTION (this << v);

  // distance from the position point (P) to earth center point (origin O)
  double op = std::sqrt( v.x * v.x + v.y * v.y + v.z * v.z );

  if ( op > 0 )
    {
      // longitude calculation
      double lon = std::atan(v.y/v.x);

      // scale longitude between - PI and PI (-180 and 180 in degrees)
      if ( v.x != 0 || v.y != 0 )
        {
          m_longitude = RadToDeg(std::atan(v.y/v.x ));

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
      double latG = std::atan(v.z/(std::sqrt( v.x * v.x + v.y * v.y )));

      // Geodetic latitude (of point Q, Q is intersection point of segment OP and reference ellipsoid)
      double latQ = std::atan(v.z/( (1 - e2Param ) * (std::sqrt( v.x * v.x + v.y * v.y ))) );

      // calculate N
      double n = N(latQ);

      // x, y, z of point Q
      double xQ = n * std::cos(latQ) * std::cos(lon);
      double yQ = n * std::cos(latQ) * std::sin(lon);
      double zQ = n * (1 - e2Param) * std::sin(latQ);

      // distance OQ
      double oq = std::sqrt( xQ * xQ + yQ * yQ + zQ * zQ );

      // distance PQ is OP - OQ
      double pq = op - oq;

      // length of the normal segment from point P of line (PO) to point T.
      // T is intersection point of linen the PO normal and ellipsoid normal from point Q.
      double tp = pq * std::sin(latG - latQ);

      m_latitude = RadToDeg(latQ + tp/op * std::cos(latQ - latG));

      m_altitude = pq * std::cos(latQ - latG);

    }

}

std::ostream &operator << (std::ostream &os, const GeoCoordinate &coordinate)
{
  double longitude = coordinate.GetLongitude();
  double latitude = coordinate.GetLatitude();
  double altitude = coordinate.GetAltitude();

  os << longitude << ":" << latitude << ":" << altitude;

  return os;
}

std::istream &operator >> (std::istream &is, GeoCoordinate &coordinate)
{
  double longitude;
  double latitude;
  double altitude;
  char c1;
  char c2;

  is >> longitude >> c1 >> latitude >> c2 >> altitude;

  coordinate.SetLongitude(longitude);
  coordinate.SetLatitude(latitude);
  coordinate.SetAltitude(altitude);

  if (c1 != ':' || c2 != ':')
    {
      is.setstate (std::ios_base::failbit);
    }
  return is;
}

} // namespace ns3
