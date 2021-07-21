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

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("sat-sgp4-position-allocator");

const gravconsttype SatSGP4PositionAllocator::WGeoSys = wgs72;   // recommended for SGP4/SDP4
const uint32_t SatSGP4PositionAllocator::TleSatInfoWidth = 69;

TypeId
SatSGP4PositionAllocator::GetTypeId () {
  static TypeId tid = TypeId ("ns3::SatSGP4PositionAllocator")
    .SetParent<SatPositionAllocator> ()
    .AddConstructor<SatSGP4PositionAllocator> ()
  ;

  return tid;
}

SatSGP4PositionAllocator::SatSGP4PositionAllocator () { }
SatSGP4PositionAllocator::~SatSGP4PositionAllocator () { }

GeoCoordinate
SatSGP4PositionAllocator::GetNextGeoPosition () const
{
  // TODO
  std::cout << "GetNextGeoPosition" << std::endl;
  return GeoCoordinate (0.0, 0.0, 400.0*0 + 360000000.0);
}

bool
SatSGP4PositionAllocator::SetTleInfo (const std::string &tle)
{
  uint32_t delimPos = tle.find("\n");
  const std::string line1 = tle.substr(0, delimPos);
  const std::string line2 = tle.substr(delimPos + 1, tle.size() - delimPos);

  double start, stop, delta;
  char l1[TleSatInfoWidth+1], l2[TleSatInfoWidth+1];
  double r[3], v[3];

  NS_ASSERT_MSG (
    line1.size () == TleSatInfoWidth && line2.size () == TleSatInfoWidth,
    "Two-Line Element info lines must be of length" << TleSatInfoWidth << "!"
  );

  m_tle1 = std::string (line1.c_str ());
  m_tle2 = std::string (line2.c_str ());

  memcpy (l1, line1.c_str (), sizeof (l1));
  memcpy (l2, line2.c_str (), sizeof (l2));

  // 'c' => catalog mode run
  // 'e' => epoch time (relative to TLE lines)
  // 'i' => improved mode of operation
  twoline2rv (
    l1, l2, 'c', 'e', 'i', WGeoSys, start, stop, delta, m_sgp4_record
  );

  // call propagator to check if it has been properly initialized
  sgp4 (WGeoSys, m_sgp4_record, 0, r, v);

  // return true if no errors occurred
  return (m_sgp4_record.error == 0);
}

} // namespace ns3
