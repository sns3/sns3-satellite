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
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/singleton.h"
#include "satellite-utils.h"
#include "satellite-rtn-link-time.h"
#include "satellite-beam-scheduler.h"

NS_LOG_COMPONENT_DEFINE ("SatBeamScheduler");

namespace ns3 {

/*
bool SatBeamScheduler::CompareCno (const UtReqInfoItem_t &first, const UtReqInfoItem_t &second)
{
  double result = false;

  double cnoFirst = first.second->GetCnoEstimation ();
  double cnoSecond = second.second->GetCnoEstimation ();

  if ( !isnan (cnoFirst) )
    {
       if ( isnan (cnoSecond) )
         {
           result = false;
         }
       else
         {
           result = (cnoFirst < cnoSecond);
         }
    }

  return result;
}*/

// UtInfo class declarations for SatBeamScheduler
SatBeamScheduler::SatUtInfo::SatUtInfo ( Ptr<SatDamaEntry> damaEntry, Ptr<SatCnoEstimator> cnoEstimator )
 : m_damaEntry (damaEntry),
   m_cnoEstimator (cnoEstimator)
{
  NS_LOG_FUNCTION (this);
}

Ptr<SatDamaEntry>
SatBeamScheduler::SatUtInfo::GetDamaEntry ()
{
  NS_LOG_FUNCTION (this);

  return m_damaEntry;
}

void
SatBeamScheduler::SatUtInfo::UpdateDamaEntriesFromCrs ()
{
  NS_LOG_FUNCTION (this);

  for ( CrMsgContainer_t::const_iterator crIt = m_crContainer.begin (); crIt != m_crContainer.end (); crIt++ )
    {
      SatCrMessage::RequestContainer_t crContent = (*crIt)->GetCapacityRequestContent ();

      for ( SatCrMessage::RequestContainer_t::const_iterator descriptorIt = crContent.begin (); descriptorIt != crContent.end (); descriptorIt++ )
        {
          switch (descriptorIt->first.second)
          {
            case SatEnums::DA_RBDC:
              m_damaEntry->UpdateRbdcInKbps (descriptorIt->first.first, descriptorIt->second);
              break;

            case SatEnums::DA_VBDC:
              m_damaEntry->UpdateVbdcInBytes (descriptorIt->first.first, descriptorIt->second * 1024);
              break;

            case SatEnums::DA_AVBDC:
              m_damaEntry->SetVbdcInBytes (descriptorIt->first.first, descriptorIt->second * 1024);
              break;

            default:
              break;
          }
        }
    }
        // clear container when CRs processed
    m_crContainer.clear ();
}

double
SatBeamScheduler::SatUtInfo::GetCnoEstimation ()
{
  NS_LOG_FUNCTION (this);

  return m_cnoEstimator->GetCnoEstimation ();
}

void
SatBeamScheduler::SatUtInfo::AddCnoSample (double sample)
{
  NS_LOG_FUNCTION (this << sample);

  m_cnoEstimator->AddSample (sample);
}

void
SatBeamScheduler::SatUtInfo::AddCrMsg (Ptr<SatCrMessage> crMsg)
{
  NS_LOG_FUNCTION (crMsg);

  m_crContainer.push_back (crMsg);
}

// SatBeamScheduler

NS_OBJECT_ENSURE_REGISTERED (SatBeamScheduler);

TypeId 
SatBeamScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatBeamScheduler")
    .SetParent<Object> ()
    .AddConstructor<SatBeamScheduler> ()
    .AddAttribute ("CnoEstimationMode",
                   "Mode of the C/N0 estimator",
                   EnumValue (SatCnoEstimator::LAST),
                   MakeEnumAccessor (&SatBeamScheduler::m_cnoEstimatorMode),
                   MakeEnumChecker (SatCnoEstimator::LAST, "Last value in window used.",
                                    SatCnoEstimator::MINIMUM, "Minimum value in window used.",
                                    SatCnoEstimator::AVERAGE, "Average value in window used."))
    .AddAttribute( "CnoEstimationWindow",
                   "Time window for C/N0 estimation.",
                   TimeValue (MilliSeconds (1000)),
                   MakeTimeAccessor (&SatBeamScheduler::m_cnoEstimationWindow),
                   MakeTimeChecker ())
    .AddAttribute( "MaxTwoWayPropagationDelay",
                   "Maximum two way propagation delay between GW and UT.",
                   TimeValue (MilliSeconds (560)),
                   MakeTimeAccessor (&SatBeamScheduler::m_maxTwoWayPropagationDelay),
                   MakeTimeChecker ())
    .AddAttribute( "MaxTBTPTxAndProcessingDelay",
                   "Maximum TBTP transmission and processing delay at the GW.",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&SatBeamScheduler::m_maxTBTPTxAndProcessingDelay),
                   MakeTimeChecker ())
  ;
  return tid;
}

