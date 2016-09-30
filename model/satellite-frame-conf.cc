/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/object.h"

#include "satellite-frame-conf.h"

NS_LOG_COMPONENT_DEFINE ("SatFrameConf");

namespace ns3 {

// BTU conf

SatBtuConf::SatBtuConf ()
  : m_allocatedBandwidthInHz (0.0),
    m_occupiedBandwidthInHz (0.0),
    m_effectiveBandwidthInHz (0.0),
    m_duration (0.0)
{
  NS_LOG_FUNCTION (this);

  // default constructor should not be used
  NS_ASSERT (false);
}

SatBtuConf::SatBtuConf (double bandwidthInHz, double rollOff, double spacing)
  : m_allocatedBandwidthInHz (bandwidthInHz)
{
  NS_LOG_FUNCTION (this);

  if ( (spacing < 0.00 ) && ( spacing > 1.00 ) )
    {
      NS_FATAL_ERROR ("Spacing for BTU is out of range. Check frame configuration parameters (attributes)!!!");
    }

  if ( (rollOff < 0.00 ) && ( rollOff > 1.00 ))
    {
      NS_FATAL_ERROR ("Roll-off for BTU is out of range. Check frame configuration parameters (attributes)!!!");
    }

  m_occupiedBandwidthInHz = m_allocatedBandwidthInHz / (rollOff + 1.00);
  m_effectiveBandwidthInHz = m_allocatedBandwidthInHz / ((spacing + 1.00) * (rollOff + 1.00));

  m_duration = Seconds (1 / m_effectiveBandwidthInHz);
}

SatBtuConf::~SatBtuConf ()
{
  NS_LOG_FUNCTION (this);
}

// Time Slot conf

SatTimeSlotConf::SatTimeSlotConf ()
  : m_startTime (0),
    m_waveFormId (0),
    m_frameCarrierId (0),
    m_rcIndex (0),
    m_slotType (SatTimeSlotConf::SLOT_TYPE_TRC)
{
  NS_LOG_FUNCTION (this);

  // default constructor should not be used
  NS_ASSERT (false);
}

SatTimeSlotConf::SatTimeSlotConf (Time startTime, uint32_t waveFormId, uint16_t frameCarrierId, SatTimeSlotConf::SatTimeSlotType_t slotType)
  : m_startTime (startTime),
    m_waveFormId (waveFormId),
    m_frameCarrierId (frameCarrierId),
    m_rcIndex (0),
    m_slotType (slotType)
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
    m_duration (0.0),
    m_isRandomAccess (false),
    m_btuConf (0),
    m_carrierCount (0),
    m_maxSymbolsPerCarrier (0),
    m_minPayloadPerCarrierInBytes (0)
{
  NS_LOG_FUNCTION (this);

  // default constructor should not be used
  NS_ASSERT (false);
}

SatFrameConf::SatFrameConf ( double bandwidthHz, Time targetDuration, Ptr<SatBtuConf> btuConf, Ptr<SatWaveformConf> waveformConf,
                             bool isRandomAccess, bool defaultWaveformInUse, bool checkSlotLimit )
  : m_bandwidthHz (bandwidthHz),
    m_isRandomAccess (isRandomAccess),
    m_btuConf (btuConf),
    m_waveformConf (waveformConf)
{
  NS_LOG_FUNCTION (this);

  m_carrierCount = bandwidthHz / m_btuConf->GetAllocatedBandwidthInHz ();

  if ( m_carrierCount == 0 )
    {
      NS_FATAL_ERROR ("No carriers can be created for the frame with given BTU and bandwidth. Check frame configuration parameters (attributes)!!! ");
    }

  uint32_t defWaveFormId = m_waveformConf->GetDefaultWaveformId ();
  Ptr<SatWaveform> defWaveform = m_waveformConf->GetWaveform (defWaveFormId);

  // calculate slot details based on given parameters and default waveform
  Time timeSlotDuration = defWaveform->GetBurstDuration (m_btuConf->GetSymbolRateInBauds ());
  uint32_t carrierSlotCount = targetDuration.GetSeconds () / timeSlotDuration.GetSeconds ();

  if ( carrierSlotCount == 0)
    {
      NS_FATAL_ERROR ("Time slots cannot be created with target frame duration. Check frame target duration!!!");
    }

  m_duration = Time ( carrierSlotCount * timeSlotDuration.GetInteger () );

  m_maxSymbolsPerCarrier = carrierSlotCount * defWaveform->GetBurstLengthInSymbols ();

  if ( defaultWaveformInUse || (m_waveformConf->IsAcmEnabled () == false ))
    {
      m_minPayloadPerCarrierInBytes = carrierSlotCount * defWaveform->GetPayloadInBytes ();
    }
  else
    {
      uint32_t mostRobustWaveFormId;

      if ( !m_waveformConf->GetMostRobustWaveformId (mostRobustWaveFormId, defWaveform->GetBurstLengthInSymbols () ) )
        {
          NS_FATAL_ERROR ("Most robust waveform not found, error in waveform configuration ???");
        }

      Ptr<SatWaveform> waveform = m_waveformConf->GetWaveform (mostRobustWaveFormId);
      m_minPayloadPerCarrierInBytes = carrierSlotCount * waveform->GetPayloadInBytes ();
    }

  uint32_t frameTimeSlotCount = 0;

  // Created time slots for every carrier and add them to frame configuration
  for (uint32_t i = 0; i < m_carrierCount; i++)
    {
      for (uint32_t j = 0; j < carrierSlotCount; j++)
        {
          Ptr<SatTimeSlotConf> timeSlot = Create<SatTimeSlotConf> (Time (j * timeSlotDuration.GetInteger ()), defWaveFormId, i, SatTimeSlotConf::SLOT_TYPE_TRC);
          AddTimeSlotConf (timeSlot);

          frameTimeSlotCount++;

          if ( checkSlotLimit && (frameTimeSlotCount > m_maxTimeSlotCount) )
            {
              NS_FATAL_ERROR ("Time slot count is over limit. Check frame configuration!!!");
            }
        }
    }
}

SatFrameConf::~SatFrameConf ()
{
  NS_LOG_FUNCTION (this);
}

double
SatFrameConf::GetCarrierFrequencyHz (uint16_t carrierId) const
{
  NS_LOG_FUNCTION (this << carrierId);

  if (carrierId >= m_carrierCount )
    {
      NS_FATAL_ERROR ( "Carrier Id out of range" );
    }

  double carrierBandwidthHz = m_btuConf->GetAllocatedBandwidthInHz ();

  return ( (carrierBandwidthHz * carrierId) + ( carrierBandwidthHz / 2.0 ) );
}

double
SatFrameConf::GetCarrierBandwidthHz (SatEnums::CarrierBandwidthType_t bandwidthType) const
{
  NS_LOG_FUNCTION (this << bandwidthType);

  double bandwidth = 0.0;

  switch (bandwidthType)
    {
    case SatEnums::ALLOCATED_BANDWIDTH:
      bandwidth = m_btuConf->GetAllocatedBandwidthInHz ();
      break;

    case SatEnums::OCCUPIED_BANDWIDTH:
      bandwidth = m_btuConf->GetOccupiedBandwidthInHz ();
      break;

    case SatEnums::EFFECTIVE_BANDWIDTH:
      bandwidth = m_btuConf->GetEffectiveBandwidthInHz ();
      break;

    default:
      NS_FATAL_ERROR ("Invalid bandwidth type!!!");
      break;
    }

  return bandwidth;
}

uint16_t
SatFrameConf::GetTimeSlotCount () const
{
  NS_LOG_FUNCTION (this);

  uint16_t slotCount = 0;

  if ( m_timeSlotConfMap.empty () == false )
    {
      slotCount = m_timeSlotConfMap.size () * m_timeSlotConfMap.begin ()->second.size ();
    }

  return slotCount;
}

Ptr<SatTimeSlotConf>
SatFrameConf::GetTimeSlotConf (uint16_t carrierId, uint16_t index) const
{
  NS_LOG_FUNCTION (this);

  Ptr<SatTimeSlotConf> foundTimeSlot = NULL;

  SatTimeSlotConfMap_t::const_iterator foundCarrier = m_timeSlotConfMap.find (carrierId);

  if ( foundCarrier != m_timeSlotConfMap.end () && index < foundCarrier->second.size () )
    {
      foundTimeSlot = foundCarrier->second[index];
    }
  else
    {
      NS_FATAL_ERROR ("Index is invalid!!!");
    }

  return foundTimeSlot;
}

Ptr<SatTimeSlotConf>
SatFrameConf::GetTimeSlotConf (uint16_t index) const
{
  NS_LOG_FUNCTION (this);

  Ptr<SatTimeSlotConf> foundTimeSlot = NULL;
  uint32_t carrierId = index / m_timeSlotConfMap.begin ()->second.size ();
  uint16_t timeSlotIndex = index % m_timeSlotConfMap.begin ()->second.size ();

  foundTimeSlot = GetTimeSlotConf (carrierId, timeSlotIndex);

  return foundTimeSlot;
}

SatFrameConf::SatTimeSlotConfContainer_t
SatFrameConf::GetTimeSlotConfs (uint16_t carrierId) const
{
  NS_LOG_FUNCTION (this);

  SatTimeSlotConfContainer_t timeSlots;

  SatTimeSlotConfMap_t::const_iterator it = m_timeSlotConfMap.find (carrierId);

  if ( it != m_timeSlotConfMap.end () )
    {
      timeSlots = it->second;
    }
  else
    {
      NS_FATAL_ERROR ("Carrier not found!!!");
    }

  return timeSlots;
}

uint16_t
SatFrameConf::AddTimeSlotConf (Ptr<SatTimeSlotConf> conf)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_FUNCTION (this << conf);

