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

SatSuperframeSeq::SatSuperframeSeq ( SatSuperframeConfList * confs)
{
  NS_LOG_FUNCTION (this);

  if ( confs != NULL)
    {
      m_superframe = *confs;
    }
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
    .AddAttribute ("MinTbtpStoreTime", "Minimum time to store sent TBTPs.",
                    TimeValue (MilliSeconds (300)),
                    MakeTimeAccessor (&SatSuperframeSeq::m_tbtpStoreTime),
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

  for (uint8_t i = 0; i < m_superframe.size(); i++)
    {
      carrierCount += m_superframe[i]->GetCarrierCount();
    }

  return carrierCount;
}

uint32_t
SatSuperframeSeq::GetCarrierCount ( uint8_t seqId ) const
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT ( seqId < m_superframe.size() );

  return m_superframe[seqId]->GetCarrierCount();
}

double
SatSuperframeSeq::GetDurationInSeconds ( uint8_t seqId ) const
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT ( seqId < m_superframe.size() );

  return m_superframe[seqId]->GetDurationInSeconds();
}

Ptr<SatSuperframeConf>
SatSuperframeSeq::GetSuperframeConf(uint8_t seqId) const
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT ( seqId < m_superframe.size());

  return m_superframe[seqId];
}

uint32_t
SatSuperframeSeq::GetCarrierId ( uint8_t superframeId, uint8_t frameId, uint16_t frameCarrierId ) const
{
  NS_LOG_FUNCTION (this << superframeId << frameId << frameCarrierId);

  NS_ASSERT ( superframeId < m_superframe.size());

  uint32_t carrierId = m_superframe[superframeId]->GetCarrierId(frameId, frameCarrierId );

  for (uint8_t i = 0; i < superframeId; i++)
    {
      carrierId += m_superframe[i]->GetCarrierCount();
    }

  return carrierId;
}

double
SatSuperframeSeq::GetCarrierFrequencyHz (uint32_t carrierId) const
{
  NS_LOG_FUNCTION (this << carrierId);

  double superFrameStartFrequency = 0.0;
  uint32_t currentSuperframe = 0;
  uint32_t lastIdInSuperframe = m_superframe[0]->GetCarrierCount() - 1;
  uint32_t carrierIdInSuperframe = carrierId;

  while( carrierId > lastIdInSuperframe )
    {
      carrierIdInSuperframe -= m_superframe[currentSuperframe]->GetCarrierCount();
      superFrameStartFrequency += m_superframe[currentSuperframe]->GetBandwidthHz();
      currentSuperframe++;
      lastIdInSuperframe += m_superframe[currentSuperframe]->GetCarrierCount();
    }

  double carrierFrequencyInSuperframe = m_superframe[currentSuperframe]->GetCarrierFrequencyHz ( carrierIdInSuperframe );

  return superFrameStartFrequency + carrierFrequencyInSuperframe;
}

double
SatSuperframeSeq::GetCarrierBandwidthHz (uint32_t carrierId, SatEnums::CarrierBandwidthType_t bandwidthType) const
{
  NS_LOG_FUNCTION (this << carrierId);

  uint32_t currentSuperframe = 0;
  uint32_t lastIdInSuperframe = m_superframe[0]->GetCarrierCount() - 1;
  uint32_t carrierIdInSuperframe = carrierId;

  while( carrierId > lastIdInSuperframe )
    {
      carrierIdInSuperframe -= m_superframe[currentSuperframe]->GetCarrierCount();
      currentSuperframe++;
      lastIdInSuperframe += m_superframe[currentSuperframe]->GetCarrierCount();
    }

  return m_superframe[currentSuperframe]->GetCarrierBandwidthHz ( carrierIdInSuperframe, bandwidthType );
}

uint32_t
SatSuperframeSeq::AddTbtpMessage (uint32_t beamId, Ptr<SatTbtpMessage> tbtpMsg)
{
  NS_LOG_FUNCTION (this << beamId << tbtpMsg);

  TbtpMap_t::const_iterator it = tbtpContainers.find (beamId);

  // create container, if not exist
  if ( it == tbtpContainers.end () )
    {
      Ptr<SatTbtpContainer> tbtpCont = Create<SatTbtpContainer> ();

      // calculate maximum number of messages to store based on given time to store and superframe duration.
      uint32_t storeCount  = (uint32_t) (m_tbtpStoreTime.GetSeconds() / m_superframe[0]->GetDurationInSeconds() );

      // store at least two messages always.
      if (storeCount < 2)
        {
          storeCount = 2;
        }

      tbtpCont->SetMaxMsgCount (storeCount);

      std::pair<TbtpMap_t::const_iterator, bool> result = tbtpContainers.insert (std::make_pair (beamId, tbtpCont));

      if ( result.second == false )
        {
          NS_FATAL_ERROR ("TBTP container creation failed!!!");
        }
    }

  return tbtpContainers[beamId]->Add (tbtpMsg);
}

Ptr<SatTbtpMessage>
SatSuperframeSeq::GetTbtpMessage (uint32_t beamId, uint32_t msgId) const
{
  NS_LOG_FUNCTION (this << beamId);

  Ptr<SatTbtpMessage> msg = NULL;

  TbtpMap_t::const_iterator it = tbtpContainers.find (beamId);

  if ( it != tbtpContainers.end () )
    {
      msg = tbtpContainers.at (beamId)->Get (msgId);
    }

  return msg;
}

}; // namespace ns3
