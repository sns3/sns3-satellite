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


#include "satellite-sgp4-position-allocator.h"

NS_LOG_COMPONENT_DEFINE ("sat-sgp4-position-allocator");

namespace ns3 {

TypeId
SatSGP4PositionAllocator::GetTypeId (void) {
  static TypeId tid = TypeId ("ns3::SatSGP4PositionAllocator")
    .SetParent<SatPositionAllocator> ()
    .AddConstructor<SatSGP4PositionAllocator> ()
  ;

  return tid;
}

SatSGP4PositionAllocator::SatSGP4PositionAllocator () { }
SatSGP4PositionAllocator::~SatSGP4PositionAllocator () { }

GeoCoordinate
SatSGP4PositionAllocator::GetNextGeoPosition (void) const
{
  // TODO
  return GeoCoordinate();
}

} // namespace ns3
