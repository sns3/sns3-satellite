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
#include <ns3/packet.h>
#include <ns3/mac48-address.h>
#include <ns3/satellite-mac-tag.h>
#include <ns3/satellite-enums.h>

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief SatUtils class is for general conversions used in satellite module.
 * This class is not planned to be instantiated or inherited.
 */
class SatUtils
{
public:
  /**
   * \brief Converts radians to degrees
   *
   * \param radian value to convert as radians
   * \return degrees converted from radians
   */
  template <typename T>
  static inline T RadiansToDegrees ( T radian )
  {
    return (T) ( ( radian ) * ( 180.0 / M_PI ) );
  }

  /**
   * \brief Converts degrees to radians
   *
   * \param degree value to convert as degrees
   * \return radians converted from degrees
   */
  template <typename T>
  static inline T DegreesToRadians ( T degree )
  {
    return (T) ( ( degree ) * ( M_PI / 180.0 ) );
  }

  /**
   * \brief Get minimum value for linear. (The smallest value greater than zero)
   *
   * \return minimum linear value
   */
  template <typename T>
  static inline T MinLin ()
  {
    return (T) ( std::numeric_limits<T>::min () );
  }

  /**
   * \brief Get maximum value for linear
   *
   * \return maximum linear value
   */
  template <typename T>
  static inline T MaxLin ()
  {
    return (T) ( std::numeric_limits<T>::max () );
  }

  /**
   * \brief Get minimum value for Decibel
   *
   * \return minimum Decibel value
   */
  template <typename T>
  static inline T MinDb ()
  {
    return (T) LinearToDb ( MinLin<T> () );
  }

  /**
   * \brief Get maximum value for Decibel
   *
   * \return maximum Decibel value
   */
  template <typename T>
  static inline T MaxDb ()
  {
    return (T) LinearToDb ( MaxLin<T> () );
  }

  /**
   * \brief Converts Decibel Watts to Watts
   *
   * \param dbw value in Decibel Watts to convert
   * \return Watts converted from Decibel Watts
   */
  template <typename T>
  static inline T DbWToW ( T dbw )
  {
    return (T) DbToLinear<T> (dbw);
  }

  /**
   * \brief Converts Watts to Decibel Watts
   *
   * \param w value in Watts to convert
   * \return Decibel Watts converted from Watts
   */
  template <typename T>
  static inline T WToDbW ( T w )
  {
    return (T) LinearToDb<T> (w);
  }

  /**
   * \brief Converts decibels to linear.
   * Accepted values for conversion are between minimum decibel value and
   * maximum decibel value. Negative infinity is also an accepted value
   * and it is converted to zero.
   *
   * \param db value in Decibels to convert
   * \return linear converted from Decibels
   */
  template <typename T>
  static inline T DbToLinear ( T db )
  {
    if ((db < MinDb<T> () && (std::isinf (-db) != false)) || db > MaxDb<T> ())
      {
        NS_FATAL_ERROR ("SatUtils::DbToLinear - unsupported value: " <<  db);
      }

    return (T) std::pow ( 10.0, db / 10.0 );
  }

  /**
   * \brief Converts linear to decibels.
   * Accepted values for conversion are between minimum linear value (greater than zero and
   * maximum linear value. Zero is also accepted and it is converted to -INF.
   *
   * \param linear value to convert
   * \return Decibels converted from linear
   */
  template <typename T>
  static inline T LinearToDb ( T linear )
  {
    if ((linear < MinLin<T> () && linear != 0) || linear > MaxLin<T> ())
      {
        NS_FATAL_ERROR ("SatUtils::LinearToDb - unsupported value: " <<  linear);
      }

    return (T) (10.0 * std::log10 ( linear ));
  }

  /**
   * \brief Converts Decibel milli Watts to Watts
   *
   * \param dbm value in Decibel milli Watts to convert
   * \return Watts converted from Decibel milli Watts
   */
  template <typename T>
  static inline T DbmToW ( T dbm )
  {
    return ( DbWToW<T> ( dbm ) / 1000 );
  }

