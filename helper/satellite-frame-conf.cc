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

SatBtuConf::SatBtuConf (double bandwidth_hz, double rollOff, double spacing)
  : m_allocatedBandwidth_hz (bandwidth_hz)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT ( (spacing >= 0.00 ) && ( spacing < 1.00 ) );
  NS_ASSERT ( (rollOff >= 0.00 ) && ( rollOff < 1.00 ) );

  m_occupiedBandwidth_hz = m_allocatedBandwidth_hz / (rollOff + 1.00);
  m_effectiveBandwidth_hz = m_occupiedBandwidth_hz / (rollOff + spacing + 1.00);

  m_length_s = 1 / m_effectiveBandwidth_hz;
}

SatBtuConf::~SatBtuConf ()
{
  NS_LOG_FUNCTION (this);
}

// Time Slot conf

SatTimeSlotConf::SatTimeSlotConf ()
{
  // default constructor should not be used
  NS_ASSERT (false);
}

SatTimeSlotConf::SatTimeSlotConf (double startTime_s, double duration_s, uint32_t waveFormId, uint32_t frameCarrierId)
  : m_startTime_s (startTime_s),
    m_duration_s (duration_s),
    m_waveFormId (waveFormId),
    m_frameCarrierId (frameCarrierId)
{
  NS_LOG_FUNCTION (this);
}

SatTimeSlotConf::~SatTimeSlotConf ()
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
                             Ptr<SatBtuConf> btu, SatTimeSlotConfList * timeSlots)
  : m_bandwidth_hz (bandwidth_hz),
    m_duration_s (duration_s),
    m_nextTimeSlotId (0),
    m_btu (btu)
{
  NS_LOG_FUNCTION (this);

  m_carrierCount = bandwidth_hz / btu->GetAllocatedBandwidth_hz();

  if ( timeSlots != NULL )
    {
      m_timeSlots = *timeSlots;

      if ( m_timeSlots.empty() != false )
        {
          SatTimeSlotConfList::iterator lastSlot = m_timeSlots.end()--;
          m_nextTimeSlotId = lastSlot->first + 1;
        }
    }
}

SatFrameConf::~SatFrameConf ()
{
  NS_LOG_FUNCTION (this);
}

double
SatFrameConf::GetCarrierFrequency_hz (uint32_t carrierId) const
{
  NS_ASSERT (carrierId < m_carrierCount);

  double carrierBandwidth_hz = m_btu->GetAllocatedBandwidth_hz();

  return ( (carrierBandwidth_hz * carrierId) + ( carrierBandwidth_hz / 2.0 ) );
}

Ptr<SatTimeSlotConf>
SatFrameConf::GetTimeSlotConf (uint16_t index) const
{
  NS_LOG_FUNCTION (this);

  SatTimeSlotConfList::const_iterator foundTimeSlot = m_timeSlots.find(index);

  NS_ASSERT (foundTimeSlot != m_timeSlots.end() );

  return foundTimeSlot->second;
}

uint16_t
SatFrameConf::AddTimeSlotConf (Ptr<SatTimeSlotConf> conf)
{
  NS_LOG_FUNCTION (this);

  std::pair< SatTimeSlotConfList::const_iterator, bool> result = m_timeSlots.insert(std::make_pair (m_nextTimeSlotId, conf) );
  NS_ASSERT (result.second == true);

  m_nextTimeSlotId++;

  // return time slot id of added configuration (key value of the map)
  return result.first->first;
}

// Super frame conf

SatSuperframeConf::SatSuperframeConf ()
{
  // default constructor should not be used
  NS_ASSERT (false);
}

SatSuperframeConf::SatSuperframeConf ( double bandwidth_hz, double duration_s, SatFrameConfList * frames)
  : m_bandwidth_hz (bandwidth_hz),
    m_duration_s (duration_s)

{
  NS_LOG_FUNCTION (this);

  if ( frames != NULL)
    {
      m_frames = *frames;
    }
}

SatSuperframeConf::~SatSuperframeConf ()
{
  NS_LOG_FUNCTION (this);
}

void
SatSuperframeConf::AddFrameConf (Ptr<SatFrameConf> conf)
{
  NS_LOG_FUNCTION (this);

  m_frames.push_back (conf);
}

Ptr<SatFrameConf>
SatSuperframeConf::GetFrameConf (uint8_t index) const
{
  NS_LOG_FUNCTION (this);

  return m_frames[index];
}

uint32_t
SatSuperframeConf::GetCarrierCount () const
{
  uint32_t carrierCount = 0;

  for (uint8_t i = 0; i < m_frames.size(); i++)
    {
      carrierCount += m_frames[i]->GetCarrierCount();
    }

  return carrierCount;
}

uint32_t
SatSuperframeConf::GetCarrierId ( uint8_t frameId, uint16_t frameCarrierId ) const
{
  uint32_t carrierId = frameCarrierId;

  for (uint8_t i = 0; i < frameId; i++)
    {
      NS_ASSERT ( i < m_frames.size());

      carrierId += m_frames[i]->GetCarrierCount();
    }

  return carrierId;
}

double
SatSuperframeConf::GetCarrierFrequency_hz (uint32_t carrierId) const
{
  double frameStartFrequency = 0.0;
  uint32_t currentFrame = 0;
  uint32_t lastIdInFrame = m_frames[0]->GetCarrierCount() - 1;
  uint32_t carrierIdInFrame = carrierId;

  while( carrierId > lastIdInFrame )
    {
      carrierIdInFrame -= m_frames[currentFrame]->GetCarrierCount();
      frameStartFrequency += m_frames[currentFrame]->GetBandwidth_hz();
      currentFrame++;
      lastIdInFrame += m_frames[currentFrame]->GetCarrierCount();
    }

  double carrierFrequencyInFrame = m_frames[currentFrame]->GetCarrierFrequency_hz( carrierIdInFrame );

  return frameStartFrequency + carrierFrequencyInFrame;
}

double
SatSuperframeConf::GetCarrierBandwidth_hz (uint32_t carrierId) const
{
  uint32_t currentFrame = 0;
  uint32_t lastIdInFrame = m_frames[0]->GetCarrierCount() - 1;
  uint32_t carrierIdInFrame = carrierId;

  while( carrierId > lastIdInFrame )
    {
      carrierIdInFrame -= m_frames[currentFrame]->GetCarrierCount();
      currentFrame++;
      lastIdInFrame += m_frames[currentFrame]->GetCarrierCount();
    }

  return m_frames[currentFrame]->GetCarrierBandwidth_hz();
}


}; // namespace ns3
