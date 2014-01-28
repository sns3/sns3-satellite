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

#include <algorithm>
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

SatBtuConf::SatBtuConf (double bandwidthHz, double rollOff, double spacing)
  : m_allocatedBandwidthHz (bandwidthHz)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT ( (spacing >= 0.00 ) && ( spacing < 1.00 ) );
  NS_ASSERT ( (rollOff >= 0.00 ) && ( rollOff < 1.00 ) );

  m_occupiedBandwidthHz = m_allocatedBandwidthHz / (rollOff + 1.00);
  m_effectiveBandwidthHz = m_allocatedBandwidthHz / (rollOff + spacing + 1.00);

  m_length_s = 1 / m_effectiveBandwidthHz;
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

SatTimeSlotConf::SatTimeSlotConf (double startTimeInSeconds, uint32_t waveFormId, uint32_t frameCarrierId)
  : m_startTimeInSeconds (startTimeInSeconds),
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

SatFrameConf::SatFrameConf ( double bandwidthHz, double durationInSeconds,
                             Ptr<SatBtuConf> btu, SatTimeSlotConfList_t * timeSlots)
  : m_bandwidthHz (bandwidthHz),
    m_durationInSeconds (durationInSeconds),
    m_nextTimeSlotId (0),
    m_btu (btu)
{
  NS_LOG_FUNCTION (this);

  m_carrierCount = bandwidthHz / btu->GetAllocatedBandwidthHz ();

  if ( timeSlots != NULL )
    {
      m_timeSlotConfs = *timeSlots;

      if ( m_timeSlotConfs.empty () == false )
        {
          SatTimeSlotConfList_t::iterator lastSlot = m_timeSlotConfs.end ()--;
          m_nextTimeSlotId = lastSlot->first + 1;
        }
    }
}

SatFrameConf::~SatFrameConf ()
{
  NS_LOG_FUNCTION (this);
}

double
SatFrameConf::GetCarrierFrequencyHz (uint32_t carrierId) const
{
  NS_ASSERT (carrierId < m_carrierCount );

  double carrierBandwidthHz = m_btu->GetAllocatedBandwidthHz ();

  return ( (carrierBandwidthHz * carrierId) + ( carrierBandwidthHz / 2.0 ) );
}

Ptr<SatTimeSlotConf>
SatFrameConf::GetTimeSlotConf (uint16_t index) const
{
  NS_LOG_FUNCTION (this);

  SatTimeSlotConfList_t::const_iterator foundTimeSlot = m_timeSlotConfs.find (index);

  NS_ASSERT (foundTimeSlot != m_timeSlotConfs.end () );

  return foundTimeSlot->second;
}

SatFrameConf::SatTimeSlotIdList_t
SatFrameConf::GetTimeSlotIds (uint32_t carrierId) const
{
  NS_LOG_FUNCTION (this);

  std::pair < SatCarrierTimeSlotId_t::const_iterator,
              SatCarrierTimeSlotId_t::const_iterator> timeSlotRange = m_carrierTimeSlotIds.equal_range (carrierId);

  SatTimeSlotIdList_t timeSlots;


  for (SatCarrierTimeSlotId_t::const_iterator it = timeSlotRange.first; it != timeSlotRange.second; it++)
    {
      timeSlots.push_back (it->second);
    }

  return timeSlots;
}

uint16_t
SatFrameConf::AddTimeSlotConf (Ptr<SatTimeSlotConf> conf)
{
  NS_LOG_FUNCTION (this);

  std::pair<SatTimeSlotConfList_t::const_iterator, bool> result = m_timeSlotConfs.insert (std::make_pair (m_nextTimeSlotId, conf) );
  NS_ASSERT (result.second == true);

  m_carrierTimeSlotIds.insert (std::make_pair (conf->GetCarrierId(), m_nextTimeSlotId));

  return m_nextTimeSlotId++;
}

// Super frame conf

SatSuperframeConf::SatSuperframeConf ()
{
  // default constructor should not be used
  NS_ASSERT (false);
}

SatSuperframeConf::SatSuperframeConf ( double bandwidthHz, double durationInSeconds, SatFrameConfList_t * frames)
  : m_bandwidthHz (bandwidthHz),
    m_durationInSeconds (durationInSeconds)

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
  NS_LOG_FUNCTION (this);

  uint32_t carrierCount = 0;

  for (uint8_t i = 0; i < m_frames.size (); i++)
    {
      carrierCount += m_frames[i]->GetCarrierCount ();
    }

  return carrierCount;
}

uint32_t
SatSuperframeConf::GetCarrierId ( uint8_t frameId, uint16_t frameCarrierId ) const
{
  NS_LOG_FUNCTION (this);

  uint32_t carrierId = frameCarrierId;

  for (uint8_t i = 0; i < frameId; i++)
    {
      NS_ASSERT ( i < m_frames.size ());

      carrierId += m_frames[i]->GetCarrierCount ();
    }

  return carrierId;
}

double
SatSuperframeConf::GetCarrierFrequencyHz (uint32_t carrierId) const
{
  NS_LOG_FUNCTION (this);

  double frameStartFrequency = 0.0;
  uint32_t currentFrame = 0;
  uint32_t lastIdInFrame = m_frames[0]->GetCarrierCount () - 1;
  uint32_t carrierIdInFrame = carrierId;

  while( carrierId > lastIdInFrame )
    {
      carrierIdInFrame -= m_frames[currentFrame]->GetCarrierCount ();
      frameStartFrequency += m_frames[currentFrame]->GetBandwidthHz ();
      currentFrame++;
      lastIdInFrame += m_frames[currentFrame]->GetCarrierCount ();
    }

  double carrierFrequencyInFrame = m_frames[currentFrame]->GetCarrierFrequencyHz (carrierIdInFrame);

  return frameStartFrequency + carrierFrequencyInFrame;
}

double
SatSuperframeConf::GetCarrierBandwidthHz (uint32_t carrierId, SatEnums::CarrierBandwidthType_t bandwidthType) const
{
  NS_LOG_FUNCTION (this);

  uint32_t currentFrame = 0;
  uint32_t lastIdInFrame = m_frames[0]->GetCarrierCount () - 1;
  uint32_t carrierIdInFrame = carrierId;

  while (carrierId > lastIdInFrame)
    {
      carrierIdInFrame -= m_frames[currentFrame]->GetCarrierCount ();
      currentFrame++;
      lastIdInFrame += m_frames[currentFrame]->GetCarrierCount ();
    }

  return m_frames[currentFrame]->GetCarrierBandwidthHz (bandwidthType);
}


}; // namespace ns3
