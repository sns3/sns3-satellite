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

#ifndef SATELLITE_ANTENNA_GAIN_PATTERN_H
#define SATELLITE_ANTENNA_GAIN_PATTERN_H

#include <vector>
#include <fstream>
#include "ns3/random-variable-stream.h"
#include "ns3/object.h"
#include "geo-coordinate.h"

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief Antenna gain pattern
 */
class SatAntennaGainPattern : public Object
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  SatAntennaGainPattern ();
  SatAntennaGainPattern (std::string filePathName);
  ~SatAntennaGainPattern () {}

  /**
   * Calculate the antenna gain value for a certain {latitude, longitude} point
   * \return The gain value
   */
  double GetAntennaGain_lin (GeoCoordinate coord) const;

  /**
   * Get a valid position under this spot-beam coverage.
   * \return GeoCoordinate
   */
  GeoCoordinate GetValidPosition () const;

private:
  /**
   * Read the antenna gain pattern from a file
   * \param filePathName Path and file name of the antenna pattern file
   */
  void ReadAntennaPatternFromFile (std::string filePathName);

  /**
   * Container for the antenna pattern from one spot-beam
   * - Outer vector holds gain values for all latitudes
   * - Inner vector holds gain values for all longitudes for a certain latitude
   */
  std::vector< std::vector <double> > m_antennaPattern;

  /**
   * Container for valid positions
   * - Latitude
   * - Longitude
   */
  std::vector< std::pair<double, double> > m_validPositions;

  /**
   * Minimum acceptable antenna gain for a serving spot-beam. Used
   * for beam selection.
   */
  double m_minAcceptableAntennaGainInDb;

  /**
   * Uniform random variable used for beam selection.
   */
  Ptr<UniformRandomVariable> m_uniformRandomVariable;

  /**
   * All valid latitudes from the file
   */
  std::vector<double> m_latitudes;

  /**
   * All valid latitudes from the file
   */
  std::vector<double> m_longitudes;

  /**
   * Minimum latitude value of the antenna gain pattern
   */
  double m_minLat;

  /**
   * Minimum longitude value of the antenna gain pattern
   */
  double m_minLon;

  /**
   * Maximum latitude value of the antenna gain pattern
   */
  double m_maxLat;

  /**
   * Minimum longitude value of the antenna gain pattern
   */
  double m_maxLon;

  /**
   * Interval between latitudes, must be constant
   */
  double m_latInterval;

  /**
   * Interval between longitudes, must be constant
   */
  double m_lonInterval;

  /**
   * Valid Not-a-Number (NaN) strings
   */
  static const std::string m_nanStringArray[4];
  std::vector<std::string> m_nanStrings;
};





} // namespace ns3

#endif /* SATELLITE_ANTENNA_GAIN_PATTERN_H */
