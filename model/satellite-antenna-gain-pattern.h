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
 * \brief SatAntennaGainPattern class holds the antenna gain pattern data
 * for a one single spot-beam. In initialization phase, the gain pattern
 * is read from a file to a container. Current implementation assumes
 * that the antenna pattern is using a constant longitude-latitude grid of
 * samples. This assumption is made to enable fast look-ups from the container
 * (= vector<vector<double>>).
 *
 * Antenna gain patter is used also for spot-beam selection. In initialization phase
 * a valid positions list is constructed based on a minimum accepted antenna gain set
 * as an attribute. This approach is selected to speed up the random UT positioning.
 *
 * Antenna gain value for a given longitude and latitude position is calculated by
 * using 4-point bilinear interpolation.
 */
class SatAntennaGainPattern : public Object
{
public:

  /**
   * \brief Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor.
   */
  SatAntennaGainPattern ();

  /**
   * Constructor with initialization parameters.
   * \param filePathName 
   */
  SatAntennaGainPattern (std::string filePathName);
  ~SatAntennaGainPattern ()
  {
  }

  /**
   * \brief Calculate the antenna gain value for a certain {latitude, longitude} point
   * \return The gain value in linear format
   */
  double GetAntennaGain_lin (GeoCoordinate coord) const;

  /**
   * \brief Get a valid random position under this spot-beam coverage.
   * \return A valid random GeoCoordinate
   */
  GeoCoordinate GetValidRandomPosition () const;

private:
  /**
   * \brief Read the antenna gain pattern from a file
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
