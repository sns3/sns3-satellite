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

#include <string>
#include <vector>

namespace ns3 {

/**
 * \ingroup satellite
 *
 * \brief SatEnums class is for simplifying the use of enumerators
 * in the satellite module. The enums specified in this class mainly
 * consist of enums used in multiple classes.
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
    SAT_MODCOD_QPSK_1_TO_3,    // 0.33
    SAT_MODCOD_QPSK_1_TO_2,    // 0.50
    SAT_MODCOD_QPSK_3_TO_5,    // 0.60
    SAT_MODCOD_QPSK_2_TO_3,    // 0.67
    SAT_MODCOD_QPSK_3_TO_4,    // 0.75
    SAT_MODCOD_QPSK_4_TO_5,    // 0.80
    SAT_MODCOD_QPSK_5_TO_6,    // 0.83
    SAT_MODCOD_QPSK_8_TO_9,    // 0.89
    SAT_MODCOD_QPSK_9_TO_10,   // 0.90
    SAT_MODCOD_8PSK_3_TO_5,    // 0.60
    SAT_MODCOD_8PSK_2_TO_3,    // 0.67
    SAT_MODCOD_8PSK_3_TO_4,    // 0.75
    SAT_MODCOD_8PSK_5_TO_6,    // 0.83
    SAT_MODCOD_8PSK_8_TO_9,    // 0.89
    SAT_MODCOD_8PSK_9_TO_10,   // 0.90
    SAT_MODCOD_16APSK_2_TO_3,  // 0.67
    SAT_MODCOD_16APSK_3_TO_4,  // 0.75
    SAT_MODCOD_16APSK_4_TO_5,  // 0.80
    SAT_MODCOD_16APSK_5_TO_6,  // 0.83
    SAT_MODCOD_16APSK_8_TO_9,  // 0.89
    SAT_MODCOD_16APSK_9_TO_10, // 0.90
    SAT_MODCOD_16QAM_3_TO_4,   // 0.75
    SAT_MODCOD_16QAM_5_TO_6,   // 0.83
    SAT_MODCOD_32APSK_3_TO_4,  // 0.75
    SAT_MODCOD_32APSK_4_TO_5,  // 0.80
    SAT_MODCOD_32APSK_5_TO_6,  // 0.83
    SAT_MODCOD_32APSK_8_TO_9   // 0.89
  } SatModcod_t;


  static inline void GetAvailableModcodsFwdLink (std::vector<SatModcod_t>& modcods)
  {
    /**
     * Note that the order of MODCODs have a meaning in ACM.
     * The MODCODs should be in decreasing order based on
     * coding rate.
     *
     * Also note that three MODCODs (QPSK 1/3, 16QAM 3/4, and 16QAM 5/6) are
     * not included because we currently don't have the link results for them.
     */
    //modcods.push_back (SAT_MODCOD_QPSK_1_TO_3);
    modcods.push_back (SAT_MODCOD_QPSK_1_TO_2);
    modcods.push_back (SAT_MODCOD_QPSK_3_TO_5);
    modcods.push_back (SAT_MODCOD_QPSK_2_TO_3);
    modcods.push_back (SAT_MODCOD_QPSK_3_TO_4);
    modcods.push_back (SAT_MODCOD_QPSK_4_TO_5);
    modcods.push_back (SAT_MODCOD_QPSK_5_TO_6);
    modcods.push_back (SAT_MODCOD_QPSK_8_TO_9);
    modcods.push_back (SAT_MODCOD_QPSK_9_TO_10);
    modcods.push_back (SAT_MODCOD_8PSK_3_TO_5);
    modcods.push_back (SAT_MODCOD_8PSK_2_TO_3);
    modcods.push_back (SAT_MODCOD_8PSK_3_TO_4);
    modcods.push_back (SAT_MODCOD_8PSK_5_TO_6);
    modcods.push_back (SAT_MODCOD_8PSK_8_TO_9);
    modcods.push_back (SAT_MODCOD_8PSK_9_TO_10);
    modcods.push_back (SAT_MODCOD_16APSK_2_TO_3);
    modcods.push_back (SAT_MODCOD_16APSK_3_TO_4);
    modcods.push_back (SAT_MODCOD_16APSK_4_TO_5);
    modcods.push_back (SAT_MODCOD_16APSK_5_TO_6);
    modcods.push_back (SAT_MODCOD_16APSK_8_TO_9);
    modcods.push_back (SAT_MODCOD_16APSK_9_TO_10);
    //modcods.push_back (SAT_MODCOD_16QAM_3_TO_4);
    //modcods.push_back (SAT_MODCOD_16QAM_5_TO_6);
    modcods.push_back (SAT_MODCOD_32APSK_3_TO_4);
    modcods.push_back (SAT_MODCOD_32APSK_4_TO_5);
    modcods.push_back (SAT_MODCOD_32APSK_5_TO_6);
    modcods.push_back (SAT_MODCOD_32APSK_8_TO_9);
  }

  static inline void GetAvailableModcodsRtnLink (std::vector<SatModcod_t>& modcods)
  {
    /**
     * Note that the order of MODCODs have a meaning in ACM.
     * The MODCODs should be in decreasing order based on
     * coding rate.
     */
    modcods.push_back (SAT_MODCOD_QPSK_1_TO_3);
    modcods.push_back (SAT_MODCOD_QPSK_1_TO_2);
    modcods.push_back (SAT_MODCOD_QPSK_2_TO_3);
    modcods.push_back (SAT_MODCOD_QPSK_3_TO_4);
    modcods.push_back (SAT_MODCOD_QPSK_4_TO_5);
    modcods.push_back (SAT_MODCOD_QPSK_5_TO_6);
    modcods.push_back (SAT_MODCOD_8PSK_2_TO_3);
    modcods.push_back (SAT_MODCOD_8PSK_3_TO_4);
    modcods.push_back (SAT_MODCOD_8PSK_5_TO_6);
    modcods.push_back (SAT_MODCOD_16QAM_3_TO_4);
    modcods.push_back (SAT_MODCOD_16QAM_5_TO_6);
  }

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
  }

  /**
   * \enum SatBbFrameType_t
   *
   * \brief BB frame type used in DVB-S2 FWD link
   */
  typedef enum
  {
    UNDEFINED_FRAME = 0,
    SHORT_FRAME = 1,
    NORMAL_FRAME = 2,
    DUMMY_FRAME = 3
  } SatBbFrameType_t;

  static inline std::string GetFrameTypeName (SatBbFrameType_t frameType)
  {
    std::string typeName = "UNDEFINED_FRAME";

    switch (frameType)
      {
      case SHORT_FRAME:
        typeName = "SHORT_FRAME";
        break;

      case NORMAL_FRAME:
        typeName = "NORMAL_FRAME";
        break;

      case DUMMY_FRAME:
        typeName = "DUMMY_FRAME";
        break;

      default:
        NS_FATAL_ERROR ("SatEnums::GetFrameTypeName - Invalid frame type");
        break;
      }

    return typeName;
  }

  /**
   * \enum SatFlowId_t
   *
   * \brief Lower layer flow identifiers
   */
  typedef enum
  {
    CONTROL_FID = 0,
    EF_FID = 1,
    AF_FID = 2,
    BE_FID = 3,
    NUM_FIDS = 4
  } SatFlowId_t;

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


  static inline std::string GetCapacityAllocationCategory (SatCapacityAllocationCategory_t cac)
  {
    switch (cac)
      {
      case DA_UNKNOWN:
        {
          return "UNKNOWN";
        }
      case DA_RBDC:
        {
          return "RBDC";
        }
      case DA_VBDC:
        {
          return "VBDC";
        }
      case DA_AVBDC:
        {
          return "AVBDC";
        }
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetCapacityAllocationCategory - invalid CAC");
          break;
        }
      }
    NS_FATAL_ERROR ("SatEnums::GetCapacityAllocationCategory - invalid CAC");
    return "";
  }



  static inline void GetAvailableBbFrameTypes (std::vector<SatBbFrameType_t>& frameTypes)
  {
    frameTypes.push_back (SHORT_FRAME);
    frameTypes.push_back (NORMAL_FRAME);
  }

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
  }

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
  }

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
  }

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
  }

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
  }

  /**
   * \enum RandomAccessTriggerType_t
   * \brief The defined random access trigger types. These help determine
   * which algorithm to use if multiple algorithms are enabled
   */
  typedef enum
  {
    RA_TRIGGER_TYPE_SLOTTED_ALOHA = 0,
    RA_TRIGGER_TYPE_CRDSA = 1
  } RandomAccessTriggerType_t;

  static inline std::string GetRandomAccessTriggerTypeName (RandomAccessTriggerType_t triggerType)
  {
    switch (triggerType)
      {
      case RA_TRIGGER_TYPE_SLOTTED_ALOHA:
        {
          return "RA_TRIGGER_TYPE_SLOTTED_ALOHA";
        }
      case RA_TRIGGER_TYPE_CRDSA:
        {
          return "RA_TRIGGER_TYPE_CRDSA";
        }
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetRandomAccessTriggerTypeName - Invalid trigger type");
          break;
        }
      }
    NS_FATAL_ERROR ("SatEnums::GetRandomAccessTriggerTypeName - Invalid trigger type");
    return "";
  }

  /**
   * \enum RandomAccessModel_t
   * \brief The defined random access models. These define the implemented algorithms
   */
  typedef enum
  {
    RA_MODEL_OFF = 0,
    RA_MODEL_SLOTTED_ALOHA = 1,
    RA_MODEL_CRDSA = 2,
    RA_MODEL_RCS2_SPECIFICATION = 3
  } RandomAccessModel_t;

  static inline std::string GetRandomAccessModelName (RandomAccessModel_t model)
  {
    switch (model)
      {
      case RA_MODEL_OFF:
        {
          return "RA_MODEL_OFF";
        }
      case RA_MODEL_SLOTTED_ALOHA:
        {
          return "RA_MODEL_SLOTTED_ALOHA";
        }
      case RA_MODEL_CRDSA:
        {
          return "RA_MODEL_CRDSA";
        }
      case RA_MODEL_RCS2_SPECIFICATION:
        {
          return "RA_MODEL_RCS2_SPECIFICATION";
        }
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetRandomAccessModelName - Invalid model");
          break;
        }
      }
    NS_FATAL_ERROR ("SatEnums::GetRandomAccessModelName - Invalid model");
    return "";
  }

  /**
   * \enum RandomAccessTxOpportunityType_t
   * \brief Random access Tx opportunity types. These are used to define to which algorithm
   * the results provided by this module applies to
   */
  typedef enum
  {
    RA_TX_OPPORTUNITY_DO_NOTHING = 0,
    RA_TX_OPPORTUNITY_SLOTTED_ALOHA = 1,
    RA_TX_OPPORTUNITY_CRDSA = 2,
  } RandomAccessTxOpportunityType_t;

  static inline std::string GetRandomAccessOpportunityTypeName (RandomAccessTxOpportunityType_t opportunityType)
  {
    switch (opportunityType)
      {
      case RA_TX_OPPORTUNITY_DO_NOTHING:
        {
          return "RA_TX_OPPORTUNITY_DO_NOTHING";
        }
      case RA_TX_OPPORTUNITY_SLOTTED_ALOHA:
        {
          return "RA_TX_OPPORTUNITY_SLOTTED_ALOHA";
        }
      case RA_TX_OPPORTUNITY_CRDSA:
        {
          return "RA_TX_OPPORTUNITY_CRDSA";
        }
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetRandomAccessOpportunityTypeName - Invalid opportunity type");
          break;
        }
      }
    NS_FATAL_ERROR ("SatEnums::GetRandomAccessOpportunityTypeName - Invalid opportunity type");
    return "";
  }

  /**
   * \enum PacketType_t
   * \brief Packet types. These are used determine Rx side actions
   */
  typedef enum
  {
    PACKET_TYPE_DEDICATED_ACCESS = 0,
    PACKET_TYPE_SLOTTED_ALOHA = 1,
    PACKET_TYPE_CRDSA = 2,
  } PacketType_t;

  static inline std::string GetPacketTypeName (PacketType_t packetType)
  {
    switch (packetType)
      {
      case PACKET_TYPE_DEDICATED_ACCESS:
        {
          return "PACKET_TYPE_DEDICATED_ACCESS";
        }
      case PACKET_TYPE_SLOTTED_ALOHA:
        {
          return "PACKET_TYPE_SLOTTED_ALOHA";
        }
      case PACKET_TYPE_CRDSA:
        {
          return "PACKET_TYPE_CRDSA";
        }
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetPacketTypeName - Invalid packet type");
          break;
        }
      }
    NS_FATAL_ERROR ("SatEnums::GetPacketTypeName - Invalid packet type");
    return "";
  }

private:
  /**
   * Destructor
   *
   * Made Pure Virtual because the class is not planned be instantiated or inherited
   *
   */
  virtual ~SatEnums () = 0;
};


} // namespace ns3

#endif /* SATELLITE_ENUMS_H */
