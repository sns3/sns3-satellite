/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Magister Solutions Ltd
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
 * Author: Mathias Ettinger <mettinger@toulouse.viveris.com>
 */

#include <algorithm>
#include <utility>
#include <sstream>

#include <ns3/log.h>
#include <ns3/double.h>
#include <ns3/boolean.h>
#include <ns3/enum.h>
#include <ns3/singleton.h>
#include <ns3/address.h>
#include <ns3/mac48-address.h>
#include <ns3/ipv4-address.h>

#include "satellite-id-mapper.h"
#include "satellite-rtn-link-time.h"
#include "satellite-const-variables.h"
#include "../stats/satellite-frame-symbol-load-probe.h"
#include "../stats/satellite-frame-user-load-probe.h"
#include "satellite-superframe-sequence.h"
#include "satellite-default-superframe-allocator.h"
#include "satellite-superframe-allocator.h"
#include "satellite-dama-entry.h"
#include "satellite-control-message.h"
#include "satellite-lower-layer-service.h"
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

void
SatBeamScheduler::SatUtInfo::ClearCrMsgs ()
{
  NS_LOG_FUNCTION (this);

  m_crContainer.clear ();
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
    .AddAttribute ( "HandoverStrategy",
                    "Strategy used when performing handover to transfer capacity requests and C/No informations",
                    EnumValue (SatBeamScheduler::BASIC),
                    MakeEnumAccessor (&SatBeamScheduler::m_handoverStrategy),
                    MakeEnumChecker (SatBeamScheduler::BASIC, "Basic",
                                     SatBeamScheduler::CHECK_GATEWAY, "CheckGateway"))
    .AddAttribute ("SuperFrameAllocatorType",
                   "Type of SuperFrameAllocator",
                   EnumValue (SatEnums::DEFAULT_SUPERFRAME_ALLOCATOR),
                   MakeEnumAccessor (&SatBeamScheduler::m_superframeAllocatorType),
                   MakeEnumChecker (SatEnums::DEFAULT_SUPERFRAME_ALLOCATOR, "Default"))
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
  m_logonChannelIndex (1),
  m_cnoEstimatorMode (SatCnoEstimator::LAST),
  m_maxBbFrameSize (0),
  m_controlSlotsEnabled (false),
  m_superframeAllocatorType (SatEnums::DEFAULT_SUPERFRAME_ALLOCATOR),
  m_receivedSatelliteCnoSample (false)
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

  m_txCallback (msg, Mac48Address::GetBroadcast ());
  return true;
}

bool
SatBeamScheduler::SendTo (Ptr<SatControlMessage> msg, Address utId)
{
  NS_LOG_FUNCTION (this << msg << utId);

  if (!HasUt (utId))
    {
      return false;
    }

  m_txCallback (msg, utId);
  return true;
}

bool
SatBeamScheduler::SendToSatellite (Ptr<SatControlMessage> msg, Address satelliteMac)
{
  NS_LOG_FUNCTION (this << msg << satelliteMac);

  m_txCallback (msg, satelliteMac);
  return true;
}

void
SatBeamScheduler::SetSendTbtpCallback (SendTbtpCallback cb)
{
  m_txTbtpCallback = cb;
}

void
SatBeamScheduler::Initialize (uint32_t beamId, SatBeamScheduler::SendCtrlMsgCallback cb, Ptr<SatSuperframeSeq> seq, uint32_t maxFrameSizeInBytes, Address gwAddress)
{
  NS_LOG_FUNCTION (this << beamId << &cb);

  m_satelliteCnoEstimator = CreateCnoEstimator ();

  m_beamId = beamId;
  m_txCallback = cb;
  m_superframeSeq = seq;
  m_maxBbFrameSize = maxFrameSizeInBytes;
  m_gwAddress = gwAddress;

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
  m_logonChannelIndex = maxIndex + 1;

  // Create the superframeAllocator object
  switch (m_superframeAllocatorType)
    {
    case SatEnums::DEFAULT_SUPERFRAME_ALLOCATOR:
      {
        Ptr<SatDefaultSuperframeAllocator> superframeAllocator = CreateObject<SatDefaultSuperframeAllocator> (m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE));
        m_superframeAllocator = DynamicCast<SatSuperframeAllocator> (superframeAllocator);
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Invalid SuperframeAllocatorType");
      }
    }

  NS_LOG_INFO ("Initialized SatBeamScheduler");

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

  Time firstCtrlSlotInterval = m_controlSlotInterval;

  if (m_superframeSeq->GetDuration (SatConstVariables::SUPERFRAME_SEQUENCE) < m_controlSlotInterval  )
    {
      Ptr<UniformRandomVariable> m_randomInterval = CreateObject<UniformRandomVariable> ();
      uint32_t randomOffset = m_randomInterval->GetInteger (m_superframeSeq->GetDuration (SatConstVariables::SUPERFRAME_SEQUENCE).GetInteger (), m_controlSlotInterval.GetInteger () );

      firstCtrlSlotInterval = Time (randomOffset);
    }

  Ptr<SatCnoEstimator> cnoEstimator = CreateCnoEstimator ();
  Ptr<SatUtInfo> utInfo = Create<SatUtInfo> (damaEntry, cnoEstimator, firstCtrlSlotInterval, m_controlSlotsEnabled);
  AddUtInfo (utId, utInfo);

  m_superframeSeq->GetSuperframeConf (SatConstVariables::SUPERFRAME_SEQUENCE)->GetRaChannelCount ();

  // return random RA channel index for the UT.
  uint32_t raChannel;
  do
    {
      raChannel = m_raChRandomIndex->GetInteger ();
    }
  while (raChannel == m_logonChannelIndex);

  return raChannel;
}

