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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */


#include <iostream>
#include <limits>
#include <utility>
#include <vector>
#include <ns3/log.h>
#include <ns3/string.h>
#include <ns3/double.h>
#include <ns3/uinteger.h>
#include <ns3/boolean.h>
#include <ns3/enum.h>
#include <ns3/satellite-utils.h>
#include "satellite-bbframe-conf.h"

NS_LOG_COMPONENT_DEFINE ("SatBbFrameConf");

namespace ns3 {

/**
 * 1st col = modulated bits
 * 2nd col = payload slots per normal frame
 * 3rnd col = payload slots per short frame
 */
const uint32_t payloadConf[7][3] = { { 2, 360, 90 },
                                     { 3, 240, 60 },
                                     { 4, 180, 45 },
                                     { 5, 144, 36 },
                                     { 6, 120, 30 },
                                     { 7, 102, 25 },
                                     { 8,  90, 22 }, };


SatDvbS2Waveform::SatDvbS2Waveform ()
  : m_modcod (SatEnums::SAT_MODCOD_QPSK_1_TO_2),
  m_frameType (SatEnums::NORMAL_FRAME),
  m_frameDuration (Seconds (0.0)),
  m_payloadBits (0),
  m_cnoRequirement (0.0)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (false);

  /**
   * Default constructor is not meant to be used!
   */
}

SatDvbS2Waveform::SatDvbS2Waveform (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t fType, Time frameDur, uint32_t payloadBits)
  : m_modcod (modcod),
  m_frameType (fType),
  m_frameDuration (frameDur),
  m_payloadBits (payloadBits),
  m_cnoRequirement (0.0)
{

}

SatEnums::SatModcod_t
SatDvbS2Waveform::GetModcod () const
{
  NS_LOG_FUNCTION (this);
  return m_modcod;
}

SatEnums::SatBbFrameType_t
SatDvbS2Waveform::GetBbFrameType () const
{
  NS_LOG_FUNCTION (this);
  return m_frameType;
}

uint32_t
SatDvbS2Waveform::GetPayloadInBits () const
{
  NS_LOG_FUNCTION (this);
  return m_payloadBits;
}

Time
SatDvbS2Waveform::GetFrameDuration () const
{
  NS_LOG_FUNCTION (this);
  return m_frameDuration;
}

double
SatDvbS2Waveform::GetCNoRequirement () const
{
  NS_LOG_FUNCTION (this);
  return m_cnoRequirement;
}

void
SatDvbS2Waveform::SetCNoRequirement (double cnoRequirement)
{
  NS_LOG_FUNCTION (this << cnoRequirement);
  m_cnoRequirement = cnoRequirement;
}

void
SatDvbS2Waveform::Dump () const
{
  NS_LOG_FUNCTION (this);

  std::cout << "Modcod, " << SatEnums::GetModcodTypeName (m_modcod) <<
    ", frameType, " << m_frameType <<
    ", payloadBits, " << m_payloadBits <<
    ", frameDuration, " << m_frameDuration <<
    ", cnoRequirement, " << m_cnoRequirement << std::endl;
}

NS_OBJECT_ENSURE_REGISTERED (SatBbFrameConf);


SatBbFrameConf::SatBbFrameConf ()
  : m_symbolRate (100000000.0),
  m_symbolsPerSlot (90),
  m_pilotBlockInSymbols (0),
  m_pilotBlockIntervalInSlots (36),
  m_plHeaderInSlots (1),
  m_dummyFrameInSlots (36),
  m_bbFrameHeaderSizeInBytes (10),
  m_bbFrameHighOccupancyThreshold (0.9),
  m_bbFrameLowOccupancyThreshold (0.5),
  m_targetBler (0.00001),
  m_acmEnabled (false),
  m_defaultModCod (SatEnums::SAT_MODCOD_QPSK_1_TO_2),
  m_shortFramePayloadInSlots (),
  m_normalFramePayloadInSlots (),
  m_waveforms (),
  m_bbFrameUsageMode (SatEnums::NORMAL_FRAMES),
  m_mostRobustShortFrameModcod (SatEnums::SAT_NONVALID_MODCOD),
  m_mostRobustNormalFrameModcod (SatEnums::SAT_NONVALID_MODCOD),
  m_dvbVersion (SatEnums::DVB_S2),
  m_bbFrameS2XPilots (true),
  m_s2XModCodsUsedStr ("")
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("Default constructor not supported!!!");
}

