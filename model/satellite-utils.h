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
#include <vector>
#include "ns3/assert.h"
#include "ns3/packet.h"
#include "ns3/mac48-address.h"
#include "ns3/satellite-mac-tag.h"
#include "satellite-enums.h"

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
   * Number of bits in a byte
   */
  static const uint32_t BITS_PER_BYTE = 8;

  /**
   * Converts radians to degrees
   *
   * \param radian value to convert as radians
   * \return degrees converted from radians
   */
  template <typename T>
  static inline T RadiansToDegrees ( T radian ) { return (T) ( ( radian ) * ( 180.0 / M_PI ) ); }

  /**
   * Converts degrees to radians
   *
   * \param degree value to convert as degrees
   * \return radians converted from degrees
   */
  template <typename T>
  static inline T DegreesToRadians ( T degree ) { return (T) ( ( degree ) * ( M_PI / 180.0 ) ); }

  /**
   * Get minimum value for linear. (The smallest value greater than zero)
   *
   * \return minimum linear value
   */
  template <typename T>
  static inline T MinLin () { return (T) ( std::numeric_limits<T>::min () ); }

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
  static inline T MinDb () { return (T) LinearToDb ( MinLin<T>() ); }

  /**
   * Get maximum value for Decibel
   *
   * \return maximum Decibel value
   */
  template <typename T>
  static inline T MaxDb () { return (T) LinearToDb ( MaxLin<T>() ); }

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
   * Accepted values for conversion are between minimum decibel value and
   * maximum decibel value. Zero is also accepted and it converts to -inf.
   *
   * \param db value in Decibels to convert, NAN means 0 Watt
   *
   * \return linear converted from Decibels
   */
  template <typename T>
  static inline T DbToLinear ( T db )
  {
    NS_ASSERT( ( db >= MinDb<T> () && db <= MaxDb<T> () ) || -isinf(db)  );

    return (T) std::pow ( 10.0, db / 10.0 );
  }

  /**
   * Converts linear to Decibels.
   * Accepted values for conversion are between minimum linear value (greater than zero and
   * maximum linear value. -inf is also accepted and it converts to 0.
   *
   * \param linear value in linear to convert
   *
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

  /**
   * Gets packet information in std::string for printing purposes
   *
   * \param p Packet
   * \return Packet information in std::string
   */
  static inline std::string GetPacketInfo (const Ptr<const Packet> p)
  {
    std::ostringstream oss;
    oss << p->GetUid () << " ";
    SatMacTag tag;
    if (p->PeekPacketTag (tag))
      {
        oss << Mac48Address::ConvertFrom(tag.GetSourceAddress ()) << " ";
        oss << Mac48Address::ConvertFrom(tag.GetDestAddress ()) << " ";
      }
    return oss.str ();
  }

  /**
   * Gets packet information in std::string for printing purposes
   *
   * \param packets A vector of packets
   * \return Packet information in std::string
   */
  static inline std::string GetPacketInfo (const std::vector< Ptr<Packet> > packets)
  {
    std::ostringstream oss;
    for (std::vector< Ptr<Packet> >::const_iterator it = packets.begin ();
         it != packets.end ();
         ++it)
      {
        oss << GetPacketInfo (*it);
      }
    return oss.str ();
  }

  /**
   * Gets the modulated bits of a certain modcod
   *
   * \param modcod Modulation and coding scheme
   * \return uint32_t modulated bits
   */
  static inline uint32_t GetModulatedBits (SatEnums::SatModcod_t modcod)
  {
    switch (modcod)
    {
      case SatEnums::SAT_MODCOD_QPSK_1_TO_2:
      case SatEnums::SAT_MODCOD_QPSK_2_TO_3:
      case SatEnums::SAT_MODCOD_QPSK_3_TO_4:
      case SatEnums::SAT_MODCOD_QPSK_3_TO_5:
      case SatEnums::SAT_MODCOD_QPSK_4_TO_5:
      case SatEnums::SAT_MODCOD_QPSK_5_TO_6:
      case SatEnums::SAT_MODCOD_QPSK_8_TO_9:
      case SatEnums::SAT_MODCOD_QPSK_9_TO_10:
        {
          return 2;
          break;
        }
      case SatEnums::SAT_MODCOD_8PSK_2_TO_3:
      case SatEnums::SAT_MODCOD_8PSK_3_TO_4:
      case SatEnums::SAT_MODCOD_8PSK_3_TO_5:
      case SatEnums::SAT_MODCOD_8PSK_5_TO_6:
      case SatEnums::SAT_MODCOD_8PSK_8_TO_9:
      case SatEnums::SAT_MODCOD_8PSK_9_TO_10:
      {
         return 3;
         break;
       }
      case SatEnums::SAT_MODCOD_16APSK_2_TO_3:
      case SatEnums::SAT_MODCOD_16APSK_3_TO_4:
      case SatEnums::SAT_MODCOD_16APSK_4_TO_5:
      case SatEnums::SAT_MODCOD_16APSK_5_TO_6:
      case SatEnums::SAT_MODCOD_16APSK_8_TO_9:
      case SatEnums::SAT_MODCOD_16APSK_9_TO_10:
        {
           return 4;
           break;
         }
      case SatEnums::SAT_MODCOD_32APSK_3_TO_4:
      case SatEnums::SAT_MODCOD_32APSK_4_TO_5:
      case SatEnums::SAT_MODCOD_32APSK_5_TO_6:
      case SatEnums::SAT_MODCOD_32APSK_8_TO_9:
        {
           return 5;
           break;
         }
      default:
        {
          NS_FATAL_ERROR ("Unsupported enum SatModcod_t!");
          break;
        }
    }
    return 0;
  }

  /**
   * Gets the coding rate of a certain modcod
   *
   * \param modcod Modulation and coding scheme
   * \return double coding rate
   */
  static inline double GetCodingRate (SatEnums::SatModcod_t modcod)
  {
    switch (modcod)
    {
      case SatEnums::SAT_MODCOD_QPSK_1_TO_2:
      {
        return 1.0/2.0;
        break;
      }
      case SatEnums::SAT_MODCOD_QPSK_2_TO_3:
      case SatEnums::SAT_MODCOD_8PSK_2_TO_3:
      case SatEnums::SAT_MODCOD_16APSK_2_TO_3:
        {
          return 2.0/3.0;
          break;
        }
      case SatEnums::SAT_MODCOD_QPSK_3_TO_4:
      case SatEnums::SAT_MODCOD_8PSK_3_TO_4:
      case SatEnums::SAT_MODCOD_16APSK_3_TO_4:
      case SatEnums::SAT_MODCOD_32APSK_3_TO_4:
        {
          return 3.0/4.0;
          break;
        }
      case SatEnums::SAT_MODCOD_QPSK_3_TO_5:
      case SatEnums::SAT_MODCOD_8PSK_3_TO_5:
        {
          return 3.0/5.0;
          break;
        }
      case SatEnums::SAT_MODCOD_QPSK_4_TO_5:
      case SatEnums::SAT_MODCOD_16APSK_4_TO_5:
      case SatEnums::SAT_MODCOD_32APSK_4_TO_5:
        {
          return 4.0/5.0;
          break;
        }
      case SatEnums::SAT_MODCOD_QPSK_5_TO_6:
      case SatEnums::SAT_MODCOD_8PSK_5_TO_6:
      case SatEnums::SAT_MODCOD_16APSK_5_TO_6:
      case SatEnums::SAT_MODCOD_32APSK_5_TO_6:
        {
          return 5.0/6.0;
          break;
        }
      case SatEnums::SAT_MODCOD_QPSK_8_TO_9:
      case SatEnums::SAT_MODCOD_8PSK_8_TO_9:
      case SatEnums::SAT_MODCOD_16APSK_8_TO_9:
      case SatEnums::SAT_MODCOD_32APSK_8_TO_9:
      {
        return 8.0/9.0;
        break;
      }
      case SatEnums::SAT_MODCOD_QPSK_9_TO_10:
      case SatEnums::SAT_MODCOD_8PSK_9_TO_10:
      case SatEnums::SAT_MODCOD_16APSK_9_TO_10:
        {
          return 9.0/10.0;
          break;
        }
      default:
        {
          NS_FATAL_ERROR ("Unsupported enum SatModcod_t!");
          break;
        }
    }
    return 0.0;
  }

  /**
   * Simple linear interpolation
   * y = y0 + (y1-y0)*(x-x0)/(x1-x0)
   * http://en.wikipedia.org/wiki/Linear_interpolation
   */
  static inline double Interpolate (double x, double x0, double x1, double y0, double y1)
  {
    double dX = x1 - x0;
    double dY = y1 - y0;
    double relX = x - x0;
    double relY = (dY / dX) * relX;
    return y0 + relY;
  }
  
private:
  /**
   * Destructor
   *
   * Made Pure Virtual because the class is not planned be instantiated or inherited
   *
   */
  virtual ~SatUtils() = 0;
};

} // namespace ns3

#endif /* SATELLITE_UTILS_H */
