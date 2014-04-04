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
                     "Trace source indicating a TBTP has sentby NCC",
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

SatNcc::SatNcc () :
  m_randomAccessHighLoadThreshold (0.3),
  m_lowLoadBackOffProbability (0),
  m_highLoadBackOffProbability (0)
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
SatNcc::DoRandomAccessDynamicLoadControl (uint32_t beamId, uint32_t carrierId, double averageNormalizedOfferedLoad)
{
  NS_LOG_FUNCTION (this << beamId << carrierId << averageNormalizedOfferedLoad);

  bool isLowRandomAccessLoad = true;
  std::map<uint32_t,bool>::iterator findResult;
  std::pair<std::map<uint32_t,bool>::iterator,bool> insertResult;

  findResult = m_isLowRandomAccessLoad.find (beamId);

  if (findResult == m_isLowRandomAccessLoad.end ())
    {
      insertResult = m_isLowRandomAccessLoad.insert (std::make_pair (beamId,isLowRandomAccessLoad));

      if (!insertResult.second)
        {
          NS_FATAL_ERROR ("SatNcc::DoRandomAccessDynamicLoad - Insert failed");
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

  NS_LOG_INFO ("SatNcc::DoRandomAccessDynamicLoad - Beam: " << beamId << ", carrier ID: " << carrierId << " - Measuring the average normalized offered random access load: " << averageNormalizedOfferedLoad);

  /// low RA load in effect
  if (isLowRandomAccessLoad)
    {
      /// check the load against the parameterized value
      if (averageNormalizedOfferedLoad >= m_randomAccessHighLoadThreshold)
        {
          /// use high load back off value
          CreateRandomAccessLoadControlMessage (m_highLoadBackOffProbability, beamId);

          NS_LOG_INFO ("SatNcc::DoRandomAccessDynamicLoad - Beam: " << beamId << ", carrier ID: " << carrierId << " - Switching to HIGH LOAD back off parameterization");

          /// flag RA load as high load
          m_isLowRandomAccessLoad.at (beamId) = false;
        }
    }
  /// high RA load in effect
  else
    {
      /// check the load against the parameterized value
      if (averageNormalizedOfferedLoad < m_randomAccessHighLoadThreshold)
        {
          /// use low load back off value
          CreateRandomAccessLoadControlMessage (m_lowLoadBackOffProbability, beamId);

          NS_LOG_INFO ("SatNcc::DoRandomAccessDynamicLoad - Beam: " << beamId << ", carrier ID: " << carrierId << " - Switching to LOW LOAD back off parameterization");

          /// flag RA load as low load
          m_isLowRandomAccessLoad.at (beamId) = true;
        }
    }
}

void
SatNcc::CreateRandomAccessLoadControlMessage (uint16_t backoffProbability, uint32_t beamId)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("SatNcc::CreateRandomAccessLoadControlMessage");

  Ptr<SatRaMessage> raMsg = CreateObject<SatRaMessage> ();
  std::map<uint32_t, Ptr<SatBeamScheduler> >::iterator iterator = m_beamSchedulers.find (beamId);

  /// TODO at the moment only one RA allocation channel is fully supported
  uint32_t allocationChannelId = 0;

  if ( iterator == m_beamSchedulers.end () )
    {
      NS_FATAL_ERROR ( "SatNcc::SendRaControlMessage - Beam scheduler not found" );
    }

  raMsg->SetAllocationChannelId (allocationChannelId);
  raMsg->SetBackoffProbability (backoffProbability);

  NS_LOG_INFO ("SatNcc::CreateRandomAccessLoadControlMessage - Send random access control message");

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

} // namespace ns3
