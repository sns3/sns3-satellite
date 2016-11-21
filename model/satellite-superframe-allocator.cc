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
#include <limits>
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "satellite-utils.h"
#include "satellite-superframe-allocator.h"

NS_LOG_COMPONENT_DEFINE ("SatSuperframeAllocator");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatSuperframeAllocator);

TypeId
SatSuperframeAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatSuperframeAllocator")
    .SetParent<Object> ()
    .AddAttribute ("TargetLoad",
                   "Target load limits upper bound of the symbols in a frame.",
                   DoubleValue (0.9),
                   MakeDoubleAccessor (&SatSuperframeAllocator::m_targetLoad),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("FcaEnabled",
                   "Free capacity allocation (FCA) enable status.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatSuperframeAllocator::m_fcaEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("RcBasedAllocationEnabled",
                   "Time slot generated per RC symbols instead of sum of UT symbols.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatSuperframeAllocator::m_rcBasedAllocationEnabled),
                   MakeBooleanChecker ())
  ;
  return tid;
}

TypeId
SatSuperframeAllocator::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatSuperframeAllocator::SatSuperframeAllocator (Ptr<SatSuperframeConf> superFrameConf)
  : m_superframeConf (superFrameConf),
    m_targetLoad (0.0),
    m_fcaEnabled (false),
    m_minCarrierPayloadInBytes (0),
    m_minimumRateBasedBytesLeft (0),
    m_rcBasedAllocationEnabled (false)
{
  NS_LOG_FUNCTION (this);

  uint32_t currentMinCarrierPayloadInBytes = std::numeric_limits<uint32_t>::max ();
  uint32_t currentMostRobustSlotPayloadInBytes = std::numeric_limits<uint32_t>::max ();

  for (uint8_t i = 0; i < superFrameConf->GetFrameCount (); i++ )
    {
      Ptr<SatFrameConf> frameConf = superFrameConf->GetFrameConf (i);

      if (frameConf->IsRandomAccess () == false )
        {
          Ptr<SatFrameAllocator> frameAllocator = Create<SatFrameAllocator> (frameConf, i, superFrameConf->GetConfigType ());
          m_frameAllocators.push_back ( frameAllocator );

          uint32_t minCarrierPayloadInBytes = frameAllocator->GetCarrierMinPayloadInBytes ();

          if ( minCarrierPayloadInBytes < currentMinCarrierPayloadInBytes )
            {
              currentMinCarrierPayloadInBytes = minCarrierPayloadInBytes;
              m_minCarrierPayloadInBytes = minCarrierPayloadInBytes;
            }

          uint32_t mostRobustSlotPayloadInBytes = frameAllocator->GetMostRobustWaveform ()->GetPayloadInBytes ();

          if ( mostRobustSlotPayloadInBytes < currentMostRobustSlotPayloadInBytes )
            {
              currentMostRobustSlotPayloadInBytes = mostRobustSlotPayloadInBytes;
              m_mostRobustSlotPayloadInBytes = mostRobustSlotPayloadInBytes;
            }

          m_minimumRateBasedBytesLeft += frameConf->GetCarrierCount () * minCarrierPayloadInBytes;
        }
    }
}

SatSuperframeAllocator::~SatSuperframeAllocator ()
{
  NS_LOG_FUNCTION (this);
}

void
SatSuperframeAllocator::RemoveAllocations ()
{
  NS_LOG_FUNCTION (this);

  for (FrameAllocatorContainer_t::iterator it = m_frameAllocators.begin (); it != m_frameAllocators.end (); it++  )
    {
      (*it)->Reset ();
    }
}

void
SatSuperframeAllocator::GenerateTimeSlots (SatFrameAllocator::TbtpMsgContainer_t& tbtpContainer, uint32_t maxSizeInBytes, SatFrameAllocator::UtAllocInfoContainer_t& utAllocContainer,
                                           TracedCallback<uint32_t> waveformTrace, TracedCallback<uint32_t, uint32_t> utLoadTrace, TracedCallback<uint32_t, double> loadTrace)
{
  NS_LOG_FUNCTION (this);

  if (tbtpContainer.empty ())
    {
      NS_FATAL_ERROR ("TBTP container must contain at least one message.");
    }

  for (FrameAllocatorContainer_t::iterator it = m_frameAllocators.begin (); it != m_frameAllocators.end (); it++  )
    {
      (*it)->GenerateTimeSlots (tbtpContainer, maxSizeInBytes, utAllocContainer, m_rcBasedAllocationEnabled, waveformTrace, utLoadTrace, loadTrace);
    }
}

