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
    .AddConstructor<SatAntennaGainPattern> ();
  return tid;
}

SatAntennaGainPattern::SatAntennaGainPattern ()
{
  // Do nothing here
}

SatAntennaGainPattern::SatAntennaGainPattern (std::string filePathName)
 :m_nanStrings(m_nanStringArray, m_nanStringArray + (sizeof m_nanStringArray / sizeof m_nanStringArray[0]))
{
  ReadAntennaPatternFromFile (filePathName);
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

  // Read a row
  *ifs >> lat >> lon >> gainString;

  while (ifs->good())
    {
      // Validity of latitude and longitude
      NS_ASSERT (lat >= -90.0 && lat <= 90.0);
      NS_ASSERT (lon >= -180.0 && lat <= 180.0);

      // The antenna gain value is read to a string, so that we may check
      // that whether the value is NaN. If not, then the number is just converted
      // to a double.
      if (find (m_nanStrings.begin(), m_nanStrings.end(), gainString) != m_nanStrings.end())
        {
          gainDouble = NAN;
        }
      else
        {
          gainDouble = atof(gainString.c_str());
        }

      // Collect the valid latitude values
      if (!m_latitudes.empty() )
        {
          if (m_latitudes.back() != lat)
            {
              m_latInterval = lat - m_latitudes.back();
              m_latitudes.push_back (lat);
            }
        }
      else
        {
          m_latitudes.push_back (lat);
        }

      // Collect the valid longitude values
      if (!m_longitudes.empty() )
        {
          if (m_longitudes.back() != lon)
            {
              m_lonInterval = lon - m_longitudes.back();
              m_longitudes.push_back (lon);
            }
        }
      else
        {
          m_longitudes.push_back (lon);
        }

      // If this is the first gain entry
      if (rowVector.empty())
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

  ifs->close ();
  delete ifs;
}



double SatAntennaGainPattern::GetAntennaGain_lin (GeoCoordinate coord) const
{
  NS_LOG_FUNCTION (this << coord.GetLatitude() << coord.GetLongitude());

  // Get the requested position {latitude, longitude}
  double latitude = coord.GetLatitude ();
  double longitude = coord.GetLongitude ();

  // Given {latitud, longitude} has to be inside the min/max latitude/longitude values
  NS_ASSERT (m_minLat <= latitude);
  NS_ASSERT (latitude <= m_maxLat);
  NS_ASSERT (m_minLon <= longitude);
  NS_ASSERT (longitude <= m_maxLon);

  // Calculate the minimum grid point {minLatIndex, minLonIndex} for the given {latitude, longitude} point
  uint32_t minLatIndex = (uint32_t)(std::floor(std::abs(latitude - m_minLat) / m_latInterval));
  uint32_t minLonIndex = (uint32_t)(std::floor(std::abs(longitude - m_minLon) / m_lonInterval));

  // All the values within the grid box has to be valid! If UT is placed (or
  // is moving outside) the valid simulation area, the simulation will crash
  // to assert.
  NS_ASSERT (!isnan(m_antennaPattern[minLatIndex][minLonIndex]));
  NS_ASSERT (!isnan(m_antennaPattern[minLatIndex][minLonIndex+1]));
  NS_ASSERT (!isnan(m_antennaPattern[minLatIndex+1][minLonIndex]));
  NS_ASSERT (!isnan(m_antennaPattern[minLatIndex+1][minLonIndex+1]));

  /**
   * 4-point bilinear interpolation
   * R(x,y1) = (x2 - x)/(x2 - x1) * Q(x1,y1)) + (x - x1)/(x2 - x1) * Q(x2,y1);
   * R(x,y2) = (x2 - x)/(x2 - x1) * Q(x1,y2)) + (x - x1)/(x2 - x1) * Q(x2,y2);
   * R = (y2 - y)/(y2 - y1) * R(x,y1) + (y - y1)/(y2 - y1) * R(x,y2);
   */

  // Longitude direction with latitude minLatIndex
  double upperLonShare = (m_longitudes[minLonIndex+1] - longitude) / m_lonInterval;
  double lowerLonShare = (longitude - m_longitudes[minLonIndex]) / m_lonInterval;

  // Change the gains to linear values , because the interpolation is done in linear domain.
  double G11 = SatUtils::DbToLinear ( m_antennaPattern[minLatIndex][minLonIndex] );
  double G12 = SatUtils::DbToLinear ( m_antennaPattern[minLatIndex][minLonIndex+1] );
  double G21 = SatUtils::DbToLinear ( m_antennaPattern[minLatIndex+1][minLonIndex] );
  double G22 = SatUtils::DbToLinear ( m_antennaPattern[minLatIndex+1][minLonIndex+1] );

  // Longitude direction with latitude minLatIndex
  double valLatLower = upperLonShare * G11 + lowerLonShare * G12;

  // Longitude direction with latitude minLatIndex+1
  double valLatUpper = upperLonShare * G21 + lowerLonShare * G22;

  // Latitude direction with longitude "longitude"
  double gain = ((m_latitudes[minLatIndex+1] - latitude) / m_latInterval) * valLatLower +
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