  // find container for the Carrier from map
  SatTimeSlotConfMap_t::iterator it = m_timeSlotConfMap.find (conf->GetCarrierId ());

  // If not found, add new Carrier container to map,
  // otherwise use container found from map
  if ( it == m_timeSlotConfMap.end () )
    {
      std::pair<SatTimeSlotConfMap_t::iterator, bool> result = m_timeSlotConfMap.insert (std::make_pair (conf->GetCarrierId (), SatTimeSlotConfContainer_t ()));

      if ( result.second )
        {
          it = result.first;
        }
      else
        {
          it = m_timeSlotConfMap.end ();
        }
    }

  // container creation for carrier has failed, so we need to crash
  if (it == m_timeSlotConfMap.end ())
    {
      NS_FATAL_ERROR ("Cannot insert slot to container!!!");
    }

  // store time slot info to carrier specific container
  it->second.push_back (conf);

  return 0;
}

NS_OBJECT_ENSURE_REGISTERED (SatSuperframeConf);

// Super frame configuration interface.

Ptr<SatSuperframeConf>
SatSuperframeConf::CreateSuperframeConf (SuperFrameConfiguration_t conf)
{
  NS_LOG_FUNCTION_NOARGS ();

  Ptr<SatSuperframeConf> superFrameConf;

  switch (conf)
    {
    case SUPER_FRAME_CONFIG_0:
      superFrameConf = CreateObject<SatSuperframeConf0> ();
      break;

    case SUPER_FRAME_CONFIG_1:
      superFrameConf = CreateObject<SatSuperframeConf1> ();
      break;

    case SUPER_FRAME_CONFIG_2:
      superFrameConf = CreateObject<SatSuperframeConf2> ();
      break;

    case SUPER_FRAME_CONFIG_3:
      superFrameConf = CreateObject<SatSuperframeConf3> ();
      break;

    default:
      NS_FATAL_ERROR ("Not supported super frame configuration!!!");
      break;
    }

  return superFrameConf;
}

