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

#include "ns3/log.h"

#include "satellite-frame-conf.h"

NS_LOG_COMPONENT_DEFINE ("SatFrameConf");

namespace ns3 {

// BTU conf

SatBtuConf::SatBtuConf ()
{
  // default constructor should not be used
  NS_ASSERT (false);
}

SatBtuConf::SatBtuConf (double bandwidth_hz, double length_s, double symbolRate_baud)
  : m_bandwidth_hz (bandwidth_hz),
    m_length_s (length_s),
    m_symbolRate_baud (symbolRate_baud)
{
  NS_LOG_FUNCTION (this);
}

SatBtuConf::~SatBtuConf ()
{
  NS_LOG_FUNCTION (this);
}

// Time Slot conf

SatTimelSlotConf::SatTimelSlotConf ()
{
  // default constructor should not be used
  NS_ASSERT (false);
}

SatTimelSlotConf::SatTimelSlotConf (double startTime_s, double duration_s, uint8_t waveFormId, uint32_t frameCarrierId)
  : m_startTime_s (startTime_s),
    m_duration_s (duration_s),
    m_waveFormId (waveFormId),
    m_frameCarrierId (frameCarrierId)
{
  NS_LOG_FUNCTION (this);
}

SatTimelSlotConf::~SatTimelSlotConf ()
{
  NS_LOG_FUNCTION (this);
}

// Frame conf

SatFrameConf::SatFrameConf ()
{
  // default constructor should not be used
  NS_ASSERT (false);
}

SatFrameConf::SatFrameConf ( double bandwidth_hz, double duration_s,
                             Ptr<SatBtuConf> btu, std::vector<Ptr<SatTimelSlotConf> > * timeSlots)
  : m_bandwidth_hz (bandwidth_hz),
    m_duration_s (duration_s),
    m_btu (btu)
{
  NS_LOG_FUNCTION (this);

  m_timeSlots = *timeSlots;
}

SatFrameConf::~SatFrameConf ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<SatTimelSlotConf>
SatFrameConf::GetTimeSlotConf (uint16_t index)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT ( index < m_timeSlots.size() );

  return m_timeSlots[index];
}

// Super frame conf

SatSuperFrameConf::SatSuperFrameConf ()
{
  // default constructor should not be used
  NS_ASSERT (false);
}

SatSuperFrameConf::SatSuperFrameConf ( double frequency_hz, double bandwidth_hz, double duration_s,
                                       std::vector<Ptr<SatFrameConf> > * frames)
  : m_frequency_hz (frequency_hz),
    m_bandwidth_hz (bandwidth_hz),
    m_duration_s (duration_s)

{
  NS_LOG_FUNCTION (this);

  m_frames = *frames;
}

SatSuperFrameConf::~SatSuperFrameConf ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<SatFrameConf>
SatSuperFrameConf::GetFrameConf (uint8_t index)
{
  NS_LOG_FUNCTION (this);

  return m_frames[index];
}

uint32_t
SatSuperFrameConf::GetCarriedId( uint8_t frameId, uint16_t frameCarrierId )
{
  uint32_t  carrierId = frameCarrierId;

  for (int i = 0; i < frameId; i++)
    {
      carrierId += m_frames[i]->GetCarrierCount();
    }

  return carrierId;
}


}; // namespace ns3
