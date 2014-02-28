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
#include <string>
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"

#include "satellite-frame-conf.h"

NS_LOG_COMPONENT_DEFINE ("SatFrameConf");

namespace ns3 {

// BTU conf

SatBtuConf::SatBtuConf ()
  : m_allocatedBandwidthHz (0.0),
    m_occupiedBandwidthHz (0.0),
    m_effectiveBandwidthHz (0.0),
    m_lengthInSeconds (0.0)
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

  m_lengthInSeconds = 1 / m_effectiveBandwidthHz;
}

SatBtuConf::~SatBtuConf ()
{
  NS_LOG_FUNCTION (this);
}

// Time Slot conf

SatTimeSlotConf::SatTimeSlotConf ()
: m_startTimeInSeconds (0.0),
  m_waveFormId (0),
  m_frameCarrierId (0)
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
 : m_bandwidthHz (0.0),
   m_durationInSeconds (0.0),
   m_nextTimeSlotId (0),
   m_isRandomAccess (false),
   m_btu (0),
   m_carrierCount (0)
{
  // default constructor should not be used
  NS_ASSERT (false);
}

SatFrameConf::SatFrameConf ( double bandwidthHz, double durationInSeconds,
                             Ptr<SatBtuConf> btu, SatTimeSlotConfContainer_t * timeSlots, bool isRandomAccess)
  : m_bandwidthHz (bandwidthHz),
    m_durationInSeconds (durationInSeconds),
    m_nextTimeSlotId (0),
    m_isRandomAccess (isRandomAccess),
    m_btu (btu)
{
  NS_LOG_FUNCTION (this);

  m_carrierCount = bandwidthHz / btu->GetAllocatedBandwidthHz ();

  if ( timeSlots != NULL )
    {
      m_timeSlotConfs = *timeSlots;

      if ( m_timeSlotConfs.empty () == false )
        {
          SatTimeSlotConfContainer_t::iterator lastSlot = m_timeSlotConfs.end ()--;
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

  SatTimeSlotConfContainer_t::const_iterator foundTimeSlot = m_timeSlotConfs.find (index);

  NS_ASSERT (foundTimeSlot != m_timeSlotConfs.end () );

  return foundTimeSlot->second;
}

SatFrameConf::SatTimeSlotIdContainer_t
SatFrameConf::GetTimeSlotIds (uint32_t carrierId) const
{
  NS_LOG_FUNCTION (this);

  std::pair < SatCarrierTimeSlotMap_t::const_iterator,
              SatCarrierTimeSlotMap_t::const_iterator> timeSlotRange = m_carrierTimeSlotIds.equal_range (carrierId);

  SatTimeSlotIdContainer_t timeSlots;

  for (SatCarrierTimeSlotMap_t::const_iterator it = timeSlotRange.first; it != timeSlotRange.second; it++)
    {
      timeSlots.push_back (it->second);
    }

  return timeSlots;
}

uint16_t
SatFrameConf::AddTimeSlotConf (Ptr<SatTimeSlotConf> conf)
{
  NS_LOG_FUNCTION (this);

  std::pair<SatTimeSlotConfContainer_t::const_iterator, bool> result = m_timeSlotConfs.insert (std::make_pair (m_nextTimeSlotId, conf) );
  NS_ASSERT (result.second == true);

  m_carrierTimeSlotIds.insert (std::make_pair (conf->GetCarrierId(), m_nextTimeSlotId));

  return m_nextTimeSlotId++;
}
NS_OBJECT_ENSURE_REGISTERED (SatSuperframeConf);

// Super frame configuration interface.

SatSuperframeConf::SatSuperframeConf ()
 : m_usedBandwidthHz (0.0),
   m_durationInSeconds (0.0),
   m_frameCount (0),
   m_configTypeIndex (0),
   m_carrierCount (0)
{
  NS_LOG_FUNCTION (this);
}


SatSuperframeConf::~SatSuperframeConf ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SatSuperframeConf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatSuperframeConf")
    .SetParent<Object> ()
  ;

  return tid;
}

TypeId
SatSuperframeConf::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

void
SatSuperframeConf::AddFrameConf (Ptr<SatFrameConf> conf)
{
  NS_LOG_FUNCTION (this);

  // in case of random access frame, store carriers to RA channel container
  if ( conf->IsRandomAccess () )
    {
      uint32_t frameId = m_frames.size ();

      for ( uint32_t i = 0; i < conf->GetCarrierCount (); i++)
        {
          m_raChannels.push_back ( std::make_pair (frameId, i) );
        }
    }

  m_frames.push_back (conf);
  m_carrierCount += conf->GetCarrierCount ();
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

  return m_carrierCount;
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

void
SatSuperframeConf::SetFrameAllocatedBandwidthHz (uint8_t frameIndex, double bandwidhtHz)
{
  NS_LOG_FUNCTION (this << frameIndex << bandwidhtHz);

  if ( frameIndex >= m_maxFrameCount )
    {
      NS_FATAL_ERROR ("Frame index out of range!!!");
    }

  m_frameAllocatedBandwidth[frameIndex] = bandwidhtHz;
}

void
SatSuperframeConf::SetFrameCarrierAllocatedBandwidthHz (uint8_t frameIndex, double bandwidhtHz)
{
  NS_LOG_FUNCTION (this << frameIndex << bandwidhtHz);

  if ( frameIndex >= m_maxFrameCount )
    {
      NS_FATAL_ERROR ("Frame index out of range!!!");
    }

  m_frameCarrierAllocatedBandwidth[frameIndex] = bandwidhtHz;
}

void
SatSuperframeConf::SetFrameCarrierSpacing (uint8_t frameIndex, double spacing)
{
  NS_LOG_FUNCTION (this << frameIndex << spacing);

  if ( frameIndex >= m_maxFrameCount )
    {
      NS_FATAL_ERROR ("Frame index out of range!!!");
    }

  m_frameCarrierSpacing[frameIndex] = spacing;
}

void
SatSuperframeConf::SetFrameCarrierRollOff (uint8_t frameIndex, double rollOff)
{
  NS_LOG_FUNCTION (this << frameIndex << rollOff);

  if ( frameIndex >= m_maxFrameCount )
    {
      NS_FATAL_ERROR ("Frame index out of range!!!");
    }

  m_frameCarrierRollOff[frameIndex] = rollOff;
}

void
SatSuperframeConf::SetFrameRandomAccess (uint8_t frameIndex, bool randomAccess)
{
  NS_LOG_FUNCTION (this << frameIndex << randomAccess);

  if ( frameIndex >= m_maxFrameCount )
    {
      NS_FATAL_ERROR ("Frame index out of range!!!");
    }

  m_frameIsRandomAccess[frameIndex] = randomAccess;
}

double
SatSuperframeConf::GetFrameAllocatedBandwidthHz (uint8_t frameIndex) const
{
  NS_LOG_FUNCTION (this << frameIndex);

  if ( frameIndex >= m_maxFrameCount )
    {
      NS_FATAL_ERROR ("Frame index out of range!!!");
    }

  return m_frameAllocatedBandwidth[frameIndex];
}

double
SatSuperframeConf::GetFrameCarrierAllocatedBandwidthHz (uint8_t frameIndex) const
{
  NS_LOG_FUNCTION (this << frameIndex);

  if ( frameIndex >= m_maxFrameCount )
    {
      NS_FATAL_ERROR ("Frame index out of range!!!");
    }

  return m_frameCarrierAllocatedBandwidth[frameIndex];
}

double
SatSuperframeConf::GetFrameCarrierSpacing (uint8_t frameIndex) const
{
  NS_LOG_FUNCTION (this << frameIndex);

  if ( frameIndex >= m_maxFrameCount )
    {
      NS_FATAL_ERROR ("Frame index out of range!!!");
    }

  return m_frameCarrierSpacing[frameIndex];
}

double
SatSuperframeConf::GetFrameCarrierRollOff (uint8_t frameIndex) const
{
  NS_LOG_FUNCTION (this << frameIndex);

  if ( frameIndex >= m_maxFrameCount )
    {
      NS_FATAL_ERROR ("Frame index out of range!!!");
    }

  return m_frameCarrierRollOff[frameIndex];
}

bool
SatSuperframeConf::GetFrameIsRandomAccess (uint8_t frameIndex) const
{
  NS_LOG_FUNCTION (this << frameIndex);

  if ( frameIndex >= m_maxFrameCount )
    {
      NS_FATAL_ERROR ("Frame index out of range!!!");
    }

  return m_frameCarrierRollOff[frameIndex];
}


void
SatSuperframeConf::Configure (double allocatedBandwidthHz, Time targetDuration, Ptr<SatWaveformConf> waveFormConf)
{
  NS_LOG_FUNCTION (this);

  DoConfigure ();

  // make actual configuration

  switch (m_configTypeIndex)
    {
      case 0:
        {
          for (uint8_t frameIndex = 0; frameIndex < m_frameCount; frameIndex++)
            {
              // Create BTU conf according to given attributes
              Ptr<SatBtuConf> btuConf = Create<SatBtuConf> ( m_frameCarrierAllocatedBandwidth[frameIndex],
                                                             m_frameCarrierRollOff[frameIndex], m_frameCarrierSpacing[frameIndex] );

              // get default waveform
              uint32_t defaultWaveFormId = waveFormConf->GetDefaultWaveformId ();
              Ptr<SatWaveform> defaultWaveForm = waveFormConf->GetWaveform (defaultWaveFormId);

              double timeSlotDuration = defaultWaveForm->GetBurstDurationInSeconds (btuConf->GetSymbolRateInBauds ());
              uint32_t slotCount = std::max<uint32_t> (1 ,(targetDuration.GetSeconds() / timeSlotDuration ));

              m_usedBandwidthHz += m_frameAllocatedBandwidth[frameIndex];
              m_durationInSeconds = slotCount * timeSlotDuration;

              // Created one frame to be used utilizing earlier created BTU
              Ptr<SatFrameConf> frameConf = Create<SatFrameConf> (m_frameAllocatedBandwidth[frameIndex], m_durationInSeconds,
                                                                  btuConf, (SatFrameConf::SatTimeSlotConfContainer_t *) NULL, m_frameIsRandomAccess[frameIndex] );

              // Created time slots for every carrier and add them to frame configuration
              for (uint32_t i = 0; i < frameConf->GetCarrierCount (); i++)
                {
                  for (uint32_t j = 0; j < slotCount; j++)
                    {
                      Ptr<SatTimeSlotConf> timeSlot = Create<SatTimeSlotConf> (j * timeSlotDuration, defaultWaveFormId, i);
                      frameConf->AddTimeSlotConf (timeSlot);
                    }
                }

              // Add created frame to super frame configuration
              AddFrameConf (frameConf);
            }

          // check if configured frames exceeds given band
          if ( m_usedBandwidthHz > allocatedBandwidthHz )
            {
              NS_FATAL_ERROR ("Bandwidth of super frame exceeds allocated bandwidth");
            }
        }
        break;

      case 1:
      case 2:
      case 3:
        // TODO: Add other static configuration..
        NS_ASSERT (false); // these are not supported yet
        break;

      default:
        NS_ASSERT (false);
        break;
    }
}

SatFrameConf::SatTimeSlotIdContainer_t
SatSuperframeConf::GetRaSlots (uint32_t raChannel)
{
  NS_LOG_FUNCTION (this);

  SatFrameConf::SatTimeSlotIdContainer_t timeSlots;

  if ( raChannel < m_raChannels.size ())
    {
      uint8_t frameId = m_raChannels[raChannel].first;
      uint32_t carrierId = m_raChannels[raChannel].second;

      timeSlots = m_frames[frameId]->GetTimeSlotIds (carrierId);
    }
  else
    {
      NS_FATAL_ERROR ("Channel out of range!!!");
    }

  return timeSlots;
}

uint32_t
SatSuperframeConf::GetRaChannelCount () const
{
  NS_LOG_FUNCTION (this);
  return m_raChannels.size ();
}

uint8_t
SatSuperframeConf::GetRaChannelFrameId (uint32_t raChannel) const
{
  NS_LOG_FUNCTION (this);

  uint8_t frameId = 0;

  if ( raChannel < m_raChannels.size ())
    {
      frameId = m_raChannels[raChannel].first;
    }
  else
    {
      NS_FATAL_ERROR ("Channel out of range!!!");
    }

  return frameId;
}

std::string
SatSuperframeConf::GetIndexAsFrameName (uint32_t index)
{
  std::string name = "Frame";
  return name + GetNumberAsString (index);
}

// macro to call base class converter function with shorter name in macro ADD_FRAME_ATTRIBUTES
#define GetIndexAsFrameName(index) SatSuperframeConf::GetIndexAsFrameName(index)

// macro to ease definition of attributes for several frames
#define ADD_FRAME_ATTRIBUTES(index, frameBandwidth, carrierBandwidth, carrierSpacing, carrierRollOff, randomAccess ) \
.AddAttribute ( GetIndexAsFrameName(index) + "_AllocatedBandwidthHz", \
                std::string ("The allocated bandwidth [Hz] for ") + GetIndexAsFrameName(index), \
                TypeId::ATTR_CONSTRUCT, \
                DoubleValue (frameBandwidth), \
                MakeDoubleAccessor (&SatSuperframeConf::SetFrame ## index ## AllocatedBandwidthHz, \
                                     &SatSuperframeConf::GetFrame ## index ## AllocatedBandwidthHz), \
                 MakeDoubleChecker<double> ()) \
.AddAttribute ( GetIndexAsFrameName(index) + std::string ("_CarrierAllocatedBandwidthHz"), \
                std::string ("The allocated carrier bandwidth [Hz] for ") + GetIndexAsFrameName(index), \
                TypeId::ATTR_CONSTRUCT, \
                DoubleValue (carrierBandwidth), \
                MakeDoubleAccessor (&SatSuperframeConf::SetFrame ## index ## CarrierAllocatedBandwidthHz, \
                                    &SatSuperframeConf::GetFrame ## index ## CarrierAllocatedBandwidthHz), \
                MakeDoubleChecker<double> ()) \
.AddAttribute ( GetIndexAsFrameName(index) + std::string ("_CarrierRollOff"), \
                std::string ("The roll-off factor for ") + GetIndexAsFrameName(index), \
                TypeId::ATTR_CONSTRUCT, \
                DoubleValue (carrierSpacing), \
                MakeDoubleAccessor (&SatSuperframeConf::SetFrame ## index ## CarrierSpacing, \
                                    &SatSuperframeConf::GetFrame ## index ## CarrierSpacing), \
                MakeDoubleChecker<double> (0.00, 1.00)) \
.AddAttribute ( GetIndexAsFrameName(index) + std::string ("_CarrierSpacing"), \
                std::string ("The carrier spacing factor for ") + GetIndexAsFrameName(index), \
                TypeId::ATTR_CONSTRUCT, \
                DoubleValue (carrierRollOff), \
                MakeDoubleAccessor (&SatSuperframeConf::SetFrame ## index ## CarrierRollOff, \
                                    &SatSuperframeConf::GetFrame ## index ## CarrierRollOff), \
                MakeDoubleChecker<double> (0.00, 1.00)) \
.AddAttribute ( GetIndexAsFrameName(index) + std::string ("_RandomAccessFrame"), \
                std::string ("Flag to tell if ") + GetIndexAsFrameName(index) + std::string (" is used for random access"), \
                TypeId::ATTR_CONSTRUCT, \
                BooleanValue (randomAccess), \
                MakeBooleanAccessor (&SatSuperframeConf::SetFrame ## index ## RandomAccess, \
                                     &SatSuperframeConf::GetFrame ## index ## RandomAccess), \
                MakeBooleanChecker ())

// macro to ease definition of attributes for several super frames
#define ADD_SUPER_FRAME_ATTRIBUTES( frameCount, frameConfigTypeIndex) \
  .AddAttribute ("FrameCount", "The number of frames in super frame.", \
		              UintegerValue (frameCount), \
		              MakeUintegerAccessor (&SatSuperframeConf::SetFrameCount, \
		                                    &SatSuperframeConf::GetFrameCount), \
		              MakeUintegerChecker<uint32_t> (1, SatSuperframeConf::m_maxFrameCount)) \
  .AddAttribute ("FrameConfigTypeIndex", "Index of the frame configuration type used for super frame.", \
                  UintegerValue (frameConfigTypeIndex), \
                  MakeUintegerAccessor (&SatSuperframeConf::SetConfigType, \
                                        &SatSuperframeConf::GetConfigType), \
                  MakeUintegerChecker<uint32_t> (0, SatSuperframeConf::m_maxFrameConfigTypeIndex))

NS_OBJECT_ENSURE_REGISTERED (SatSuperframeConf0);

// Super frame configuration 0.

SatSuperframeConf0::SatSuperframeConf0 ()
{
  NS_LOG_FUNCTION (this);
}

SatSuperframeConf0::~SatSuperframeConf0 ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SatSuperframeConf0::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatSuperframeConf0")
    .SetParent<ns3::SatSuperframeConf> ()
    .AddConstructor<SatSuperframeConf0> ()
    ADD_SUPER_FRAME_ATTRIBUTES (10, 0)
    ADD_FRAME_ATTRIBUTES (0, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (1, 1.25e6, 1.25e6, 0.20, 0.30, true)
    ADD_FRAME_ATTRIBUTES (2, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (3, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (4, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (5, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (6, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (7, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (8, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (9, 1.25e6, 1.25e5, 0.20, 0.30, false)
  ;

  return tid;
}

TypeId
SatSuperframeConf0::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

void
SatSuperframeConf0::DoConfigure ()
{
  NS_LOG_FUNCTION (this);
}

NS_OBJECT_ENSURE_REGISTERED (SatSuperframeConf1);

// Super frame configuration 1.

SatSuperframeConf1::SatSuperframeConf1 ()
{
  NS_LOG_FUNCTION (this);
}

SatSuperframeConf1::~SatSuperframeConf1 ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SatSuperframeConf1::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatSuperframeConf1")
    .SetParent<ns3::SatSuperframeConf> ()
    .AddConstructor<SatSuperframeConf1> ()
    ADD_SUPER_FRAME_ATTRIBUTES (10, 0)
    ADD_FRAME_ATTRIBUTES (0, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (1, 1.25e6, 1.25e6, 0.20, 0.30, true)
    ADD_FRAME_ATTRIBUTES (2, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (3, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (4, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (5, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (6, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (7, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (8, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (9, 1.25e6, 1.25e5, 0.20, 0.30, false)
  ;

  return tid;
}

TypeId
SatSuperframeConf1::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

void
SatSuperframeConf1::DoConfigure ()
{
  NS_LOG_FUNCTION (this);
}

NS_OBJECT_ENSURE_REGISTERED (SatSuperframeConf2);

// Super frame configuration 2.

SatSuperframeConf2::SatSuperframeConf2 ()
{
  NS_LOG_FUNCTION (this);
}

SatSuperframeConf2::~SatSuperframeConf2 ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SatSuperframeConf2::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatSuperframeConf2")
    .SetParent<ns3::SatSuperframeConf> ()
    .AddConstructor<SatSuperframeConf2> ()
    ADD_SUPER_FRAME_ATTRIBUTES (10, 0)
    ADD_FRAME_ATTRIBUTES (0, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (1, 1.25e6, 1.25e6, 0.20, 0.30, true)
    ADD_FRAME_ATTRIBUTES (2, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (3, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (4, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (5, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (6, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (7, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (8, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (9, 1.25e6, 1.25e5, 0.20, 0.30, false)
  ;

  return tid;
}

TypeId
SatSuperframeConf2::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

void
SatSuperframeConf2::DoConfigure ()
{
  NS_LOG_FUNCTION (this);
}

NS_OBJECT_ENSURE_REGISTERED (SatSuperframeConf3);

// Super frame configuration 3.

SatSuperframeConf3::SatSuperframeConf3 ()
{
  NS_LOG_FUNCTION (this);
}

SatSuperframeConf3::~SatSuperframeConf3 ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SatSuperframeConf3::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatSuperframeConf3")
    .SetParent<ns3::SatSuperframeConf> ()
    .AddConstructor<SatSuperframeConf3> ()
    ADD_SUPER_FRAME_ATTRIBUTES (10, 0)
    ADD_FRAME_ATTRIBUTES (0, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (1, 1.25e6, 1.25e6, 0.20, 0.30, true)
    ADD_FRAME_ATTRIBUTES (2, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (3, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (4, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (5, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (6, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (7, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (8, 1.25e6, 1.25e5, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (9, 1.25e6, 1.25e5, 0.20, 0.30, false)
  ;

  return tid;
}

TypeId
SatSuperframeConf3::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

void
SatSuperframeConf3::DoConfigure ()
{
  NS_LOG_FUNCTION (this);
}


}; // namespace ns3
