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
#include <utility>
#include <sstream>
#include <ns3/log.h>
#include <ns3/double.h>
#include <ns3/boolean.h>
#include <ns3/enum.h>
#include <ns3/singleton.h>
#include <ns3/satellite-id-mapper.h>
#include <ns3/satellite-rtn-link-time.h>
#include <ns3/satellite-const-variables.h>
#include <ns3/satellite-frame-symbol-load-probe.h>
#include <ns3/satellite-frame-user-load-probe.h>
#include <ns3/address.h>
#include <ns3/mac48-address.h>
#include <ns3/satellite-superframe-sequence.h>
#include <ns3/satellite-superframe-allocator.h>
#include <ns3/satellite-dama-entry.h>
#include <ns3/satellite-control-message.h>
#include <ns3/satellite-lower-layer-service.h>
#include "satellite-beam-scheduler.h"


NS_LOG_COMPONENT_DEFINE ("SatBeamScheduler");

namespace ns3 {

// UtInfo class declarations for SatBeamScheduler
SatBeamScheduler::SatUtInfo::SatUtInfo ( Ptr<SatDamaEntry> damaEntry, Ptr<SatCnoEstimator> cnoEstimator, Time controlSlotOffset, bool controlSlotsEnabled )
  : m_damaEntry (damaEntry),
    m_cnoEstimator (cnoEstimator),
    m_controlSlotsEnabled (controlSlotsEnabled)
{
  NS_LOG_FUNCTION (this);

  SetControlSlotGenerationTime (controlSlotOffset);
}

Ptr<SatDamaEntry>
SatBeamScheduler::SatUtInfo::GetDamaEntry ()
{
  NS_LOG_FUNCTION (this);

  return m_damaEntry;
}

void
SatBeamScheduler::SatUtInfo::UpdateDamaEntryFromCrs ()
{
  NS_LOG_FUNCTION (this);

  // map to sum up RBDC requests per RC
  std::map<uint8_t, uint16_t> rbdcReqs;

  for ( CrMsgContainer_t::const_iterator crIt = m_crContainer.begin (); crIt != m_crContainer.end (); crIt++ )
    {
      SatCrMessage::RequestContainer_t crContent = (*crIt)->GetCapacityRequestContent ();

      for ( SatCrMessage::RequestContainer_t::const_iterator descriptorIt = crContent.begin (); descriptorIt != crContent.end (); descriptorIt++ )
        {
          switch (descriptorIt->first.second)
            {
            case SatEnums::DA_RBDC:
              {
                rbdcReqs[descriptorIt->first.first] += descriptorIt->second;
                break;
              }

            case SatEnums::DA_VBDC:
              {
                m_damaEntry->ResetVolumeBacklogPersistence ();
                m_damaEntry->UpdateVbdcInBytes (descriptorIt->first.first, descriptorIt->second);
                break;
              }

            case SatEnums::DA_AVBDC:
              {
                m_damaEntry->ResetVolumeBacklogPersistence ();
                m_damaEntry->SetVbdcInBytes (descriptorIt->first.first, descriptorIt->second);
                break;
              }

            default:
              break;
            }
        }
    }

  // update RBDC with summed up requests
  for (std::map<uint8_t, uint16_t>::iterator it = rbdcReqs.begin (); it != rbdcReqs.end (); it++ )
    {
      m_damaEntry->ResetDynamicRatePersistence ();
      m_damaEntry->UpdateRbdcInKbps (it->first, it->second);
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
  NS_LOG_FUNCTION (this << crMsg);

  m_crContainer.push_back (crMsg);
}

bool
SatBeamScheduler::SatUtInfo::IsControlSlotGenerationTime () const
{
  NS_LOG_FUNCTION (this);

  bool isGenerationTime = false;

  if ( m_controlSlotsEnabled && ( m_controlSlotGenerationTime <= Simulator::Now () ) )
    {
      isGenerationTime = true;
    }

  return isGenerationTime;
}

void
SatBeamScheduler::SatUtInfo::SetControlSlotGenerationTime (Time offset)
{
  NS_LOG_FUNCTION (this);

  m_controlSlotGenerationTime =  Simulator::Now () + offset;
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
                   MakeEnumChecker (SatCnoEstimator::LAST, "LastValueInWindow",
                                    SatCnoEstimator::MINIMUM, "MinimumValueInWindow",
                                    SatCnoEstimator::AVERAGE, "AverageValueInWindow"))
    .AddAttribute ( "CnoEstimationWindow",
                    "Time window for C/N0 estimation.",
                    TimeValue (MilliSeconds (1000)),
                    MakeTimeAccessor (&SatBeamScheduler::m_cnoEstimationWindow),
                    MakeTimeChecker ())
    .AddAttribute ( "MaxTwoWayPropagationDelay",
                    "Maximum two way propagation delay between GW and UT.",
                    TimeValue (MilliSeconds (560)),
                    MakeTimeAccessor (&SatBeamScheduler::m_maxTwoWayPropagationDelay),
                    MakeTimeChecker ())
    .AddAttribute ( "MaxTBTPTxAndProcessingDelay",
                    "Maximum TBTP transmission and processing delay at the GW.",
                    TimeValue (MilliSeconds (100)),
                    MakeTimeAccessor (&SatBeamScheduler::m_maxTbtpTxAndProcessingDelay),
                    MakeTimeChecker ())
    .AddAttribute ( "ControlSlotsEnabled",
                    "Control slots generation enabled according to ControlSlotInterval attribute.",
                    BooleanValue (false),
                    MakeBooleanAccessor (&SatBeamScheduler::m_controlSlotsEnabled),
                    MakeBooleanChecker ())
    .AddAttribute ( "ControlSlotInterval",
                    "Time interval to generate time slots for the UT(s).",
                    TimeValue (MilliSeconds (1000)),
                    MakeTimeAccessor (&SatBeamScheduler::m_controlSlotInterval),
                    MakeTimeChecker ())
    .AddTraceSource ("BacklogRequestsTrace",
                     "Trace for backlog requests done to beam scheduler.",
                     MakeTraceSourceAccessor (&SatBeamScheduler::m_backlogRequestsTrace),
                     "ns3::SatBeamScheduler::BacklogRequestsTraceCallback")
    .AddTraceSource ("WaveformTrace",
                     "Trace scheduled wave forms (called once per UT per round).",
                     MakeTraceSourceAccessor (&SatBeamScheduler::m_waveformTrace),
                     "ns3::SatBeamScheduler::WaveformTrace")
    .AddTraceSource ("FrameUtLoadTrace",
                     "Trace UT load per the frame.",
                     MakeTraceSourceAccessor (&SatBeamScheduler::m_frameUtLoadTrace),
                     "ns3::SatFrameUserLoadProbe::FrameUserLoadCallback")
    .AddTraceSource ("FrameLoadTrace",
                     "Trace load per the frame allocated symbols / total symbols.",
                     MakeTraceSourceAccessor (&SatBeamScheduler::m_frameLoadTrace),
                     "ns3::SatFrameSymbolLoadProbe::FrameSymbolLoadCallback")
    .AddTraceSource ("UsableCapacityTrace",
                     "Trace usable capacity per beam in kbps.",
                     MakeTraceSourceAccessor (&SatBeamScheduler::m_usableCapacityTrace),
                     "ns3::SatBeamScheduler::UsableCapacityTraceCallback")
    .AddTraceSource ("UnmetCapacityTrace",
                     "Trace unmet capacity per beam in kbps.",
                     MakeTraceSourceAccessor (&SatBeamScheduler::m_unmetCapacityTrace),
                     "ns3::SatBeamScheduler::UnmetCapacityTrace")
    .AddTraceSource ("ExceedingCapacityTrace",
                     "Trace exceeding capacity per beam in kbps.",
                     MakeTraceSourceAccessor (&SatBeamScheduler::m_exceedingCapacityTrace),
                     "ns3::SatBeamScheduler::ExceedingCapacityTrace")
  ;
  return tid;
}

SatBeamScheduler::SatBeamScheduler ()
  : m_beamId (0),
    m_superframeSeq (0),
    m_superFrameCounter (0),
    m_txCallback (0),
    m_cnoEstimatorMode (SatCnoEstimator::LAST),
    m_maxBbFrameSize (0),
    m_controlSlotsEnabled (false)
{
  NS_LOG_FUNCTION (this);
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

bool
SatBeamScheduler::Send (Ptr<SatControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);
  NS_LOG_INFO ("p=" << msg );

  m_txCallback (msg, Mac48Address::GetBroadcast ());

  return true;
}

void
SatBeamScheduler::Initialize (uint32_t beamId, SatBeamScheduler::SendCtrlMsgCallback cb, Ptr<SatSuperframeSeq> seq, uint32_t maxFrameSizeInBytes)
{
  NS_LOG_FUNCTION (this << beamId << &cb);

  m_beamId = beamId;
  m_txCallback = cb;
  m_superframeSeq = seq;
  m_maxBbFrameSize = maxFrameSizeInBytes;

  /**
   * Calculating to start time for super frame counts to start the scheduling from.
   * The offset is calculated by estimating the maximum delay between GW and UT,
   * so that the sent TBTP will be received by UT in time to be able to still send
   * the packet in time.
   */
  Time totalDelay = m_maxTwoWayPropagationDelay + m_maxTbtpTxAndProcessingDelay;
  uint32_t sfCountOffset = (uint32_t)(totalDelay.GetInteger () / seq->GetDuration (0).GetInteger () + 1);

  // Scheduling starts after one empty super frame.
  m_superFrameCounter = Singleton<SatRtnLinkTime>::Get ()->GetNextSuperFrameCount (SatConstVariables::SUPERFRAME_SEQUENCE) + sfCountOffset;

  /**
   * It is assumed currently, that a random RA channel index is selected
   * for each UT. If there is only one RA possible RA channel, then all
   * UTs shall be using the same channel.
   */
  m_raChRandomIndex = CreateObject<UniformRandomVariable> ();
  m_raChRandomIndex->SetAttribute ("Min", DoubleValue (0));

  // by default we give index 0, even if there is no RA channels configured.
  uint32_t maxIndex = 0;

  if ( m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE)->GetRaChannelCount () > 0 )
    {
      maxIndex = m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE)->GetRaChannelCount () - 1;
    }

  m_raChRandomIndex->SetAttribute ("Max", DoubleValue (maxIndex));
  m_superframeAllocator = CreateObject<SatSuperframeAllocator> (m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE));

  NS_LOG_INFO ("Initialize SatBeamScheduler at " << Simulator::Now ().GetSeconds ());

  Time delay;
  Time txTime = Singleton<SatRtnLinkTime>::Get ()->GetNextSuperFrameStartTime (SatConstVariables::SUPERFRAME_SEQUENCE);

  if (txTime > Now ())
    {
      delay = txTime - Now ();
    }
  else
    {
      NS_FATAL_ERROR ("Trying to schedule a super frame in the past!");
    }

  Simulator::Schedule (delay, &SatBeamScheduler::Schedule, this);
}