SatBeamScheduler::SatBeamScheduler ()
  : m_beamId (0),
    m_superframeSeq (0),
    m_superFrameCounter (0),
    m_txCallback (0),
    m_tbtpAddCb (0),
    m_currentFrame (0),
    m_totalSlotsLeft (0),
    m_additionalSlots (0),
    m_slotsPerUt (0),
    m_cnoEstimatorMode (SatCnoEstimator::LAST),
    m_maxBbFrameSize (0)
{
  NS_LOG_FUNCTION (this);

  m_currentCarrier = m_carrierIds.end ();
  m_currentSlot = m_timeSlots.end ();
}

SatBeamScheduler::~SatBeamScheduler ()
{
  NS_LOG_FUNCTION (this);
}

void
SatBeamScheduler::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_txCallback.Nullify ();
  Object::DoDispose ();
}

void
SatBeamScheduler::Receive (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
}

bool
SatBeamScheduler::Send (Ptr<SatControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);
  NS_LOG_LOGIC ("p=" << msg );

  m_txCallback (msg, Mac48Address::GetBroadcast ());

  return true;
}

void
SatBeamScheduler::Initialize (uint32_t beamId, SatBeamScheduler::SendCtrlMsgCallback cb, Ptr<SatSuperframeSeq> seq, uint8_t maxRcCount, uint32_t maxFrameSizeInBytes)
{
  NS_LOG_FUNCTION (this << beamId << &cb);

  m_beamId = beamId;
  m_txCallback = cb;
  m_superframeSeq = seq;
  m_maxBbFrameSize = maxFrameSizeInBytes;

  /**
   * Calculating to start time for superframe counts to start the scheduling from.
   * The offset is calculated by estimating the maximum delay between GW and UT,
   * so that the sent TBTP will be received by UT in time to be able to still send
   * the packet in time.
   */
  Time totalDelay = m_maxTwoWayPropagationDelay + m_maxTBTPTxAndProcessingDelay;
  uint32_t sfCountOffset = (uint32_t)(totalDelay.GetInteger () / seq->GetDuration (0).GetInteger () + 1);

  // Scheduling starts after one empty super frame.
  m_superFrameCounter = Singleton<SatRtnLinkTime>::Get ()->GetNextSuperFrameCount (m_currentSequence) + sfCountOffset;

  // TODO: If RA channel is wanted to allocate to UT with some other means than randomizing
  // this part of implementation is needed to change
  m_raChRandomIndex = CreateObject<UniformRandomVariable> ();
  m_raChRandomIndex->SetAttribute ("Min", DoubleValue (0));

  // by default we give index 0, even if there is no RA channels configured.
  uint32_t maxIndex = 0;

  if ( m_superframeSeq->GetSuperframeConf (m_currentSequence)->GetRaChannelCount () > 0 )
    {
      maxIndex = m_superframeSeq->GetSuperframeConf (m_currentSequence)->GetRaChannelCount () - 1;
    }

  m_raChRandomIndex->SetAttribute("Max", DoubleValue (maxIndex));
  m_frameAllocator = CreateObject<SatFrameAllocator> (m_superframeSeq->GetSuperframeConf (m_currentSequence), m_superframeSeq->GetWaveformConf (), maxRcCount);

  NS_LOG_LOGIC ("Initialize SatBeamScheduler at " << Simulator::Now ().GetSeconds ());

  Time delay;
  Time txTime = Singleton<SatRtnLinkTime>::Get ()->GetNextSuperFrameStartTime (m_currentSequence);

  if (txTime > Now ())
    {
      delay = txTime - Now ();
    }
  else
    {
      NS_FATAL_ERROR ("Trying to schedule a superframe in the past!");
    }

  Simulator::Schedule (delay, &SatBeamScheduler::Schedule, this);
}