SatBbFrameConf::SatBbFrameConf (double symbolRate, SatEnums::DvbVersion_t dvbVersion)
  : m_symbolRate (symbolRate),
  m_symbolsPerSlot (90),
  m_pilotBlockInSymbols (0),
  m_pilotBlockIntervalInSlots (36),
  m_plHeaderInSlots (1),
  m_dummyFrameInSlots (36),
  m_bbFrameHeaderSizeInBytes (10),
  m_bbFrameHighOccupancyThreshold (0.9),
  m_bbFrameLowOccupancyThreshold (0.5),
  m_targetBler (0.00001),
  m_acmEnabled (false),
  m_defaultModCod (SatEnums::SAT_MODCOD_QPSK_1_TO_2),
  m_shortFramePayloadInSlots (),
  m_normalFramePayloadInSlots (),
  m_waveforms (),
  m_bbFrameUsageMode (SatEnums::NORMAL_FRAMES),
  m_mostRobustShortFrameModcod (SatEnums::SAT_NONVALID_MODCOD),
  m_mostRobustNormalFrameModcod (SatEnums::SAT_NONVALID_MODCOD),
  m_dvbVersion (dvbVersion),
  m_bbFrameS2XPilots (true),
  m_s2XModCodsUsedStr ("")
{
  ObjectBase::ConstructSelf (AttributeConstructionList ());

  GetModCodsList ();

  // Initialize the payloads
  for (uint32_t i = 0; i < 7; ++i)
    {
      m_normalFramePayloadInSlots.insert (std::make_pair (payloadConf[i][0], payloadConf[i][1]));
      m_shortFramePayloadInSlots.insert (std::make_pair (payloadConf[i][0], payloadConf[i][2]));
    }

  // Available MODCODs
  std::vector<SatEnums::SatModcod_t> modcods;

  switch(m_dvbVersion)
  {
    case SatEnums::DVB_S2:
      m_defaultModCod = m_defaultModCodS2;
      SatEnums::GetAvailableModcodsFwdLink (modcods);
      break;
    case SatEnums::DVB_S2X:
      if (m_bbFrameUsageMode == SatEnums::SHORT_AND_NORMAL_FRAMES)
        {
          NS_FATAL_ERROR ("Cannot use NORMAL_AND_SHORT_FRAMES with DVB-S2X ModCods");
        }
      if (m_bbFrameS2XPilots)
        {
          m_defaultModCod = (m_bbFrameUsageMode == SatEnums::NORMAL_FRAMES ? m_defaultModCodNormalFrameS2XPilots : m_defaultModCodShortFrameS2XPilots);
        }
      else
        {
          m_defaultModCod = (m_bbFrameUsageMode == SatEnums::NORMAL_FRAMES ? m_defaultModCodNormalFrameS2XNoPilots : m_defaultModCodShortFrameS2XNoPilots);
        }
      SatEnums::GetAvailableModcodsFwdLinkS2X (modcods, m_bbFrameUsageMode, m_bbFrameS2XPilots);
      break;
  }

  // Available frame types
  std::vector<SatEnums::SatBbFrameType_t> frameTypes;
  SatEnums::GetAvailableBbFrameTypes (frameTypes);

  uint32_t wfCount (1);
  for (std::vector<SatEnums::SatModcod_t>::const_iterator mit = modcods.begin ();
       mit != modcods.end ();
       ++mit)
    {
      switch(m_dvbVersion)
        {
          case SatEnums::DVB_S2:
            for (std::vector<SatEnums::SatBbFrameType_t>::const_iterator fit = frameTypes.begin ();
                   fit != frameTypes.end ();
                   ++fit)
                {
                  // Calculate the payload
                  uint32_t pl = CalculateBbFramePayloadBits (*mit, *fit);

                  // Calculate the frame length
                  Time len = CalculateBbFrameDuration (*mit, *fit);

                  Ptr<SatDvbS2Waveform> wf = Create<SatDvbS2Waveform> (*mit, *fit, len, pl);
                  m_waveforms.insert (std::make_pair (std::make_pair (*mit, *fit), wf));
                  wfCount++;
                }
            break;
          case SatEnums::DVB_S2X:
            SatEnums::SatBbFrameType_t fit = SatUtils::GetFrameTypeS2X (*mit);
            uint32_t pl = CalculateBbFramePayloadBits (*mit, fit);

            // Calculate the frame length
            Time len = CalculateBbFrameDuration (*mit, fit);

            Ptr<SatDvbS2Waveform> wf = Create<SatDvbS2Waveform> (*mit, fit, len, pl);
            m_waveforms.insert (std::make_pair (std::make_pair (*mit, fit), wf));
            wfCount++;
            break;
        }
    }

  /* New to add waveform for short frame with Default ModCod when using
   * DVB-S2X and Normal Frames.
   * Used to create dummy frames which are short frames.
   */
  if (m_dvbVersion == SatEnums::DVB_S2X && m_bbFrameUsageMode == SatEnums::NORMAL_FRAMES)
    {
      SatEnums::SatBbFrameType_t fit = SatEnums::SHORT_FRAME;
      SatEnums::SatModcod_t mit = m_bbFrameS2XPilots ? m_defaultModCodShortFrameS2XPilots : m_defaultModCodShortFrameS2XNoPilots;
      uint32_t pl = CalculateBbFramePayloadBits (mit, fit);

      // Calculate the frame length
      Time len = CalculateBbFrameDuration (mit, fit);

      Ptr<SatDvbS2Waveform> wf = Create<SatDvbS2Waveform> (mit, fit, len, pl);
      m_waveforms.insert (std::make_pair (std::make_pair (mit, fit), wf));
      wfCount++;
    }

  uint32_t payloadBitsForShortFrame = std::numeric_limits<uint32_t>::max ();
  uint32_t payloadBitsForNormalFrame = std::numeric_limits<uint32_t>::max ();

  // find the waveform with the more robust MODCOD than previous one
  for ( waveformMap_t::const_reverse_iterator rit = m_waveforms.rbegin ();
        rit != m_waveforms.rend ();
        ++rit )
    {
      if (rit->second->GetBbFrameType () == SatEnums::SHORT_FRAME)
        {
          // The first waveform over the threshold
          if (rit->second->GetPayloadInBits () < payloadBitsForShortFrame)
            {
              payloadBitsForShortFrame = rit->second->GetPayloadInBits ();
              m_mostRobustShortFrameModcod = rit->second->GetModcod ();
            }
        }

      if (rit->second->GetBbFrameType () == SatEnums::NORMAL_FRAME)
        {
          // The first waveform over the threshold
          if (rit->second->GetPayloadInBits () < payloadBitsForNormalFrame)
            {
              payloadBitsForNormalFrame = rit->second->GetPayloadInBits ();
              m_mostRobustNormalFrameModcod = rit->second->GetModcod ();
            }
        }
    }

  if (     ( (m_bbFrameUsageMode == SatEnums::SHORT_FRAMES) || (m_bbFrameUsageMode == SatEnums::SHORT_AND_NORMAL_FRAMES) )
        && (m_mostRobustShortFrameModcod == SatEnums::SAT_NONVALID_MODCOD ) )
    {
      NS_FATAL_ERROR ("The most robust MODCOD not found for short frames, BB frame configuration error???");
    }
  if (     ( (m_bbFrameUsageMode == SatEnums::NORMAL_FRAMES) || (m_bbFrameUsageMode == SatEnums::SHORT_AND_NORMAL_FRAMES) )
        && (m_mostRobustNormalFrameModcod == SatEnums::SAT_NONVALID_MODCOD ) )
    {
      NS_FATAL_ERROR ("The most robust MODCOD not found for normal frames, BB frame configuration error???");
    }

  if ( (m_bbFrameUsageMode == SatEnums::SHORT_AND_NORMAL_FRAMES) && (m_mostRobustNormalFrameModcod != m_mostRobustShortFrameModcod) )
    {
      // currently is assumed that the most robust MODCODs are same for both short and normal frames
      NS_FATAL_ERROR ("The most robust MODCODs are different for short and normal frames!!!");
    }
}