uint32_t
SatBeamScheduler::AddUt (Address utId, Ptr<SatLowerLayerServiceConf> llsConf)
{
  NS_LOG_FUNCTION (this << utId);

  Ptr<SatDamaEntry> damaEntry = Create<SatDamaEntry> (llsConf);

  // this method call acts as CAC check, if allocation fails fatal error is occurred.
  m_superframeAllocator->ReserveMinimumRate (damaEntry->GetMinRateBasedBytes (m_superframeAllocator->GetSuperframeDuration ()), m_controlSlotsEnabled);

  Time firstCtrlSlotInterval = m_controlSlotInterval;

  if (m_superframeSeq->GetDuration (SatConstVariables::SUPERFRAME_SEQUENCE) < m_controlSlotInterval  )
    {
      Ptr<UniformRandomVariable> m_randomInterval = CreateObject<UniformRandomVariable> ();
      uint32_t randomOffset = m_randomInterval->GetInteger (m_superframeSeq->GetDuration (SatConstVariables::SUPERFRAME_SEQUENCE).GetInteger (), m_controlSlotInterval.GetInteger () );

      firstCtrlSlotInterval = Time (randomOffset);
    }

  Ptr<SatCnoEstimator> cnoEstimator = CreateCnoEstimator ();
  Ptr<SatUtInfo> utInfo = Create<SatUtInfo> (damaEntry, cnoEstimator, firstCtrlSlotInterval, m_controlSlotsEnabled);

  std::pair<UtInfoMap_t::iterator, bool > result = m_utInfos.insert (std::make_pair (utId, utInfo));

  if (result.second)
    {
      SatFrameAllocator::SatFrameAllocReqItemContainer_t reqContainer (damaEntry->GetRcCount (), SatFrameAllocator::SatFrameAllocReqItem () );
      SatFrameAllocator::SatFrameAllocReq allocReq (reqContainer);
      allocReq.m_cno = NAN;
      allocReq.m_address = utId;

      m_utRequestInfos.push_back (std::make_pair (utId, allocReq));
    }
  else
    {
      NS_FATAL_ERROR ("UT (Address: " << utId << ") already added to Beam scheduler.");
    }

  m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE)->GetRaChannelCount ();

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

  uint32_t requestedKbpsSum (0);
  uint32_t offeredKbpsSum (0);

  // check that there is UTs to schedule
  if ( m_utInfos.size () > 0 )
    {
      requestedKbpsSum = UpdateDamaEntriesWithReqs ();

      DoPreResourceAllocation ();

      // generate time slots
      Ptr<SatTbtpMessage> firstTbtp = CreateObject<SatTbtpMessage> (SatConstVariables::SUPERFRAME_SEQUENCE);
      firstTbtp->SetSuperframeCounter (m_superFrameCounter++);

      std::vector<Ptr<SatTbtpMessage> > tbtps;
      tbtps.push_back (firstTbtp);

      // Add RA slots (channels)
      AddRaChannels (tbtps);

      SatFrameAllocator::UtAllocInfoContainer_t utAllocs;

      // Add DA slots to TBTP(s)
      m_superframeAllocator->GenerateTimeSlots (tbtps, m_maxBbFrameSize, utAllocs, m_waveformTrace, m_frameUtLoadTrace, m_frameLoadTrace);

      // update VBDC counter of the UT/RCs
      offeredKbpsSum += UpdateDamaEntriesWithAllocs (utAllocs);

      // send TBTPs
      for ( std::vector <Ptr<SatTbtpMessage> > ::const_iterator it = tbtps.begin (); it != tbtps.end (); it++ )
        {
          if ( (*it)->GetSizeInBytes () > m_maxBbFrameSize )
            {
              NS_FATAL_ERROR ("Too long TBTP generated by Frame allocator!!!");
            }

          Send (*it);
        }

      NS_LOG_INFO ("TBTP sent at: " << Simulator::Now ().GetSeconds ());
    }

  uint32_t usableCapacity = std::min (offeredKbpsSum, requestedKbpsSum);
  uint32_t unmetCapacity = requestedKbpsSum - usableCapacity;
  uint32_t exceedingCapacity = (uint32_t)(std::max (((double)(offeredKbpsSum) - requestedKbpsSum), 0.0) + 0.5);
  m_usableCapacityTrace (usableCapacity);
  m_unmetCapacityTrace (unmetCapacity);
  m_exceedingCapacityTrace (exceedingCapacity);

  // re-schedule next TBTP sending (call of this function)
  Simulator::Schedule ( m_superframeSeq->GetDuration (SatConstVariables::SUPERFRAME_SEQUENCE), &SatBeamScheduler::Schedule, this);
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

  Ptr<SatSuperframeConf> superFrameConf = m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE);

  int32_t prevFrameId = -1;

  for (uint32_t i = 0; i < superFrameConf->GetRaChannelCount (); i++)
    {
      uint8_t frameId = superFrameConf->GetRaChannelFrameId (i);
      Ptr<SatFrameConf> frameConf = superFrameConf->GetFrameConf (frameId);
      uint16_t timeSlotCount = frameConf->GetTimeSlotCount () / frameConf->GetCarrierCount ();

      // In case of carrier belong to same frame than previous we don't need to check
      // size for frame info when adding slot to TBTP, so it is set to 0.
      uint32_t frameInfoSize = 0;

      if ( prevFrameId != (int32_t) frameId )
        {
          // frame changes, so check size needed for frame info in TBTP too
          frameInfoSize = tbtpToFill->GetFrameInfoSize ();
        }

      if ( timeSlotCount > 0 )
        {
          if ( (tbtpToFill->GetSizeInBytes () + (tbtpToFill->GetTimeSlotInfoSizeInBytes () * timeSlotCount) + frameInfoSize) > m_maxBbFrameSize )
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

uint32_t
SatBeamScheduler::UpdateDamaEntriesWithReqs ()
{
  NS_LOG_FUNCTION (this);

  uint32_t requestedCraRbdcKbps (0);

  for (UtReqInfoContainer_t::iterator it = m_utRequestInfos.begin (); it != m_utRequestInfos.end (); it++)
    {
      // estimation of the C/N0 is done when scheduling UT

      Ptr<SatDamaEntry> damaEntry = m_utInfos.at (it->first)->GetDamaEntry ();

      // process received CRs
      m_utInfos.at (it->first)->UpdateDamaEntryFromCrs ();

      // update allocation request information to be used later to request capacity from frame allocator
      it->second.m_cno = m_utInfos.at (it->first)->GetCnoEstimation ();

      // set control slot generation on or off
      it->second.m_generateCtrlSlot = m_utInfos.at (it->first)->IsControlSlotGenerationTime ();

      for (uint8_t i = 0; i < damaEntry->GetRcCount (); i++ )
        {
          double superFrameDurationInSeconds = m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE)->GetDuration ().GetSeconds ();

          it->second.m_reqPerRc[i].m_craBytes = (SatConstVariables::BITS_IN_KBIT * damaEntry->GetCraInKbps (i) * superFrameDurationInSeconds ) / (double)(SatConstVariables::BITS_PER_BYTE);
          it->second.m_reqPerRc[i].m_rbdcBytes = (SatConstVariables::BITS_IN_KBIT * damaEntry->GetRbdcInKbps (i) * superFrameDurationInSeconds ) / (double)(SatConstVariables::BITS_PER_BYTE);
          it->second.m_reqPerRc[i].m_vbdcBytes = damaEntry->GetVbdcInBytes (i);

          // Collect the requested rate for all UTs per beam
          requestedCraRbdcKbps += damaEntry->GetCraInKbps (i);
          requestedCraRbdcKbps += damaEntry->GetRbdcInKbps (i);

          uint16_t minRbdcCraDeltaRateInKbps = std::max (0, damaEntry->GetMinRbdcInKbps (i) - damaEntry->GetCraInKbps (i));
          it->second.m_reqPerRc[i].m_minRbdcBytes = (SatConstVariables::BITS_IN_KBIT * minRbdcCraDeltaRateInKbps  * superFrameDurationInSeconds ) / (double)(SatConstVariables::BITS_PER_BYTE);

          // if UT is not requesting any RBDC for this RC then set minimum RBDC 0
          // This means that no RBDC is actively requested for this RC
          if (it->second.m_reqPerRc[i].m_rbdcBytes == 0)
            {
              it->second.m_reqPerRc[i].m_minRbdcBytes = 0;
            }

          NS_ASSERT ((it->second.m_reqPerRc[i].m_minRbdcBytes <= it->second.m_reqPerRc[i].m_rbdcBytes));

          //it->second.m_reqPerRc[i].m_rbdcBytes = std::max(it->second.m_reqPerRc[i].m_minRbdcBytes, it->second.m_reqPerRc[i].m_rbdcBytes);

          // write backlog requests traces starts ...
          std::stringstream head;
          head << Now ().GetSeconds () << ", ";
          head << m_beamId << ", ";
          head << Singleton<SatIdMapper>::Get ()->GetUtIdWithMac (it->first) << ", ";

          std::stringstream rbdcTail;
          rbdcTail << SatEnums::DA_RBDC << ", ";
          rbdcTail << damaEntry->GetRbdcInKbps (i);

          m_backlogRequestsTrace ( head.str () + rbdcTail.str () );

          std::stringstream vbdcTail;
          vbdcTail << SatEnums::DA_VBDC << ", ";
          vbdcTail << damaEntry->GetVbdcInBytes (i);

          m_backlogRequestsTrace ( head.str () + vbdcTail.str () );
          // ... write backlog requests traces ends
        }
    }

  return requestedCraRbdcKbps;
}

void SatBeamScheduler::DoPreResourceAllocation ()
{
  NS_LOG_FUNCTION (this);

  if ( m_utInfos.size () > 0 )
    {
      // sort UT requests according to C/N0 of the UTs
      m_utRequestInfos.sort (CnoCompare (m_utInfos));

      SatFrameAllocator::SatFrameAllocContainer_t allocReqs;

      for (UtReqInfoContainer_t::iterator it = m_utRequestInfos.begin (); it != m_utRequestInfos.end (); it++)
        {
          allocReqs.push_back (&(it->second));
        }

      // request capacity for UTs from frame allocator
      m_superframeAllocator->PreAllocateSymbols (allocReqs);
    }
}

uint32_t
SatBeamScheduler::UpdateDamaEntriesWithAllocs (SatFrameAllocator::UtAllocInfoContainer_t& utAllocContainer)
{
  NS_LOG_FUNCTION (this);

  uint32_t offeredCraRbdcKbps (0);

  for (UtReqInfoContainer_t::iterator it = m_utRequestInfos.begin (); it != m_utRequestInfos.end (); it++)
    {
      Ptr<SatDamaEntry> damaEntry = m_utInfos.at (it->first)->GetDamaEntry ();
      SatFrameAllocator::UtAllocInfoContainer_t::const_iterator allocInfo = utAllocContainer.find (it->first);

      if ( allocInfo != utAllocContainer.end ())
        {
          // update time to send next control slot, if control slot is allocated
          if ( allocInfo->second.second )
            {
              m_utInfos.at (allocInfo->first)->SetControlSlotGenerationTime (m_controlSlotInterval);
            }

          double superFrameDurationInSeconds = m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE)->GetDuration ().GetSeconds ();

          for (uint32_t i = 0; i < allocInfo->second.first.size (); i++ )
            {
              uint32_t rateBasedBytes = (SatConstVariables::BITS_IN_KBIT * damaEntry->GetCraInKbps (i) * superFrameDurationInSeconds ) / (double)(SatConstVariables::BITS_PER_BYTE);
              rateBasedBytes += (SatConstVariables::BITS_IN_KBIT * damaEntry->GetRbdcInKbps (i) * superFrameDurationInSeconds ) / (double)(SatConstVariables::BITS_PER_BYTE);

              offeredCraRbdcKbps += (uint32_t)((allocInfo->second.first[i] * (double)(SatConstVariables::BITS_PER_BYTE) / superFrameDurationInSeconds / (double)(SatConstVariables::BITS_IN_KBIT)) + 0.5);

              NS_LOG_INFO ("UT: " << allocInfo->first << " RC index: " << i <<
                            " rate based bytes: " << rateBasedBytes <<
                            " allocated bytes: " << allocInfo->second.first[i]);

              // The scheduler has allocated more than the rate based bytes (CRA+RBDC)
              if ( rateBasedBytes < allocInfo->second.first[i] )
                {
                  // Requested VBDC
                  uint32_t vbdcBytes = damaEntry->GetVbdcInBytes (i);

                  NS_LOG_INFO ("UT: " << allocInfo->first << " RC index: " << i <<" requested VBDC bytes: " << vbdcBytes);

                  // Allocated VBDC for this RC index
                  uint32_t allocVbdcBytes = allocInfo->second.first[i] - rateBasedBytes;

                  // Allocated less than requested
                  if ( vbdcBytes > allocVbdcBytes)
                    {
                      uint32_t remainingVbdcBytes = vbdcBytes - allocVbdcBytes;

                      NS_LOG_INFO ("UT: " << allocInfo->first << " RC index: " << i <<
                                    " VBDC allocation: " << allocVbdcBytes <<
                                    " remaining VBDC bytes: " << remainingVbdcBytes);

                      damaEntry->SetVbdcInBytes (i, remainingVbdcBytes);
                    }
                  // Allocated more or equal to requested bytes
                  else
                    {
                      NS_LOG_INFO ("UT: " << allocInfo->first << " RC index: " << i <<
                                    " VBDC allocation: " << allocVbdcBytes <<
                                    " remaining VBDC bytes: " << 0);

                      damaEntry->SetVbdcInBytes (i, 0);
                    }
                }
            }
        }
      // decrease persistence values
      damaEntry->DecrementDynamicRatePersistence ();
      damaEntry->DecrementVolumeBacklogPersistence ();
    }
  return offeredCraRbdcKbps;
}

} // namespace ns3