uint32_t
SatBeamScheduler::AddUt (Address utId, Ptr<SatLowerLayerServiceConf> llsConf)
{
  NS_LOG_FUNCTION (this << utId);

  Ptr<SatDamaEntry> damaEntry = Create<SatDamaEntry> (llsConf);

  // this method call acts as CAC check, if allocation fails fatal error is occured.
  m_frameAllocator->ReserveMinimumRate (damaEntry->GetMinRateBasedBytes (m_frameAllocator->GetSuperframeDuration ()));

  Ptr<SatCnoEstimator> cnoEstimator = CreateCnoEstimator ();
  Ptr<SatUtInfo> utInfo = Create<SatUtInfo> (damaEntry, cnoEstimator);

  std::pair<UtInfoMap_t::iterator, bool > result = m_utInfos.insert (std::make_pair (utId, utInfo));

  if (result.second)
    {
      SatFrameAllocator::SatFrameAllocReqItemContainer_t reqContainer (damaEntry->GetRcCount(), SatFrameAllocator::SatFrameAllocReqItem () );
      SatFrameAllocator::SatFrameAllocReq allocReq (reqContainer);
      allocReq.cno = NAN;
      allocReq.m_address = utId;

      m_utSortedInfos.push_back (std::make_pair (utId, allocReq));
    }
  else
    {
      NS_FATAL_ERROR ("UT (Address: " << utId << ") already added to Beam scheduler.");
    }

  m_superframeSeq->GetSuperframeConf (m_currentSequence)->GetRaChannelCount ();

  // return random RA channel index for the UT.
  return m_raChRandomIndex->GetInteger ();
}

void
SatBeamScheduler::UpdateUtCno (Address utId, double cno)
{
  NS_LOG_FUNCTION (this << utId << cno);

  // check that UT is added to this scheduler.
  UtInfoMap_t::iterator result = m_utInfos.find (utId);
  NS_ASSERT (result != m_utInfos.end ());

  m_utInfos[utId]->AddCnoSample (cno);
}

void
SatBeamScheduler::UtCrReceived (Address utId, Ptr<SatCrMessage> crMsg)
{
  NS_LOG_FUNCTION (this << utId << crMsg);

  // check that UT is added to this scheduler.
  UtInfoMap_t::iterator result = m_utInfos.find (utId);
  NS_ASSERT (result != m_utInfos.end ());

  NS_LOG_INFO ("SatBeamScheduler::UtCrReceived - UT: " << utId << " @ " << Now ().GetSeconds ());

  m_utInfos[utId]->AddCrMsg (crMsg);
}

Ptr<SatCnoEstimator>
SatBeamScheduler::CreateCnoEstimator ()
{
  NS_LOG_FUNCTION (this);

  Ptr<SatCnoEstimator> estimator = NULL;

  switch (m_cnoEstimatorMode)
  {
    case SatCnoEstimator::LAST:
    case SatCnoEstimator::MINIMUM:
    case SatCnoEstimator::AVERAGE:
      estimator = Create<SatBasicCnoEstimator> (m_cnoEstimatorMode, m_cnoEstimationWindow);
      break;

    default:
      NS_FATAL_ERROR ("Not supported C/N0 estimation mode!!!");
      break;

  }

  return estimator;
}