void
SatBeamScheduler::AddUtInfo (Address utId, Ptr<SatUtInfo> utInfo)
{
  NS_LOG_FUNCTION (this << utId << utInfo);

  Ptr<SatDamaEntry> damaEntry = utInfo->GetDamaEntry ();

  // this method call acts as CAC check, if allocation fails fatal error is occurred.
  m_superframeAllocator->ReserveMinimumRate (damaEntry->GetMinRateBasedBytes (m_superframeAllocator->GetSuperframeDuration ()), m_controlSlotsEnabled);

  std::pair<UtInfoMap_t::iterator, bool > result = m_utInfos.insert (std::make_pair (utId, utInfo));

  if (result.second)
    {
      SatFrameAllocator::SatFrameAllocReqItemContainer_t reqContainer (damaEntry->GetRcCount (), SatFrameAllocator::SatFrameAllocReqItem ());
      SatFrameAllocator::SatFrameAllocReq allocReq (reqContainer);
      allocReq.m_cno = NAN;
      allocReq.m_address = utId;

      m_utRequestInfos.push_back (std::make_pair (utId, allocReq));
    }
  else
    {
      NS_FATAL_ERROR ("UT (Address: " << utId << ") already added to Beam scheduler.");
    }
}

void
SatBeamScheduler::RemoveUtInfo (UtInfoMap_t::iterator iterator)
{
  Address utId = iterator->first;
  Ptr<SatUtInfo> utInfo = iterator->second;
  m_utInfos.erase (iterator);

  UtReqInfoContainer_t::iterator it = m_utRequestInfos.begin ();
  for (; it != m_utRequestInfos.end (); ++it)
    {
      if (it->first == utId)
        {
          m_utRequestInfos.erase (it);
          break;
        }
    }

  Ptr<SatDamaEntry> damaEntry = utInfo->GetDamaEntry ();
  m_superframeAllocator->ReleaseMinimumRate (
    damaEntry->GetMinRateBasedBytes (m_superframeAllocator->GetSuperframeDuration ()),
    m_controlSlotsEnabled);
}

bool
SatBeamScheduler::HasUt (Address utId)
{
  NS_LOG_FUNCTION (this << utId);

  UtInfoMap_t::iterator result = m_utInfos.find (utId);
  return result != m_utInfos.end ();
}

void
SatBeamScheduler::UpdateUtCno (Address utId, double cno)
{
  NS_LOG_FUNCTION (this << utId << cno);

  if (HasUt (utId))
    {
      m_utInfos[utId]->AddCnoSample (cno);
    }
}

void
SatBeamScheduler::UpdateSatelliteCno (Address satelliteMac, double cno)
{
  NS_LOG_FUNCTION (this << satelliteMac << cno);

  m_satelliteCnoEstimator->AddSample (cno);

  m_satelliteMac = satelliteMac;
  m_receivedSatelliteCnoSample = true;
}

