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

#ifndef SAT_SGP4_POSITION_ALLOCATOR_H
#define SAT_SGP4_POSITION_ALLOCATOR_H

#include "satellite-position-allocator.h"

#include "satellite-sgp4unit.h"

#include "satellite-sgp4io.h"

namespace ns3 {

/**
 * TODO
 */
class SatSGP4PositionAllocator : public SatPositionAllocator {
public:
  /// World Geodetic System (WGS) constants to be used by SGP4/SDP4 models.
  static const gravconsttype WGeoSys;
  /// Satellite's information line size defined by TLE data format.
  static const uint32_t TleSatInfoWidth;

  /**
   * @brief Get the type ID.
   * @return the object TypeId.
   */
  static TypeId GetTypeId ();

  /**
   * @brief Default constructor.
   */
  SatSGP4PositionAllocator ();

  /**
   * @brief Destructor.
   */
  virtual ~SatSGP4PositionAllocator ();

  /**
   * @brief Set satellite's TLE information required for its initialization.
   * @param tle The two lines of the TLE data format.
   * @return a boolean indicating whether the initialization succeeded.
   */
  bool SetTleInfo (const std::string &tle);

  virtual GeoCoordinate GetNextGeoPosition () const;

private:
  std::string m_tle1, m_tle2;                       //!< satellite's TLE data.
  mutable elsetrec m_sgp4_record;                   //!< SGP4/SDP4 record.
};

} // namespace ns3

#endif /* SAT_SGP4_POSITION_ALLOCATOR_H */
