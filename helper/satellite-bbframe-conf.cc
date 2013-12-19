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
#include "satellite-bbframe-conf.h"
#include "ns3/satellite-enums.h"

NS_LOG_COMPONENT_DEFINE ("SatBbFrameConf");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatBbFrameConf);

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


SatBbFrameConf::SatBbFrameConf ()
{
  for (uint32_t i = 0; i < 4; ++i)
    {
      m_payloadsNormalFrame.insert (std::make_pair (payloadConf[i][0], payloadConf[i][1]));
      m_payloadsShortFrame.insert (std::make_pair (payloadConf[i][0], payloadConf[i][2]));
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
    .AddConstructor<SatBbFrameConf> ()
  ;
  return tid;
}

SatBbFrameConf::~SatBbFrameConf ()
{
  NS_LOG_FUNCTION (this);
}

Time
SatBbFrameConf::GetBbFrameLength (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType, double symbolRate) const
{
  uint32_t dataSlots (0);
  uint32_t modulatedBits = GetModulatedBits (modcod);

  switch (frameType)
  {
    case SatEnums::SHORT_FRAME:
      {
        dataSlots = m_payloadsShortFrame.at (modulatedBits);
        break;
      }
    case SatEnums::NORMAL_FRAME:
      {
        dataSlots = m_payloadsNormalFrame.at (modulatedBits);
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
  double frameLength = totalSymbols / symbolRate;

  return Seconds (frameLength);
}

Time
SatBbFrameConf::GetDummyBbFrameLength (double symbolRate) const
{
  return Seconds (m_dummyFrameInSlots * m_symbolsPerSlot / symbolRate);
}

uint32_t
SatBbFrameConf::GetBbFramePayloadBits (SatEnums::SatModcod_t modcod, SatEnums::SatBbFrameType_t frameType) const
{
  uint32_t dataSlots (0);
  uint32_t modulatedBits = GetModulatedBits (modcod);

  switch (frameType)
  {
    case SatEnums::SHORT_FRAME:
      {
        dataSlots = m_payloadsShortFrame.at (modulatedBits);
        break;
      }
    case SatEnums::NORMAL_FRAME:
      {
        dataSlots = m_payloadsNormalFrame.at (modulatedBits);
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported enum SatBbFrameType_t!");
        break;
      }
  }
  return dataSlots * m_symbolsPerSlot * modulatedBits * GetCodingRate (modcod);
}

uint32_t
SatBbFrameConf::GetModulatedBits (SatEnums::SatModcod_t modcod) const
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
    case  SatEnums::SAT_MODCOD_8PSK_3_TO_5:
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

double
SatBbFrameConf::GetCodingRate (SatEnums::SatModcod_t modcod) const
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


}; // namespace ns3
