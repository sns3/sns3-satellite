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


#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/enum.h"
#include "ns3/satellite-enums.h"
#include "ns3/satellite-link-results.h"
#include "ns3/satellite-utils.h"

#include "satellite-bbframe-conf.h"

NS_LOG_COMPONENT_DEFINE ("SatBbFrameConf");

namespace ns3 {

/**
 * 1st col = modulated bits
 * 2nd col = payload slots per normal frame
 * 3rnd col = payload slots per short frame
 */
const uint32_t payloadConf[4][3] = { { 2, 360, 90 },
                                     { 3, 240, 60 },
                                     { 4, 180, 45 },
                                     { 5, 144, 36 },
};


SatDvbS2Waveform::SatDvbS2Waveform ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (true);
}

SatDvbS2Waveform::SatDvbS2Waveform(SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t fType, Time frameLen, uint32_t payloadBits)
:m_modcod (modcod),
 m_frameType (fType),
 m_frameLength (frameLen),
 m_payloadBits (payloadBits),
 m_cnoRequirement (0.0)
{

}

SatEnums::SatModcod_t
SatDvbS2Waveform::GetModcod () const
{
  return m_modcod;
}

SatEnums::SatBbFrameType_t
SatDvbS2Waveform::GetBbFrameType () const
{
  return m_frameType;
}

uint32_t
SatDvbS2Waveform::GetPayloadInBits () const
{
  return m_payloadBits;
}

Time
SatDvbS2Waveform::GetFrameLength () const
{
  return m_frameLength;
}

double
SatDvbS2Waveform::GetCNoRequirement () const
{
  return m_cnoRequirement;
}

void
SatDvbS2Waveform::SetCNoRequirement (double cnoRequirement)
{
  m_cnoRequirement = cnoRequirement;
}

void
SatDvbS2Waveform::Dump () const
{
  std::cout << "Modcod, " << SatEnums::GetModcodTypeName (m_modcod) <<
      ", frameType, " << m_frameType <<
      ", payloadBits, " << m_payloadBits <<
      ", frameLength, " << m_frameLength <<
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
  m_targetBler (0.00001),
  m_acmEnabled (false),
  m_defaultModCod (SatEnums::SAT_MODCOD_QPSK_1_TO_2),
  m_shortFramePayloadBits (),
  m_normalFramePayloadBits (),
  m_waveforms ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (true);
}

SatBbFrameConf::SatBbFrameConf (double symbolRate)
: m_symbolRate (symbolRate),
  m_symbolsPerSlot (90),
  m_pilotBlockInSymbols (0),
  m_pilotBlockIntervalInSlots (36),
  m_plHeaderInSlots (1),
  m_dummyFrameInSlots (36),
  m_targetBler (0.00001),
  m_acmEnabled (false),
  m_defaultModCod (SatEnums::SAT_MODCOD_QPSK_1_TO_2),
  m_shortFramePayloadBits (),
  m_normalFramePayloadBits (),
  m_waveforms ()
{
  ObjectBase::ConstructSelf(AttributeConstructionList ());

  // Initialize the payloads
  for (uint32_t i = 0; i < 4; ++i)
    {
      m_normalFramePayloadBits.insert (std::make_pair (payloadConf[i][0], payloadConf[i][1]));
      m_shortFramePayloadBits.insert (std::make_pair (payloadConf[i][0], payloadConf[i][2]));
    }

  // Available MODCODs
  std::vector<SatEnums::SatModcod_t> modcods;
  SatEnums::GetAvailableModcods (modcods);

  // Available frame types
  std::vector<SatEnums::SatBbFrameType_t> frameTypes;
  SatEnums::GetAvailableBbFrameTypes (frameTypes);

  uint32_t wfCount (1);
  for (std::vector<SatEnums::SatModcod_t>::const_iterator mit = modcods.begin ();
      mit != modcods.end ();
      ++mit)
    {
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
    }
}

