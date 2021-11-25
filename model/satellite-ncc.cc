/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
 * Copyright (c) 2018 CNES
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.fr>
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
    .AddAttribute ("HandoverDelay",
                   "Delay between handover acceptance and effective information transfer",
                   TimeValue (Seconds (0.0)),
                   MakeTimeAccessor (&SatNcc::m_utHandoverDelay),
                   MakeTimeChecker ())
    .AddAttribute ("UtTimeout",
                   "Timeout to logoff a UT. Set to zero to never logoff",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&SatNcc::m_utTimeout),
                   MakeTimeChecker ())
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
  : m_utHandoverDelay (Seconds (0.0)),
  m_utTimeout (Seconds (0))
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

  m_updateRoutingCallback.Nullify ();
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
  std::map<std::pair<uint32_t, uint8_t>, bool>::iterator findResult;
  std::pair<std::map<std::pair<uint32_t, uint8_t>, bool>::iterator, bool> insertResult;

  /// search for the current status of load control
  findResult = m_isLowRandomAccessLoad.find (std::make_pair (beamId, allocationChannelId));

  if (findResult == m_isLowRandomAccessLoad.end ())
    {
      insertResult = m_isLowRandomAccessLoad.insert (std::make_pair (std::make_pair (beamId, allocationChannelId), isLowRandomAccessLoad));

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

  NS_LOG_INFO ("Beam: " << beamId << ", carrier ID: " << carrierId << ", AC: " << (uint32_t)allocationChannelId << " - Measuring the average normalized offered random access load: " << averageNormalizedOfferedLoad);

  std::map<uint8_t, double>::iterator itThreshold = m_randomAccessAverageNormalizedOfferedLoadThreshold.find (allocationChannelId);

  if (itThreshold == m_randomAccessAverageNormalizedOfferedLoadThreshold.end ())
    {
      NS_FATAL_ERROR ("SatNcc::DoRandomAccessDynamicLoadControl - Average normalized offered load threshold not set for beam: " << beamId << ", carrier: " << carrierId << ", allocation channel: " << (uint32_t)allocationChannelId);
    }

  /// low RA load in effect
  if (isLowRandomAccessLoad)
    {
      NS_LOG_INFO ("Beam: " << beamId << ", carrier ID: " << carrierId << " - Currently low load in effect for allocation channel: " << (uint32_t)allocationChannelId);
      /// check the load against the parameterized value
      if (averageNormalizedOfferedLoad >= itThreshold->second)
        {
          std::map<uint8_t, uint16_t>::iterator it;

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

          NS_LOG_INFO ("Beam: " << beamId << ", carrier ID: " << carrierId << ", AC: " << (uint32_t)allocationChannelId << " - Switching to HIGH LOAD back off parameterization");

          /// flag RA load as high load
          m_isLowRandomAccessLoad.at (std::make_pair (beamId, allocationChannelId)) = false;
        }
    }
  /// high RA load in effect
  else
    {
      NS_LOG_INFO ("Beam: " << beamId << ", carrier ID: " << carrierId << " - Currently high load in effect for allocation channel: " << (uint32_t)allocationChannelId);

      /// check the load against the parameterized value
      if (averageNormalizedOfferedLoad < itThreshold->second)
        {
          std::map<uint8_t, uint16_t>::iterator it;

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

          NS_LOG_INFO ("Beam: " << beamId << ", carrier ID: " << carrierId << ", AC: " << (uint32_t)allocationChannelId << " - Switching to LOW LOAD back off parameterization");

          /// flag RA load as low load
          m_isLowRandomAccessLoad.at (std::make_pair (beamId, allocationChannelId)) = true;
        }
    }
}

