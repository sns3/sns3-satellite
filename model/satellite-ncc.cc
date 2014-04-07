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
                     MakeTraceSourceAccessor (&SatNcc::m_nccRxTrace))
    .AddTraceSource ("NccTx",
                     "Trace source indicating a TBTP has sent by NCC",
                     MakeTraceSourceAccessor (&SatNcc::m_nccTxTrace))
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
SatNcc::Receive (Ptr<Packet> packet, uint32_t beamId)
{
  NS_LOG_FUNCTION (this << packet << beamId);
}

void
SatNcc::UtCnoUpdated (uint32_t beamId, Address utId, Address /*gwId*/, double cno)
{
  NS_LOG_FUNCTION (this << beamId << utId << cno);

  m_beamSchedulers[beamId]->UpdateUtCno (utId, cno);
}

void
SatNcc::DoRandomAccessDynamicLoadControl (uint32_t beamId, uint32_t carrierId, uint32_t allocationChannelId, double averageNormalizedOfferedLoad)
{
  NS_LOG_FUNCTION (this << beamId << carrierId << averageNormalizedOfferedLoad);

  bool isLowRandomAccessLoad = true;
  std::map<std::pair<uint32_t,uint32_t>,bool>::iterator findResult;
  std::pair<std::map<std::pair<uint32_t,uint32_t>,bool>::iterator,bool> insertResult;

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

  NS_LOG_INFO ("SatNcc::DoRandomAccessDynamicLoadControl - Beam: " << beamId << ", carrier ID: " << carrierId << ", AC: " << allocationChannelId << " - Measuring the average normalized offered random access load: " << averageNormalizedOfferedLoad);

  std::map<uint32_t,double>::iterator itThreshold = m_randomAccessAverageNormalizedOfferedLoadThreshold.find (allocationChannelId);

  if (itThreshold == m_randomAccessAverageNormalizedOfferedLoadThreshold.end ())
    {
      NS_FATAL_ERROR ("SatNcc::DoRandomAccessDynamicLoadControl - Average normalized offered load threshold not set for beam: " << beamId << ", carrier: " << carrierId << ", allocation channel: " << allocationChannelId);
    }

  /// low RA load in effect
  if (isLowRandomAccessLoad)
    {
      NS_LOG_INFO ("SatNcc::DoRandomAccessDynamicLoadControl - Beam: " << beamId << ", carrier ID: " << carrierId << " - Currently low load in effect for allocation channel: " << allocationChannelId);
      /// check the load against the parameterized value
      if (averageNormalizedOfferedLoad >= itThreshold->second)
        {
          std::map<uint32_t,uint16_t>::iterator it = m_highLoadBackOffProbability.find (allocationChannelId);

          if (it == m_highLoadBackOffProbability.end ())
            {
              NS_FATAL_ERROR ("SatNcc::DoRandomAccessDynamicLoadControl - High load backoff probability not set for allocation channel: " << allocationChannelId);
            }

          /// use high load back off value
          CreateRandomAccessLoadControlMessage (it->second, beamId, allocationChannelId);

          NS_LOG_INFO ("SatNcc::DoRandomAccessDynamicLoadControl - Beam: " << beamId << ", carrier ID: " << carrierId << ", AC: " << allocationChannelId << " - Switching to HIGH LOAD back off parameterization");

          /// flag RA load as high load
          m_isLowRandomAccessLoad.at (std::make_pair (beamId,allocationChannelId)) = false;
        }
    }
  /// high RA load in effect
  else
    {
      NS_LOG_INFO ("SatNcc::DoRandomAccessDynamicLoadControl - Beam: " << beamId << ", carrier ID: " << carrierId << " - Currently high load in effect for allocation channel: " << allocationChannelId);

      /// check the load against the parameterized value
      if (averageNormalizedOfferedLoad < itThreshold->second)
        {
          std::map<uint32_t,uint16_t>::iterator it = m_lowLoadBackOffProbability.find (allocationChannelId);

          if (it == m_lowLoadBackOffProbability.end ())
            {
              NS_FATAL_ERROR ("SatNcc::DoRandomAccessDynamicLoadControl - Low load backoff probability not set for allocation channel: " << allocationChannelId);
            }

          /// use low load back off value
          CreateRandomAccessLoadControlMessage (it->second, beamId, allocationChannelId);

          NS_LOG_INFO ("SatNcc::DoRandomAccessDynamicLoadControl - Beam: " << beamId << ", carrier ID: " << carrierId << ", AC: " << allocationChannelId << " - Switching to LOW LOAD back off parameterization");

          /// flag RA load as low load
          m_isLowRandomAccessLoad.at (std::make_pair (beamId,allocationChannelId)) = true;
        }
    }
}