TypeId
SatBbFrameConf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatBbFrameConf")
    .SetParent<Object> ()
    .AddAttribute( "SymbolsPerSlot",
                   "Number of symbols per slot",
                    UintegerValue (90),
                    MakeUintegerAccessor(&SatBbFrameConf::m_symbolsPerSlot),
                    MakeUintegerChecker<uint32_t>())
    .AddAttribute( "PilotBlockInSymbols",
                   "Pilot block size in symbols",
                   UintegerValue (36),
                   MakeUintegerAccessor (&SatBbFrameConf::m_pilotBlockInSymbols),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute( "PilotBlockIntervalInSlots",
                   "Pilot block interval in slots",
                   UintegerValue (16),
                   MakeUintegerAccessor (&SatBbFrameConf::m_pilotBlockIntervalInSlots),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute( "PlHeaderInSlots",
                   "PL header size in slots",
                   UintegerValue (1),
                   MakeUintegerAccessor (&SatBbFrameConf::m_plHeaderInSlots),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute( "DummyFrameInSlots",
                   "Dummy frame size in slots",
                   UintegerValue (36),
                   MakeUintegerAccessor (&SatBbFrameConf::m_dummyFrameInSlots),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute( "TargetBLER",
                   "Block error rate target",
                   DoubleValue (0.00001),
                   MakeDoubleAccessor(&SatBbFrameConf::m_targetBler),
                   MakeDoubleChecker<double>())
    .AddAttribute( "AcmEnabled",
                   "Enable ACM",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatBbFrameConf::m_acmEnabled),
                   MakeBooleanChecker ())
    .AddAttribute( "DefaultModCod",
                   "Default MODCOD",
                   // TODO: most robust MODCOD needed to set SAT_MODCOD_QPSK_1_TO_2 when TBTP message length problem is solved.
                   EnumValue (SatEnums::SAT_MODCOD_QPSK_1_TO_2),
                   MakeEnumAccessor (&SatBbFrameConf::m_defaultModCod),
                   MakeEnumChecker (SatEnums::SAT_MODCOD_QPSK_1_TO_2,  "QPSK_1_TO_2",
                                    SatEnums::SAT_MODCOD_QPSK_2_TO_3, "QPSK_2_TO_3",
                                    SatEnums::SAT_MODCOD_QPSK_3_TO_4, "QPSK_3_TO_4",
                                    SatEnums::SAT_MODCOD_QPSK_3_TO_5, "QPSK_3_TO_5",
                                    SatEnums::SAT_MODCOD_QPSK_4_TO_5, "QPSK_4_TO_5",
                                    SatEnums::SAT_MODCOD_QPSK_5_TO_6, "QPSK_5_TO_6",
                                    SatEnums::SAT_MODCOD_QPSK_8_TO_9, "QPSK_8_TO_9",
                                    SatEnums::SAT_MODCOD_QPSK_9_TO_10, "QPSK_9_TO_10",
                                    SatEnums::SAT_MODCOD_8PSK_2_TO_3, "8PSK_2_TO_3",
                                    SatEnums::SAT_MODCOD_8PSK_3_TO_4, "8PSK_3_TO_4",
                                    SatEnums::SAT_MODCOD_8PSK_3_TO_5, "8PSK_3_TO_5",
                                    SatEnums::SAT_MODCOD_8PSK_5_TO_6, "8PSK_5_TO_6",
                                    SatEnums::SAT_MODCOD_8PSK_8_TO_9, "8PSK_8_TO_9",
                                    SatEnums::SAT_MODCOD_8PSK_9_TO_10, "8PSK_9_TO_10",
                                    SatEnums::SAT_MODCOD_16APSK_2_TO_3, "16APSK_2_TO_3",
                                    SatEnums::SAT_MODCOD_16APSK_3_TO_4, "16APSK_3_TO_4",
                                    SatEnums::SAT_MODCOD_16APSK_4_TO_5, "16APSK_4_TO_5",
                                    SatEnums::SAT_MODCOD_16APSK_5_TO_6, "16APSK_5_TO_6",
                                    SatEnums::SAT_MODCOD_16APSK_8_TO_9, "16APSK_8_TO_9",
                                    SatEnums::SAT_MODCOD_16APSK_9_TO_10, "16APSK_9_TO_10",
                                    SatEnums::SAT_MODCOD_32APSK_3_TO_4, "32APSK_3_TO_4",
                                    SatEnums::SAT_MODCOD_32APSK_4_TO_5, "32APSK_4_TO_5"))
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
SatBbFrameConf::InitializeCNoRequirements( Ptr<SatLinkResultsDvbS2> linkResults )
{
  NS_LOG_FUNCTION (this);

  for (waveformMap_t::iterator it = m_waveforms.begin ();
      it != m_waveforms.end ();
      ++it)
    {
      /**
       * TODO, We have link results for only normal BB frames! The link results for short
       * BB frames should be added and the interface changed to be able to GetEsNoDb for
       * both frame types.
       */
      double esnoRequirementDb = linkResults->GetEsNoDb (it->second->GetModcod(), m_targetBler);

      /**
       * In forward link the link results are in Es/No format, thus here we need
       * to convert the Es/No into C/No:
       * Es/No = (C*Ts)/No = C/No * (1/fs) = C/N
      */
      it->second->SetCNoRequirement (SatUtils::DbToLinear (esnoRequirementDb) * m_symbolRate);
    }
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
        dataSlots = m_shortFramePayloadBits.at (modulatedBits);
        break;
      }
    case SatEnums::NORMAL_FRAME:
      {
        dataSlots = m_normalFramePayloadBits.at (modulatedBits);
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported enum SatBbFrameType_t!");
        break;
      }
  }
  return dataSlots * m_symbolsPerSlot * modulatedBits * SatUtils::GetCodingRate (modcod);
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
        dataSlots = m_shortFramePayloadBits.at (modulatedBits);
        break;
      }
    case SatEnums::NORMAL_FRAME:
      {
        dataSlots = m_normalFramePayloadBits.at (modulatedBits);
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
  uint32_t pilotSlots = (uint32_t)(floor( (double)slots / m_pilotBlockIntervalInSlots ));
  uint32_t pilotSymbols = pilotSlots * m_pilotBlockInSymbols;

  uint32_t totalSymbols = dataSymbols + pilotSymbols;
  double frameLength = totalSymbols / m_symbolRate;

  return Seconds (frameLength);
}

uint32_t
SatBbFrameConf::GetBbFramePayloadBits (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType) const
{
  NS_LOG_FUNCTION (this << modcod << frameType);
  return m_waveforms.at (std::make_pair(modcod, frameType))->GetPayloadInBits ();
}

Time
SatBbFrameConf::GetBbFrameDuration (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType) const
{
  NS_LOG_FUNCTION (this << modcod << frameType);
  return m_waveforms.at (std::make_pair(modcod, frameType))->GetFrameLength ();
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

  // Return the waveform with best spectral efficiency
  // JPU, Note, that this algorithm is not final, but just a skeleton which shall be enhanced
  // when implementing the actual NCC RTN link burst scheduler algorithm!

  // If ACM is disabled, return the default MODCOD
  if (!m_acmEnabled)
    {
      return m_defaultModCod;
    }

  for ( waveformMap_t::const_reverse_iterator rit = m_waveforms.rbegin ();
      rit != m_waveforms.rend ();
      ++rit )
    {
      if (rit->second->GetBbFrameType() == frameType)
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
SatBbFrameConf::GetDefaultModCod () const
{
  return m_defaultModCod;
}


}; // namespace ns3