void
SatBeamScheduler::UtCrReceived (Address utId, Ptr<SatCrMessage> crMsg)
{
  NS_LOG_FUNCTION (this << utId << crMsg);

  if (HasUt (utId))
    {
      m_utInfos[utId]->AddCrMsg (crMsg);
    }
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
SatBeamScheduler::SendCnoToSatellite ()
{
  if (m_receivedSatelliteCnoSample)
    {
      double cno = m_satelliteCnoEstimator->GetCnoEstimation ();

      Ptr<SatCnoReportMessage> cnoReportMessage = CreateObject<SatCnoReportMessage> ();
      cnoReportMessage->SetCnoEstimate (cno);

      SendToSatellite (cnoReportMessage, m_satelliteMac);
    }

  m_receivedSatelliteCnoSample = false;
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
      firstTbtp->SetSuperframeCounter (m_superFrameCounter);

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
      uint16_t error = 0;
      for ( std::vector <Ptr<SatTbtpMessage> > ::const_iterator it = tbtps.begin (); it != tbtps.end (); it++ )
        {
          if ( (*it)->GetSizeInBytes () > m_maxBbFrameSize )
            {
               ++error;
            }
          else
            {
              m_txTbtpCallback (*it);
              Send (*it);
            }
        }

      if (error) { NS_FATAL_ERROR ("Too big for " << error << " TBTP messages"); }

      NS_LOG_INFO ("TBTP sent");
    }

  uint32_t usableCapacity = std::min (offeredKbpsSum, requestedKbpsSum);
  uint32_t unmetCapacity = requestedKbpsSum - usableCapacity;
  uint32_t exceedingCapacity = (uint32_t)(std::max (((double)(offeredKbpsSum) - requestedKbpsSum), 0.0) + 0.5);
  m_usableCapacityTrace (usableCapacity);
  m_unmetCapacityTrace (unmetCapacity);
  m_exceedingCapacityTrace (exceedingCapacity);
  ++m_superFrameCounter;

  SendCnoToSatellite ();

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

      while (timeSlotCount > 0)
        {
          // Get max nb of time slots in this TBTP
          uint32_t timeSlotCountMaxFrame = (m_maxBbFrameSize - tbtpToFill->GetSizeInBytes () - frameInfoSize) / tbtpToFill->GetTimeSlotInfoSizeInBytes ();
          if (timeSlotCountMaxFrame > timeSlotCount)
            {
              timeSlotCountMaxFrame = timeSlotCount;
            }

          tbtpToFill->SetRaChannel (i, superFrameConf->GetRaChannelFrameId (i), timeSlotCountMaxFrame);
          timeSlotCount -= timeSlotCountMaxFrame;

          // if still room, create new tbtp and do it again
          if (timeSlotCount > 0)
            {
              Ptr<SatTbtpMessage> newTbtp = CreateObject<SatTbtpMessage> (tbtpToFill->GetSuperframeSeqId ());
              newTbtp->SetSuperframeCounter ( tbtpToFill->GetSuperframeCounter ());
              tbtpContainer.push_back (newTbtp);
              tbtpToFill = newTbtp;
            }
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

                  NS_LOG_INFO ("UT: " << allocInfo->first << " RC index: " << i << " requested VBDC bytes: " << vbdcBytes);

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

void
SatBeamScheduler::TransferUtToBeam (Address utId, Ptr<SatBeamScheduler> destination)
{
  NS_LOG_FUNCTION (this << utId << destination->m_beamId);

  UtInfoMap_t::iterator utIterator = m_utInfos.find (utId);
  if (utIterator == m_utInfos.end ())
    {
      // Check if handover already happened
      NS_ASSERT_MSG (destination->HasUt (utId), "UT is not part of the source beam");
    }
  else
    {
      // Moving UT infos between beams
      Ptr<SatUtInfo> utInfo = utIterator->second;
      destination->AddUtInfo (utId, utInfo);
      RemoveUtInfo (utIterator);

      // Handling capacity requests left and C/No estimations
      switch (m_handoverStrategy)
        {
        case BASIC:
          {
            utInfo->ClearCrMsgs ();
            break;
          }
        case CHECK_GATEWAY:
          {
            if (m_gwAddress != destination->m_gwAddress)
              {
                utInfo->ClearCrMsgs ();
              }
            break;
          }
        default:
          NS_FATAL_ERROR ("Unknown handover strategy");
        }
    }
}

void
SatBeamScheduler::RemoveUt (Address utId)
{
  NS_LOG_FUNCTION (this << utId);

  UtInfoMap_t::iterator utIterator = m_utInfos.find (utId);
  if (utIterator == m_utInfos.end ())
    {
      NS_FATAL_ERROR ("Trying to remove a UT not connected to a beam: " << utId);
    }

  // Moving UT infos between beams
  Ptr<SatUtInfo> utInfo = utIterator->second;
  RemoveUtInfo (utIterator);
}

Ptr<SatTimuMessage>
SatBeamScheduler::CreateTimu () const
{
  NS_LOG_FUNCTION (this);

  Ptr<SatTimuMessage> timuMsg = CreateObject<SatTimuMessage> ();
  timuMsg->SetAllocatedBeamId (m_beamId);
  timuMsg->SetGwAddress (m_gwAddress);
  return timuMsg;
}

void
SatBeamScheduler::ReserveLogonChannel (uint32_t logonChannelId)
{
  NS_LOG_FUNCTION (this << logonChannelId);

  DoubleValue maxId;
  m_raChRandomIndex->GetAttribute ("Max", maxId);
  uint32_t maxIndex = maxId.Get();

  if (logonChannelId > maxIndex)
    {
      NS_FATAL_ERROR ("Cannot use channel ID " << logonChannelId << " for logon as it doesn't exist");
    }

  if (maxIndex != 0)
    {
      // Allows to still return the channel 0 as the random access
      // channel for UTs without entering in an infinite loop in AddUt
      m_logonChannelIndex = logonChannelId;
    }
}

} // namespace ns3

