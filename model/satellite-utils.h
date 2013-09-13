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
#include <limits>
#include "ns3/assert.h"

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
  template <typename T>
  static inline T RadiansToDegrees ( T radian ) { return (T) ( ( radian ) * ( 180.0 / M_PI ) ); }

  /**
   * Converts degrees to radians
   *
   * \param degrees value to convert as degrees
   * \return radians converted from degrees
   */
  template <typename T>
  static inline T DegreesToRadians ( T degree ) { return (T) ( ( degree ) * ( M_PI / 180.0 ) ); }

  /**
   * Get minimum value for linear
   *
   * \return minimum linear value
   */
  template <typename T>
  static inline T MinLin () { return (T) ( std::numeric_limits<T>::min () * 10.0 ); }

  /**
   * Get maximum value for linear
   *
   * \return maximum linear value
   */
  template <typename T>
  static inline T MaxLin () { return (T) ( std::numeric_limits<T>::max () ); }

  /**
   * Get minimum value for Decibel
   *
   * \return minimum Decibel value
   */
  template <typename T>
  static inline T MinDb () { return (T) ( 10.0 * std::log (MinLin<T>()) ); }

  /**
   * Get maximum value for Decibel
   *
   * \return maximum Decibel value
   */
  template <typename T>
  static inline T MaxDb () { return (T) ( 10.0 * std::log (MaxLin<T>()) ); }

  /**
   * Converts Decibel Watts to Watts
   *
   * \param dbw value in Decibel Watts to convert
   * \return Watts converted from Decibel Watts
   */
  template <typename T>
  static inline T DbWToW ( T dbw ) { return (T) DbToLinear<T> (dbw); }

  /**
   * Converts Watts to Decibel Watts
   *
   * \param w value in Watts to convert
   * \return Decibel Watts converted from Watts
   */
  template <typename T>
  static inline T WToDbW ( T w ) { return (T) LinearToDb<T> (w); }

  /**
   * Converts Decibels to linear
   *
   * \param db value in Decibels to convert, NAN means 0 Watt
   *
   * \return linear converted from Decibels
   */
  template <typename T>
  static inline T DbToLinear ( T db )
  {
    NS_ASSERT( db >= MinDb<T> () && db <= MaxDb<T> ()  );

    return (T) std::pow ( 10.0, db / 10.0 );
  }

  /**
   * Converts linear to Decibels
   *
   * \param linear value in linear to convert
   * \return Decibels converted from linear
   */
  template <typename T>
  static inline T LinearToDb ( T linear )
  {
    NS_ASSERT( ( linear >= MinLin<T> () && linear <= MaxLin<T> () ) || linear == 0  );

    return (T) (10.0 * std::log10 ( linear ));
  }

  /**
   * Converts Decibel milli Watts to Watts
   *
   * \param dbm value in Decibel milli Watts to convert
   * \return Watts converted from Decibel milli Watts
   */
  template <typename T>
  static inline T DbmToW ( T dbm ) { return ( DbWToW<T> ( dbm ) / 1000 ); }

  /**
   * Converts Watts to Decibel milli Watts
   *
   * \param w value in Watts to convert
   * \return Decibel milli Watts converted from Watts
   */
  template <typename T>
  static inline T WToDbm ( T w ) { return (T) ( WToDbW<T> ( w * 1000.0 ) ); }

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
