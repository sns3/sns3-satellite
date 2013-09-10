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
#ifndef SATELLITE_UTILS_H
#define SATELLITE_UTILS_H

#include <cmath>

namespace ns3 {

/**
 * \ingroup satellite
 * 
 * \brief SatUtils class is for general conversions and constants used in satellite module.
 * This class is not planned to instantiate or inherted.
 */
class SatUtils
{
public:
  /**
   * Constant definition for the speed of light in m/s
   */
  static const double SPEED_OF_LIGHT = 299792458.0;

  /**
   * Converts radians to degrees
   *
   * \param radians value to convert as radians
   * \return degrees converted from radians
   */
  static inline double RadiansToDegrees ( double radian ) { return (double) ( ( radian ) * ( 180.0 / M_PI ) ); }

  /**
   * Converts degrees to radians
   *
   * \param degrees value to convert as degrees
   * \return radians converted from degrees
   */
  static inline double DegreesToRadians ( double degree ) { return (double) ( ( degree ) * ( M_PI / 180.0 ) ); }

  /**
   * Converts Decibel Watts to Watts
   *
   * \param dBW value in Decibel Watts to convert
   * \return Watts converted from Decibel Watts
   */
  static inline double DbWToW ( double dBW ) { return DbToLinear( dBW ); }

  /**
   * Converts Watts to Decibel Watts
   *
   * \param W value in Watts to convert
   * \return Decibel Watts converted from Watts
   */
  static inline double WToDbW ( double W ) { return LinearToDb ( W ); }

  /**
   * Converts Decibels to linear
   *
   * \param dB value in Decibels to convert
   * \return linear converted from Decibels
   */
  static inline double DbToLinear ( double dB ) { return std::pow ( 10.0, dB / 10.0 ); }

  /**
   * Converts linear to Decibels
   *
   * \param linear value in linear to convert
   * \return Decibels converted from linear
   */
  static inline double LinearToDb ( double linear ) { return (double) ( 10.0 * std::log10 ( linear ) );}

  /**
   * Converts Decibel milli Watts to Watts
   *
   * \param dBm value in Decibel milli Watts to convert
   * \return Watts converted from Decibel milli Watts
   */
  static inline double DbmToW ( double dBmW ) { return ( DbWToW ( dBmW ) / 1000 ); }

  /**
   * Converts Watts to Decibel milli Watts
   *
   * \param W value in Watts to convert
   * \return Decibel milli Watts converted from Watts
   */
  static inline double WToDbm ( double W ) { return (double) ( WToDbW ( W * 1000.0 ) ); }

private:
  /**
   * Desctructor
   *
   * Made Pure Virtual because the class is not planned be instantiated or inherited
   *
   */
  virtual ~SatUtils() = 0;
};

} // namespace ns3

#endif /* SATELLITE_UTILS_H */
