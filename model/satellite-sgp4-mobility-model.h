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

namespace ns3 {

/**
 * TODO
 */
class SatSGP4MobilityModel : public SatMobilityModel {
public:
  /**
   * @brief Get the type ID.
   * @return the object TypeId.
   */
  static TypeId GetTypeId ();

  /**
   * @brief Default constructor.
   */
  SatSGP4MobilityModel ();

  /**
   * @brief Destructor.
   */
  virtual ~SatSGP4MobilityModel ();

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

private:
  virtual Vector DoGetVelocity (void) const;

  virtual GeoCoordinate DoGetGeoPosition (void) const;
  virtual void DoSetGeoPosition (const GeoCoordinate &position);

  mutable GeoCoordinate m_geoPosition;
};

} // namespace ns3

#endif /* SAT_SGP4_MOBILITY_MODEL_H */
