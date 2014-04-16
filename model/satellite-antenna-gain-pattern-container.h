/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#ifndef SATELLITE_ANTENNA_GAIN_PATTERN_CONTAINER_H_
#define SATELLITE_ANTENNA_GAIN_PATTERN_CONTAINER_H_

#include "satellite-antenna-gain-pattern.h"
#include "geo-coordinate.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Antenna gain pattern container holds all antenna patterns
 * related to the reference system. Current reference system consists
 * of 72 spot-beams. Each antenna gain pattern is stored in a separate
 * class SatAntennaGainPattern. The best beam may be chosen based on
 * the antenna patterns by using GetBestBeamId for a given position.
 */
class SatAntennaGainPatternContainer : public Object
{
public:
  static TypeId GetTypeId (void);

  SatAntennaGainPatternContainer ();
  ~SatAntennaGainPatternContainer () {}

  /**
   * Definition of number of beams (72-beam reference scenario)
   */
  typedef std::map<uint32_t, Ptr<SatAntennaGainPattern> >::const_iterator gpIterator;

  /**
   * Get the antenna pattern of a specified beam id
   * \param beamId Beam identifier
   * \return The antenna gain pattern instance of the specified beam id
   */
  Ptr<SatAntennaGainPattern> GetAntennaGainPattern (uint32_t beamId) const;

  /**
   * Get the best beam id based on the antenna patterns in a
   * specified geo coordinate
   * \param coord Geo coordinate
   * \return best beam id in the specified geo coordinate
   */
  uint32_t GetBestBeamId (GeoCoordinate coord) const;

private:
  /**
   * Definition of number of beams (72-beam reference scenario)
   */
  static const uint32_t NUMBER_OF_BEAMS = 72;

  /**
   * Container of antenna patterns
   */
  std::map< uint32_t, Ptr<SatAntennaGainPattern> > m_antennaPatternMap;

};

} // namespace ns3

#endif /* SATELLITE_ANTENNA_GAIN_PATTERN_CONTAINER_H_ */