void
SatSuperframeAllocator::PreAllocateSymbols (SatFrameAllocator::SatFrameAllocContainer_t& allocReqs)
{
  NS_LOG_FUNCTION (this);

  RemoveAllocations ();

  for (SatFrameAllocator::SatFrameAllocContainer_t::iterator itReq = allocReqs.begin (); itReq != allocReqs.end (); itReq++  )
    {
      AllocateToFrame (*itReq);
    }

  for (FrameAllocatorContainer_t::iterator it = m_frameAllocators.begin (); it != m_frameAllocators.end (); it++  )
    {
      (*it)->PreAllocateSymbols (m_targetLoad, m_fcaEnabled);
    }
}

void SatSuperframeAllocator::ReserveMinimumRate (uint32_t minimumRateBytes, bool controlSlotsEnabled)
{
  NS_LOG_FUNCTION (this << minimumRateBytes);

  uint32_t rateBasedByteToCheck = minimumRateBytes;

  if ( controlSlotsEnabled )
    {
      rateBasedByteToCheck += m_mostRobustSlotPayloadInBytes;
    }

  if ( rateBasedByteToCheck > m_minCarrierPayloadInBytes )
    {
      NS_FATAL_ERROR ("Minimum requested bytes (" << minimumRateBytes << ") for UT is greater than bytes in minimum carrier (" << m_minCarrierPayloadInBytes << ")");
    }
  else if ( rateBasedByteToCheck > m_minimumRateBasedBytesLeft )
    {
      NS_FATAL_ERROR ("Minimum requested bytes (" << minimumRateBytes << ") for UT is greater than minimum bytes left (" << m_minimumRateBasedBytesLeft << ")");
    }
  else
    {
      m_minimumRateBasedBytesLeft -= minimumRateBytes;
    }
}

bool
SatSuperframeAllocator::AllocateToFrame (SatFrameAllocator::SatFrameAllocReq * allocReq)
{
  NS_LOG_FUNCTION (this);

  bool allocated = false;

  SupportedFramesMap_t supportedFrames;

  // find supported symbol rates (frames)
  for (FrameAllocatorContainer_t::iterator it = m_frameAllocators.begin (); it != m_frameAllocators.end (); it++  )
    {
      uint32_t waveformId = 0;

      if ( (*it)->GetBestWaveform (allocReq->m_cno, waveformId) )
        {
          supportedFrames.insert (std::make_pair (*it, waveformId));
        }
    }

  if ( supportedFrames.empty () == false )
    {
      // allocate with CC level CRA + RBDC + VBDC
      allocated = AllocateBasedOnCc (SatFrameAllocator::CC_LEVEL_CRA_RBDC_VBDC, allocReq, supportedFrames );

      if ( allocated == false )
        {
          // allocate with CC level CRA + RBDC
          allocated = AllocateBasedOnCc (SatFrameAllocator::CC_LEVEL_CRA_RBDC, allocReq, supportedFrames );

          if ( allocated == false )
            {
              // allocate with CC level CRA + MIM RBDC
              allocated = AllocateBasedOnCc (SatFrameAllocator::CC_LEVEL_CRA_MIN_RBDC, allocReq, supportedFrames );

              if ( allocated == false )
                {
                  // allocate with CC level CRA
                  allocated = AllocateBasedOnCc (SatFrameAllocator::CC_LEVEL_CRA, allocReq, supportedFrames );
                }
            }
        }
    }

  return allocated;
}

bool
SatSuperframeAllocator::AllocateBasedOnCc (SatFrameAllocator::CcLevel_t ccLevel, SatFrameAllocator::SatFrameAllocReq * allocReq, const SupportedFramesMap_t &frames)
{
  NS_LOG_FUNCTION (this << ccLevel);

  double loadInSymbols = 0;
  SupportedFramesMap_t::const_iterator selectedFrame = frames.begin ();

  if (frames.empty ())
    {
      NS_FATAL_ERROR ("Tried to allocate without frames!!!");
    }

  // find the lowest load frame
  for (SupportedFramesMap_t::const_iterator it = frames.begin (); it != frames.end (); it++  )
    {
      if ( it == frames.begin () )
        {
          loadInSymbols = it->first->GetCcLoad (ccLevel);
        }
      else if ( it->first->GetCcLoad (ccLevel) < loadInSymbols)
        {
          selectedFrame = it;
          loadInSymbols = it->first->GetCcLoad (ccLevel);
        }
    }

  return selectedFrame->first->Allocate (ccLevel, allocReq, selectedFrame->second);
}

} // namespace ns3
