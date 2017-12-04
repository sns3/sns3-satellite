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

#include <algorithm>
#include <stdlib.h>
#include "ns3/double.h"
#include "ns3/log.h"
#include "satellite-utils.h"
#include "satellite-antenna-gain-pattern.h"

NS_LOG_COMPONENT_DEFINE ("SatAntennaGainPattern");

namespace ns3 {

const std::string SatAntennaGainPattern::m_nanStringArray[4] = {"nan", "NaN", "Nan", "NAN"};


NS_OBJECT_ENSURE_REGISTERED (SatAntennaGainPattern);


TypeId
SatAntennaGainPattern::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatAntennaGainPattern")
    .SetParent<Object> ()
    .AddConstructor<SatAntennaGainPattern> ()
    .AddAttribute ("MinAcceptableAntennaGainDb", "Minimum acceptable antenna gain in dBs",
                   DoubleValue (48.0),
                   MakeDoubleAccessor (&SatAntennaGainPattern::m_minAcceptableAntennaGainInDb),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

TypeId
SatAntennaGainPattern::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}


SatAntennaGainPattern::SatAntennaGainPattern ()
  : m_antennaPattern (),
    m_validPositions (),
    m_minAcceptableAntennaGainInDb (40.0),
    m_uniformRandomVariable (),
    m_latitudes (),
    m_longitudes (),
    m_minLat (0.0),
    m_minLon (0.0),
    m_maxLat (0.0),
    m_maxLon (0.0),
    m_latInterval (0.0),
    m_lonInterval (0.0),
    m_nanStrings ()
{
  // Do nothing here
}

SatAntennaGainPattern::SatAntennaGainPattern (std::string filePathName)
  : m_nanStrings (m_nanStringArray, m_nanStringArray + (sizeof m_nanStringArray / sizeof m_nanStringArray[0]))
{
  // Attributes are needed already in construction phase:
  // - ConstructSelf call in constructor
  // - GetInstanceTypeId is needed to be implemented
  ObjectBase::ConstructSelf (AttributeConstructionList ());

  ReadAntennaPatternFromFile (filePathName);
  m_uniformRandomVariable = CreateObject<UniformRandomVariable> ();
}


void SatAntennaGainPattern::ReadAntennaPatternFromFile (std::string filePathName)
{
  NS_LOG_FUNCTION (this << filePathName);

  // READ FROM THE SPECIFIED INPUT FILE
  std::ifstream *ifs = new std::ifstream (filePathName.c_str (), std::ifstream::in);

  if (!ifs->is_open ())
    {
      // script might be launched by test.py, try a different base path
      delete ifs;
      filePathName = "../../" + filePathName;
      ifs = new std::ifstream (filePathName.c_str (), std::ifstream::in);

      if (!ifs->is_open ())
        {
          NS_FATAL_ERROR ("The file " << filePathName << " is not found.");
        }
    }

  // Row vector containing all the gain values for a certain latitude
  std::vector<double> rowVector;

  // Start conditions
  double lat, lon, gainDouble;
  std::string gainString;
  bool firstRowDone (false);

  // Read a row
  *ifs >> lat >> lon >> gainString;

  while (ifs->good ())
    {
      // Validity of latitude and longitude
      if (lat < -90.0 || lat > 90.0
          || lon < -180.0 || lon > 180.0)
        {
          NS_FATAL_ERROR ("SatAntennaGainPattern::ReadAntennaPatternFromFile - unvalid latitude: " << lat << " or longitude. " << lon);
        }

      // The antenna gain value is read to a string, so that we may check
      // that whether the value is NaN. If not, then the number is just converted
      // to a double.
      if (find (m_nanStrings.begin (), m_nanStrings.end (), gainString) != m_nanStrings.end ())
        {
          gainDouble = NAN;
        }
      else
        {
          gainDouble = atof (gainString.c_str ());

          // Add the position to valid positions vector if the gain is
          // above a specified threshold.
          if ( gainDouble >= m_minAcceptableAntennaGainInDb )
            {
              m_validPositions.push_back (std::make_pair (lat, lon));
            }
        }

      // Collect the valid latitude values
      if (!m_latitudes.empty () )
        {
          if (m_latitudes.back () != lat)
            {
              firstRowDone = true;
              m_latInterval = lat - m_latitudes.back ();
              m_latitudes.push_back (lat);
            }
        }
      else
        {
          m_latitudes.push_back (lat);
        }

      // Collect the valid longitude values
      if (!m_longitudes.empty () )
        {
          if (!firstRowDone && m_longitudes.back () != lon)
            {
              m_lonInterval = lon - m_longitudes.back ();
              m_longitudes.push_back (lon);
            }
        }
      else
        {
          m_longitudes.push_back (lon);
        }

      // If this is the first gain entry
      if (rowVector.empty ())
        {
          m_minLat = lat;
          m_minLon = lon;
          rowVector.push_back (gainDouble);
        }
      // We are still in the same row (= latitude)
      else if (lat == m_maxLat)
        {
          rowVector.push_back (gainDouble);
        }
      // Latitude changed
      // - Store the vector
      // - Clean-up
      // - Start from another row
      else
        {
          m_antennaPattern.push_back (rowVector);
          rowVector.clear ();
          rowVector.push_back (gainDouble);
        }

      // Update the maximum values
      m_maxLat = lat;
      m_maxLon = lon;

      // get next row
      *ifs >> lat >> lon >> gainString;
    }

  // At this point, the last row should not be stored, since the storing
  // happens every time the row changes. I.e. the last row is stored here!
  NS_ASSERT ( rowVector.size () == m_longitudes.size ());

  m_antennaPattern.push_back (rowVector);
  rowVector.clear ();

  ifs->close ();
  delete ifs;
}


GeoCoordinate SatAntennaGainPattern::GetValidRandomPosition () const
{
  NS_LOG_FUNCTION (this);

  uint32_t numPosGridPoints = m_validPositions.size ();
  uint32_t ind (0);
  std::pair<double, double> lowerLeftCoord;

  while (1)
    {
      // Get random position (=lower left corner of a grid) from the valid ones
      ind = m_uniformRandomVariable->GetInteger (0, numPosGridPoints - 1);
      lowerLeftCoord = m_validPositions[ind];

      // Test if the three other corners for interpolation are found.
      // If they do not, loop again to find another position.

      std::pair<double, double> testPos;

      // Upper left corner
      testPos.first = lowerLeftCoord.first + m_latInterval;
      testPos.second = lowerLeftCoord.second;
      if (find (m_validPositions.begin (), m_validPositions.end (), testPos) == m_validPositions.end ())
        {
          continue;
        }

      // Upper right corner
      testPos.second = lowerLeftCoord.second + m_lonInterval;
      if (find (m_validPositions.begin (), m_validPositions.end (), testPos) == m_validPositions.end ())
        {
          continue;
        }

      // Lower right corner
      testPos.first = lowerLeftCoord.first;
      if (find (m_validPositions.begin (), m_validPositions.end (), testPos) == m_validPositions.end ())
        {
          continue;
        }

      // None of the previous checks triggered, thus we have a valid position.
      break;
    }

  // Pick a random position within a grid square
  double latOffset = m_uniformRandomVariable->GetValue (0.0, m_latInterval - 0.001);
  double lonOffset = m_uniformRandomVariable->GetValue (0.0, m_lonInterval - 0.001);
  GeoCoordinate coord (lowerLeftCoord.first + latOffset, lowerLeftCoord.second + lonOffset, 0.0);

  return coord;
}


double SatAntennaGainPattern::GetAntennaGain_lin (GeoCoordinate coord) const
{
  NS_LOG_FUNCTION (this << coord.GetLatitude () << coord.GetLongitude ());

  // Get the requested position {latitude, longitude}
  double latitude = coord.GetLatitude ();
  double longitude = coord.GetLongitude ();

  // Given {latitude, longitude} has to be inside the min/max latitude/longitude values
  if (m_minLat > latitude
      || latitude > m_maxLat
      || m_minLon > longitude
      || longitude > m_maxLon)
    {
      NS_FATAL_ERROR (this << " given latitude and longitude out of range!");
    }

  // Calculate the minimum grid point {minLatIndex, minLonIndex} for the given {latitude, longitude} point
  uint32_t minLatIndex = (uint32_t)(std::floor (std::abs (latitude - m_minLat) / m_latInterval));
  uint32_t minLonIndex = (uint32_t)(std::floor (std::abs (longitude - m_minLon) / m_lonInterval));

  // All the values within the grid box has to be valid! If UT is placed (or
  // is moving outside) the valid simulation area, the simulation will crash
  // to a fatal error.
  if (std::isnan (m_antennaPattern[minLatIndex][minLonIndex])
      || std::isnan (m_antennaPattern[minLatIndex][minLonIndex + 1])
      || std::isnan (m_antennaPattern[minLatIndex + 1][minLonIndex])
      || std::isnan (m_antennaPattern[minLatIndex + 1][minLonIndex + 1]))
    {
      NS_FATAL_ERROR (this << ", some value(s) of the interpolated grid point(s) is/are NAN!");
    }

  /**
   * 4-point bilinear interpolation
   * R(x,y1) = (x2 - x)/(x2 - x1) * Q(x1,y1)) + (x - x1)/(x2 - x1) * Q(x2,y1);
   * R(x,y2) = (x2 - x)/(x2 - x1) * Q(x1,y2)) + (x - x1)/(x2 - x1) * Q(x2,y2);
   * R = (y2 - y)/(y2 - y1) * R(x,y1) + (y - y1)/(y2 - y1) * R(x,y2);
   */

  // Longitude direction with latitude minLatIndex
  double upperLonShare = (m_longitudes[minLonIndex + 1] - longitude) / m_lonInterval;
  double lowerLonShare = (longitude - m_longitudes[minLonIndex]) / m_lonInterval;

  // Change the gains to linear values , because the interpolation is done in linear domain.
  double G11 = SatUtils::DbToLinear ( m_antennaPattern[minLatIndex][minLonIndex] );
  double G12 = SatUtils::DbToLinear ( m_antennaPattern[minLatIndex][minLonIndex + 1] );
  double G21 = SatUtils::DbToLinear ( m_antennaPattern[minLatIndex + 1][minLonIndex] );
  double G22 = SatUtils::DbToLinear ( m_antennaPattern[minLatIndex + 1][minLonIndex + 1] );

  // Longitude direction with latitude minLatIndex
  double valLatLower = upperLonShare * G11 + lowerLonShare * G12;

  // Longitude direction with latitude minLatIndex+1
  double valLatUpper = upperLonShare * G21 + lowerLonShare * G22;

  // Latitude direction with longitude "longitude"
  double gain = ((m_latitudes[minLatIndex + 1] - latitude) / m_latInterval) * valLatLower +
    ((latitude - m_latitudes[minLatIndex]) / m_latInterval) * valLatUpper;

  /*
  std::cout << "minLonIndex = " << minLonIndex <<
      ", minLatIndex = " << minLatIndex <<
      ", m_lonInterval = " << m_lonInterval <<
      ", m_LatInterval = " << m_latInterval <<
      ", x1 = " << m_longitudes[minLonIndex] <<
      ", y1 = " << m_latitudes[minLatIndex] <<
      ", x2 = " << m_longitudes[minLonIndex+1] <<
      ", y2 = " << m_latitudes[minLatIndex+1] <<
      ", G(x1,y1) = " << m_antennaPattern[minLatIndex][minLonIndex] <<
      ", G(x1,y2) = " << m_antennaPattern[minLatIndex+1][minLonIndex] <<
      ", G(x2,y1) = " << m_antennaPattern[minLatIndex][minLonIndex+1] <<
      ", G(x2,y2) = " << m_antennaPattern[minLatIndex+1][minLonIndex+1] <<
      ", x = " << longitude <<
      ", y = " << latitude <<
      ", interpolated gain: " << gain << std::endl;
   */
  return gain;
}



} // namespace ns3
