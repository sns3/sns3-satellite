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
 :m_superframeSeq ()
{
  NS_LOG_FUNCTION (this);
}


SatRtnLinkTime::~SatRtnLinkTime ()
{

}

void
SatRtnLinkTime::Initialize (Ptr<SatSuperframeSeq> seq)
{
  NS_LOG_LOGIC (this);
  m_superframeSeq = seq;
}

Time
SatRtnLinkTime::GetSuperFrameDuration (uint8_t superFrameSeqId) const
{
  NS_LOG_LOGIC (this << superFrameSeqId);

  return m_superframeSeq->GetDuration (superFrameSeqId);
}

uint32_t
SatRtnLinkTime::GetCurrentSuperFrameCount (uint8_t superFrameSeqId) const
{
  NS_LOG_LOGIC (this << superFrameSeqId);

  return (uint32_t)(floor(Simulator::Now ().GetSeconds () / m_superframeSeq->GetDuration (superFrameSeqId).GetSeconds ()));
}

uint32_t
SatRtnLinkTime::GetNextSuperFrameCount (uint8_t superFrameSeqId) const
{
  NS_LOG_LOGIC (this << superFrameSeqId);

  return GetCurrentSuperFrameCount (superFrameSeqId) + 1;
}

Time
SatRtnLinkTime::GetCurrentSuperFrameStartTime (uint8_t superFrameSeqId) const
{
  uint32_t count = GetCurrentSuperFrameCount (superFrameSeqId);
  return (Seconds (count * m_superframeSeq->GetDuration (superFrameSeqId).GetSeconds ()));
}

Time
SatRtnLinkTime::GetNextSuperFrameStartTime (uint8_t superFrameSeqId) const
{
  uint32_t count = GetNextSuperFrameCount (superFrameSeqId);
  return (Seconds (count * m_superframeSeq->GetDuration (superFrameSeqId).GetSeconds ()));
}

Time
SatRtnLinkTime::GetSuperFrameTxTime (uint8_t superFrameSeqId, uint32_t superFrameCount, Time timingAdvance) const
{
  return (Seconds (superFrameCount * m_superframeSeq->GetDuration (superFrameSeqId).GetSeconds ()) - timingAdvance);
}

uint32_t
SatRtnLinkTime::GetCurrentSuperFrameCount (uint8_t superFrameSeqId, Time timingAdvance) const
{
  NS_LOG_FUNCTION (this << superFrameSeqId << timingAdvance.GetSeconds ());

  Time earliestRxTime = Simulator::Now () + timingAdvance;
  return (uint32_t)(floor(earliestRxTime.GetSeconds () / m_superframeSeq->GetDuration (superFrameSeqId).GetSeconds ()));
}

uint32_t
SatRtnLinkTime::GetNextSuperFrameCount (uint8_t superFrameSeqId, Time timingAdvance) const
{
  NS_LOG_FUNCTION (this << superFrameSeqId << timingAdvance.GetSeconds ());

  return GetCurrentSuperFrameCount (superFrameSeqId, timingAdvance) + 1;
}

Time
SatRtnLinkTime::GetCurrentSuperFrameTxTime (uint8_t superFrameSeqId, Time timingAdvance) const
{
  NS_LOG_FUNCTION (this << superFrameSeqId << timingAdvance.GetSeconds ());

  return Seconds (GetCurrentSuperFrameCount (superFrameSeqId, timingAdvance) * m_superframeSeq->GetDuration (superFrameSeqId).GetSeconds ());
}

Time
SatRtnLinkTime::GetNextSuperFrameTxTime (uint8_t superFrameSeqId, Time timingAdvance) const
{
  NS_LOG_FUNCTION (this << superFrameSeqId << timingAdvance.GetSeconds ());

  return Seconds (GetNextSuperFrameCount (superFrameSeqId, timingAdvance) * m_superframeSeq->GetDuration (superFrameSeqId).GetSeconds ());
}


} // namespace ns3