TypeId
SatBbFrameConf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatBbFrameConf")
    .SetParent<Object> ()
    .AddAttribute ( "SymbolsPerSlot",
                    "Number of symbols per slot",
                    UintegerValue (90),
                    MakeUintegerAccessor (&SatBbFrameConf::m_symbolsPerSlot),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ( "PilotBlockInSymbols",
                    "Pilot block size in symbols",
                    UintegerValue (36),
                    MakeUintegerAccessor (&SatBbFrameConf::m_pilotBlockInSymbols),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ( "PilotBlockIntervalInSlots",
                    "Pilot block interval in slots",
                    UintegerValue (16),
                    MakeUintegerAccessor (&SatBbFrameConf::m_pilotBlockIntervalInSlots),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ( "PlHeaderInSlots",
                    "PL header size in slots",
                    UintegerValue (1),
                    MakeUintegerAccessor (&SatBbFrameConf::m_plHeaderInSlots),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ( "DummyFrameInSlots",
                    "Dummy frame size in slots",
                    UintegerValue (36),
                    MakeUintegerAccessor (&SatBbFrameConf::m_dummyFrameInSlots),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ( "TargetBLER",
                    "Block error rate target",
                    DoubleValue (0.00001),
                    MakeDoubleAccessor (&SatBbFrameConf::m_targetBler),
                    MakeDoubleChecker<double> ())
    .AddAttribute ( "AcmEnabled",
                    "Enable ACM",
                    BooleanValue (false),
                    MakeBooleanAccessor (&SatBbFrameConf::m_acmEnabled),
                    MakeBooleanChecker ())
    .AddAttribute ( "DefaultModCodS2",
                    "Default MODCOD",
                    EnumValue (SatEnums::SAT_MODCOD_QPSK_1_TO_2),
                    MakeEnumAccessor (&SatBbFrameConf::m_defaultModCodS2),
                    // only the top 22 valid MODCODs are included below
                    MakeEnumChecker (SatEnums::SAT_MODCOD_QPSK_1_TO_2,    "QPSK_1_TO_2",
                                     SatEnums::SAT_MODCOD_QPSK_3_TO_5,    "QPSK_3_TO_5",
                                     SatEnums::SAT_MODCOD_QPSK_2_TO_3,    "QPSK_2_TO_3",
                                     SatEnums::SAT_MODCOD_QPSK_3_TO_4,    "QPSK_3_TO_4",
                                     SatEnums::SAT_MODCOD_QPSK_4_TO_5,    "QPSK_4_TO_5",
                                     SatEnums::SAT_MODCOD_QPSK_5_TO_6,    "QPSK_5_TO_6",
                                     SatEnums::SAT_MODCOD_QPSK_8_TO_9,    "QPSK_8_TO_9",
                                     SatEnums::SAT_MODCOD_QPSK_9_TO_10,   "QPSK_9_TO_10",
                                     SatEnums::SAT_MODCOD_8PSK_3_TO_5,    "8PSK_3_TO_5",
                                     SatEnums::SAT_MODCOD_8PSK_2_TO_3,    "8PSK_2_TO_3",
                                     SatEnums::SAT_MODCOD_8PSK_3_TO_4,    "8PSK_3_TO_4",
                                     SatEnums::SAT_MODCOD_8PSK_5_TO_6,    "8PSK_5_TO_6",
                                     SatEnums::SAT_MODCOD_8PSK_8_TO_9,    "8PSK_8_TO_9",
                                     SatEnums::SAT_MODCOD_8PSK_9_TO_10,   "8PSK_9_TO_10",
                                     SatEnums::SAT_MODCOD_16APSK_2_TO_3,  "16APSK_2_TO_3",
                                     SatEnums::SAT_MODCOD_16APSK_3_TO_4,  "16APSK_3_TO_4",
                                     SatEnums::SAT_MODCOD_16APSK_4_TO_5,  "16APSK_4_TO_5",
                                     SatEnums::SAT_MODCOD_16APSK_5_TO_6,  "16APSK_5_TO_6",
                                     SatEnums::SAT_MODCOD_16APSK_8_TO_9,  "16APSK_8_TO_9",
                                     SatEnums::SAT_MODCOD_16APSK_9_TO_10, "16APSK_9_TO_10",
                                     SatEnums::SAT_MODCOD_32APSK_3_TO_4,  "32APSK_3_TO_4",
                                     SatEnums::SAT_MODCOD_32APSK_4_TO_5,  "32APSK_4_TO_5"))
    .AddAttribute ( "DefaultModCodShortFrameS2XNoPilots",
                    "Default MODCOD",
                    EnumValue (SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_NOPILOTS),
                    MakeEnumAccessor (&SatBbFrameConf::m_defaultModCodShortFrameS2XNoPilots),
                    MakeEnumChecker (SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_NOPILOTS,  "QPSK_11_TO_45_SHORT_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_NOPILOTS,    "QPSK_1_TO_4_SHORT_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_NOPILOTS,   "QPSK_4_TO_15_SHORT_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_NOPILOTS,  "QPSK_14_TO_45_SHORT_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_NOPILOTS,    "QPSK_1_TO_3_SHORT_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_NOPILOTS,    "QPSK_2_TO_5_SHORT_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_NOPILOTS,   "QPSK_7_TO_15_SHORT_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_NOPILOTS,    "QPSK_1_TO_2_SHORT_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_NOPILOTS,   "QPSK_8_TO_15_SHORT_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_NOPILOTS,    "QPSK_3_TO_5_SHORT_NOPILOTS"))
    .AddAttribute ( "DefaultModCodNormalFrameS2XNoPilots",
                    "Default MODCOD",
                    EnumValue (SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_NOPILOTS),
                    MakeEnumAccessor (&SatBbFrameConf::m_defaultModCodNormalFrameS2XNoPilots),
                    MakeEnumChecker (SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_NOPILOTS,   "QPSK_1_TO_4_NORMAL_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_NOPILOTS, "QPSK_13_TO_45_NORMAL_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_NOPILOTS,   "QPSK_1_TO_3_NORMAL_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_NOPILOTS,   "QPSK_2_TO_5_NORMAL_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_NOPILOTS,  "QPSK_9_TO_20_NORMAL_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_NOPILOTS,   "QPSK_1_TO_2_NORMAL_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_NOPILOTS, "QPSK_11_TO_20_NORMAL_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_NOPILOTS,   "QPSK_3_TO_5_NORMAL_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_NOPILOTS,   "QPSK_2_TO_3_NORMAL_NOPILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_NOPILOTS,   "QPSK_3_TO_4_NORMAL_NOPILOTS"))
    .AddAttribute ( "DefaultModCodShortFrameS2XPilots",
                    "Default MODCOD",
                    EnumValue (SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_PILOTS),
                    MakeEnumAccessor (&SatBbFrameConf::m_defaultModCodShortFrameS2XPilots),
                    MakeEnumChecker (SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_45_SHORT_NOPILOTS,  "QPSK_11_TO_45_SHORT_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_SHORT_NOPILOTS,    "QPSK_1_TO_4_SHORT_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_4_TO_15_SHORT_NOPILOTS,   "QPSK_4_TO_15_SHORT_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_14_TO_45_SHORT_NOPILOTS,  "QPSK_14_TO_45_SHORT_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_SHORT_NOPILOTS,    "QPSK_1_TO_3_SHORT_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_SHORT_NOPILOTS,    "QPSK_2_TO_5_SHORT_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_7_TO_15_SHORT_NOPILOTS,   "QPSK_7_TO_15_SHORT_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_SHORT_NOPILOTS,    "QPSK_1_TO_2_SHORT_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_8_TO_15_SHORT_NOPILOTS,   "QPSK_8_TO_15_SHORT_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_SHORT_NOPILOTS,    "QPSK_3_TO_5_SHORT_PILOTS"))
    .AddAttribute ( "DefaultModCodNormalFrameS2XPilots",
                    "Default MODCOD",
                    EnumValue (SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_PILOTS),
                    MakeEnumAccessor (&SatBbFrameConf::m_defaultModCodNormalFrameS2XPilots),
                    MakeEnumChecker (SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_4_NORMAL_NOPILOTS,   "QPSK_1_TO_4_NORMAL_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_13_TO_45_NORMAL_NOPILOTS, "QPSK_13_TO_45_NORMAL_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_3_NORMAL_NOPILOTS,   "QPSK_1_TO_3_NORMAL_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_5_NORMAL_NOPILOTS,   "QPSK_2_TO_5_NORMAL_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_9_TO_20_NORMAL_NOPILOTS,  "QPSK_9_TO_20_NORMAL_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_1_TO_2_NORMAL_NOPILOTS,   "QPSK_1_TO_2_NORMAL_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_11_TO_20_NORMAL_NOPILOTS, "QPSK_11_TO_20_NORMAL_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_5_NORMAL_NOPILOTS,   "QPSK_3_TO_5_NORMAL_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_2_TO_3_NORMAL_NOPILOTS,   "QPSK_2_TO_3_NORMAL_PILOTS",
                                     SatEnums::SAT_MODCOD_S2X_QPSK_3_TO_4_NORMAL_NOPILOTS,   "QPSK_3_TO_4_NORMAL_PILOTS"))
    .AddAttribute ( "BbFrameHeaderInBytes",
                    "BB Frame header size in bytes",
                    UintegerValue (10), // ETSI EN 302 307 V1.3.1 specified 80 bits
                    MakeUintegerAccessor (&SatBbFrameConf::m_bbFrameHeaderSizeInBytes),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ( "BbFrameHighOccupancyThreshold",
                    "BB Frame high occupancy threshold [0-1]",
                    DoubleValue (0.9),
                    MakeDoubleAccessor (&SatBbFrameConf::m_bbFrameHighOccupancyThreshold),
                    MakeDoubleChecker<double> (0.0, 1.0))
    .AddAttribute ( "BbFrameLowOccupancyThreshold",
                    "BB Frame low occupancy threshold [0-1]",
                    DoubleValue (0.5),
                    MakeDoubleAccessor (&SatBbFrameConf::m_bbFrameLowOccupancyThreshold),
                    MakeDoubleChecker<double> (0.0, 1.0))
    .AddAttribute ("BBFrameUsageMode",
                   "Mode for selecting used BB Frames.",
                   EnumValue (SatEnums::NORMAL_FRAMES),
                   MakeEnumAccessor (&SatBbFrameConf::m_bbFrameUsageMode),
                   MakeEnumChecker (SatEnums::SHORT_FRAMES, "ShortFrames",
                                    SatEnums::NORMAL_FRAMES, "NormalFrames",
                                    SatEnums::SHORT_AND_NORMAL_FRAMES, "ShortAndNormalFrames"))
    .AddAttribute ("BBFramePilotsS2X",
                   "Use Pilots in DVB-S2X BB Frames.",
                    BooleanValue (true),
                    MakeBooleanAccessor (&SatBbFrameConf::m_bbFrameS2XPilots),
                    MakeBooleanChecker ())
    .AddAttribute ("S2XModCodsUsed",
                   "List of DVB-S2X ModCods used. If nothing specified, all available ModCods are used",
                   StringValue (""),
                   MakeStringAccessor (&SatBbFrameConf::m_s2XModCodsUsedStr),
                   MakeStringChecker ())
    .AddConstructor<SatBbFrameConf> ()
  ;
  return tid;
}

TypeId
SatBbFrameConf::GetInstanceTypeId () const
{
  return GetTypeId ();
}

SatBbFrameConf::~SatBbFrameConf ()
{
  NS_LOG_FUNCTION (this);
}

void
SatBbFrameConf::InitializeCNoRequirements ( Ptr<SatLinkResultsFwd> linkResults )
{
  NS_LOG_FUNCTION (this);

  for (waveformMap_t::iterator it = m_waveforms.begin ();
       it != m_waveforms.end ();
       ++it)
    {
      /**
       * TODO: We have link results for only normal BB frames! The link results for short
       * BB frames should be added and the interface changed to be able to GetEsNoDb for
       * both frame types.
       */
      double esnoRequirementDb = linkResults->GetEsNoDb (it->second->GetModcod (), it->first.second, m_targetBler);

      /**
       * In forward link the link results are in Es/No format, thus here we need
       * to convert the Es/No into C/No:
       * Es/No = (C*Ts)/No = C/No * (1/fs) = C/N
      */
      it->second->SetCNoRequirement (SatUtils::DbToLinear (esnoRequirementDb) * m_symbolRate);
    }
}

void
SatBbFrameConf::DumpWaveforms () const
{
  NS_LOG_FUNCTION (this);

  for (waveformMap_t::const_iterator it = m_waveforms.begin ();
       it != m_waveforms.end ();
       ++it)
    {
      it->second->Dump ();
    }
}

SatEnums::DvbVersion_t
SatBbFrameConf::GetDvbVersion ()
{
  return m_dvbVersion;
}

bool
SatBbFrameConf::IsBbFrameS2XPilots ()
{
  return m_bbFrameS2XPilots;
}

uint32_t
SatBbFrameConf::CalculateBbFramePayloadBits (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType) const
{
  NS_LOG_FUNCTION (this << modcod << frameType);

  uint32_t dataSlots (0);
  uint32_t modulatedBits = SatUtils::GetModulatedBits (modcod);

  switch (frameType)
    {
    case SatEnums::SHORT_FRAME:
      {
        dataSlots = m_shortFramePayloadInSlots.at (modulatedBits);
        break;
      }
    case SatEnums::NORMAL_FRAME:
      {
        dataSlots = m_normalFramePayloadInSlots.at (modulatedBits);
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported enum SatBbFrameType_t!");
        break;
      }
    }

  return (uint32_t)(dataSlots * m_symbolsPerSlot * modulatedBits * SatUtils::GetCodingRate (modcod));
}


Time
SatBbFrameConf::CalculateBbFrameDuration (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType) const
{
  NS_LOG_FUNCTION (this << modcod << frameType);

  uint32_t dataSlots (0);
  uint32_t modulatedBits = SatUtils::GetModulatedBits (modcod);

  switch (frameType)
    {
    case SatEnums::SHORT_FRAME:
      {
        dataSlots = m_shortFramePayloadInSlots.at (modulatedBits);
        break;
      }
    case SatEnums::NORMAL_FRAME:
      {
        dataSlots = m_normalFramePayloadInSlots.at (modulatedBits);
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported enum SatBbFrameType_t!");
        break;
      }
    }

  uint32_t slots = dataSlots + m_plHeaderInSlots;
  uint32_t dataSymbols = slots * m_symbolsPerSlot;
  uint32_t pilotSlots = (uint32_t)(floor ( (double)slots / m_pilotBlockIntervalInSlots ));
  uint32_t pilotSymbols = pilotSlots * m_pilotBlockInSymbols;

  uint32_t totalSymbols = dataSymbols + pilotSymbols;
  double frameLength = totalSymbols / m_symbolRate;

  return Seconds (frameLength);
}

void
SatBbFrameConf::GetModCodsList ()
{
  if (m_s2XModCodsUsedStr.size () == 0)
  {
    SatEnums::GetAvailableModcodsFwdLinkS2X (m_s2XModCodsUsed, m_bbFrameUsageMode, m_bbFrameS2XPilots);
    return;
  }
  std::stringstream strm (m_s2XModCodsUsedStr);
  while (!strm.eof ())
    {
      std::string name;
      strm >> name;
      if (strm.fail ())
        {
          strm.clear ();
          std::string garbage;
          strm >> garbage;
        }
      else
        {
          std::cout << name << std::endl;
          m_s2XModCodsUsed.push_back (SatEnums::GetModcodFromName ("SAT_MODCOD_S2X_" + name +
            "_" + (m_bbFrameUsageMode == SatEnums::NORMAL_FRAMES ? "NORMAL" : "SHORT") +
            "_" + (m_bbFrameS2XPilots ? "PILOTS" : "NOPILOTS")));
          // TODO check if correct: DVB version, pilots, size, etc.
        }
    }
  for (SatEnums::SatModcod_t mc : m_s2XModCodsUsed)
    {
      std::cout << SatEnums::GetModcodTypeName (mc) << std::endl;
    }
  // TODO sort ModCods
}

uint32_t
SatBbFrameConf::GetBbFramePayloadBits (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType) const
{
  NS_LOG_FUNCTION (this << modcod << frameType);
  return m_waveforms.at (std::make_pair (modcod, frameType))->GetPayloadInBits ();
}

double
SatBbFrameConf::GetSymbolRate ()
{
  NS_LOG_FUNCTION (this);
  return m_symbolRate;
}

Time
SatBbFrameConf::GetBbFrameDuration (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType) const
{
  NS_LOG_FUNCTION (this << modcod << frameType);
  return m_waveforms.at (std::make_pair (modcod, frameType))->GetFrameDuration ();
}

Time
SatBbFrameConf::GetDummyBbFrameDuration () const
{
  NS_LOG_FUNCTION (this);
  return Seconds (m_dummyFrameInSlots * m_symbolsPerSlot / m_symbolRate);
}

SatEnums::SatModcod_t
SatBbFrameConf::GetBestModcod (double cNo, SatEnums::SatBbFrameType_t frameType) const
{
  NS_LOG_FUNCTION (this << frameType);

  // If ACM is disabled, return the default MODCOD
  if (!m_acmEnabled)
    {
      return m_defaultModCod;
    }

  // Return the waveform with best spectral efficiency
  for ( waveformMap_t::const_reverse_iterator rit = m_waveforms.rbegin ();
        rit != m_waveforms.rend ();
        ++rit )
    {
      if (rit->second->GetBbFrameType () == frameType)
        {
          double cnoReq = rit->second->GetCNoRequirement ();
          // The first waveform over the threshold
          if (cnoReq <= cNo)
            {
              return rit->second->GetModcod ();
            }
        }
    }
  return m_defaultModCod;
}

SatEnums::SatModcod_t
SatBbFrameConf::GetMostRobustModcod (SatEnums::SatBbFrameType_t frameType) const
{
  NS_LOG_FUNCTION (this << frameType);

  SatEnums::SatModcod_t mostRobustModcod = SatEnums::SAT_NONVALID_MODCOD;

  if ( frameType == SatEnums::SHORT_FRAME )
    {
      mostRobustModcod = m_mostRobustShortFrameModcod;
    }
  else if ( frameType == SatEnums::NORMAL_FRAME )
    {
      mostRobustModcod = m_mostRobustNormalFrameModcod;
    }
  else
    {
      NS_FATAL_ERROR ("Not supported frame type!!!");
    }

  return mostRobustModcod;
}

SatEnums::SatModcod_t
SatBbFrameConf::GetDefaultModCod () const
{
  NS_LOG_FUNCTION (this);
  return m_defaultModCod;
}

SatEnums::SatModcod_t
SatBbFrameConf::GetDefaultModCodShortFramesS2X () const
{
  NS_LOG_FUNCTION (this);
  return m_bbFrameS2XPilots ? m_defaultModCodShortFrameS2XPilots : m_defaultModCodShortFrameS2XNoPilots;
}


}  // namespace ns3
