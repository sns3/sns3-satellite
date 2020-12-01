/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
 * Copyright (c) 2019 CNES
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
 * Author: Joaquin Muguerza <jmuguerza@viveris.fr>
 */

#include <algorithm>
#include <limits>
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "satellite-utils.h"
#include "satellite-default-superframe-allocator.h"

NS_LOG_COMPONENT_DEFINE ("SatDefaultSuperframeAllocator");


namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (SatDefaultSuperframeAllocator);


TypeId
SatDefaultSuperframeAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatDefaultSuperframeAllocator")
    .SetParent<SatSuperframeAllocator> ()
    .AddAttribute ("TargetLoad",
                   "Target load limits upper bound of the symbols in a frame.",
                   DoubleValue (0.9),
                   MakeDoubleAccessor (&SatDefaultSuperframeAllocator::m_targetLoad),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("FcaEnabled",
                   "Free capacity allocation (FCA) enable status.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatDefaultSuperframeAllocator::m_fcaEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("RcBasedAllocationEnabled",
                   "Time slot generated per RC symbols instead of sum of UT symbols.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SatDefaultSuperframeAllocator::m_rcBasedAllocationEnabled),
                   MakeBooleanChecker ())
  ;
  return tid;
}

TypeId
SatDefaultSuperframeAllocator::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatDefaultSuperframeAllocator::SatDefaultSuperframeAllocator (Ptr<SatSuperframeConf> superFrameConf)
  : SatSuperframeAllocator (superFrameConf),
  m_targetLoad (0.0),
  m_fcaEnabled (false),
  m_minCarrierPayloadInBytes (0),
  m_minimumRateBasedBytesLeft (0),
  m_rcBasedAllocationEnabled (false),
  m_totalBandwidth (0.0)
{
  NS_LOG_FUNCTION (this);

  uint32_t currentMinCarrierPayloadInBytes = std::numeric_limits<uint32_t>::max ();
  uint32_t currentMostRobustSlotPayloadInBytes = std::numeric_limits<uint32_t>::max ();

  Ptr<SatFrameConf> parentFrameConf = nullptr;
  Ptr<SatFrameAllocator> parentFrameAllocator = nullptr;
  for (uint8_t i = 0; i < superFrameConf->GetFrameCount (); i++ )
    {
      Ptr<SatFrameConf> frameConf = superFrameConf->GetFrameConf (i);
      if (!frameConf->IsRandomAccess ())
        {
          Ptr<SatFrameConf> parentConf = frameConf->GetParent ();
          if (parentConf != parentFrameConf)
            {
              NS_ASSERT_MSG (parentConf == nullptr, "Wrong ordering of frame confs. Need to have subdivided ones right after their parents.");
              parentFrameAllocator = nullptr;
            }
          parentFrameConf = frameConf;

          Ptr<SatFrameAllocator> frameAllocator = Create<SatFrameAllocator> (
                  frameConf,
                  i,
                  superFrameConf->GetConfigType (),
                  parentFrameAllocator);
          m_frameAllocators.push_back ( frameAllocator );
          parentFrameAllocator = frameAllocator;

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

          m_minimumRateBasedBytesLeft += frameAllocator->GetCarrierCount () * minCarrierPayloadInBytes;
          m_totalBandwidth += frameAllocator->GetCarrierBandwidthHz (true);
        }
    }
}

SatDefaultSuperframeAllocator::~SatDefaultSuperframeAllocator ()
{
  NS_LOG_FUNCTION (this);
}

