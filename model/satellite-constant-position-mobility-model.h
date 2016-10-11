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

#ifndef SATELLITE_CONSTANT_POSITION_MOBILITY_MODEL_H
#define SATELLITE_CONSTANT_POSITION_MOBILITY_MODEL_H

#include "satellite-mobility-model.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Satellite mobility model for which the current position does not change
 * once it has been set and until it is set again explicitly to a new value.
 */
class SatConstantPositionMobilityModel : public SatMobilityModel
{
public:

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * Create a position located at coordinates (0,0,0)
   */
  SatConstantPositionMobilityModel ();

  /**
   * Destructor for SatConstantPositionMobilityModel
   */
  virtual ~SatConstantPositionMobilityModel ();

private:
  Vector DoGetVelocity (void) const;

  virtual GeoCoordinate DoGetGeoPosition (void) const;
  virtual void DoSetGeoPosition (const GeoCoordinate &position);

  mutable GeoCoordinate m_geoPosition;
};

} // namespace ns3

#endif /* SATELLITE_CONSTANT_POSITION_MOBILITY_MODEL_H */
