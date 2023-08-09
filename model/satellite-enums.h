/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
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
   * \enum RegenerationMode_t
   * \brief The regeneration mode used in satellites.
   * It can be set for each link with different values.
   */
  typedef enum
  {
    TRANSPARENT,
    REGENERATION_PHY,     // regenerate only on physical layer, to compute is the packet is lost or not
    REGENERATION_LINK,    // regenerate on physical and MAC layers (only on return link). Satellite can perform packet TX scheduling and handle control messages
    REGENERATION_NETWORK  // regenerate all stack. The GSE/RLE encapsulation is removed at reception and added again when transmitting to next hop
  } RegenerationMode_t;

  /**
   * \enum IslArbiterType_t
   * \brief Choose the arbiter to use to route packets on ISLs
   */
  typedef enum
  {
    UNICAST,              // Only one route for a pair source satellite / destination satellite, using shortest path (in hops)
    ECMP,                 // Get all routes possible with minimum hops. For each incoming packet, the route is randomly selected among all available
  } IslArbiterType_t;

  /**
   * \enum Standard_t
   * \brief The global standard used. Can be either DVB or Lora
   */
  typedef enum
  {
    DVB,
    LORA
  } Standard_t;

  /**
   * \enum SatLoraNodeType_t
   * \brief Specifies standard used and the king of node
   */
  typedef enum
  {
    DVB_GW,
    DVB_UT,
    LORA_GW,
    LORA_UT,
    GEO
  } SatLoraNodeType_t;

  /**
   * \enum DvbVersion_t
   * \brief The scheduling algorithm used to fill the BBFrames.
   */
  typedef enum
  {
    DVB_S2,
    DVB_S2X
  } DvbVersion_t;

  /**
   * \enum FwdSchedulingAlgorithm_t
   * \brief The scheduling algorithm used to fill the BBFrames.
   */
  typedef enum
  {
    NO_TIME_SLICING,
    TIME_SLICING
  } FwdSchedulingAlgorithm_t;

  /**
   * BBFrame usage modes.
   */
  typedef enum
  {
    SHORT_FRAMES,          //!< SHORT_FRAMES
    NORMAL_FRAMES,         //!< NORMAL_FRAMES
    SHORT_AND_NORMAL_FRAMES //!< SHORT_AND_NORMAL_FRAMES
  } BbFrameUsageMode_t;

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
    SAT_MODCOD_32APSK_8_TO_9,  // 0.89
    SAT_MODCOD_BPSK_1_TO_3,    // 0.33

    SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_NOPILOTS,       //0.24
    SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_PILOTS,         //0.24
    SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_NOPILOTS,        //0.25
    SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_PILOTS,          //0.25
    SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_NOPILOTS,         //0.25
    SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_PILOTS,           //0.25
    SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_NOPILOTS,        //0.27
    SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_PILOTS,          //0.27
    SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_NOPILOTS,      //0.29
    SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_PILOTS,        //0.29
    SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_NOPILOTS,       //0.31
    SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_PILOTS,         //0.31
    SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_NOPILOTS,        //0.33
    SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_PILOTS,          //0.33
    SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_NOPILOTS,         //0.33
    SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_PILOTS,           //0.33
    SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_NOPILOTS,        //0.4
    SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_PILOTS,          //0.4
    SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_NOPILOTS,         //0.4
    SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_PILOTS,           //0.4
    SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_NOPILOTS,       //0.45
    SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_PILOTS,         //0.45
    SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_NOPILOTS,        //0.47
    SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_PILOTS,          //0.47
    SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_NOPILOTS,        //0.5
    SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_PILOTS,          //0.5
    SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_NOPILOTS,         //0.5
    SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_PILOTS,           //0.5
    SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_NOPILOTS,        //0.53
    SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_PILOTS,          //0.53
    SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_NOPILOTS,      //0.55
    SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_PILOTS,        //0.55
    SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_NOPILOTS,        //0.6
    SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_PILOTS,          //0.6
    SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_NOPILOTS,         //0.6
    SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_PILOTS,           //0.6
    SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_NOPILOTS,        //0.67
    SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_PILOTS,          //0.67
    SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_NOPILOTS,         //0.67
    SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_PILOTS,           //0.67
    SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_NOPILOTS,       //0.71
    SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_PILOTS,         //0.71
    SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_NOPILOTS,        //0.75
    SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_PILOTS,          //0.75
    SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_NOPILOTS,         //0.75
    SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_PILOTS,           //0.75
    SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_NOPILOTS,        //0.8
    SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_PILOTS,          //0.8
    SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_NOPILOTS,         //0.8
    SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_PILOTS,           //0.8
    SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_NOPILOTS,        //0.83
    SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_PILOTS,          //0.83
    SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_NOPILOTS,         //0.83
    SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_PILOTS,           //0.83
    SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_NOPILOTS,        //0.89
    SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_PILOTS,          //0.89
    SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_NOPILOTS,         //0.89
    SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_PILOTS,           //0.89
    SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_NOPILOTS,       //0.9
    SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_PILOTS,         //0.9
    SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_NOPILOTS,        //0.47
    SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_PILOTS,          //0.47
    SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_NOPILOTS,        //0.53
    SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_PILOTS,          //0.53
    SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_NOPILOTS,       //0.58
    SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_PILOTS,         //0.58
    SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_NOPILOTS,        //0.6
    SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_PILOTS,          //0.6
    SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_NOPILOTS,         //0.6
    SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_PILOTS,           //0.6
    SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_NOPILOTS,      //0.64
    SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_PILOTS,        //0.64
    SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_NOPILOTS,        //0.67
    SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_PILOTS,          //0.67
    SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_NOPILOTS,         //0.67
    SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_PILOTS,           //0.67
    SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_NOPILOTS,      //0.69
    SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_PILOTS,        //0.69
    SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_NOPILOTS,       //0.71
    SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_PILOTS,         //0.71
    SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_NOPILOTS,      //0.72
    SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_PILOTS,        //0.72
    SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_NOPILOTS,        //0.75
    SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_PILOTS,          //0.75
    SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_NOPILOTS,         //0.75
    SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_PILOTS,           //0.75
    SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_NOPILOTS,        //0.83
    SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_PILOTS,          //0.83
    SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_NOPILOTS,         //0.83
    SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_PILOTS,           //0.83
    SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_NOPILOTS,        //0.89
    SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_PILOTS,          //0.89
    SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_NOPILOTS,         //0.89
    SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_PILOTS,           //0.89
    SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_NOPILOTS,       //0.9
    SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_PILOTS,         //0.9
    SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_NOPILOTS,     //0.56
    SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_PILOTS,       //0.56
    SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_NOPILOTS,   //0.58
    SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_PILOTS,     //0.58
    SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_NOPILOTS,      //0.47
    SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_PILOTS,        //0.47
    SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_NOPILOTS,    //0.5
    SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_PILOTS,      //0.5
    SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_NOPILOTS,   //0.53
    SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_PILOTS,     //0.53
    SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_NOPILOTS,      //0.53
    SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_PILOTS,        //0.53
    SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_NOPILOTS,    //0.56
    SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_PILOTS,      //0.56
    SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_NOPILOTS,    //0.58
    SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_PILOTS,      //0.58
    SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_NOPILOTS,     //0.58
    SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_PILOTS,       //0.58
    SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_NOPILOTS,      //0.58
    SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_PILOTS,        //0.58
    SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_NOPILOTS,    //0.6
    SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_PILOTS,      //0.6
    SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_NOPILOTS,       //0.6
    SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_PILOTS,         //0.6
    SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_NOPILOTS,    //0.62
    SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_PILOTS,      //0.62
    SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_NOPILOTS,    //0.64
    SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_PILOTS,      //0.64
    SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_NOPILOTS,    //0.67
    SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_PILOTS,      //0.67
    SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_NOPILOTS,      //0.67
    SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_PILOTS,        //0.67
    SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_NOPILOTS,       //0.67
    SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_PILOTS,         //0.67
    SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_NOPILOTS,    //0.69
    SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_PILOTS,      //0.69
    SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_NOPILOTS,     //0.71
    SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_PILOTS,       //0.71
    SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_NOPILOTS,    //0.72
    SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_PILOTS,      //0.72
    SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_NOPILOTS,      //0.75
    SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_PILOTS,        //0.75
    SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_NOPILOTS,       //0.75
    SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_PILOTS,         //0.75
    SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_NOPILOTS,      //0.78
    SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_PILOTS,        //0.78
    SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_NOPILOTS,      //0.8
    SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_PILOTS,        //0.8
    SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_NOPILOTS,       //0.8
    SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_PILOTS,         //0.8
    SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_NOPILOTS,      //0.83
    SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_PILOTS,        //0.83
    SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_NOPILOTS,       //0.83
    SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_PILOTS,         //0.83
    SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_NOPILOTS,    //0.86
    SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_PILOTS,      //0.86
    SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_NOPILOTS,      //0.89
    SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_PILOTS,        //0.89
    SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_NOPILOTS,       //0.89
    SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_PILOTS,         //0.89
    SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_NOPILOTS,     //0.9
    SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_PILOTS,       //0.9
    SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_NOPILOTS,    //0.67
    SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_PILOTS,      //0.67
    SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_NOPILOTS,       //0.67
    SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_PILOTS,         //0.67
    SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_NOPILOTS,    //0.71
    SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_PILOTS,      //0.71
    SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_NOPILOTS,     //0.71
    SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_PILOTS,       //0.71
    SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_NOPILOTS,    //0.73
    SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_PILOTS,      //0.73
    SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_NOPILOTS,      //0.75
    SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_PILOTS,        //0.75
    SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_NOPILOTS,       //0.75
    SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_PILOTS,         //0.75
    SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_NOPILOTS,      //0.78
    SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_PILOTS,        //0.78
    SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_NOPILOTS,      //0.8
    SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_PILOTS,        //0.8
    SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_NOPILOTS,       //0.8
    SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_PILOTS,         //0.8
    SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_NOPILOTS,      //0.83
    SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_PILOTS,        //0.83
    SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_NOPILOTS,       //0.83
    SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_PILOTS,         //0.83
    SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_NOPILOTS,      //0.89
    SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_PILOTS,        //0.89
    SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_NOPILOTS,       //0.89
    SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_PILOTS,         //0.89
    SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_NOPILOTS,     //0.9
    SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_PILOTS,       //0.9
    SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_NOPILOTS,  //0.71
    SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_PILOTS,    //0.71
    SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_NOPILOTS,    //0.73
    SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_PILOTS,      //0.73
    SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_NOPILOTS,      //0.78
    SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_PILOTS,        //0.78
    SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_NOPILOTS,      //0.8
    SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_PILOTS,        //0.8
    SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_NOPILOTS,      //0.83
    SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_PILOTS,        //0.83
    SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_NOPILOTS,     //0.75
    SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_PILOTS,       //0.75
    SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_NOPILOTS,     //0.78
    SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_PILOTS,       //0.78
    SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_NOPILOTS, //0.64
    SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_PILOTS,   //0.64
    SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_NOPILOTS,   //0.67
    SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_PILOTS,     //0.67
    SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_NOPILOTS, //0.69
    SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_PILOTS,   //0.69
    SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_NOPILOTS,   //0.71
    SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_PILOTS,     //0.71
    SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_NOPILOTS, //0.73
    SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_PILOTS,   //0.73
    SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_NOPILOTS,     //0.75
    SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_PILOTS        //0.75
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

  static inline void GetAvailableModcodsFwdLinkS2X (std::vector<SatModcod_t>& modcods, SatEnums::BbFrameUsageMode_t type, bool pilots)
  {
    switch(type)
      {
        case SatEnums::NORMAL_FRAMES:
          GetAvailableModcodsFwdLinkS2XNormalFrames (modcods, pilots);
          break;
        case SatEnums::SHORT_FRAMES:
          GetAvailableModcodsFwdLinkS2XShortFrames (modcods, pilots);
          break;
        case SatEnums::SHORT_AND_NORMAL_FRAMES:
          NS_FATAL_ERROR ("Cannot use SHORT_AND_NORMAL_FRAMES mode in DVB-S2X");
        default:
          NS_FATAL_ERROR ("Unknown SatEnums::BbFrameUsageMode_t type");
      }
  }

  static inline void GetAvailableModcodsFwdLinkS2XNormalFrames (std::vector<SatModcod_t>& modcods, bool pilots)
  {
    /**
     * Note that the order of MODCODs have a meaning in ACM.
     * The MODCODs should be in decreasing order based on
     * coding rate.
     */

    if (pilots)
      {
        modcods.push_back (SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_PILOTS);
      }
    else
      {
        modcods.push_back (SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_NOPILOTS);
      }
  }

  static inline void GetAvailableModcodsFwdLinkS2XShortFrames (std::vector<SatModcod_t>& modcods, bool pilots)
  {
    /**
     * Note that the order of MODCODs have a meaning in ACM.
     * The MODCODs should be in decreasing order based on
     * coding rate.
     */

    if (pilots)
      {
        modcods.push_back (SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_PILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_PILOTS);
      }
    else
      {
        modcods.push_back (SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_NOPILOTS);
        modcods.push_back (SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_NOPILOTS);
      }
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
      case SAT_MODCOD_BPSK_1_TO_3:
        return "BPSK_1_TO_3";
      case SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_NOPILOTS:
        return "QPSK_11_TO_45_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_PILOTS:
        return "QPSK_11_TO_45_SHORT_PILOTS";
      case SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_NOPILOTS:
        return "QPSK_1_TO_4_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_PILOTS:
        return "QPSK_1_TO_4_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_NOPILOTS:
        return "QPSK_1_TO_4_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_PILOTS:
        return "QPSK_1_TO_4_SHORT_PILOTS";
      case SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_NOPILOTS:
        return "QPSK_4_TO_15_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_PILOTS:
        return "QPSK_4_TO_15_SHORT_PILOTS";
      case SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_NOPILOTS:
        return "QPSK_13_TO_45_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_PILOTS:
        return "QPSK_13_TO_45_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_NOPILOTS:
        return "QPSK_14_TO_45_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_PILOTS:
        return "QPSK_14_TO_45_SHORT_PILOTS";
      case SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_NOPILOTS:
        return "QPSK_1_TO_3_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_PILOTS:
        return "QPSK_1_TO_3_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_NOPILOTS:
        return "QPSK_1_TO_3_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_PILOTS:
        return "QPSK_1_TO_3_SHORT_PILOTS";
      case SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_NOPILOTS:
        return "QPSK_2_TO_5_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_PILOTS:
        return "QPSK_2_TO_5_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_NOPILOTS:
        return "QPSK_2_TO_5_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_PILOTS:
        return "QPSK_2_TO_5_SHORT_PILOTS";
      case SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_NOPILOTS:
        return "QPSK_9_TO_20_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_PILOTS:
        return "QPSK_9_TO_20_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_NOPILOTS:
        return "QPSK_7_TO_15_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_PILOTS:
        return "QPSK_7_TO_15_SHORT_PILOTS";
      case SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_NOPILOTS:
        return "QPSK_1_TO_2_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_PILOTS:
        return "QPSK_1_TO_2_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_NOPILOTS:
        return "QPSK_1_TO_2_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_PILOTS:
        return "QPSK_1_TO_2_SHORT_PILOTS";
      case SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_NOPILOTS:
        return "QPSK_8_TO_15_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_PILOTS:
        return "QPSK_8_TO_15_SHORT_PILOTS";
      case SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_NOPILOTS:
        return "QPSK_11_TO_20_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_PILOTS:
        return "QPSK_11_TO_20_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_NOPILOTS:
        return "QPSK_3_TO_5_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_PILOTS:
        return "QPSK_3_TO_5_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_NOPILOTS:
        return "QPSK_3_TO_5_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_PILOTS:
        return "QPSK_3_TO_5_SHORT_PILOTS";
      case SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_NOPILOTS:
        return "QPSK_2_TO_3_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_PILOTS:
        return "QPSK_2_TO_3_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_NOPILOTS:
        return "QPSK_2_TO_3_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_PILOTS:
        return "QPSK_2_TO_3_SHORT_PILOTS";
      case SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_NOPILOTS:
        return "QPSK_32_TO_45_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_PILOTS:
        return "QPSK_32_TO_45_SHORT_PILOTS";
      case SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_NOPILOTS:
        return "QPSK_3_TO_4_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_PILOTS:
        return "QPSK_3_TO_4_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_NOPILOTS:
        return "QPSK_3_TO_4_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_PILOTS:
        return "QPSK_3_TO_4_SHORT_PILOTS";
      case SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_NOPILOTS:
        return "QPSK_4_TO_5_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_PILOTS:
        return "QPSK_4_TO_5_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_NOPILOTS:
        return "QPSK_4_TO_5_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_PILOTS:
        return "QPSK_4_TO_5_SHORT_PILOTS";
      case SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_NOPILOTS:
        return "QPSK_5_TO_6_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_PILOTS:
        return "QPSK_5_TO_6_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_NOPILOTS:
        return "QPSK_5_TO_6_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_PILOTS:
        return "QPSK_5_TO_6_SHORT_PILOTS";
      case SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_NOPILOTS:
        return "QPSK_8_TO_9_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_PILOTS:
        return "QPSK_8_TO_9_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_NOPILOTS:
        return "QPSK_8_TO_9_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_PILOTS:
        return "QPSK_8_TO_9_SHORT_PILOTS";
      case SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_NOPILOTS:
        return "QPSK_9_TO_10_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_PILOTS:
        return "QPSK_9_TO_10_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_NOPILOTS:
        return "8PSK_7_TO_15_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_PILOTS:
        return "8PSK_7_TO_15_SHORT_PILOTS";
      case SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_NOPILOTS:
        return "8PSK_8_TO_15_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_PILOTS:
        return "8PSK_8_TO_15_SHORT_PILOTS";
      case SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_NOPILOTS:
        return "8PSK_26_TO_45_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_PILOTS:
        return "8PSK_26_TO_45_SHORT_PILOTS";
      case SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_NOPILOTS:
        return "8PSK_3_TO_5_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_PILOTS:
        return "8PSK_3_TO_5_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_NOPILOTS:
        return "8PSK_3_TO_5_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_PILOTS:
        return "8PSK_3_TO_5_SHORT_PILOTS";
      case SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_NOPILOTS:
        return "8PSK_23_TO_36_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_PILOTS:
        return "8PSK_23_TO_36_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_NOPILOTS:
        return "8PSK_2_TO_3_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_PILOTS:
        return "8PSK_2_TO_3_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_NOPILOTS:
        return "8PSK_2_TO_3_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_PILOTS:
        return "8PSK_2_TO_3_SHORT_PILOTS";
      case SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_NOPILOTS:
        return "8PSK_25_TO_36_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_PILOTS:
        return "8PSK_25_TO_36_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_NOPILOTS:
        return "8PSK_32_TO_45_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_PILOTS:
        return "8PSK_32_TO_45_SHORT_PILOTS";
      case SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_NOPILOTS:
        return "8PSK_13_TO_18_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_PILOTS:
        return "8PSK_13_TO_18_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_NOPILOTS:
        return "8PSK_3_TO_4_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_PILOTS:
        return "8PSK_3_TO_4_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_NOPILOTS:
        return "8PSK_3_TO_4_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_PILOTS:
        return "8PSK_3_TO_4_SHORT_PILOTS";
      case SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_NOPILOTS:
        return "8PSK_5_TO_6_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_PILOTS:
        return "8PSK_5_TO_6_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_NOPILOTS:
        return "8PSK_5_TO_6_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_PILOTS:
        return "8PSK_5_TO_6_SHORT_PILOTS";
      case SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_NOPILOTS:
        return "8PSK_8_TO_9_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_PILOTS:
        return "8PSK_8_TO_9_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_NOPILOTS:
        return "8PSK_8_TO_9_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_PILOTS:
        return "8PSK_8_TO_9_SHORT_PILOTS";
      case SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_NOPILOTS:
        return "8PSK_9_TO_10_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_PILOTS:
        return "8PSK_9_TO_10_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_NOPILOTS:
        return "8APSK_5_TO_9_L_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_PILOTS:
        return "8APSK_5_TO_9_L_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_NOPILOTS:
        return "8APSK_26_TO_45_L_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_PILOTS:
        return "8APSK_26_TO_45_L_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_NOPILOTS:
        return "16APSK_7_TO_15_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_PILOTS:
        return "16APSK_7_TO_15_SHORT_PILOTS";
      case SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_NOPILOTS:
        return "16APSK_1_TO_2_L_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_PILOTS:
        return "16APSK_1_TO_2_L_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_NOPILOTS:
        return "16APSK_8_TO_15_L_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_PILOTS:
        return "16APSK_8_TO_15_L_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_NOPILOTS:
        return "16APSK_8_TO_15_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_PILOTS:
        return "16APSK_8_TO_15_SHORT_PILOTS";
      case SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_NOPILOTS:
        return "16APSK_5_TO_9_L_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_PILOTS:
        return "16APSK_5_TO_9_L_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_NOPILOTS:
        return "16APSK_26_TO_45_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_PILOTS:
        return "16APSK_26_TO_45_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_NOPILOTS:
        return "16APSK_26_TO_45_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_PILOTS:
        return "16APSK_26_TO_45_SHORT_PILOTS";
      case SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_NOPILOTS:
        return "16APSK_3_TO_5_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_PILOTS:
        return "16APSK_3_TO_5_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_NOPILOTS:
        return "16APSK_3_TO_5_L_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_PILOTS:
        return "16APSK_3_TO_5_L_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_NOPILOTS:
        return "16APSK_3_TO_5_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_PILOTS:
        return "16APSK_3_TO_5_SHORT_PILOTS";
      case SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_NOPILOTS:
        return "16APSK_28_TO_45_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_PILOTS:
        return "16APSK_28_TO_45_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_NOPILOTS:
        return "16APSK_23_TO_36_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_PILOTS:
        return "16APSK_23_TO_36_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_NOPILOTS:
        return "16APSK_2_TO_3_L_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_PILOTS:
        return "16APSK_2_TO_3_L_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_NOPILOTS:
        return "16APSK_2_TO_3_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_PILOTS:
        return "16APSK_2_TO_3_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_NOPILOTS:
        return "16APSK_2_TO_3_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_PILOTS:
        return "16APSK_2_TO_3_SHORT_PILOTS";
      case SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_NOPILOTS:
        return "16APSK_25_TO_36_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_PILOTS:
        return "16APSK_25_TO_36_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_NOPILOTS:
        return "16APSK_32_TO_45_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_PILOTS:
        return "16APSK_32_TO_45_SHORT_PILOTS";
      case SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_NOPILOTS:
        return "16APSK_13_TO_18_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_PILOTS:
        return "16APSK_13_TO_18_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_NOPILOTS:
        return "16APSK_3_TO_4_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_PILOTS:
        return "16APSK_3_TO_4_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_NOPILOTS:
        return "16APSK_3_TO_4_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_PILOTS:
        return "16APSK_3_TO_4_SHORT_PILOTS";
      case SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_NOPILOTS:
        return "16APSK_7_TO_9_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_PILOTS:
        return "16APSK_7_TO_9_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_NOPILOTS:
        return "16APSK_4_TO_5_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_PILOTS:
        return "16APSK_4_TO_5_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_NOPILOTS:
        return "16APSK_4_TO_5_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_PILOTS:
        return "16APSK_4_TO_5_SHORT_PILOTS";
      case SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_NOPILOTS:
        return "16APSK_5_TO_6_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_PILOTS:
        return "16APSK_5_TO_6_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_NOPILOTS:
        return "16APSK_5_TO_6_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_PILOTS:
        return "16APSK_5_TO_6_SHORT_PILOTS";
      case SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_NOPILOTS:
        return "16APSK_77_TO_90_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_PILOTS:
        return "16APSK_77_TO_90_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_NOPILOTS:
        return "16APSK_8_TO_9_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_PILOTS:
        return "16APSK_8_TO_9_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_NOPILOTS:
        return "16APSK_8_TO_9_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_PILOTS:
        return "16APSK_8_TO_9_SHORT_PILOTS";
      case SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_NOPILOTS:
        return "16APSK_9_TO_10_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_PILOTS:
        return "16APSK_9_TO_10_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_NOPILOTS:
        return "32APSK_2_TO_3_L_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_PILOTS:
        return "32APSK_2_TO_3_L_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_NOPILOTS:
        return "32APSK_2_TO_3_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_PILOTS:
        return "32APSK_2_TO_3_SHORT_PILOTS";
      case SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_NOPILOTS:
        return "32APSK_32_TO_45_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_PILOTS:
        return "32APSK_32_TO_45_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_NOPILOTS:
        return "32APSK_32_TO_45_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_PILOTS:
        return "32APSK_32_TO_45_SHORT_PILOTS";
      case SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_NOPILOTS:
        return "32APSK_11_TO_15_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_PILOTS:
        return "32APSK_11_TO_15_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_NOPILOTS:
        return "32APSK_3_TO_4_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_PILOTS:
        return "32APSK_3_TO_4_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_NOPILOTS:
        return "32APSK_3_TO_4_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_PILOTS:
        return "32APSK_3_TO_4_SHORT_PILOTS";
      case SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_NOPILOTS:
        return "32APSK_7_TO_9_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_PILOTS:
        return "32APSK_7_TO_9_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_NOPILOTS:
        return "32APSK_4_TO_5_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_PILOTS:
        return "32APSK_4_TO_5_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_NOPILOTS:
        return "32APSK_4_TO_5_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_PILOTS:
        return "32APSK_4_TO_5_SHORT_PILOTS";
      case SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_NOPILOTS:
        return "32APSK_5_TO_6_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_PILOTS:
        return "32APSK_5_TO_6_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_NOPILOTS:
        return "32APSK_5_TO_6_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_PILOTS:
        return "32APSK_5_TO_6_SHORT_PILOTS";
      case SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_NOPILOTS:
        return "32APSK_8_TO_9_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_PILOTS:
        return "32APSK_8_TO_9_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_NOPILOTS:
        return "32APSK_8_TO_9_SHORT_NOPILOTS";
      case SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_PILOTS:
        return "32APSK_8_TO_9_SHORT_PILOTS";
      case SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_NOPILOTS:
        return "32APSK_9_TO_10_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_PILOTS:
        return "32APSK_9_TO_10_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_NOPILOTS:
        return "64APSK_32_TO_45_L_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_PILOTS:
        return "64APSK_32_TO_45_L_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_NOPILOTS:
        return "64APSK_11_TO_15_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_PILOTS:
        return "64APSK_11_TO_15_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_NOPILOTS:
        return "64APSK_7_TO_9_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_PILOTS:
        return "64APSK_7_TO_9_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_NOPILOTS:
        return "64APSK_4_TO_5_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_PILOTS:
        return "64APSK_4_TO_5_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_NOPILOTS:
        return "64APSK_5_TO_6_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_PILOTS:
        return "64APSK_5_TO_6_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_NOPILOTS:
        return "128APSK_3_TO_4_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_PILOTS:
        return "128APSK_3_TO_4_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_NOPILOTS:
        return "128APSK_7_TO_9_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_PILOTS:
        return "128APSK_7_TO_9_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_NOPILOTS:
        return "256APSK_29_TO_45_L_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_PILOTS:
        return "256APSK_29_TO_45_L_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_NOPILOTS:
        return "256APSK_2_TO_3_L_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_PILOTS:
        return "256APSK_2_TO_3_L_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_NOPILOTS:
        return "256APSK_31_TO_45_L_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_PILOTS:
        return "256APSK_31_TO_45_L_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_NOPILOTS:
        return "256APSK_32_TO_45_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_PILOTS:
        return "256APSK_32_TO_45_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_NOPILOTS:
        return "256APSK_11_TO_15_L_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_PILOTS:
        return "256APSK_11_TO_15_L_NORMAL_PILOTS";
      case SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_NOPILOTS:
        return "256APSK_3_TO_4_NORMAL_NOPILOTS";
      case SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_PILOTS:
        return "256APSK_3_TO_4_NORMAL_PILOTS";
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetModcodTypeName - Invalid modcod type");
          break;
        }
      }
    NS_FATAL_ERROR ("SatEnums::GetModcodTypeName - Invalid modcod type");
    return "";
  }

  static inline SatModcod_t GetModcodFromName (const std::string name)
  {
    if (name == "SAT_NONVALID_MODCOD")
      return SAT_NONVALID_MODCOD;
    // DVB-S2X ModCods
    if (name == "SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_PILOTS")
      return SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_PILOTS")
      return SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_PILOTS")
      return SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_PILOTS")
      return SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_PILOTS")
      return SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_PILOTS")
      return SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_PILOTS")
      return SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_PILOTS")
      return SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_PILOTS")
      return SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_PILOTS")
      return SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_PILOTS")
      return SAT_MODCOD_S2X_QPSK_2_TO_3_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_PILOTS")
      return SAT_MODCOD_S2X_QPSK_32_TO_45_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_PILOTS")
      return SAT_MODCOD_S2X_QPSK_3_TO_4_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_QPSK_4_TO_5_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_PILOTS")
      return SAT_MODCOD_S2X_QPSK_4_TO_5_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_QPSK_5_TO_6_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_PILOTS")
      return SAT_MODCOD_S2X_QPSK_5_TO_6_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_QPSK_8_TO_9_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_PILOTS")
      return SAT_MODCOD_S2X_QPSK_8_TO_9_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_QPSK_9_TO_10_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_PILOTS")
      return SAT_MODCOD_S2X_8PSK_7_TO_15_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_PILOTS")
      return SAT_MODCOD_S2X_8PSK_8_TO_15_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_PILOTS")
      return SAT_MODCOD_S2X_8PSK_26_TO_45_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_8PSK_3_TO_5_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_PILOTS")
      return SAT_MODCOD_S2X_8PSK_3_TO_5_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_8PSK_23_TO_36_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_8PSK_2_TO_3_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_PILOTS")
      return SAT_MODCOD_S2X_8PSK_2_TO_3_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_8PSK_25_TO_36_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_PILOTS")
      return SAT_MODCOD_S2X_8PSK_32_TO_45_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_8PSK_13_TO_18_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_8PSK_3_TO_4_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_PILOTS")
      return SAT_MODCOD_S2X_8PSK_3_TO_4_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_8PSK_5_TO_6_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_PILOTS")
      return SAT_MODCOD_S2X_8PSK_5_TO_6_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_8PSK_8_TO_9_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_PILOTS")
      return SAT_MODCOD_S2X_8PSK_8_TO_9_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_8PSK_9_TO_10_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_8APSK_5_TO_9_L_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_8APSK_26_TO_45_L_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_PILOTS")
      return SAT_MODCOD_S2X_16APSK_7_TO_15_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_1_TO_2_L_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_8_TO_15_L_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_PILOTS")
      return SAT_MODCOD_S2X_16APSK_8_TO_15_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_5_TO_9_L_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_26_TO_45_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_PILOTS")
      return SAT_MODCOD_S2X_16APSK_26_TO_45_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_3_TO_5_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_3_TO_5_L_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_PILOTS")
      return SAT_MODCOD_S2X_16APSK_3_TO_5_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_28_TO_45_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_23_TO_36_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_2_TO_3_L_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_2_TO_3_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_PILOTS")
      return SAT_MODCOD_S2X_16APSK_2_TO_3_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_25_TO_36_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_PILOTS")
      return SAT_MODCOD_S2X_16APSK_32_TO_45_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_13_TO_18_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_3_TO_4_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_PILOTS")
      return SAT_MODCOD_S2X_16APSK_3_TO_4_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_7_TO_9_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_4_TO_5_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_PILOTS")
      return SAT_MODCOD_S2X_16APSK_4_TO_5_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_5_TO_6_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_PILOTS")
      return SAT_MODCOD_S2X_16APSK_5_TO_6_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_77_TO_90_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_8_TO_9_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_PILOTS")
      return SAT_MODCOD_S2X_16APSK_8_TO_9_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_16APSK_9_TO_10_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_32APSK_2_TO_3_L_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_PILOTS")
      return SAT_MODCOD_S2X_32APSK_2_TO_3_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_32APSK_32_TO_45_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_PILOTS")
      return SAT_MODCOD_S2X_32APSK_32_TO_45_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_32APSK_11_TO_15_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_32APSK_3_TO_4_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_PILOTS")
      return SAT_MODCOD_S2X_32APSK_3_TO_4_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_32APSK_7_TO_9_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_32APSK_4_TO_5_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_PILOTS")
      return SAT_MODCOD_S2X_32APSK_4_TO_5_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_32APSK_5_TO_6_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_PILOTS")
      return SAT_MODCOD_S2X_32APSK_5_TO_6_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_32APSK_8_TO_9_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_NOPILOTS")
      return SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_PILOTS")
      return SAT_MODCOD_S2X_32APSK_8_TO_9_SHORT_PILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_32APSK_9_TO_10_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_64APSK_32_TO_45_L_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_64APSK_11_TO_15_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_64APSK_7_TO_9_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_64APSK_4_TO_5_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_64APSK_5_TO_6_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_128APSK_3_TO_4_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_128APSK_7_TO_9_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_256APSK_29_TO_45_L_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_256APSK_2_TO_3_L_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_256APSK_31_TO_45_L_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_256APSK_32_TO_45_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_256APSK_11_TO_15_L_NORMAL_PILOTS;
    if (name == "SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_NOPILOTS")
      return SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_NOPILOTS;
    if (name == "SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_PILOTS")
      return SAT_MODCOD_S2X_256APSK_3_TO_4_NORMAL_PILOTS;
    // DVB-S2 ModCods
    if (name == "SAT_MODCOD_QPSK_1_TO_3")
      return SAT_MODCOD_QPSK_1_TO_3;
    if (name == "SAT_MODCOD_QPSK_1_TO_2")
      return SAT_MODCOD_QPSK_1_TO_2;
    if (name == "SAT_MODCOD_QPSK_2_TO_3")
      return SAT_MODCOD_QPSK_2_TO_3;
    if (name == "SAT_MODCOD_QPSK_3_TO_4")
      return SAT_MODCOD_QPSK_3_TO_4;
    if (name == "SAT_MODCOD_QPSK_3_TO_5")
      return SAT_MODCOD_QPSK_3_TO_5;
    if (name == "SAT_MODCOD_QPSK_4_TO_5")
      return SAT_MODCOD_QPSK_4_TO_5;
    if (name == "SAT_MODCOD_QPSK_5_TO_6")
      return SAT_MODCOD_QPSK_5_TO_6;
    if (name == "SAT_MODCOD_QPSK_8_TO_9")
      return SAT_MODCOD_QPSK_8_TO_9;
    if (name == "SAT_MODCOD_QPSK_9_TO_10")
      return SAT_MODCOD_QPSK_9_TO_10;
    if (name == "SAT_MODCOD_8PSK_2_TO_3")
      return SAT_MODCOD_8PSK_2_TO_3;
    if (name == "SAT_MODCOD_8PSK_3_TO_4")
      return SAT_MODCOD_8PSK_3_TO_4;
    if (name == "SAT_MODCOD_8PSK_3_TO_5")
      return SAT_MODCOD_8PSK_3_TO_5;
    if (name == "SAT_MODCOD_8PSK_5_TO_6")
      return SAT_MODCOD_8PSK_5_TO_6;
    if (name == "SAT_MODCOD_8PSK_8_TO_9")
      return SAT_MODCOD_8PSK_8_TO_9;
    if (name == "SAT_MODCOD_8PSK_9_TO_10")
      return SAT_MODCOD_8PSK_9_TO_10;
    if (name == "SAT_MODCOD_16APSK_2_TO_3")
      return SAT_MODCOD_16APSK_2_TO_3;
    if (name == "SAT_MODCOD_16APSK_3_TO_4")
      return SAT_MODCOD_16APSK_3_TO_4;
    if (name == "SAT_MODCOD_16APSK_4_TO_5")
      return SAT_MODCOD_16APSK_4_TO_5;
    if (name == "SAT_MODCOD_16APSK_5_TO_6")
      return SAT_MODCOD_16APSK_5_TO_6;
    if (name == "SAT_MODCOD_16APSK_8_TO_9")
      return SAT_MODCOD_16APSK_8_TO_9;
    if (name == "SAT_MODCOD_16APSK_9_TO_10")
      return SAT_MODCOD_16APSK_9_TO_10;
    if (name == "SAT_MODCOD_16QAM_3_TO_4")
      return SAT_MODCOD_16QAM_3_TO_4;
    if (name == "SAT_MODCOD_16QAM_5_TO_6")
      return SAT_MODCOD_16QAM_5_TO_6;
    if (name == "SAT_MODCOD_32APSK_3_TO_4")
      return SAT_MODCOD_32APSK_3_TO_4;
    if (name == "SAT_MODCOD_32APSK_4_TO_5")
      return SAT_MODCOD_32APSK_4_TO_5;
    if (name == "SAT_MODCOD_32APSK_5_TO_6")
      return SAT_MODCOD_32APSK_5_TO_6;
    if (name == "SAT_MODCOD_32APSK_8_TO_9")
      return SAT_MODCOD_32APSK_8_TO_9;
  NS_FATAL_ERROR ("Cannot fing ModCod: " + name);
  return SAT_NONVALID_MODCOD;
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
    UNKNOWN_BURST = 0,
    SHORT_BURST = 1,
    LONG_BURST = 2,
    SHORT_AND_LONG_BURST = 3
  } SatWaveFormBurstLength_t;

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
    RX_CNO_INPUT_TRACE = 2,
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
    RA_TRIGGER_TYPE_CRDSA = 1,
    RA_TRIGGER_TYPE_ESSA = 2
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
      case RA_TRIGGER_TYPE_ESSA:
        {
          return "RA_TRIGGER_TYPE_ESSA";
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
    RA_MODEL_RCS2_SPECIFICATION = 3,
    RA_MODEL_MARSALA = 4,
    RA_MODEL_ESSA = 5
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
      case RA_MODEL_ESSA:
        {
          return "RA_MODEL_ESSA";
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
    RA_TX_OPPORTUNITY_ESSA = 3,
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
      case RA_TX_OPPORTUNITY_ESSA:
        {
          return "RA_TX_OPPORTUNITY_ESSA";
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
    PACKET_TYPE_ESSA = 3,
    PACKET_TYPE_LOGON = 4
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
      case PACKET_TYPE_ESSA:
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

  /**
   * \enum RbcdCapacityRequestAlgorithm_t
   * \brief The defined RBDC capacity request algorithms. These help determine
   * which algorithm to use when calculating RBDC requests.
   */
  typedef enum
  {
    CR_RBDC_LEGACY = 0,
  } RbdcCapacityRequestAlgorithm_t;

  static inline std::string GetRbdcCapacityRequestAlgorithmName (RbdcCapacityRequestAlgorithm_t rbdcAlgorithm)
  {
    switch (rbdcAlgorithm)
      {
      case CR_RBDC_LEGACY:
        {
          return "CR_RBDC_LEGACY";
        }
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetRbdcCapacityRequestAlgorithmName - Invalid algorithm name");
          break;
        }
      }
    NS_FATAL_ERROR ("SatEnums::GetRbdcCapacityRequestAlgorithmName - Invalid algorithm name");
    return "";
  }

  /**
   * \enum VbcdCapacityRequestAlgorithm_t
   * \brief The defined VBDC capacity request algorithms. These help determine
   * which algorithm to use when calculating VBDC requests.
   */
  typedef enum
  {
    CR_VBDC_LEGACY = 0,
  } VbdcCapacityRequestAlgorithm_t;

  static inline std::string GetVbdcCapacityRequestAlgorithmName (VbdcCapacityRequestAlgorithm_t vbdcAlgorithm)
  {
    switch (vbdcAlgorithm)
      {
      case CR_VBDC_LEGACY:
        {
          return "CR_VBDC_LEGACY";
        }
      default:
        {
          NS_FATAL_ERROR ("SatEnums::GetVbdcCapacityRequestAlgorithmName - Invalid algorithm name");
          break;
        }
      }
    NS_FATAL_ERROR ("SatEnums::GetVbdcCapacityRequestAlgorithmName - Invalid algorithm name");
    return "";
  }


  /**
   * \enum SatSuperframeAllocatorType_t
   * \brief SuperframeAllocator type to use
   */
  typedef enum
  {
    DEFAULT_SUPERFRAME_ALLOCATOR = 0,
  } SuperframeAllocatorType_t;

  /**
   * \enum LinkResults_t
   * \brief Return link Link result types. These are used to define which type of
   * waveforms are used to check link results on the return link.
   */
  typedef enum
  {
    LR_RCS2 = 0,
    LR_FSIM = 1,
    LR_LORA = 2
  } LinkResults_t;

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