SatSuperframeConf::SatSuperframeConf ()
  : m_usedBandwidthHz (0.0),
    m_duration (0.0),
    m_frameCount (0),
    m_configType (CONFIG_TYPE_0),
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
      uint16_t raBaseIndex = m_raChannels.size ();

      for ( uint32_t i = 0; i < conf->GetCarrierCount (); i++)
        {
          // Only number of the RA channels definable by uint8_t data type can used
          // This is needed to take into account when configuring frames and defined which of them
          // are random access channels
          if ( (raBaseIndex + i) >= std::numeric_limits<uint8_t>::max () )
            {
              NS_FATAL_ERROR ("RA channels maximum count is exceeded!!!");
            }

          m_raChannels.push_back ( std::make_pair (frameId, raBaseIndex + i) );
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
      if ( i >= m_frames.size ())
        {
          NS_FATAL_ERROR ("Carrier ID out of range.");
        }

      carrierId += m_frames[i]->GetCarrierCount ();
    }

  return carrierId;
}

double
SatSuperframeConf::GetCarrierFrequencyHz (uint32_t carrierId) const
{
  NS_LOG_FUNCTION (this);

  double frameStartFrequency = 0.0;
  uint32_t carrierIdInFrame = carrierId;

  uint8_t frameId = GetCarrierFrame (carrierId);

  for ( uint8_t i = 0; i < frameId; i++ )
    {
      carrierIdInFrame -= m_frames[i]->GetCarrierCount ();
      frameStartFrequency += m_frames[i]->GetBandwidthHz ();
    }

  double carrierFrequencyInFrame = m_frames[frameId]->GetCarrierFrequencyHz (carrierIdInFrame);

  return frameStartFrequency + carrierFrequencyInFrame;
}

