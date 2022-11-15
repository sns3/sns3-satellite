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
#include "ns3/satellite-beam-user-info.h"
#include "geo-coordinate.h"

namespace ns3 {

class SatMobilityModel;

/**
 * \ingroup satellite
 *
 * \brief Antenna gain pattern container holds all antenna patterns
 * related to a satellite system. Current reference system consists
 * of 72 spot-beams. It is assumed that all links use the same set of
 * antenna patterns (forward feeder and user, return feeder and user).
 * Each antenna gain pattern is stored in a separate class
 * SatAntennaGainPattern. The best beam may be chosen based on
 * the antenna patterns by using GetBestBeamId for a given position.
 */
class SatAntennaGainPatternContainer : public Object
{
public:
  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  TypeId GetInstanceTypeId () const;

  /**
   * Default constructor.
   * \param nbSats Number of satellites to consider
   */
  SatAntennaGainPatternContainer (uint32_t nbSats = 1);
  ~SatAntennaGainPatternContainer ();

  /**
   * definition for beam map key is beam ID and value is UT/user info.
   */
  typedef std::map<uint32_t, SatBeamUserInfo > BeamUserInfoMap_t;

  /**
   * \brief Get the antenna pattern of a specified beam id
   * \param satelliteId Satellite identifier
   * \param beamId Beam identifier
   * \return The antenna gain pattern instance of the specified beam id
   */
  Ptr<SatAntennaGainPattern> GetAntennaGainPattern (uint32_t beamId, uint32_t satelliteId = 0) const;

  /**
   * \brief Get the number of stored antenna pattern
   * \return The total number of antenna gain pattern instance
   */
  uint32_t GetNAntennaGainPatterns () const;

  /**
   * \brief Get the best beam id based on the antenna patterns in a
   * specified geo coordinate
   * \param satelliteId ID of satellite to search
   * \param coord Geo coordinate
   * \return best beam id in the specified geo coordinate
   */
  uint32_t GetBestBeamId (GeoCoordinate coord, uint32_t satelliteId = 0) const;

  void ConfigureBeamsMobility (Ptr<SatMobilityModel> mobility, uint32_t satelliteId = 0);

  void SetEnabledBeams (BeamUserInfoMap_t info, uint32_t satelliteId = 0);

private:
  std::string m_patternsFolder;

  /**
   * Container of antenna patterns
   */
  std::map< std::pair<uint32_t, uint32_t>, Ptr<SatAntennaGainPattern> > m_antennaPatternMap;

};

} // namespace ns3

#endif /* SATELLITE_ANTENNA_GAIN_PATTERN_CONTAINER_H_ */