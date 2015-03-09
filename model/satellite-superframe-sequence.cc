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
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "satellite-superframe-sequence.h"

NS_LOG_COMPONENT_DEFINE ("SatSuperframeSeq");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatSuperframeSeq);

// Super frame conf

SatSuperframeSeq::SatSuperframeSeq ()
{
  NS_LOG_FUNCTION (this);
}

SatSuperframeSeq::~SatSuperframeSeq ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SatSuperframeSeq::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatSuperframeSeq")
    .SetParent<Object> ()
    .AddConstructor<SatSuperframeSeq> ()
    .AddAttribute ("TargetDuration", "Target duration time.",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&SatSuperframeSeq::m_targetDuration),
                   MakeTimeChecker ())
  ;

  return tid;
}

TypeId
SatSuperframeSeq::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

void
SatSuperframeSeq::AddWaveformConf (Ptr<SatWaveformConf> wfConf)
{
  NS_LOG_FUNCTION (this);
  m_wfConf = wfConf;
}

Ptr<SatWaveformConf>
SatSuperframeSeq::GetWaveformConf () const
{
  NS_LOG_FUNCTION (this);
  return m_wfConf;
}

void
SatSuperframeSeq::AddSuperframe (Ptr<SatSuperframeConf> conf)
{
  NS_LOG_FUNCTION (this);

  m_superframe.push_back (conf);
}

uint32_t
SatSuperframeSeq::GetCarrierCount () const
{
  NS_LOG_FUNCTION (this);

  uint32_t carrierCount = 0;

  for (uint8_t i = 0; i < m_superframe.size (); i++)
    {
      carrierCount += m_superframe[i]->GetCarrierCount ();
    }

  return carrierCount;
}

uint32_t
SatSuperframeSeq::GetCarrierCount ( uint8_t seqId ) const
{
  NS_LOG_FUNCTION (this << (uint32_t) seqId);

  if (seqId >= m_superframe.size ())
    {
      NS_FATAL_ERROR ("SatSuperframeSeq::GetCarrierCount - unsupported sequence id: " << seqId);
    }

  return m_superframe[seqId]->GetCarrierCount ();
}

Time
SatSuperframeSeq::GetDuration ( uint8_t seqId ) const
{
  NS_LOG_FUNCTION (this << (uint32_t) seqId);

  if (seqId >= m_superframe.size ())
    {
      NS_FATAL_ERROR ("SatSuperframeSeq::GetDuration - unsupported sequence id: " << seqId);
    }

  return m_superframe[seqId]->GetDuration ();
}

Ptr<SatSuperframeConf>
SatSuperframeSeq::GetSuperframeConf (uint8_t seqId) const
{
  NS_LOG_FUNCTION (this << (uint32_t) seqId);

  if (seqId >= m_superframe.size ())
    {
      NS_FATAL_ERROR ("SatSuperframeSeq::GetSuperframeConf - unsupported sequence id: " << seqId);
    }

  return m_superframe[seqId];
}

uint32_t
SatSuperframeSeq::GetCarrierId ( uint8_t superframeId, uint8_t frameId, uint16_t frameCarrierId ) const
{
  NS_LOG_FUNCTION (this << superframeId << frameId << frameCarrierId);

  if (superframeId >= m_superframe.size ())
    {
      NS_FATAL_ERROR ("SatSuperframeSeq::GetCarrierCount - unsupported sequence id: " << superframeId);
    }

  uint32_t carrierId = m_superframe[superframeId]->GetCarrierId (frameId, frameCarrierId );

  for (uint8_t i = 0; i < superframeId; i++)
    {
      carrierId += m_superframe[i]->GetCarrierCount ();
    }

  return carrierId;
}

double
SatSuperframeSeq::GetCarrierFrequencyHz (uint32_t carrierId) const
{
  NS_LOG_FUNCTION (this << carrierId);

  double superFrameStartFrequency = 0.0;
  uint32_t currentSuperframe = 0;
  uint32_t lastIdInSuperframe = m_superframe[0]->GetCarrierCount () - 1;
  uint32_t carrierIdInSuperframe = carrierId;

  while ( carrierId > lastIdInSuperframe )
    {
      carrierIdInSuperframe -= m_superframe[currentSuperframe]->GetCarrierCount ();
      superFrameStartFrequency += m_superframe[currentSuperframe]->GetBandwidthHz ();
      currentSuperframe++;
      lastIdInSuperframe += m_superframe[currentSuperframe]->GetCarrierCount ();
    }

  double carrierFrequencyInSuperframe = m_superframe[currentSuperframe]->GetCarrierFrequencyHz ( carrierIdInSuperframe );

  return superFrameStartFrequency + carrierFrequencyInSuperframe;
}

double
SatSuperframeSeq::GetCarrierBandwidthHz (uint32_t carrierId, SatEnums::CarrierBandwidthType_t bandwidthType) const
{
  NS_LOG_FUNCTION (this << carrierId);

  uint32_t currentSuperframe = 0;
  uint32_t lastIdInSuperframe = m_superframe[0]->GetCarrierCount () - 1;
  uint32_t carrierIdInSuperframe = carrierId;

  while ( carrierId > lastIdInSuperframe )
    {
      carrierIdInSuperframe -= m_superframe[currentSuperframe]->GetCarrierCount ();
      currentSuperframe++;
      lastIdInSuperframe += m_superframe[currentSuperframe]->GetCarrierCount ();
    }

  return m_superframe[currentSuperframe]->GetCarrierBandwidthHz ( carrierIdInSuperframe, bandwidthType );
}

}; // namespace ns3