double
SatSuperframeConf::GetCarrierBandwidthHz (uint32_t carrierId, SatEnums::CarrierBandwidthType_t bandwidthType) const
{
  NS_LOG_FUNCTION (this);

  uint8_t frameId = GetCarrierFrame (carrierId);

  return m_frames[frameId]->GetCarrierBandwidthHz (bandwidthType);
}

bool
SatSuperframeConf::IsRandomAccessCarrier (uint32_t carrierId) const
{
  NS_LOG_FUNCTION (this);

  uint8_t frameId = GetCarrierFrame (carrierId);

  return m_frameIsRandomAccess[frameId];
}

void
SatSuperframeConf::SetFrameAllocatedBandwidthHz (uint8_t frameIndex, double bandwidthHz)
{
  NS_LOG_FUNCTION (this << frameIndex << bandwidthHz);

  if ( frameIndex >= m_maxFrameCount )
    {
      NS_FATAL_ERROR ("Frame index out of range!!!");
    }

  m_frameAllocatedBandwidth[frameIndex] = bandwidthHz;
}

void
SatSuperframeConf::SetFrameCarrierAllocatedBandwidthHz (uint8_t frameIndex, double bandwidthHz)
{
  NS_LOG_FUNCTION (this << frameIndex << bandwidthHz);

  if ( frameIndex >= m_maxFrameCount )
    {
      NS_FATAL_ERROR ("Frame index out of range!!!");
    }

  m_frameCarrierAllocatedBandwidth[frameIndex] = bandwidthHz;
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
SatSuperframeConf::IsFrameRandomAccess (uint8_t frameIndex) const
{
  NS_LOG_FUNCTION (this << frameIndex);

  if ( frameIndex >= m_maxFrameCount )
    {
      NS_FATAL_ERROR ("Frame index out of range!!!");
    }

  return m_frameIsRandomAccess[frameIndex];
}


void
SatSuperframeConf::Configure (double allocatedBandwidthHz, Time targetDuration, Ptr<SatWaveformConf> waveformConf)
{
  NS_LOG_FUNCTION (this);

  if (m_configType == CONFIG_TYPE_0 && waveformConf->IsAcmEnabled() == true)
    {
      NS_LOG_WARN ("Superframe is configured to use config type 0, thus ACM should be disabled!");
    }

  if (m_configType == CONFIG_TYPE_1 && waveformConf->IsAcmEnabled() == false)
    {
      NS_FATAL_ERROR ("Superframe is configured to use config type 1, thus ACM should be enabled!");
    }

  DoConfigure ();

  bool useDefaultWaveform = false;
  bool checkSlotLimit = true;

  /**
   * Note, that the superframe duration may be a bit longer than the frame
   * durations, since the frame duration is counted based on the time slot
   * duration.
   */
  m_duration = targetDuration;

  // make actual configuration

  switch (m_configType)
    {
    case CONFIG_TYPE_0:
    case CONFIG_TYPE_1:
    case CONFIG_TYPE_2:
      {
        m_raChannels.clear ();
        m_frames.clear ();
        m_carrierCount = 0;

        if ( m_configType == CONFIG_TYPE_0)
          {
            useDefaultWaveform = true;
          }

        if ( m_configType == CONFIG_TYPE_2)
          {
            checkSlotLimit = false;
          }

        for (uint8_t frameIndex = 0; frameIndex < m_frameCount; frameIndex++)
          {
            // Create BTU conf according to given attributes
            Ptr<SatBtuConf> btuConf = Create<SatBtuConf> ( m_frameCarrierAllocatedBandwidth[frameIndex],
                                                           m_frameCarrierRollOff[frameIndex], m_frameCarrierSpacing[frameIndex] );

            // Create frame utilizing earlier created BTU
            Ptr<SatFrameConf> frameConf = Create<SatFrameConf> (m_frameAllocatedBandwidth[frameIndex], targetDuration, btuConf,
                                                                waveformConf, m_frameIsRandomAccess[frameIndex], useDefaultWaveform, checkSlotLimit );


            m_usedBandwidthHz += m_frameAllocatedBandwidth[frameIndex];

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

    case CONFIG_TYPE_3:
      NS_FATAL_ERROR ("Configuration type 3 is not supported!!!");
      break;

    default:
      NS_FATAL_ERROR ("Not supported configuration type!!!");
      break;
    }
}

SatFrameConf::SatTimeSlotConfContainer_t
SatSuperframeConf::GetRaSlots (uint8_t raChannel)
{
  NS_LOG_FUNCTION (this);

  SatFrameConf::SatTimeSlotConfContainer_t timeSlots;

  if ( raChannel < m_raChannels.size ())
    {
      uint8_t frameId = m_raChannels[raChannel].first;
      uint32_t carrierId = m_raChannels[raChannel].second;

      timeSlots = m_frames[frameId]->GetTimeSlotConfs (carrierId);
    }
  else
    {
      NS_FATAL_ERROR ("Channel out of range!!!");
    }

  return timeSlots;
}

uint16_t
SatSuperframeConf::GetRaSlotCount (uint8_t raChannel)
{
  NS_LOG_FUNCTION (this);

  uint16_t slotCount = 0;

  if ( raChannel < m_raChannels.size ())
    {
      uint8_t frameId = m_raChannels[raChannel].first;
      uint32_t carrierId = m_raChannels[raChannel].second;

      slotCount = m_frames[frameId]->GetTimeSlotConfs (carrierId).size ();
    }
  else
    {
      NS_FATAL_ERROR ("Channel out of range!!!");
    }

  return slotCount;
}

uint8_t
SatSuperframeConf::GetRaChannelCount () const
{
  NS_LOG_FUNCTION (this);
  return m_raChannels.size ();
}

uint8_t
SatSuperframeConf::GetRaChannelFrameId (uint8_t raChannel) const
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

uint32_t
SatSuperframeConf::GetRaChannelTimeSlotPayloadInBytes (uint8_t raChannel) const
{
  NS_LOG_FUNCTION (this);

  uint32_t payloadInBytes = 0;

  if ( raChannel < m_raChannels.size ())
    {
      uint8_t frameId = m_raChannels[raChannel].first;
      Ptr<SatTimeSlotConf> timeSlotConf = (*m_frames[frameId]->GetTimeSlotConfs (0).begin ());
      Ptr<SatWaveform> waveform = m_frames[frameId]->GetWaveformConf ()->GetWaveform ( timeSlotConf->GetWaveFormId ());

      payloadInBytes = waveform->GetPayloadInBytes ();
    }
  else
    {
      NS_FATAL_ERROR ("Channel " << raChannel << " out of range!!!");
    }

  return payloadInBytes;

}

std::string
SatSuperframeConf::GetIndexAsFrameName (uint32_t index)
{
  std::string name = "Frame";
  return name + GetNumberAsString (index);
}

// macro to call base class converter function with shorter name in macro ADD_FRAME_ATTRIBUTES
#define GetIndexAsFrameName(index) SatSuperframeConf::GetIndexAsFrameName (index)

// macro to ease definition of attributes for several frames
#define ADD_FRAME_ATTRIBUTES(index, frameBandwidth, carrierBandwidth, carrierSpacing, carrierRollOff, randomAccess ) \
  .AddAttribute ( GetIndexAsFrameName (index) + "_AllocatedBandwidthHz", \
                  std::string ("The allocated bandwidth [Hz] for ") + GetIndexAsFrameName (index), \
                  TypeId::ATTR_CONSTRUCT, \
                  DoubleValue (frameBandwidth), \
                  MakeDoubleAccessor (&SatSuperframeConf::SetFrame ## index ## AllocatedBandwidthHz, \
                                      &SatSuperframeConf::GetFrame ## index ## AllocatedBandwidthHz), \
                  MakeDoubleChecker<double> ()) \
  .AddAttribute ( GetIndexAsFrameName (index) + std::string ("_CarrierAllocatedBandwidthHz"), \
                  std::string ("The allocated carrier bandwidth [Hz] for ") + GetIndexAsFrameName (index), \
                  TypeId::ATTR_CONSTRUCT, \
                  DoubleValue (carrierBandwidth), \
                  MakeDoubleAccessor (&SatSuperframeConf::SetFrame ## index ## CarrierAllocatedBandwidthHz, \
                                      &SatSuperframeConf::GetFrame ## index ## CarrierAllocatedBandwidthHz), \
                  MakeDoubleChecker<double> ()) \
  .AddAttribute ( GetIndexAsFrameName (index) + std::string ("_CarrierRollOff"), \
                  std::string ("The roll-off factor for ") + GetIndexAsFrameName (index), \
                  TypeId::ATTR_CONSTRUCT, \
                  DoubleValue (carrierSpacing), \
                  MakeDoubleAccessor (&SatSuperframeConf::SetFrame ## index ## CarrierRollOff, \
                                      &SatSuperframeConf::GetFrame ## index ## CarrierRollOff), \
                  MakeDoubleChecker<double> (0.00, 1.00)) \
  .AddAttribute ( GetIndexAsFrameName (index) + std::string ("_CarrierSpacing"), \
                  std::string ("The carrier spacing factor for ") + GetIndexAsFrameName (index), \
                  TypeId::ATTR_CONSTRUCT, \
                  DoubleValue (carrierRollOff), \
                  MakeDoubleAccessor (&SatSuperframeConf::SetFrame ## index ## CarrierSpacing, \
                                      &SatSuperframeConf::GetFrame ## index ## CarrierSpacing), \
                  MakeDoubleChecker<double> (0.00, 1.00)) \
  .AddAttribute ( GetIndexAsFrameName (index) + std::string ("_RandomAccessFrame"), \
                  std::string ("Flag to tell if ") + GetIndexAsFrameName (index) + std::string (" is used for random access"), \
                  TypeId::ATTR_CONSTRUCT, \
                  BooleanValue (randomAccess), \
                  MakeBooleanAccessor (&SatSuperframeConf::SetFrame ## index ## RandomAccess, \
                                       &SatSuperframeConf::IsFrame ## index ## RandomAccess), \
                  MakeBooleanChecker ())

// macro to ease definition of attributes for several super frames
#define ADD_SUPER_FRAME_ATTRIBUTES( frameCount, configType ) \
  .AddAttribute ("FrameCount", "The number of frames in super frame.", \
                 TypeId::ATTR_CONSTRUCT, \
                 UintegerValue (frameCount), \
                 MakeUintegerAccessor (&SatSuperframeConf::SetFrameCount, \
                                       &SatSuperframeConf::GetFrameCount), \
                 MakeUintegerChecker<uint32_t> (1, SatSuperframeConf::m_maxFrameCount)) \
  .AddAttribute ("FrameConfigType", "The frame configuration type used for super frame.", \
                 TypeId::ATTR_CONSTRUCT, \
                 EnumValue (configType), \
                 MakeEnumAccessor ( &SatSuperframeConf::SetConfigType, \
                                    &SatSuperframeConf::GetConfigType), \
                 MakeEnumChecker ( SatSuperframeConf::CONFIG_TYPE_0, "ConfigType_0", \
                                   SatSuperframeConf::CONFIG_TYPE_1, "ConfigType_1", \
                                   SatSuperframeConf::CONFIG_TYPE_2, "ConfigType_2"))


uint8_t
SatSuperframeConf::GetCarrierFrame (uint32_t carrierId) const
{
  NS_LOG_FUNCTION (this);

  uint32_t currentFrame = 0;
  uint32_t lastIdInFrame = m_frames[0]->GetCarrierCount () - 1;

  while (carrierId > lastIdInFrame)
    {
      currentFrame++;
      lastIdInFrame += m_frames[currentFrame]->GetCarrierCount ();
    }

  return currentFrame;
}

uint8_t
SatSuperframeConf::GetRaChannel (uint32_t carrierId) const
{
  NS_LOG_FUNCTION (this);

  uint8_t raChannelId = 0;
  uint8_t frameId = GetCarrierFrame (carrierId);

  if ( m_frames[frameId]->IsRandomAccess () )
    {
      uint32_t carrierIdInFrame = carrierId;

      for ( uint8_t i = 0; i < frameId; i++ )
        {
          carrierIdInFrame -= m_frames[i]->GetCarrierCount ();

          // increase RA channel id (index) by count of RA carriers before asked carrier/frame
          if ( m_frames[i]->IsRandomAccess () )
            {
              raChannelId += m_frames[i]->GetCarrierCount ();
            }
        }

      // increase RA channel id (index) by carrier id of the asked carrier/frame
      raChannelId += carrierIdInFrame;
    }

  return raChannelId;
}


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
    ADD_SUPER_FRAME_ATTRIBUTES (10, SatSuperframeConf::CONFIG_TYPE_0 )
    ADD_FRAME_ATTRIBUTES (0, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (1, 1.25e6, 1.25e6, 0.20, 0.30, true)
    ADD_FRAME_ATTRIBUTES (2, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (3, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (4, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (5, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (6, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (7, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (8, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (9, 1.25e7, 1.25e6, 0.20, 0.30, false)
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
    ADD_SUPER_FRAME_ATTRIBUTES (10, SatSuperframeConf::CONFIG_TYPE_1)
    ADD_FRAME_ATTRIBUTES (0, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (1, 1.25e6, 1.25e6, 0.20, 0.30, true)
    ADD_FRAME_ATTRIBUTES (2, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (3, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (4, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (5, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (6, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (7, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (8, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (9, 1.25e7, 1.25e6, 0.20, 0.30, false)
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
    ADD_SUPER_FRAME_ATTRIBUTES (10, SatSuperframeConf::CONFIG_TYPE_2)
    ADD_FRAME_ATTRIBUTES (0, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (1, 1.25e6, 1.25e6, 0.20, 0.30, true)
    ADD_FRAME_ATTRIBUTES (2, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (3, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (4, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (5, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (6, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (7, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (8, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (9, 1.25e7, 1.25e6, 0.20, 0.30, false)
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
    ADD_SUPER_FRAME_ATTRIBUTES (10, SatSuperframeConf::CONFIG_TYPE_2)
    ADD_FRAME_ATTRIBUTES (0, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (1, 1.25e6, 1.25e6, 0.20, 0.30, true)
    ADD_FRAME_ATTRIBUTES (2, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (3, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (4, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (5, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (6, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (7, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (8, 1.25e7, 1.25e6, 0.20, 0.30, false)
    ADD_FRAME_ATTRIBUTES (9, 1.25e7, 1.25e6, 0.20, 0.30, false)
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
