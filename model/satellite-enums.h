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
   * \enum ChannelType_t
   * \brief Types of channel.
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
   * \enum PropagationDelayModel_t
   * \brief Propagation delay model.
   */
  typedef enum
  {
    PD_CONSTANT = 0,
    PD_CONSTANT_SPEED
  } PropagationDelayModel_t;

  /**
   * \enum CarrierBandwidthType_t
   * \brief Types of bandwidth.
   */
  typedef enum
  {
    ALLOCATED_BANDWIDTH,
    OCCUPIED_BANDWIDTH,
    EFFECTIVE_BANDWIDTH
  } CarrierBandwidthType_t;

  /**
   * \enum FadingModel_t
   * \brief Fading models
   */
  typedef enum
  {
    FADING_OFF,
    FADING_TRACE,
    FADING_MARKOV
  } FadingModel_t;

  /**
   * \enum SatModcod_t
   * \brief Modulation scheme and coding rate for DVB-S2.
   */
  typedef enum
  {
    SAT_NONVALID_MODCOD = 0,
    SAT_MODCOD_QPSK_1_TO_3,
    SAT_MODCOD_QPSK_1_TO_2,
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
    SAT_MODCOD_16QAM_3_TO_4,
    SAT_MODCOD_16QAM_5_TO_6,
    SAT_MODCOD_32APSK_3_TO_4,
    SAT_MODCOD_32APSK_4_TO_5,
    SAT_MODCOD_32APSK_5_TO_6,
    SAT_MODCOD_32APSK_8_TO_9
  } SatModcod_t;


  static inline void GetAvailableModcodsFwdLink (std::vector<SatModcod_t>& modcods)
  {
    modcods.push_back (SAT_MODCOD_QPSK_1_TO_2);
    modcods.push_back (SAT_MODCOD_QPSK_2_TO_3);
    modcods.push_back (SAT_MODCOD_QPSK_3_TO_4);
    modcods.push_back (SAT_MODCOD_QPSK_3_TO_5);
    modcods.push_back (SAT_MODCOD_QPSK_4_TO_5);
    modcods.push_back (SAT_MODCOD_QPSK_5_TO_6);
    modcods.push_back (SAT_MODCOD_QPSK_8_TO_9);
    modcods.push_back (SAT_MODCOD_QPSK_9_TO_10);
    modcods.push_back (SAT_MODCOD_8PSK_2_TO_3);
    modcods.push_back (SAT_MODCOD_8PSK_3_TO_4);
    modcods.push_back (SAT_MODCOD_8PSK_3_TO_5);
    modcods.push_back (SAT_MODCOD_8PSK_5_TO_6);
    modcods.push_back (SAT_MODCOD_8PSK_8_TO_9);
    modcods.push_back (SAT_MODCOD_8PSK_9_TO_10);
    modcods.push_back (SAT_MODCOD_16APSK_2_TO_3);
    modcods.push_back (SAT_MODCOD_16APSK_3_TO_4);
    modcods.push_back (SAT_MODCOD_16APSK_4_TO_5);
    modcods.push_back (SAT_MODCOD_16APSK_5_TO_6);
    modcods.push_back (SAT_MODCOD_16APSK_8_TO_9);
    modcods.push_back (SAT_MODCOD_16APSK_9_TO_10);
    modcods.push_back (SAT_MODCOD_32APSK_3_TO_4);
    modcods.push_back (SAT_MODCOD_32APSK_4_TO_5);
    modcods.push_back (SAT_MODCOD_32APSK_5_TO_6);
    modcods.push_back (SAT_MODCOD_32APSK_8_TO_9);
  };

  static inline void GetAvailableModcodsRtnLink (std::vector<SatModcod_t>& modcods)
  {
    modcods.push_back (SAT_MODCOD_QPSK_1_TO_3);
    modcods.push_back (SAT_MODCOD_QPSK_1_TO_2);
    modcods.push_back (SAT_MODCOD_QPSK_2_TO_3);
    modcods.push_back (SAT_MODCOD_QPSK_3_TO_4);
    modcods.push_back (SAT_MODCOD_QPSK_4_TO_5);
    modcods.push_back (SAT_MODCOD_QPSK_5_TO_6);;
    modcods.push_back (SAT_MODCOD_8PSK_2_TO_3);
    modcods.push_back (SAT_MODCOD_8PSK_3_TO_4);
    modcods.push_back (SAT_MODCOD_8PSK_5_TO_6);
    modcods.push_back (SAT_MODCOD_16QAM_3_TO_4);
    modcods.push_back (SAT_MODCOD_16QAM_5_TO_6);
  };

  static inline std::string GetModcodTypeName (SatModcod_t modcod)
  {
    switch (modcod)
    {
      case SAT_NONVALID_MODCOD:
        return "SAT_NONVALID_MODCOD";
      case SAT_MODCOD_QPSK_1_TO_3:
        return "QPSK_1_TO_3";
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
      case SAT_MODCOD_16QAM_3_TO_4:
        return "16QAM_3_TO_4";
      case SAT_MODCOD_16QAM_5_TO_6:
        return "16QAM_5_TO_6";
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
    NORMAL_FRAME = 1,
    DUMMY_FRAME = 2
  } SatBbFrameType_t;


  /**
   * \brief Definition for different types of Capacity Request (CR) messages.
   */
  typedef enum
  {
    DA_UNKNOWN = 0,
    DA_RBDC = 1,
    DA_VBDC = 2,
    DA_AVBDC = 3
  } SatCapacityAllocationCategory_t;


  static inline void GetAvailableBbFrameTypes (std::vector<SatBbFrameType_t>& frameTypes)
  {
    frameTypes.push_back (SHORT_FRAME);
    frameTypes.push_back (NORMAL_FRAME);
  };

  /**
   * \enum RxPowerCalculationMode_t
   * \brief Modes of Rx power calculation.
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
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetChannelTypeName - Invalid channel type");
          break;
        }
    }
    NS_FATAL_ERROR ("SatEnums::GetChannelTypeName - Invalid channel type");
    return "";
  };

  /**
   * \enum SatPacketEvent_t
   *
   * \brief Packet event used for packet tracing
   */
  typedef enum
  {
    PACKET_SENT = 0,
    PACKET_RECV = 1,
    PACKET_ENQUE = 2,
    PACKET_DROP = 3
  } SatPacketEvent_t;

  static inline std::string GetPacketEventName (SatPacketEvent_t packetEvent)
  {
    switch (packetEvent)
    {
      case PACKET_SENT:
        {
          return "SND";
        }
      case PACKET_RECV:
        {
          return "RCV";
        }
      case PACKET_ENQUE:
        {
          return "ENQ";
        }
      case PACKET_DROP:
        {
          return "DRP";
        }
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetPacketEventName - Invalid packet event");
          break;
        }
    }
    NS_FATAL_ERROR ("SatEnums::GetPacketEventName - Invalid packet event");
    return "";
  };

  /**
   * \enum SatNodeType_t
   *
   * \brief Node type used for packet tracing
   */
  typedef enum
  {
    NT_UT = 0,
    NT_SAT = 1,
    NT_GW = 2,
    NT_NCC = 3,
    NT_TER = 4,
    NT_UNDEFINED = 5
  } SatNodeType_t;

  static inline std::string GetNodeTypeName (SatNodeType_t nodeType)
  {
    switch (nodeType)
    {
      case NT_UT:
        {
          return "UT";
        }
      case NT_SAT:
        {
          return "SAT";
        }
      case NT_GW:
        {
          return "GW";
        }
      case NT_NCC:
        {
          return "NCC";
        }
      case NT_TER:
        {
          return "TER";
        }
      case NT_UNDEFINED:
        {
          return "UNDEF";
        }
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetNodeTypeName - Invalid node type");
          break;
        }
    }
    NS_FATAL_ERROR ("SatEnums::GetNodeTypeName - Invalid node type");
    return "";
  };

  /**
   * \enum SatLinkDir_t
   *
   * \brief Link direction used for packet tracing
   */
  typedef enum
  {
    LD_FORWARD = 0,
    LD_RETURN = 1,
    LD_UNDEFINED = 2
  } SatLinkDir_t;

  static inline std::string GetLinkDirName (SatLinkDir_t linkDir)
  {
    switch (linkDir)
    {
      case LD_FORWARD:
        {
          return "FWD";
        }
      case LD_RETURN:
        {
          return "RTN";
        }
      case LD_UNDEFINED:
        {
          return "UNDEF";
        }
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetLinkDirName - Invalid link direction");
          break;
        }
    }
    NS_FATAL_ERROR ("SatEnums::GetLinkDirName - Invalid link direction");
    return "";
  };

  /**
   * \enum SatLogLevel_t
   *
   * \brief Log level used for packet tracing
   */
  typedef enum
  {
    LL_ND = 0,
    LL_LLC = 1,
    LL_MAC = 2,
    LL_PHY = 3,
    LL_CH = 4,
  } SatLogLevel_t;

  static inline std::string GetLogLevelName (SatLogLevel_t logLevel)
  {
    switch (logLevel)
    {
      case LL_ND:
        {
          return "ND";
        }
      case LL_LLC:
        {
          return "LLC";
        }
      case LL_MAC:
        {
          return "MAC";
        }
      case LL_PHY:
        {
          return "PHY";
        }
      case LL_CH:
        {
          return "CH";
        }
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetLogLevelName - Invalid log level");
          break;
        }
    }
    NS_FATAL_ERROR ("SatEnums::GetLogLevelName - Invalid log level");
    return "";
  };

  /**
   * \enum RandomAccessTriggerType_t
   * \brief The defined random access trigger types. These help determine
   * which algorithm to use if multiple algorithms are enabled
   */
  typedef enum
  {
    RA_SLOTTED_ALOHA_TRIGGER = 0,
    RA_CRDSA_TRIGGER = 1
  } RandomAccessTriggerType_t;

  static inline std::string GetRandomAccessTriggerTypeName (RandomAccessTriggerType_t triggerType)
  {
    switch (triggerType)
    {
      case RA_SLOTTED_ALOHA_TRIGGER:
        {
          return "SLOTTED_ALOHA_TRIGGER";
        }
      case RA_CRDSA_TRIGGER:
        {
          return "RA_CRDSA_TRIGGER";
        }
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetRandomAccessTriggerTypeName - Invalid trigger type");
          break;
        }
    }
    NS_FATAL_ERROR ("SatEnums::GetRandomAccessTriggerTypeName - Invalid trigger type");
    return "";
  };

  /**
   * \enum RandomAccessModel_t
   * \brief The defined random access models. These define the implemented algorithms
   */
  typedef enum
  {
    RA_OFF = 0,
    RA_SLOTTED_ALOHA = 1,
    RA_CRDSA = 2,
    RA_ANY_AVAILABLE = 3
  } RandomAccessModel_t;

  static inline std::string GetRandomAccessModelName (RandomAccessModel_t model)
  {
    switch (model)
    {
      case RA_OFF:
        {
          return "RA_OFF";
        }
      case RA_SLOTTED_ALOHA:
        {
          return "RA_SLOTTED_ALOHA";
        }
      case RA_CRDSA:
        {
          return "RA_CRDSA";
        }
      case RA_ANY_AVAILABLE:
        {
          return "RA_ANY_AVAILABLE";
        }
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetRandomAccessModelName - Invalid model");
          break;
        }
    }
    NS_FATAL_ERROR ("SatEnums::GetRandomAccessModelName - Invalid model");
    return "";
  };

  /**
   * \enum RandomAccessTxOpportunityType_t
   * \brief Random access Tx opportunity types. These are used to define to which algorithm
   * the results provided by this module applies to
   */
  typedef enum
  {
    RA_DO_NOTHING = 0,
    RA_SLOTTED_ALOHA_TX_OPPORTUNITY = 1,
    RA_CRDSA_TX_OPPORTUNITY = 2,
  } RandomAccessTxOpportunityType_t;

  static inline std::string GetRandomAccessOpportunityTypeName (RandomAccessTxOpportunityType_t opportunityType)
  {
    switch (opportunityType)
    {
      case RA_DO_NOTHING:
        {
          return "RA_DO_NOTHING";
        }
      case RA_SLOTTED_ALOHA_TX_OPPORTUNITY:
        {
          return "RA_SLOTTED_ALOHA_TX_OPPORTUNITY";
        }
      case RA_CRDSA_TX_OPPORTUNITY:
        {
          return "RA_CRDSA_TX_OPPORTUNITY";
        }
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetRandomAccessOpportunityTypeName - Invalid opportunity type");
          break;
        }
    }
    NS_FATAL_ERROR ("SatEnums::GetRandomAccessOpportunityTypeName - Invalid opportunity type");
    return "";
  };

  /**
   * \enum PacketType_t
   * \brief Packet types. These are used determine Rx side actions
   */
  typedef enum
  {
    DEDICATED_ACCESS_PACKET = 0,
    SLOTTED_ALOHA_PACKET = 1,
    CRDSA_PACKET = 2,
  } PacketType_t;

  static inline std::string GetPacketTypeName (PacketType_t packetType)
  {
    switch (packetType)
    {
      case DEDICATED_ACCESS_PACKET:
        {
          return "DEDICATED_ACCESS_PACKET";
        }
      case SLOTTED_ALOHA_PACKET:
        {
          return "SLOTTED_ALOHA_PACKET";
        }
      case CRDSA_PACKET:
        {
          return "CRDSA_PACKET";
        }
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetPacketTypeName - Invalid packet type");
          break;
        }
    }
    NS_FATAL_ERROR ("SatEnums::GetPacketTypeName - Invalid packet type");
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