void
SatDefaultSuperframeAllocator::SelectCarriers (SatFrameAllocator::SatFrameAllocContainer_t& allocReqs)
{
  NS_LOG_FUNCTION (this << &allocReqs);

  std::map<Ptr<SatFrameAllocator>, uint32_t> wsrDemand;

  NS_LOG_LOGIC ("Select best carrier for each terminal request");
  for (auto& allocRequest : allocReqs)
    {
      uint32_t bestWaveFormId = 0;
      Ptr<SatFrameAllocator> selectedFrameAllocator = SelectBestCarrier (allocRequest->m_cno, bestWaveFormId);
      if (selectedFrameAllocator != nullptr)
        {
          for (auto& request : allocRequest->m_reqPerRc)
            {
              wsrDemand[selectedFrameAllocator] += request.m_craBytes + std::max(request.m_minRbdcBytes, request.m_rbdcBytes) + request.m_vbdcBytes;
            }
        }
      else
        {
          NS_LOG_WARN ("SatDefaultSuperframeAllocator::SelectCarriers: No suitable frame and waveform found for "
                       "terminal at " << allocRequest->m_address << " with C/N0 of " << allocRequest->m_cno << ". "
                       "Ignoring this terminal in carrier selection.");
        }
    }

  NS_LOG_LOGIC ("Calculate Load Coefficient");
  double requestedBandwidth = 0.0;
  for (auto& demand : wsrDemand)
    {
      requestedBandwidth += demand.first->GetCarrierBandwidthHz () * demand.second / demand.first->GetVolumeBytes ();
    }
  double loadCoefficient = std::min(std::max(0.1, m_totalBandwidth / requestedBandwidth), 10.0);
  NS_LOG_INFO ("" << allocReqs.size () << " requested " << requestedBandwidth << "Hz through " <<
               wsrDemand.size () << " subdivision levels; giving a load coefficient of " << loadCoefficient);

  std::map<Ptr<SatFrameAllocator>, double, SatFrameAllocator::BandwidthComparator> scaledDemand;
  for (auto& demand : wsrDemand)
    {
      scaledDemand[demand.first] = loadCoefficient * demand.second / demand.first->GetVolumeBytes () + 1;
    }

  NS_LOG_LOGIC ("Zero-out old carrier selection");
  for (auto& frameAllocator : m_frameAllocators)
    {
      uint16_t zeroReference = 0;
      frameAllocator->SelectCarriers (zeroReference, 0);
    }

  NS_LOG_LOGIC ("Allocate carriers in subdivision levels");
  while (!scaledDemand.empty ())
    {
      NS_LOG_LOGIC ("Find bandwidth of the original frame");
      Ptr<SatFrameAllocator> frameAllocator = scaledDemand.begin()->first;
      while (frameAllocator->GetParent () != nullptr)
        {
          frameAllocator = frameAllocator->GetParent ();
        }
      double remainingBandwidth = frameAllocator->GetCarrierBandwidthHz (true);
      NS_ASSERT_MSG (remainingBandwidth != 0.0, "Could not find bandwidth of original frame");

      // Select carriers from the most subdivided version of the frame up to the original
      frameAllocator = scaledDemand.begin()->first;
      uint16_t offset = 0;
      while (frameAllocator != nullptr)
        {
          auto frameDemand = scaledDemand.find (frameAllocator);
          uint16_t demand = 0;
          if (frameDemand != scaledDemand.end ())
            {
              demand = frameDemand->second;
              scaledDemand.erase (frameDemand);
            }
          NS_LOG_LOGIC (demand << " carriers requested on (subdivided) frame " << frameAllocator->GetCarrierBandwidthHz());
          uint16_t carriersCount = 0;
          double carrierBandwidth = frameAllocator->GetCarrierBandwidthHz ();
          if (frameAllocator->GetParent () != nullptr)
            {
              // Select requested carriers of subdivided frames
              uint16_t remainingCarriers = remainingBandwidth / carrierBandwidth;
              carriersCount = std::max (uint16_t (0), std::min (demand, remainingCarriers));
            }

          frameAllocator->SelectCarriers (carriersCount, offset);
          remainingBandwidth -= carriersCount * carrierBandwidth;
          NS_LOG_INFO ("Remaining bandwidth on non-subdivided frame: " << remainingBandwidth);
          offset = (carriersCount + offset) / 2;
          frameAllocator = frameAllocator->GetParent ();
        }

      if (remainingBandwidth < 0.0)
        {
          NS_FATAL_ERROR ("SatDefaultSuperframeAllocator::SelectCarriers: remaining bandwidth is negative");
        }
    }
}

Ptr<SatFrameAllocator>
SatDefaultSuperframeAllocator::SelectBestCarrier (double cno, uint32_t& bestWaveFormId)
{
  NS_LOG_FUNCTION (this << cno);

  double cnoDiff = std::numeric_limits<double>::infinity ();
  Ptr<SatFrameAllocator> selectedFrameAllocator = nullptr;

  for (auto& frameAllocator : m_frameAllocators)
    {
      uint32_t waveFormId;
      double waveformCNo;
      if (frameAllocator->GetBestWaveform (cno, waveFormId, waveformCNo))
        {
          double diff = cno - waveformCNo;
          if (diff < cnoDiff)
            {
              cnoDiff = diff;
              bestWaveFormId = waveFormId;
              selectedFrameAllocator = frameAllocator;
            }
        }
    }

  return selectedFrameAllocator;
}

