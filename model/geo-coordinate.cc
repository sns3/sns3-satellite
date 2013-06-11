/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <cmath>
#include <sstream>

#include "ns3/fatal-error.h"
#include "ns3/log.h"

#include "geo-coordinate.h"


NS_LOG_COMPONENT_DEFINE ("geo-coordinate");

namespace ns3 {

GeoCoordinate::GeoCoordinate (double longitude, double latitude, double altitude)
{
  NS_LOG_FUNCTION (this << longitude << latitude << altitude);

  NS_ASSERT(IsValidLongtitude(longitude));
  NS_ASSERT(IsValidLongtitude(latitude));
  NS_ASSERT(IsValidLongtitude(altitude));

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

double GeoCoordinate::GetLongitude()
{
  NS_LOG_FUNCTION (this);
  return m_longitude;
}

double GeoCoordinate::GetLatitude()
{
  NS_LOG_FUNCTION (this);
  return m_latitude;
}

double GeoCoordinate::GetAltitude()
{
  NS_LOG_FUNCTION (this);
  return m_altitude;
}

void GeoCoordinate::FromVector(const Vector &v)
{
  NS_LOG_FUNCTION (this << v);

  double radial = std::sqrt( v.x * v.x + v.y * v.y + v.z * v.z );

  m_altitude = radial - EARTH_RADIUS;

  if ( radial > 0 )
    {
      m_latitude = RadToDeg(std::asin(v.z/radial));

      if (v.x != 0 || v.y != 0 )
        {
          m_longitude = RadToDeg(std::atan2(v.y , v.x ));
        }
    }
}

} // namespace ns3
