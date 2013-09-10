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
#ifndef GEO_COORDINATE_H
#define GEO_COORDINATE_H

#include <cmath>

#include "ns3/attribute.h"
#include "ns3/attribute-helper.h"
#include "ns3/vector.h"

namespace ns3 {

/**
 * \brief GeoCoordinate class is used to store and operate with geodetic coordinates.
 * Latitude is in the degree range (-90, 90) with negative values -> south
 * Longitude is in the degree range (-180, 180) with negative values -> west
 * Altitude is in meters.
 */
class GeoCoordinate
{
public:
  /**
   * \param latitude latitude of position
   * \param longitude longitude of position
   * \param altitute altitute of position
   *
   * Creates GeoCoordinate from given latitude, longitude and altitude decimal degree values
   *
   * Accepted values:
   *
   * Latitude:       -90 ... 90 degrees
   * Longitude:     -180 ... 180 degrees
   * Altitude:      >= -EARTH_RADIUS
   *
   */
  GeoCoordinate (double latitude, double longitude, double altitude);
  /**
   * \param vector, coordinates as Vector (x,y,z) (Cartesian)
   *
   * Creates GeoCoordinate from given vector (x,y,z)
   */
  GeoCoordinate (Vector vector);
  /**
   * Creates GeoCoordinate with zero values of longitude, latitude and altitude
   */
  GeoCoordinate ();
  /**
   * Gets longitude value of coordinate.
   *
   * \returns longitude
   */
  double GetLongitude() const;
  /**
     * Gets latitude value of coordinate.
     *
     * \returns latitude
     */
  double GetLatitude() const;
  /**
   * Gets altitude value of coordinate.
   *
   * \returns altitude
   */
  double GetAltitude() const;
  /**
   * Sets longitude value of coordinate.
   *
   * \param longitude longitude to set
   */
  void SetLongitude(double longitude);
  /**
   * Sets latitude value of coordinate.
   *
   * \param latitude longitude to set
   */
  void SetLatitude(double latitude);
  /**
   * Sets altitude value of coordinate.
   *
   * \param altitude longitude to set
   */
  void SetAltitude(double altitude);
  /**
   * Converts Geodetic coordinates to Cartesian coordinates
   * \returns Vector containing Cartesian coordinates
   */
  Vector ToVector ();
  /**
   * Used Earth Ellipsoid Parameters (WGS 84)
   *
   * Semi-major axis A, m:
   * Semi-minor axis B, m:
   */

  // First eccentricity squared
  static const double e2Param = 0.00669437999014;  // First eccentricity squared
  static const double aParam = 6378137;            // Semi-major axis A, m:
  static const double bParam = 6356752.3142;       // Semi-minor axis B, m:

private:
  /**
   * Gets the radius of curvature in the prime vertical (N)
   * \param latitude latitude in radians for to get N
   * \return value of the N (meters)
   */
  static inline double N(double latitude) {return ( aParam / std::sqrt(1 - e2Param * std::sin(latitude)*std::sin(latitude)));}
  /**
   * Checks if longtitude is in valid range
   *
   * \param longtitude to check
   * \return bool true longitude valid, false invalid
   */
  static inline bool IsValidLongtitude(double longtitude) {return (longtitude >= -180 && longtitude <= 180);}
  /**
   * Checks if latitude is in valid range
   *
   * \param latitude to check
   * \return bool true latitude valid, false invalid
   */
  static inline bool IsValidLatitude(double latitude) {return (latitude >= -90 && latitude <= 90);}
  /**
   * Checks if altitude is in valid range
   *
   * \param altitude to check
   * \return bool true altitude valid, false invalid.
   *
   */
  static inline bool IsValidAltitude(double altitude) {return ( (bParam + altitude) >= 0 );}
  /**
   * Creates Geodetic coordinates from given Cartesian coordinates.
   *
   * \param vector reference to vector containing Cartesian coordinates for creation.
   */
  void FromVector(const Vector &vector);

  /**
   * latitude of coordinate
   */
  double m_latitude;
  /**
   * longitude of coordinate
   */
  double m_longitude;
  /**
   * altitude of coordinate
   */
  double m_altitude;
};

/**
 * \class ns3::GeoCoordinateValue
 * \brief hold objects of type ns3::GeoCoordinate
 */
ATTRIBUTE_HELPER_HEADER (GeoCoordinate);

std::ostream &operator << (std::ostream &os, const GeoCoordinate &coordinate);
std::istream &operator >> (std::istream &is, GeoCoordinate &coordinate);

Ptr<const AttributeChecker> MakeGeoCoordinateChecker (void);

} // namespace ns3

#endif /* GEO_COORDINATE_H */