void
SatDefaultSuperframeAllocator::RemoveAllocations ()
{
  NS_LOG_FUNCTION (this);

  for (FrameAllocatorContainer_t::iterator it = m_frameAllocators.begin (); it != m_frameAllocators.end (); it++  )
    {
      (*it)->Reset ();
    }
}

void
SatDefaultSuperframeAllocator::GenerateTimeSlots (SatFrameAllocator::TbtpMsgContainer_t& tbtpContainer,
                                                  uint32_t maxSizeInBytes,
                                                  SatFrameAllocator::UtAllocInfoContainer_t& utAllocContainer,
                                                  TracedCallback<uint32_t> waveformTrace,
                                                  TracedCallback<uint32_t, uint32_t> utLoadTrace,
                                                  TracedCallback<uint32_t, double> loadTrace)
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
SatDefaultSuperframeAllocator::PreAllocateSymbols (SatFrameAllocator::SatFrameAllocContainer_t& allocReqs)
{
  NS_LOG_FUNCTION (this);

  if (m_superframeConf->GetConfigType () == SatSuperframeConf::CONFIG_TYPE_3)
    {
      SelectCarriers (allocReqs);
    }

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

void
SatDefaultSuperframeAllocator::ReserveMinimumRate (uint32_t minimumRateBytes, bool controlSlotsEnabled)
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

void
SatDefaultSuperframeAllocator::ReleaseMinimumRate (uint32_t minimumRateBytes, bool controlSlotsEnabled)
{
  NS_LOG_FUNCTION (this << minimumRateBytes);

  uint32_t rateBasedByteToCheck = minimumRateBytes;

  if ( controlSlotsEnabled )
    {
      rateBasedByteToCheck += m_mostRobustSlotPayloadInBytes;
    }

  if ( rateBasedByteToCheck > m_minCarrierPayloadInBytes )
    {
      NS_FATAL_ERROR ("Minimum released bytes (" << minimumRateBytes << ") for UT is greater than bytes in minimum carrier (" << m_minCarrierPayloadInBytes << ")");
    }
  else
    {
      m_minimumRateBasedBytesLeft += minimumRateBytes;
    }
}

bool
SatDefaultSuperframeAllocator::AllocateToFrame (SatFrameAllocator::SatFrameAllocReq * allocReq)
{
  NS_LOG_FUNCTION (this);

  bool allocated = false;

  SupportedFramesMap_t supportedFrames;

  // find supported symbol rates (frames)
  for (auto& frameAllocator : m_frameAllocators)
    {
      uint32_t waveformId = 0;
      double cnoThreshold = std::numeric_limits<double>::quiet_NaN();
      if ( frameAllocator->GetCarrierCount () && frameAllocator->GetBestWaveform (allocReq->m_cno, waveformId, cnoThreshold) )
        {
          supportedFrames.insert (std::make_pair (frameAllocator, waveformId));
        }
    }

  NS_LOG_INFO ("Terminal with C/N0 of " << allocReq->m_cno << " found " << supportedFrames.size () << " supported frames.");

  if (!supportedFrames.empty ())
    {
      // allocate with CC level CRA + RBDC + VBDC
      allocated = AllocateBasedOnCc (SatFrameAllocator::CC_LEVEL_CRA_RBDC_VBDC, allocReq, supportedFrames);

      if (!allocated)
        {
          // allocate with CC level CRA + RBDC
          allocated = AllocateBasedOnCc (SatFrameAllocator::CC_LEVEL_CRA_RBDC, allocReq, supportedFrames);

          if (!allocated)
            {
              // allocate with CC level CRA + MIM RBDC
              allocated = AllocateBasedOnCc (SatFrameAllocator::CC_LEVEL_CRA_MIN_RBDC, allocReq, supportedFrames);

              if (!allocated)
                {
                  // allocate with CC level CRA
                  allocated = AllocateBasedOnCc (SatFrameAllocator::CC_LEVEL_CRA, allocReq, supportedFrames);
                }
            }
        }
    }

  return allocated;
}

bool
SatDefaultSuperframeAllocator::AllocateBasedOnCc (SatFrameAllocator::CcLevel_t ccLevel, SatFrameAllocator::SatFrameAllocReq * allocReq, const SupportedFramesMap_t &frames)
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
