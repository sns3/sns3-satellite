/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 INESC TEC
 * Copyright (c) 2021 CNES
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
 * Author: Pedro Silva  <pmms@inesctec.pt>
 * Author: Bastien Tauran <bastien.tauran@viveris.fr>
 */

#ifndef SAT_SGP4_MOBILITY_MODEL_H
#define SAT_SGP4_MOBILITY_MODEL_H

#include "ns3/nstime.h"
#include "ns3/vector.h"

#include "satellite-mobility-model.h"
#include "julian-date.h"

#include "satellite-sgp4unit.h"
#include "satellite-sgp4io.h"

namespace ns3 {

/**
 * TODO
 */
class SatSGP4MobilityModel : public SatMobilityModel {
public:
  /// World Geodetic System (WGS) constants to be used by SGP4/SDP4 models.
  static const gravconsttype WGeoSys;
  /// Satellite's information line size defined by TLE data format.
  static const uint32_t TleSatInfoWidth;

  /**
   * @brief Get the type ID.
   * @return the object TypeId.
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  /**
   * @brief Default constructor.
   */
  SatSGP4MobilityModel ();

  /**
   * @brief Destructor.
   */
  virtual ~SatSGP4MobilityModel ();

  /**
   * @brief Check if the satellite has already been initialized.
   * @return a boolean indicating whether the satellite is initialized.
   */
  bool IsInitialized (void) const;

  /**
   * @brief Get the time instant considered as the simulation start.
   * @return a JulianDate object with the time considered as simulation start.
   */
  JulianDate GetStartTime () const;

  /**
   * @brief Set the time instant considered as the simulation start.
   * @param t the time instant to be considered as simulation start.
   */
  void SetStartTime (const JulianDate &t);

  /**
   * @brief Set satellite's TLE information required for its initialization.
   * @param tle The two lines of the TLE data format.
   * @return a boolean indicating whether the initialization succeeded.
   */
  bool SetTleInfo (const std::string &tle);

private:
  /// row of a Matrix
  struct Row {
    double r[3];

    double& operator[] (uint32_t i) { return r[i]; }
    const double& operator[] (uint32_t i) const { return r[i]; }
  };

  /// Matrix data structure to make coordinate conversion code clearer and
  /// less verbose
  struct Matrix {
  public:
    Matrix (void) { }
    Matrix (
      double c00, double c01, double c02,
      double c10, double c11, double c12,
      double c20, double c21, double c22
    );

    Row& operator[] (uint32_t i) { return m[i]; }
    const Row& operator[] (uint32_t i) const { return m[i]; }

    Vector3D operator* (const Vector3D &v) const;

    Matrix Transpose (void) const;

  private:
    Row m[3];
  };

  std::string m_tle1, m_tle2;                       //!< satellite's TLE data.
  mutable elsetrec m_sgp4_record;                   //!< SGP4/SDP4 record.

  virtual Vector DoGetVelocity () const;

  virtual GeoCoordinate DoGetGeoPosition () const;
  virtual void DoSetGeoPosition (const GeoCoordinate &position);
  virtual Vector DoGetPosition () const;
  virtual void DoSetPosition (const Vector &position);

  /**
   * @brief Retrieve the TLE epoch time.
   * @return the TLE epoch time or 0h, 1 January 1992 if the satellite has not
   *         yet been initialized.
   */
  JulianDate GetTleEpoch (void) const;

  /**
   * @brief Retrieve the satellite's position vector in ITRF coordinates.
   * @param t When.
   * @return the satellite's position vector in ITRF coordinates (meters).
   */
  static Vector3D rTemeTorItrf (const Vector3D &rteme, const JulianDate &t);

  /**
   * @brief Retrieve the matrix for converting from PEF to ITRF at a given time.
   * @param t When.
   * @return the PEF-ITRF conversion matrix.
   */
  static Matrix PefToItrf (const JulianDate &t);

  /**
   * @brief Retrieve the matrix for converting from TEME to PEF at a given time.
   * @param t When.
   * @return the TEME-PEF conversion matrix.
   */
  static Matrix TemeToPef (const JulianDate &t);

  /**
   * @brief Retrieve the satellite's velocity vector in ITRF coordinates.
   * @param t When.
   * @return the satellite's velocity vector in ITRF coordinates (m/s).
   */
  static Vector3D rvTemeTovItrf (const Vector3D &rteme, const Vector3D &vteme, const JulianDate& t);

  /**
   * Last saved satellite position
   */
  mutable GeoCoordinate m_lastPosition;

  /**
   * Simulation absolute start time
   */
  JulianDate m_start;

  /**
   * Simulation absolute start time in string format
   */
  std::string m_startStr;
};

} // namespace ns3

#endif /* SAT_SGP4_MOBILITY_MODEL_H */
