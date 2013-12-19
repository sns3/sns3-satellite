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
 * Author: Frans Laakso <frans.laakso@magister.fi>
 */
#ifndef SATELLITE_ENUMS_H
#define SATELLITE_ENUMS_H

#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

/**
 * \ingroup satellite
 * 
 * \brief SatEnums class is for general enumerators used in satellite module.
 */
class SatEnums
{
public:

  /**
   * Possible types of channel.
   */
  typedef enum
  {
    UNKNOWN_CH = 0,
    FORWARD_FEEDER_CH = 1,
    FORWARD_USER_CH = 2,
    RETURN_USER_CH = 3,
    RETURN_FEEDER_CH = 4
  } ChannelType_t;

  /**
   * Possible types of bandwidth.
   */
  typedef enum
  {
    ALLOCATED_BANDWIDTH,
    OCCUPIED_BANDWIDTH,
    EFFECTIVE_BANDWIDTH
  } CarrierBandwidthType_t;

  /**
   * Possible fading models
   */
  typedef enum
  {
    FADING_OFF, FADING_TRACE, FADING_MARKOV
  } FadingModel_t;


  /**
   * \enum SatModcod_t
   *
   * \brief Modulation scheme and coding rate for DVB-S2.
   */
  typedef enum
  {
    SAT_MODCOD_QPSK_1_TO_2 = 0,
    SAT_MODCOD_QPSK_2_TO_3,
    SAT_MODCOD_QPSK_3_TO_4,
    SAT_MODCOD_QPSK_3_TO_5,
    SAT_MODCOD_QPSK_4_TO_5,
    SAT_MODCOD_QPSK_5_TO_6,
    SAT_MODCOD_QPSK_8_TO_9,
    SAT_MODCOD_QPSK_9_TO_10,
    SAT_MODCOD_8PSK_2_TO_3,
    SAT_MODCOD_8PSK_3_TO_4,
    SAT_MODCOD_8PSK_3_TO_5,
    SAT_MODCOD_8PSK_5_TO_6,
    SAT_MODCOD_8PSK_8_TO_9,
    SAT_MODCOD_8PSK_9_TO_10,
    SAT_MODCOD_16APSK_2_TO_3,
    SAT_MODCOD_16APSK_3_TO_4,
    SAT_MODCOD_16APSK_4_TO_5,
    SAT_MODCOD_16APSK_5_TO_6,
    SAT_MODCOD_16APSK_8_TO_9,
    SAT_MODCOD_16APSK_9_TO_10,
    SAT_MODCOD_32APSK_3_TO_4,
    SAT_MODCOD_32APSK_4_TO_5,
    SAT_MODCOD_32APSK_5_TO_6,
    SAT_MODCOD_32APSK_8_TO_9
  } SatModcod_t;


  static inline std::string GetModcodTypeName (SatModcod_t modcod)
  {
    switch (modcod)
    {
      case SAT_MODCOD_QPSK_1_TO_2:
        return "QPSK_1_TO_2";
      case SAT_MODCOD_QPSK_2_TO_3:
        return "QPSK_2_TO_3";
      case SAT_MODCOD_QPSK_3_TO_4:
        return "QPSK_3_TO_4";
      case SAT_MODCOD_QPSK_3_TO_5:
        return "QPSK_3_TO_5";
      case SAT_MODCOD_QPSK_4_TO_5:
        return "QPSK_4_TO_5";
      case SAT_MODCOD_QPSK_5_TO_6:
        return "QPSK_5_TO_6";
      case SAT_MODCOD_QPSK_8_TO_9:
        return "QPSK_8_TO_9";
      case SAT_MODCOD_QPSK_9_TO_10:
        return "QPSK_9_TO_10";
      case SAT_MODCOD_8PSK_2_TO_3:
        return "8PSK_2_TO_3";
      case SAT_MODCOD_8PSK_3_TO_4:
        return "8PSK_3_TO_4";
      case SAT_MODCOD_8PSK_3_TO_5:
        return "8PSK_3_TO_5";
      case SAT_MODCOD_8PSK_5_TO_6:
        return "8PSK_5_TO_6";
      case SAT_MODCOD_8PSK_8_TO_9:
        return "8PSK_8_TO_9";
      case SAT_MODCOD_8PSK_9_TO_10:
        return "8PSK_9_TO_10";
      case SAT_MODCOD_16APSK_2_TO_3:
        return "16APSK_2_TO_3";
      case SAT_MODCOD_16APSK_3_TO_4:
        return "16APSK_3_TO_4";
      case SAT_MODCOD_16APSK_4_TO_5:
        return "16APSK_4_TO_5";
      case SAT_MODCOD_16APSK_5_TO_6:
        return "16APSK_5_TO_6";
      case SAT_MODCOD_16APSK_8_TO_9:
        return "16APSK_8_TO_9";
      case SAT_MODCOD_16APSK_9_TO_10:
        return "16APSK_9_TO_10";
      case SAT_MODCOD_32APSK_3_TO_4:
        return "32APSK_3_TO_4";
      case SAT_MODCOD_32APSK_4_TO_5:
        return "32APSK_4_TO_5";
      case SAT_MODCOD_32APSK_5_TO_6:
        return "32APSK_5_TO_6";
      case SAT_MODCOD_32APSK_8_TO_9:
        return "32APSK_8_TO_9";
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetModcodTypeName - Invalid modcod type");
          break;
        }
    }
    NS_FATAL_ERROR ("SatEnums::GetModcodTypeName - Invalid modcod type");
    return "";
  };

  /**
   * \enum SatBbFrameType_t
   *
   * \brief BB frame type used in DVB-S2 FWD link
   */
  typedef enum
  {
    SHORT_FRAME = 0,
    NORMAL_FRAME = 1
  } SatBbFrameType_t;


  /**
   * Possible modes of Rx power calculation.
   */
  typedef enum
  {
    RX_PWR_CALCULATION = 0,
    RX_PWR_INPUT_TRACE = 1,
  } RxPowerCalculationMode_t;

  static inline std::string GetChannelTypeName (ChannelType_t channelType)
  {
    switch (channelType)
    {
      case UNKNOWN_CH:
        {
          return "UNKNOWN_CH";
        }
      case FORWARD_FEEDER_CH:
        {
          return "FORWARD_FEEDER_CH";
        }
      case FORWARD_USER_CH:
        {
          return "FORWARD_USER_CH";
        }
      case RETURN_USER_CH:
        {
          return "RETURN_USER_CH";
        }
      case RETURN_FEEDER_CH:
        {
          return "RETURN_FEEDER_CH";
        }
      default :
        {
          NS_FATAL_ERROR ("SatEnums::GetChannelTypeName - Invalid channel type");
        }
    }
    NS_FATAL_ERROR ("SatEnums::GetChannelTypeName - Invalid channel type");
    return "";
  };

private:

  /**
   * Destructor
   *
   * Made Pure Virtual because the class is not planned be instantiated or inherited
   *
   */
  virtual ~SatEnums() = 0;
};

} // namespace ns3

#endif /* SATELLITE_ENUMS_H */