  /**
   * \brief Converts Watts to Decibel milli Watts
   *
   * \param w value in Watts to convert
   * \return Decibel milli Watts converted from Watts
   */
  template <typename T>
  static inline T WToDbm ( T w )
  {
    return (T) ( WToDbW<T> ( w * 1000.0 ) );
  }

  /**
   * \brief Get packet information in std::string for printing purposes
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
        oss << tag.GetSourceAddress () << " ";
        oss << tag.GetDestAddress () << " ";
      }
    return oss.str ();
  }

  /**
   * \brief Gets packet information in std::string for printing purposes
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
   * \brief Get the modulated bits of a certain MODCOD
   *
   * \param modcod Modulation and coding scheme
   * \return Modulated bits
   */
  static inline uint32_t GetModulatedBits (SatEnums::SatModcod_t modcod)
  {
    switch (modcod)
      {
      //DVB-S2
      case SatEnums::SAT_MODCOD_QPSK_1_TO_3:
      case SatEnums::SAT_MODCOD_QPSK_1_TO_2:
      case SatEnums::SAT_MODCOD_QPSK_3_TO_5:
      case SatEnums::SAT_MODCOD_QPSK_2_TO_3:
      case SatEnums::SAT_MODCOD_QPSK_3_TO_4:
      case SatEnums::SAT_MODCOD_QPSK_4_TO_5:
      case SatEnums::SAT_MODCOD_QPSK_5_TO_6:
      case SatEnums::SAT_MODCOD_QPSK_8_TO_9:
      case SatEnums::SAT_MODCOD_QPSK_9_TO_10:
        {
          return 2;
          break;
        }
      case SatEnums::SAT_MODCOD_8PSK_3_TO_5:
      case SatEnums::SAT_MODCOD_8PSK_2_TO_3:
      case SatEnums::SAT_MODCOD_8PSK_3_TO_4:
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
      case SatEnums::SAT_MODCOD_16QAM_3_TO_4:
      case SatEnums::SAT_MODCOD_16QAM_5_TO_6:
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
      //DVB-S2X
      case SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_PILOTS:
        {
          return 2;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_PILOTS:
        {
          return 3;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_PILOTS:
        {
          return 4;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_PILOTS:
        {
          return 5;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_PILOTS:
        {
          return 6;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_PILOTS:
        {
          return 7;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_PILOTS:
        {
          return 8;
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
   * \brief Gets the coding rate of a certain MODCOD
   *
   * \param modcod Modulation and coding scheme
   * \return double coding rate
   */
  static inline double GetCodingRate (SatEnums::SatModcod_t modcod)
  {
    switch (modcod)
      {
      // DVB-S2
      case SatEnums::SAT_MODCOD_QPSK_1_TO_3:
        {
          return 1.0 / 3.0;
          break;
        }
      case SatEnums::SAT_MODCOD_QPSK_1_TO_2:
        {
          return 1.0 / 2.0;
          break;
        }
      case SatEnums::SAT_MODCOD_QPSK_2_TO_3:
      case SatEnums::SAT_MODCOD_8PSK_2_TO_3:
      case SatEnums::SAT_MODCOD_16APSK_2_TO_3:
        {
          return 2.0 / 3.0;
          break;
        }
      case SatEnums::SAT_MODCOD_QPSK_3_TO_4:
      case SatEnums::SAT_MODCOD_8PSK_3_TO_4:
      case SatEnums::SAT_MODCOD_16APSK_3_TO_4:
      case SatEnums::SAT_MODCOD_32APSK_3_TO_4:
      case SatEnums::SAT_MODCOD_16QAM_3_TO_4:
        {
          return 3.0 / 4.0;
          break;
        }
      case SatEnums::SAT_MODCOD_QPSK_3_TO_5:
      case SatEnums::SAT_MODCOD_8PSK_3_TO_5:
        {
          return 3.0 / 5.0;
          break;
        }
      case SatEnums::SAT_MODCOD_QPSK_4_TO_5:
      case SatEnums::SAT_MODCOD_16APSK_4_TO_5:
      case SatEnums::SAT_MODCOD_32APSK_4_TO_5:
        {
          return 4.0 / 5.0;
          break;
        }
      case SatEnums::SAT_MODCOD_QPSK_5_TO_6:
      case SatEnums::SAT_MODCOD_8PSK_5_TO_6:
      case SatEnums::SAT_MODCOD_16APSK_5_TO_6:
      case SatEnums::SAT_MODCOD_32APSK_5_TO_6:
      case SatEnums::SAT_MODCOD_16QAM_5_TO_6:
        {
          return 5.0 / 6.0;
          break;
        }
      case SatEnums::SAT_MODCOD_QPSK_8_TO_9:
      case SatEnums::SAT_MODCOD_8PSK_8_TO_9:
      case SatEnums::SAT_MODCOD_16APSK_8_TO_9:
      case SatEnums::SAT_MODCOD_32APSK_8_TO_9:
        {
          return 8.0 / 9.0;
          break;
        }
      case SatEnums::SAT_MODCOD_QPSK_9_TO_10:
      case SatEnums::SAT_MODCOD_8PSK_9_TO_10:
      case SatEnums::SAT_MODCOD_16APSK_9_TO_10:
        {
          return 9.0 / 10.0;
          break;
        }
      // DVB-S2X
      case SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_NOPILOTS:
        {
          return 11.0 / 45.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_PILOTS:
        {
          return 1.0 / 4.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_PILOTS:
        {
          return 4.0 / 15.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_PILOTS:
        {
          return 13.0 / 45.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_NOPILOTS:
        {
          return 14.0 / 45.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_PILOTS:
        {
          return 1.0 / 3.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_NOPILOTS:
        {
          return 2.0 / 5.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_NOPILOTS:
        {
          return 9.0 / 20.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_NOPILOTS:
        {
          return 7.0 / 15.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_PILOTS:
        {
          return 1.0 / 2.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_NOPILOTS:
        {
          return 8.0 / 15.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_NOPILOTS:
        {
          return 11.0 / 20.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_NOPILOTS:
        {
          return 5.0 / 9.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_PILOTS:
        {
          return 26.0 / 45.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_PILOTS:
        {
          return 3.0 / 5.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_PILOTS:
        {
          return 28.0 / 45.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_NOPILOTS:
        {
          return 23.0 / 36.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_NOPILOTS:
        {
          return 29.0 / 45.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_NOPILOTS:
        {
          return 2.0 / 3.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_NOPILOTS:
        {
          return 31.0 / 45.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_PILOTS:
        {
          return 25.0 / 36.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_NOPILOTS:
        {
          return 32.0 / 45.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_NOPILOTS:
        {
          return 13.0 / 18.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_PILOTS:
        {
          return 11.0 / 15.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_NOPILOTS:
        {
          return 3.0 / 4.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_PILOTS:
        {
          return 7.0 / 9.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_PILOTS:
        {
          return 4.0 / 5.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_NOPILOTS:
        {
          return 5.0 / 6.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_PILOTS:
        {
          return 77.0 / 90.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_NOPILOTS:
        {
          return 8.0 / 9.0;
          break;
        }
      case SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_PILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_NOPILOTS:
      case SatEnums::SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_PILOTS:
        {
          return 9.0 / 10.0;
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

  static inline SatEnums::SatBbFrameType_t GetFrameTypeS2X (SatEnums::SatModcod_t modcod)
  {
    switch (modcod)
      {
        case SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_PILOTS:
          {
            return SatEnums::SHORT_FRAME;
            break;
          }
        case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_PILOTS:
        case SatEnums::SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_NOPILOTS:
        case SatEnums::SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_PILOTS:
          {
            return SatEnums::NORMAL_FRAME;
            break;
          }
        default:
          {
            NS_FATAL_ERROR ("Frame type not found");
            break;
          }
      }

    return SatEnums::NORMAL_FRAME;
  }

  /**
   * \brief Simple linear interpolation.
   * y = y0 + (y1-y0)*(x-x0)/(x1-x0)
   * http://en.wikipedia.org/wiki/Linear_interpolation
   * \param x Interpolated x
   * \param x0 Lower x value
   * \param x1 Higher x value
   * \param y0 Lower y value
   * \param y1 Higher y value
   * \return Interpolated value
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
  virtual ~SatUtils () = 0;
};

} // namespace ns3

#endif /* SATELLITE_UTILS_H */