void
SatNcc::CreateRandomAccessLoadControlMessage (uint16_t backoffProbability, uint16_t backoffTime, uint32_t beamId, uint8_t allocationChannelId)
{
  NS_LOG_FUNCTION (this);

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

  NS_LOG_INFO ("Sending random access control message for AC: " << (uint32_t)allocationChannelId <<
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
SatNcc::AddBeam (uint32_t beamId, SatNcc::SendCallback cb, Ptr<SatSuperframeSeq> seq, uint32_t maxFrameSize, Address gwAddress)
{
  NS_LOG_FUNCTION (this << &cb);

  Ptr<SatBeamScheduler> scheduler;
  std::map<uint32_t, Ptr<SatBeamScheduler> >::iterator iterator = m_beamSchedulers.find (beamId);

  if ( iterator != m_beamSchedulers.end () )
    {
      NS_FATAL_ERROR ( "Beam tried to add, already added." );
    }

  scheduler = CreateObject<SatBeamScheduler> ();
  scheduler->Initialize (beamId, cb, seq, maxFrameSize, gwAddress);

  scheduler->SetSendTbtpCallback (MakeCallback (&SatNcc::TbtpSent, this));

  m_beamSchedulers.insert (std::make_pair (beamId, scheduler));
}

void
SatNcc::AddUt (Ptr<SatLowerLayerServiceConf> llsConf, Address utId, uint32_t beamId, Callback<void, uint32_t> setRaChannelCallback, bool verifyExisting)
{
  NS_LOG_FUNCTION (this << utId << beamId);

  std::map<uint32_t, Ptr<SatBeamScheduler> >::iterator iterator = m_beamSchedulers.find (beamId);

  if ( iterator == m_beamSchedulers.end () )
    {
      NS_FATAL_ERROR ( "Beam where tried to add, not found." );
    }

  if (!verifyExisting || !(m_beamSchedulers[beamId]->HasUt (utId)))
    {
      setRaChannelCallback (m_beamSchedulers[beamId]->AddUt (utId, llsConf));
    }
}

void
SatNcc::SetRandomAccessLowLoadBackoffProbability (uint8_t allocationChannelId, uint16_t lowLoadBackOffProbability)
{
  NS_LOG_FUNCTION (this << (uint32_t) allocationChannelId << lowLoadBackOffProbability);

  NS_LOG_INFO ("AC: " << (uint32_t)allocationChannelId << ", low load backoff probability: " << lowLoadBackOffProbability);
  m_lowLoadBackOffProbability[allocationChannelId] = lowLoadBackOffProbability;
}

void
SatNcc::SetRandomAccessHighLoadBackoffProbability (uint8_t allocationChannelId, uint16_t highLoadBackOffProbability)
{
  NS_LOG_FUNCTION (this << (uint32_t)allocationChannelId << highLoadBackOffProbability);

  NS_LOG_INFO ("AC: " << (uint32_t)allocationChannelId << ", high load backoff probability: " << highLoadBackOffProbability);
  m_highLoadBackOffProbability[allocationChannelId] = highLoadBackOffProbability;
}

void
SatNcc::SetRandomAccessLowLoadBackoffTime (uint8_t allocationChannelId, uint16_t lowLoadBackOffTime)
{
  NS_LOG_FUNCTION (this << (uint32_t) allocationChannelId << lowLoadBackOffTime);

  NS_LOG_INFO ("AC: " << (uint32_t)allocationChannelId << ", low load backoff time: " << lowLoadBackOffTime);
  m_lowLoadBackOffTime[allocationChannelId] = lowLoadBackOffTime;
}

void
SatNcc::SetRandomAccessHighLoadBackoffTime (uint8_t allocationChannelId, uint16_t highLoadBackOffTime)
{
  NS_LOG_FUNCTION (this << (uint32_t)allocationChannelId << highLoadBackOffTime);

  NS_LOG_INFO ("AC: " << (uint32_t)allocationChannelId << ", high load backoff time: " << highLoadBackOffTime);
  m_highLoadBackOffTime[allocationChannelId] = highLoadBackOffTime;
}

void
SatNcc::SetRandomAccessAverageNormalizedOfferedLoadThreshold (uint8_t allocationChannelId, double threshold)
{
  NS_LOG_FUNCTION (this << (uint32_t) allocationChannelId << threshold);

  NS_LOG_INFO ("AC: " << (uint32_t)allocationChannelId << ", average normalized offered load threshold: " << threshold);
  m_randomAccessAverageNormalizedOfferedLoadThreshold[allocationChannelId] = threshold;
}

Ptr<SatBeamScheduler>
SatNcc::GetBeamScheduler (uint32_t beamId) const
{
  NS_LOG_FUNCTION (this << beamId);

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

void
SatNcc::DoMoveUtBetweenBeams (Address utId, uint32_t srcBeamId, uint32_t destBeamId)
{
  NS_LOG_FUNCTION (this << utId << srcBeamId << destBeamId);

  Ptr<SatBeamScheduler> srcScheduler = GetBeamScheduler (srcBeamId);
  Ptr<SatBeamScheduler> destScheduler = GetBeamScheduler (destBeamId);

  if (!srcScheduler || !destScheduler)
    {
      NS_FATAL_ERROR ("Source or destination beam not configured");
    }

  srcScheduler->TransferUtToBeam (utId, destScheduler);
  m_updateRoutingCallback (utId, srcScheduler->GetGwAddress (), destScheduler->GetGwAddress ());
}

void
SatNcc::MoveUtBetweenBeams (Address utId, uint32_t srcBeamId, uint32_t destBeamId)
{
  NS_LOG_FUNCTION (this << utId << srcBeamId << destBeamId);

  Ptr<SatBeamScheduler> scheduler = GetBeamScheduler (srcBeamId);
  Ptr<SatBeamScheduler> destination = GetBeamScheduler (destBeamId);

  if (!scheduler)
    {
      NS_FATAL_ERROR ("Source beam does not exist!");
    }

  if (!destination)
    {
      NS_LOG_WARN ("Destination beam does not exist, cancel handover");

      Ptr<SatTimuMessage> timuMsg = scheduler->CreateTimu ();
      scheduler->SendTo (timuMsg, utId);
    }
  else if (scheduler->HasUt (utId) && !destination->HasUt (utId))
    {
      NS_LOG_INFO ("Performing handover!");

      Ptr<SatTimuMessage> timuMsg = destination->CreateTimu ();
      scheduler->SendTo (timuMsg, utId);

      Simulator::Schedule (m_utHandoverDelay, &SatNcc::DoMoveUtBetweenBeams, this, utId, srcBeamId, destBeamId);
    }
  else if (!scheduler->HasUt (utId) && destination->HasUt (utId))
    {
      NS_LOG_INFO ("Handover already performed, sending back TIM-U just in case!");

      Ptr<SatTimuMessage> timuMsg = destination->CreateTimu ();
      scheduler->SendTo (timuMsg, utId);
    }
  else
    {
      NS_FATAL_ERROR ("Inconsistent handover state: UT is neither in source nor destination beam; or in both");
    }
}

void
SatNcc::SetUpdateRoutingCallback (SatNcc::UpdateRoutingCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_updateRoutingCallback = cb;
}

void
SatNcc::ReserveLogonChannel (uint32_t logonChannelId)
{
  NS_LOG_FUNCTION (this << logonChannelId);

  for (auto& beamScheduler : m_beamSchedulers)
    {
      beamScheduler.second->ReserveLogonChannel (logonChannelId);
    }
}

void
SatNcc::SetSendTbtpCallback (SendTbtpCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);

  m_txTbtpCallback = cb;
}

void
SatNcc::TbtpSent (Ptr<SatTbtpMessage> tbtp)
{
  NS_LOG_FUNCTION (this << tbtp);

  m_txTbtpCallback (tbtp);
}

void
SatNcc::ReceiveControlBurst (Address utId, uint32_t beamId)
{
  NS_LOG_FUNCTION (this << utId);

  std::pair <Address, uint32_t> id = std::make_pair (utId, beamId);

  if ((m_lastControlBurstReception.find (id) == m_lastControlBurstReception.end ()) && (m_utTimeout != Seconds (0)))
    {
      Simulator::Schedule (m_utTimeout, &SatNcc::CheckTimeout, this, utId, beamId);
    }
  m_lastControlBurstReception[id] = Simulator::Now ();
}

void
SatNcc::CheckTimeout (Address utId, uint32_t beamId)
{
  NS_LOG_FUNCTION (this << utId);
  std::pair <Address, uint32_t> id = std::make_pair (utId, beamId);
  NS_ASSERT_MSG (m_lastControlBurstReception.find (id) != m_lastControlBurstReception.end (), "UT address should be in map");

  Time lastReceptionDate = m_lastControlBurstReception[id];
  if (Simulator::Now () >= lastReceptionDate + m_utTimeout)
    {
      m_lastControlBurstReception.erase (id);
      m_beamSchedulers[beamId]->RemoveUt (utId);
    }
  else
    {
      Simulator::Schedule (lastReceptionDate + m_utTimeout - Simulator::Now (), &SatNcc::CheckTimeout, this, utId, beamId);
    }
}

} // namespace ns3
