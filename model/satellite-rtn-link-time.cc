/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include <math.h>
#include "ns3/nstime.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "satellite-rtn-link-time.h"


NS_LOG_COMPONENT_DEFINE ("SatRtnLinkTime");

namespace ns3 {


SatRtnLinkTime::SatRtnLinkTime ()
  : m_superframeSeq ()
{
  NS_LOG_FUNCTION (this);
}


SatRtnLinkTime::~SatRtnLinkTime ()
{

}

void
SatRtnLinkTime::Initialize (Ptr<SatSuperframeSeq> seq)
{
  NS_LOG_FUNCTION (this);
  m_superframeSeq = seq;
}

Time
SatRtnLinkTime::GetSuperFrameDuration (uint8_t superFrameSeqId) const
{
  NS_LOG_FUNCTION (this << (uint32_t) superFrameSeqId);

  return m_superframeSeq->GetDuration (superFrameSeqId);
}

uint32_t
SatRtnLinkTime::GetCurrentSuperFrameCount (uint8_t superFrameSeqId) const
{
  NS_LOG_FUNCTION (this << (uint32_t) superFrameSeqId);

  return (uint32_t)(Simulator::Now ().GetInteger () / m_superframeSeq->GetDuration (superFrameSeqId).GetInteger ());
}

uint32_t
SatRtnLinkTime::GetNextSuperFrameCount (uint8_t superFrameSeqId) const
{
  NS_LOG_FUNCTION (this << (uint32_t) superFrameSeqId);

  return GetCurrentSuperFrameCount (superFrameSeqId) + 1;
}

Time
SatRtnLinkTime::GetCurrentSuperFrameStartTime (uint8_t superFrameSeqId) const
{
  NS_LOG_FUNCTION (this << (uint32_t) superFrameSeqId);

  uint32_t count = GetCurrentSuperFrameCount (superFrameSeqId);
  return Time (count * m_superframeSeq->GetDuration (superFrameSeqId).GetInteger ());
}

Time
SatRtnLinkTime::GetNextSuperFrameStartTime (uint8_t superFrameSeqId) const
{
  NS_LOG_FUNCTION (this << superFrameSeqId);

  uint32_t count = GetNextSuperFrameCount (superFrameSeqId);
  return Time (count * m_superframeSeq->GetDuration (superFrameSeqId).GetInteger ());
}

Time
SatRtnLinkTime::GetSuperFrameTxTime (uint8_t superFrameSeqId, uint32_t superFrameCount, Time timingAdvance) const
{
  NS_LOG_FUNCTION (this << (uint32_t) superFrameSeqId << superFrameCount << timingAdvance.GetSeconds ());

  return (Time (superFrameCount * m_superframeSeq->GetDuration (superFrameSeqId).GetInteger ()) - timingAdvance);
}

uint32_t
SatRtnLinkTime::GetCurrentSuperFrameCount (uint8_t superFrameSeqId, Time timingAdvance) const
{
  NS_LOG_FUNCTION (this << superFrameSeqId << timingAdvance.GetSeconds ());

  Time earliestRxTime = Simulator::Now () + timingAdvance;
  uint32_t count = earliestRxTime.GetInteger () / m_superframeSeq->GetDuration (superFrameSeqId).GetInteger ();

  return count;
}

uint32_t
SatRtnLinkTime::GetNextSuperFrameCount (uint8_t superFrameSeqId, Time timingAdvance) const
{
  NS_LOG_FUNCTION (this << (uint32_t) superFrameSeqId << timingAdvance.GetSeconds ());

  uint32_t nextCount = GetCurrentSuperFrameCount (superFrameSeqId, timingAdvance) + 1;

  return nextCount;
}

Time
SatRtnLinkTime::GetCurrentSuperFrameTxTime (uint8_t superFrameSeqId, Time timingAdvance) const
{
  NS_LOG_FUNCTION (this << superFrameSeqId << timingAdvance.GetSeconds ());

  Time expectedSuperframeReceiveStartTime = Time (GetCurrentSuperFrameCount (superFrameSeqId, timingAdvance) * m_superframeSeq->GetDuration (superFrameSeqId).GetInteger ());
  Time transmitStart = expectedSuperframeReceiveStartTime - timingAdvance;

  return transmitStart;
}

Time
SatRtnLinkTime::GetNextSuperFrameTxTime (uint8_t superFrameSeqId, Time timingAdvance) const
{
  NS_LOG_FUNCTION (this << (uint32_t) superFrameSeqId << timingAdvance.GetSeconds ());

  Time expectedSuperframeReceiveStartTime = Time (GetNextSuperFrameCount (superFrameSeqId, timingAdvance) * m_superframeSeq->GetDuration (superFrameSeqId).GetInteger ());
  Time transmitStart = expectedSuperframeReceiveStartTime - timingAdvance;

  return transmitStart;
}

} // namespace ns3
