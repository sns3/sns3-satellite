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
 * Altitude is in kilometers.
 */
class GeoCoordinate
{
public:
  /**
   * \param longitude longitude of position
   * \param latitude latitude of position
   * \param altitute altitute of position
   *
   * Creates GeoCoordinate from given longitude, latitude and altitude decimal degree values
   *
   * Accepted values:
   *
   * Longitude:     -180 ... 180 degrees
   * Latitude:       -90 ... 90 degrees
   * Altitude:      >= -EARTH_RADIUS
   *
   */
  GeoCoordinate (double longitude, double latitude, double altitude);
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
   * Volume metric mean radius of the earth in kilometers.
   */
  static const double EARTH_RADIUS = 6371.0;

private:
  /**
   * Converts radians to degrees
   *
   * \param x value to convert as radians
   * \returns degrees converted from radians
   */
  static inline double RadToDeg(double x) {return ((x) * 180/M_PI);}
  /**
   * Converts degrees to radians
   *
   * \param x value to convert as degrees
   * \returns radians converted from degrees
   */
  static inline double DegToRad(double x) {return ((x) * M_PI/180);}
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
  static inline bool IsValidLatitude(double latitude) {return (latitude >= -90 || latitude <= 90);}
  /**
   * Checks if altitude is in valid range
   *
   * \param altitude to check
   * \return bool true altitude valid, false invalid
   *
   */
  static inline bool IsValidAltitude(double altitude) {return ( (EARTH_RADIUS + altitude) >= 0 );}
  /**
   * Creates Geodetic coordinates from given Cartesian coordinates.
   *
   * \param vector reference to vector containing Cartesian coordinates for creation.
   */
  void FromVector(const Vector &vector);

  /**
   * longitude of coordinate
   */
  double m_longitude;
  /**
   * latitude of coordinate
   */
  double m_latitude;
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
