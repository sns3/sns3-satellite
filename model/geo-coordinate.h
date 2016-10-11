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

#include "ns3/attribute.h"
#include "ns3/attribute-helper.h"
#include "ns3/vector.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief GeoCoordinate class is used to store and operate with geodetic coordinates.
 * Latitude is in the degree range (-90, 90) with negative values -> south
 * Longitude is in the degree range (-180, 180) with negative values -> west
 * Altitude is in meters.
 */
class GeoCoordinate
{
public:
  typedef enum
  {
    SPHERE,
    WGS84,
    GRS80
  } ReferenceEllipsoid_t;

  /**
   * Create GeoCoordinate from given latitude, longitude and altitude decimal degree values.
   * Reference ellipsoid to be used is sphere.
   *
   * \param latitude latitude of position, -90 ... 90 degrees
   * \param longitude longitude of position, -180 ... 180 degrees
   * \param altitude altitude of position, >= -EARTH_RADIUS
   */
  GeoCoordinate (double latitude, double longitude, double altitude);

  /**
   * Create GeoCoordinate from given latitude, longitude and altitude decimal degree values
   *
   * \param latitude latitude of position, -90 ... 90 degrees
   * \param longitude longitude of position, -180 ... 180 degrees
   * \param altitude altitude of position, >= -EARTH_RADIUS
   * \param refEllipsoid Reference ellipsoid to be used
   */
  GeoCoordinate (double latitude, double longitude, double altitude, ReferenceEllipsoid_t refEllipsoid);

  /**
   * Create GeoCoordinate from given vector (x,y,z). Reference ellipsoid to be used is sphere.
   *
   * \param vector coordinates as Vector (x,y,z) (Cartesian)
   */
  GeoCoordinate (Vector vector);

  /**
   * Create GeoCoordinate from given vector (x,y,z) and reference ellipsoid.
   *
   * \param vector coordinates as Vector (x,y,z) (Cartesian)
   * \param refEllipsoid Reference ellipsoid to be used
   */
  GeoCoordinate (Vector vector, ReferenceEllipsoid_t refEllipsoid);

  /**
   * Create GeoCoordinate with zero values of longitude, latitude and altitude. Reference ellipsoid to be used is sphere.
   */
  GeoCoordinate ();

  /**
   * Gets reference ellipsoid used by GeoCoordinate object.
   *
   * \return  reference ellipsoid.
   */
  ReferenceEllipsoid_t GetRefEllipsoid ()
  {
    return m_refEllipsoid;
  }

  /**
   * Gets longitude value of coordinate.
   *
   * \return longitude
   */
  double GetLongitude () const;
  /**
     * Gets latitude value of coordinate.
     *
     * \return latitude
     */
  double GetLatitude () const;
  /**
   * Gets altitude value of coordinate.
   *
   * \return altitude
   */
  double GetAltitude () const;
  /**
   * Sets longitude value of coordinate.
   *
   * \param longitude longitude to set
   */
  void SetLongitude (double longitude);
  /**
   * Sets latitude value of coordinate.
   *
   * \param latitude longitude to set
   */
  void SetLatitude (double latitude);
  /**
   * Sets altitude value of coordinate.
   *
   * \param altitude longitude to set
   */
  void SetAltitude (double altitude);
  /**
   * Converts Geodetic coordinates to Cartesian coordinates
   * \return Vector containing Cartesian coordinates
   */
  Vector ToVector ();

  // Definitions for reference Earth Ellipsoid parameters.
  // Sphere, WGS84 and GRS80 reference ellipsoides supported.

  // length (in meters) for semi-minor axis A. (same for all reference ellipsoides)
  static constexpr double equatorRadius = 6378137.0;

  // length (in meters) for semi-minor axis B
  static constexpr double polarRadius_sphere = 6378137.0;         // Sphere
  static constexpr double polarRadius_wgs84 = 6356752.314245;     // WGS84 ellipsoide
  static constexpr double polarRadius_grs80 = 6356752.314103;     // GRS80 ellipsoide

private:
  /**
   * Gets the radius of curvature in the prime vertical
   * \param latitude latitude in radians at to get the radius of curvature.
   * \return value of the radius of curvature (meters)
   */
  double GetRadiusCurvature (double latitude);
  /**
   * Checks if longtitude is in valid range
   *
   * \param longtitude to check
   * \return bool true longitude valid, false invalid
   */
  static inline bool IsValidLongtitude (double longtitude)
  {
    return (longtitude >= -180 && longtitude <= 180);
  }
  /**
   * Checks if latitude is in valid range
   *
   * \param latitude to check
   * \return bool true latitude valid, false invalid
   */
  static inline bool IsValidLatitude (double latitude)
  {
    return (latitude >= -90 && latitude <= 90);
  }
  /**
   * Checks if altitude is in valid range
   *
   * \param altitude to check
   * \return bool true altitude valid, false invalid.
   *
   */
  static inline bool IsValidAltitude (double altitude, ReferenceEllipsoid_t refEllipsoide);
  /**
   * Creates Geodetic coordinates from given Cartesian coordinates.
   *
   * \param vector reference to vector containing Cartesian coordinates for creation.
   */
  void ConstructFromVector (const Vector &vector);

  /**
   * \param latitude latitude of position
   * \param longitude longitude of position
   * \param altitude altitude of position
   *
   * Helper for constructor to create GeoCoordinate from given latitude, longitude and altitude decimal degree values.
   *
   * Accepted values:
   *
   * Latitude:       -90 ... 90 degrees
   * Longitude:     -180 ... 180 degrees
   * Altitude:      >= -EARTH_RADIUS
   *
   */
  void Construct (double latitude, double longitude, double altitude);

  /**
   * This method is called to initialize needed parameters according to used reference ellipsoide.
   *
   */
  void Initialize ();

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

  ReferenceEllipsoid_t  m_refEllipsoid;   // reference ellipsoid
  double                m_e2Param;        // First eccentricity squared
  double                m_equatorRadius;  // Semi-major axis A, meters
  double                m_polarRadius;    // Semi-major axis B, meters
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
