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

#include <ns3/log.h>
#include <ns3/satellite-control-message.h>
#include <ns3/satellite-superframe-sequence.h>
#include <ns3/satellite-lower-layer-service.h>
#include <ns3/packet.h>
#include <ns3/address.h>
#include "satellite-ncc.h"

NS_LOG_COMPONENT_DEFINE ("SatNcc");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatNcc);

TypeId
SatNcc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatNcc")
    .SetParent<Object> ()
    .AddConstructor<SatNcc> ()
    //
    // Trace sources
    //
    .AddTraceSource ("NccRx",
                     "Trace source indicating a CR has received by NCC",
                     MakeTraceSourceAccessor (&SatNcc::m_nccRxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("NccTx",
                     "Trace source indicating a TBTP has sent by NCC",
                     MakeTraceSourceAccessor (&SatNcc::m_nccTxTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

TypeId
SatNcc::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatNcc::SatNcc ()
{
  NS_LOG_FUNCTION (this);
}

SatNcc::~SatNcc ()
{
  NS_LOG_FUNCTION (this);
}

void
SatNcc::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_isLowRandomAccessLoad.clear ();

  Object::DoDispose ();
}


void
SatNcc::UtCnoUpdated (uint32_t beamId, Address utId, Address /*gwId*/, double cno)
{
  NS_LOG_FUNCTION (this << beamId << utId << cno);

  m_beamSchedulers[beamId]->UpdateUtCno (utId, cno);
}

void
SatNcc::DoRandomAccessDynamicLoadControl (uint32_t beamId, uint32_t carrierId, uint8_t allocationChannelId, double averageNormalizedOfferedLoad)
{
  NS_LOG_FUNCTION (this << beamId << carrierId << (uint32_t) allocationChannelId << averageNormalizedOfferedLoad);

  bool isLowRandomAccessLoad = true;
  std::map<std::pair<uint32_t,uint8_t>,bool>::iterator findResult;
  std::pair<std::map<std::pair<uint32_t,uint8_t>,bool>::iterator,bool> insertResult;

  /// search for the current status of load control
  findResult = m_isLowRandomAccessLoad.find (std::make_pair (beamId,allocationChannelId));

  if (findResult == m_isLowRandomAccessLoad.end ())
    {
      insertResult = m_isLowRandomAccessLoad.insert (std::make_pair (std::make_pair (beamId,allocationChannelId),isLowRandomAccessLoad));

      if (!insertResult.second)
        {
          NS_FATAL_ERROR ("SatNcc::DoRandomAccessDynamicLoadControl - Insert failed");
        }
      else
        {
          isLowRandomAccessLoad = insertResult.second;
        }
    }
  else
    {
      isLowRandomAccessLoad = findResult->second;
    }

  NS_LOG_INFO ("SatNcc::DoRandomAccessDynamicLoadControl - Beam: " << beamId << ", carrier ID: " << carrierId << ", AC: " << (uint32_t)allocationChannelId << " - Measuring the average normalized offered random access load: " << averageNormalizedOfferedLoad);

  std::map<uint8_t,double>::iterator itThreshold = m_randomAccessAverageNormalizedOfferedLoadThreshold.find (allocationChannelId);

  if (itThreshold == m_randomAccessAverageNormalizedOfferedLoadThreshold.end ())
    {
      NS_FATAL_ERROR ("SatNcc::DoRandomAccessDynamicLoadControl - Average normalized offered load threshold not set for beam: " << beamId << ", carrier: " << carrierId << ", allocation channel: " << (uint32_t)allocationChannelId);
    }

  /// low RA load in effect
  if (isLowRandomAccessLoad)
    {
      NS_LOG_INFO ("SatNcc::DoRandomAccessDynamicLoadControl - Beam: " << beamId << ", carrier ID: " << carrierId << " - Currently low load in effect for allocation channel: " << (uint32_t)allocationChannelId);
      /// check the load against the parameterized value
      if (averageNormalizedOfferedLoad >= itThreshold->second)
        {
          std::map<uint8_t,uint16_t>::iterator it;

          it = m_highLoadBackOffProbability.find (allocationChannelId);

          if (it == m_highLoadBackOffProbability.end ())
            {
              NS_FATAL_ERROR ("SatNcc::DoRandomAccessDynamicLoadControl - High load backoff probability not set for allocation channel: " << (uint32_t)allocationChannelId);
            }

          uint16_t probability = it->second;

          it = m_highLoadBackOffTime.find (allocationChannelId);

          if (it == m_highLoadBackOffTime.end ())
            {
              NS_FATAL_ERROR ("SatNcc::DoRandomAccessDynamicLoadControl - High load backoff time not set for allocation channel: " << (uint32_t)allocationChannelId);
            }

          uint16_t time = it->second;

          /// use high load back off value
          CreateRandomAccessLoadControlMessage (probability, time, beamId, allocationChannelId);

          NS_LOG_INFO ("SatNcc::DoRandomAccessDynamicLoadControl - Beam: " << beamId << ", carrier ID: " << carrierId << ", AC: " << (uint32_t)allocationChannelId << " - Switching to HIGH LOAD back off parameterization");

          /// flag RA load as high load
          m_isLowRandomAccessLoad.at (std::make_pair (beamId,allocationChannelId)) = false;
        }
    }
  /// high RA load in effect
  else
    {
      NS_LOG_INFO ("SatNcc::DoRandomAccessDynamicLoadControl - Beam: " << beamId << ", carrier ID: " << carrierId << " - Currently high load in effect for allocation channel: " << (uint32_t)allocationChannelId);

      /// check the load against the parameterized value
      if (averageNormalizedOfferedLoad < itThreshold->second)
        {
          std::map<uint8_t,uint16_t>::iterator it;

          it = m_lowLoadBackOffProbability.find (allocationChannelId);

          if (it == m_lowLoadBackOffProbability.end ())
            {
              NS_FATAL_ERROR ("SatNcc::DoRandomAccessDynamicLoadControl - Low load backoff probability not set for allocation channel: " << (uint32_t)allocationChannelId);
            }

          uint16_t probability = it->second;

          it = m_lowLoadBackOffTime.find (allocationChannelId);

          if (it == m_lowLoadBackOffTime.end ())
            {
              NS_FATAL_ERROR ("SatNcc::DoRandomAccessDynamicLoadControl - Low load backoff time not set for allocation channel: " << (uint32_t)allocationChannelId);
            }

          uint16_t time = it->second;

          /// use low load back off value
          CreateRandomAccessLoadControlMessage (probability, time, beamId, allocationChannelId);

          NS_LOG_INFO ("SatNcc::DoRandomAccessDynamicLoadControl - Beam: " << beamId << ", carrier ID: " << carrierId << ", AC: " << (uint32_t)allocationChannelId << " - Switching to LOW LOAD back off parameterization");

          /// flag RA load as low load
          m_isLowRandomAccessLoad.at (std::make_pair (beamId,allocationChannelId)) = true;
        }
    }
}

void
SatNcc::CreateRandomAccessLoadControlMessage (uint16_t backoffProbability, uint16_t backoffTime, uint32_t beamId, uint8_t allocationChannelId)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("SatNcc::CreateRandomAccessLoadControlMessage");

  Ptr<SatRaMessage> raMsg = CreateObject<SatRaMessage> ();
  std::map<uint32_t, Ptr<SatBeamScheduler> >::iterator iterator = m_beamSchedulers.find (beamId);

  if (iterator == m_beamSchedulers.end ())
    {
      NS_FATAL_ERROR ("SatNcc::SendRaControlMessage - Beam scheduler not found");
    }

  /// set the random access allocation channel this message affects
  raMsg->SetAllocationChannelId (allocationChannelId);

  /// attach the new load control parameters to the message
  raMsg->SetBackoffProbability (backoffProbability);
  raMsg->SetBackoffTime (backoffTime);

  NS_LOG_INFO ("SatNcc::CreateRandomAccessLoadControlMessage - Sending random access control message for AC: " << (uint32_t)allocationChannelId <<
               ", backoff probability: " << backoffProbability <<
               ", backoff time: " << backoffTime);

  iterator->second->Send (raMsg);
}

void
SatNcc::UtCrReceived (uint32_t beamId, Address utId, Ptr<SatCrMessage> crMsg)
{
  NS_LOG_FUNCTION (this << beamId << utId << crMsg);

  m_beamSchedulers[beamId]->UtCrReceived (utId, crMsg);
}

void
SatNcc::AddBeam (uint32_t beamId, SatNcc::SendCallback cb, Ptr<SatSuperframeSeq> seq, uint32_t maxFrameSize)
{
  NS_LOG_FUNCTION (this << &cb);

  Ptr<SatBeamScheduler> scheduler;
  std::map<uint32_t, Ptr<SatBeamScheduler> >::iterator iterator = m_beamSchedulers.find (beamId);

  if ( iterator != m_beamSchedulers.end () )
    {
      NS_FATAL_ERROR ( "Beam tried to add, already added." );
    }

  scheduler = CreateObject<SatBeamScheduler> ();
  scheduler->Initialize (beamId, cb, seq, maxFrameSize );

  m_beamSchedulers.insert (std::make_pair (beamId, scheduler));
}

uint32_t
SatNcc::AddUt (Address utId, Ptr<SatLowerLayerServiceConf> llsConf, uint32_t beamId)
{
  NS_LOG_FUNCTION (this << utId << beamId);

  std::map<uint32_t, Ptr<SatBeamScheduler> >::iterator iterator = m_beamSchedulers.find (beamId);

  if ( iterator == m_beamSchedulers.end () )
    {
      NS_FATAL_ERROR ( "Beam where tried to add, not found." );
    }

  return m_beamSchedulers[beamId]->AddUt (utId, llsConf);
}

void
SatNcc::SetRandomAccessLowLoadBackoffProbability (uint8_t allocationChannelId, uint16_t lowLoadBackOffProbability)
{
  NS_LOG_FUNCTION (this << (uint32_t) allocationChannelId << lowLoadBackOffProbability);

  NS_LOG_INFO ("SatNcc::SetRandomAccessLowLoadBackoffProbability - AC: " << (uint32_t)allocationChannelId << ", low load backoff probability: " << lowLoadBackOffProbability);
  m_lowLoadBackOffProbability[allocationChannelId] = lowLoadBackOffProbability;
}

void
SatNcc::SetRandomAccessHighLoadBackoffProbability (uint8_t allocationChannelId, uint16_t highLoadBackOffProbability)
{
  NS_LOG_FUNCTION (this << (uint32_t)allocationChannelId << highLoadBackOffProbability);

  NS_LOG_INFO ("SatNcc::SetRandomAccessHighLoadBackoffProbability - AC: " << (uint32_t)allocationChannelId << ", high load backoff probability: " << highLoadBackOffProbability);
  m_highLoadBackOffProbability[allocationChannelId] = highLoadBackOffProbability;
}

void
SatNcc::SetRandomAccessLowLoadBackoffTime (uint8_t allocationChannelId, uint16_t lowLoadBackOffTime)
{
  NS_LOG_FUNCTION (this << (uint32_t) allocationChannelId << lowLoadBackOffTime);

  NS_LOG_INFO ("SatNcc::SetRandomAccessLowLoadBackoffTime - AC: " << (uint32_t)allocationChannelId << ", low load backoff time: " << lowLoadBackOffTime);
  m_lowLoadBackOffTime[allocationChannelId] = lowLoadBackOffTime;
}

void
SatNcc::SetRandomAccessHighLoadBackoffTime (uint8_t allocationChannelId, uint16_t highLoadBackOffTime)
{
  NS_LOG_FUNCTION (this << (uint32_t)allocationChannelId << highLoadBackOffTime);

  NS_LOG_INFO ("SatNcc::SetRandomAccessHighLoadBackoffTime - AC: " << (uint32_t)allocationChannelId << ", high load backoff time: " << highLoadBackOffTime);
  m_highLoadBackOffTime[allocationChannelId] = highLoadBackOffTime;
}

void
SatNcc::SetRandomAccessAverageNormalizedOfferedLoadThreshold (uint8_t allocationChannelId, double threshold)
{
  NS_LOG_FUNCTION (this << (uint32_t) allocationChannelId << threshold);

  NS_LOG_INFO ("SatNcc::SetRandomAccessAverageNormalizedOfferedLoadThreshold - AC: " << (uint32_t)allocationChannelId << ", average normalized offered load threshold: " << threshold);
  m_randomAccessAverageNormalizedOfferedLoadThreshold[allocationChannelId] = threshold;
}

Ptr<SatBeamScheduler>
SatNcc::GetBeamScheduler (uint32_t beamId) const
{
  std::map<uint32_t, Ptr<SatBeamScheduler> >::const_iterator it = m_beamSchedulers.find (beamId);

  if (it == m_beamSchedulers.end ())
    {
      return 0;
    }
  else
    {
      return it->second;
    }
}

} // namespace ns3