void
SatBeamScheduler::Schedule ()
{
  NS_LOG_FUNCTION (this);

  // check that there is UTs to schedule
  if ( m_utInfos.size() > 0 )
    {
      UpdateDamaEntriesWithReqs ();

      DoPreResourceAllocation ();

      // generate time slots
      Ptr<SatTbtpMessage> firstTbtp = CreateObject<SatTbtpMessage> (m_currentSequence);
      firstTbtp->SetSuperframeCounter (m_superFrameCounter++);

      std::vector<Ptr<SatTbtpMessage> > tbtps;
      tbtps.push_back (firstTbtp);

        // Add RA slots (channels)
      AddRaChannels (tbtps);

      SatFrameAllocator::UtAllocInfoContainer_t utAllocs;

        // Add DA slots to TBTP(s)
      m_frameAllocator->GenerateTimeSlots (tbtps, m_maxBbFrameSize, utAllocs);

      // update VBDC counter of the UT/RCs
      UpdateDamaEntriesWithAllocs (utAllocs);

      // send TBTPs
      for ( std::vector <Ptr<SatTbtpMessage> > ::const_iterator it = tbtps.begin (); it != tbtps.end (); it++ )
        {
          Send (*it);
        }

      NS_LOG_LOGIC ("TBTP sent at: " << Simulator::Now ().GetSeconds ());
    }

  // re-schedule next TBTP sending (call of this function)
  Simulator::Schedule ( m_superframeSeq->GetDuration (m_currentSequence), &SatBeamScheduler::Schedule, this);
}

void
SatBeamScheduler::AddRaChannels (std::vector <Ptr<SatTbtpMessage> >& tbtpContainer)
{
  NS_LOG_FUNCTION (this);

  if (tbtpContainer.empty ())
    {
      NS_FATAL_ERROR ("TBTP container must contain at least one message.");
    }

  Ptr<SatTbtpMessage> tbtpToFill = tbtpContainer.back ();

  Ptr<SatSuperframeConf> superFrameConf = m_superframeSeq->GetSuperframeConf (m_currentSequence);

  for (uint32_t i = 0; i < superFrameConf->GetRaChannelCount (); i++)
    {
      uint8_t frameId = superFrameConf->GetRaChannelFrameId (i);
      Ptr<SatFrameConf> frameConf = superFrameConf->GetFrameConf (frameId);
      uint16_t timeSlotCount = frameConf->GetTimeSlotCount() / frameConf->GetCarrierCount();

      if ( timeSlotCount > 0 )
        {
          if ( (tbtpToFill->GetSizeInBytes () + (tbtpToFill->GetTimeSlotInfoSizeInBytes () * timeSlotCount)) > m_maxBbFrameSize )
            {
              Ptr<SatTbtpMessage> newTbtp = CreateObject<SatTbtpMessage> (tbtpToFill->GetSuperframeSeqId ());
              newTbtp->SetSuperframeCounter ( tbtpToFill->GetSuperframeCounter ());

              tbtpContainer.push_back (newTbtp);

              tbtpToFill = newTbtp;
            }

          tbtpToFill->SetRaChannel (i, superFrameConf->GetRaChannelFrameId (i), timeSlotCount);
        }
    }
}