void
SatNcc::CreateRandomAccessLoadControlMessage (uint16_t backoffProbability, uint32_t beamId, uint32_t allocationChannelId)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("SatNcc::CreateRandomAccessLoadControlMessage");

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

  NS_LOG_INFO ("SatNcc::CreateRandomAccessLoadControlMessage - Sending random access control message for AC: " << allocationChannelId << ", backoff probability: " << backoffProbability);

  iterator->second->Send (raMsg);
}

void
SatNcc::UtCrReceived (uint32_t beamId, Address utId, Ptr<SatCrMessage> crMsg)
{
  NS_LOG_FUNCTION (this << beamId << utId << crMsg);

  m_beamSchedulers[beamId]->UtCrReceived (utId, crMsg);
}

void
SatNcc::AddBeam (uint32_t beamId, SatNcc::SendCallback cb, Ptr<SatSuperframeSeq> seq, uint8_t maxRcCount, uint32_t maxFrameSize)
{
  NS_LOG_FUNCTION (this << &cb);

  Ptr<SatBeamScheduler> scheduler;
  std::map<uint32_t, Ptr<SatBeamScheduler> >::iterator iterator = m_beamSchedulers.find (beamId);

  if ( iterator != m_beamSchedulers.end () )
    {
      NS_FATAL_ERROR ( "Beam tried to add, already added." );
    }

  scheduler = CreateObject<SatBeamScheduler> ();
  scheduler->Initialize (beamId, cb, seq, maxRcCount, maxFrameSize );

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
SatNcc::SetRandomAccessLowLoadBackoffProbability (uint32_t allocationChannelId, uint16_t lowLoadBackOffProbability)
{
  NS_LOG_FUNCTION (this << allocationChannelId << lowLoadBackOffProbability);

  NS_LOG_INFO ("SatNcc::SetRandomAccessLowLoadBackoffProbability - AC: " << allocationChannelId << ", low load backoff probability: " << lowLoadBackOffProbability);
  m_lowLoadBackOffProbability[allocationChannelId] = lowLoadBackOffProbability;
}

void
SatNcc::SetRandomAccessHighLoadBackoffProbability (uint32_t allocationChannelId, uint16_t highLoadBackOffProbability)
{
  NS_LOG_FUNCTION (this << allocationChannelId << highLoadBackOffProbability);

  NS_LOG_INFO ("SatNcc::SetRandomAccessHighLoadBackoffProbability - AC: " << allocationChannelId << ", high load backoff probability: " << highLoadBackOffProbability);
  m_highLoadBackOffProbability[allocationChannelId] = highLoadBackOffProbability;
}

void
SatNcc::SetRandomAccessAverageNormalizedOfferedLoadThreshold (uint32_t allocationChannelId, double threshold)
{
  NS_LOG_FUNCTION (this << allocationChannelId << threshold);

  NS_LOG_INFO ("SatNcc::SetRandomAccessAverageNormalizedOfferedLoadThreshold - AC: " << allocationChannelId << ", average normalized offered load threshold: " << threshold);
  m_randomAccessAverageNormalizedOfferedLoadThreshold[allocationChannelId] = threshold;
}

} // namespace ns3
