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

namespace ns3 {

/**
 * TODO
 */
class SatSGP4PositionAllocator : public SatPositionAllocator {
public:
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

  virtual GeoCoordinate GetNextGeoPosition (void) const;

};

} // namespace ns3

#endif /* SAT_SGP4_POSITION_ALLOCATOR_H */