void
SatBeamScheduler::UpdateDamaEntriesWithReqs ()
{
  NS_LOG_FUNCTION (this);

  for (UtReqInfoContainer_t::iterator it = m_utSortedInfos.begin (); it != m_utSortedInfos.end (); it++)
    {
      // estimation of the C/N0 is done when scheduling UT

      Ptr<SatDamaEntry> damaEntry = m_utInfos.at (it->first)->GetDamaEntry ();

      // process received CRs
      m_utInfos.at (it->first)->UpdateDamaEntriesFromCrs ();

      it->second.cno = m_utInfos.at (it->first)->GetCnoEstimation ();

      for (uint8_t i = 0; i < damaEntry->GetRcCount (); i++ )
        {
          double superFrameDurationInSeconds = m_superframeSeq->GetSuperframeConf (m_currentSequence)->GetDuration ().GetSeconds ();

          it->second.m_reqPerRc[i].m_craBytes = ( 1000.0 * damaEntry->GetCraInKbps (i) * superFrameDurationInSeconds ) / SatUtils::BITS_PER_BYTE;
          it->second.m_reqPerRc[i].m_rbdcBytes = ( 1000.0 * damaEntry->GetRbdcInKbps (i) * superFrameDurationInSeconds ) / SatUtils::BITS_PER_BYTE;
          it->second.m_reqPerRc[i].m_vbdcBytes = damaEntry->GetVbdcInBytes (i);

          uint16_t minRbdcCraDeltaRateInKbps = std::max (0, damaEntry->GetMinRbdcInKbps (i) - damaEntry->GetCraInKbps (i));
          it->second.m_reqPerRc[i].m_minRbdcBytes = ( 1000.0 * minRbdcCraDeltaRateInKbps  * superFrameDurationInSeconds ) / SatUtils::BITS_PER_BYTE;
        }

      // decrease persistence values
      damaEntry->DecrementDynamicRatePersistence ();
      damaEntry->DecrementVolumeBacklogPersistence ();
    }
}

void SatBeamScheduler::DoPreResourceAllocation ()
{
  NS_LOG_FUNCTION (this);

  m_totalSlotsLeft = 0;

  if ( m_utInfos.size () > 0 )
    {
      // sort UTs according to C/N0
      m_utSortedInfos.sort (CnoCompare (m_utInfos));

      SatFrameAllocator::SatFrameAllocContainer_t allocReqs;

      for (UtReqInfoContainer_t::iterator it = m_utSortedInfos.begin (); it != m_utSortedInfos.end (); it++)
        {
          allocReqs.push_back (&(it->second));
        }

      m_frameAllocator->AllocateSymbols (allocReqs);
    }
}

void
SatBeamScheduler::UpdateDamaEntriesWithAllocs (SatFrameAllocator::UtAllocInfoContainer_t& utAllocContainer)
{
  NS_LOG_FUNCTION (this);


  for (SatFrameAllocator::UtAllocInfoContainer_t::const_iterator it = utAllocContainer.begin (); it != utAllocContainer.end (); it ++ )
    {
      Ptr<SatDamaEntry> damaEntry = m_utInfos.at (it->first)->GetDamaEntry ();
      double superFrameDurationInSeconds = m_superframeSeq->GetSuperframeConf (m_currentSequence)->GetDuration ().GetSeconds ();

      for (uint32_t i = 0; i < it->second.size (); i++ )
        {
          uint32_t rateBasedBytes = ( 1000.0 * damaEntry->GetCraInKbps (i) * superFrameDurationInSeconds ) / SatUtils::BITS_PER_BYTE;
          rateBasedBytes += ( 1000.0 * damaEntry->GetRbdcInKbps (i) * superFrameDurationInSeconds ) / SatUtils::BITS_PER_BYTE;

          if ( rateBasedBytes < it->second[i] )
            {
              uint32_t vbdcBytes = damaEntry->GetVbdcInBytes (i);

              if ( vbdcBytes > (it->second[i] - rateBasedBytes) )
                {
                  damaEntry->SetVbdcInBytes (i, (it->second[i] - rateBasedBytes));
                }
              else
                {
                  damaEntry->SetVbdcInBytes (i, 0);
                }
            }
        }
    }
}

} // namespace ns3
