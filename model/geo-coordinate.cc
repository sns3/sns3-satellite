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
  : m_longitude (0.0),
    m_latitude (0.0),
    m_altitude (0.0)
{
  NS_LOG_FUNCTION (this);
}

Vector GeoCoordinate::ToVector()
{
  NS_LOG_FUNCTION (this);

  Vector cartesian;

  double radial = m_altitude + EARTH_RADIUS;
  double latitude = DegToRad(m_latitude);
  double longitude = DegToRad(m_longitude);

  cartesian.x = radial * std::cos (latitude) * std::cos (longitude);
  cartesian.y = radial * std::cos (latitude) * std::sin (longitude);
  cartesian.z = radial * std::sin (latitude);

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

  double radial = std::sqrt( v.x * v.x + v.y * v.y + v.z * v.z );

  m_altitude = radial - EARTH_RADIUS;

  NS_ASSERT(radial > 0);

  if ( radial > 0 )
    {
      m_latitude = RadToDeg(std::atan(v.z/(std::sqrt( v.x * v.x + v.y * v.y ))));

      if ( v.x != 0 || v.y != 0 )
        {
          m_longitude = RadToDeg(std::atan(v.y/v.x ));

          if ( v.x < 0 )
            {
              if ( v.y < 0)
                {
                  m_longitude = m_longitude - 180;
                }
              else
                {
                  m_longitude = 180 + m_longitude;
                }
            }
        }
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
