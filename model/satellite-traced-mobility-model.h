/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#ifndef SATELLITE_TRACED_MOBILITY_MODEL_H
#define SATELLITE_TRACED_MOBILITY_MODEL_H

#include <ns3/nstime.h>
#include "satellite-mobility-model.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Satellite mobility model for which the current position change
 * based on values read from a file.
 */
class SatTracedMobilityModel : public SatMobilityModel
{
public:
  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor.
   */
  SatTracedMobilityModel ();

  /**
   * Destructor for SatMobilityModel
   */
  virtual ~SatTracedMobilityModel ();

private:
  /**
   * \return the current velocity.
   */
  virtual Vector DoGetVelocity (void) const;

  /**
   * \return the current position.
   */
  virtual GeoCoordinate DoGetGeoPosition (void) const;

  /**
   * \param position the position to set.
   */
  virtual void DoSetGeoPosition (const GeoCoordinate &position);

  void UpdateGeoPositionFromFile (void);

  Time m_updateInterval;
  GeoCoordinate::ReferenceEllipsoid_t m_refEllipsoid;
  GeoCoordinate m_geoPosition;
  Vector m_velocity;
};

} // namespace ns3

#endif /* SATELLITE_TRACED_MOBILITY_MODEL_H */
