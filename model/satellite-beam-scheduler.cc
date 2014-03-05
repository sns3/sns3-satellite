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
#include "satellite-beam-scheduler.h"

NS_LOG_COMPONENT_DEFINE ("SatBeamScheduler");

namespace ns3 {

bool SatBeamScheduler::CompareCno (const UtInfoItem_t &first, const UtInfoItem_t &second)
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
}

// UtInfo class declarations for SatBeamScheduler
SatBeamScheduler::SatUtInfo::SatUtInfo ( Ptr<SatDamaEntry> damaEntry, Ptr<SatCnoEstimator> cnoEstimator )
 : m_damaEntry (damaEntry),
   m_cnoEstimator (cnoEstimator),
   m_isAllocated (false),
   m_frameId (0),
   m_waveformId (0)
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
SatBeamScheduler::SatUtInfo::SetAllocated (uint8_t frameId, uint32_t waveformId)
{
  NS_LOG_FUNCTION (this << frameId << waveformId) ;

  m_frameId = frameId;
  m_waveformId = waveformId;
  m_isAllocated = true;
}

void
SatBeamScheduler::SatUtInfo::SetDeallocated ()
{
  NS_LOG_FUNCTION (this) ;

  m_isAllocated = false;
}

bool
SatBeamScheduler::SatUtInfo::IsAllocated () const
{
  NS_LOG_FUNCTION (this) ;

  return m_isAllocated;
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
              m_damaEntry->UpdateDynamicRateInKbps (descriptorIt->first.first, descriptorIt->second);
              break;

            case SatEnums::DA_VBDC:
              m_damaEntry->UpdateVolumeBacklogInBytes (descriptorIt->first.first, descriptorIt->second);
              break;

            case SatEnums::DA_AVBDC:
              m_damaEntry->SetVolumeBacklogInBytes (descriptorIt->first.first, descriptorIt->second);
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
    .AddAttribute( "RttEstimate",
                   "Round trip time estimate for NCC scheduling",
                   TimeValue (MilliSeconds (560)),
                   MakeTimeAccessor (&SatBeamScheduler::m_rttEstimate),
                   MakeTimeChecker ())
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
    m_craBasedBytes (0),
    m_rbdcBasedBytes (0),
    m_vbdcBasedBytes (0),
    m_cnoEstimatorMode (SatCnoEstimator::LAST)
{
  NS_LOG_FUNCTION (this);

  m_currentUt = m_utSortedInfos.end ();
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
SatBeamScheduler::Initialize (uint32_t beamId, SatBeamScheduler::SendCtrlMsgCallback cb, Ptr<SatSuperframeSeq> seq)
{
  NS_LOG_FUNCTION (this << beamId << &cb);
  m_beamId = beamId;
  m_txCallback = cb;
  m_superframeSeq = seq;
  m_superFrameCounter = 0;

  // How many TBTPs is transmitted during RTT?
  uint32_t tbtpsPerRtt = (uint32_t)(std::ceil (m_rttEstimate.GetSeconds () / m_superframeSeq->GetDurationInSeconds (0)));

  // Scheduling starts after one empty super frame.
  m_superFrameCounter = tbtpsPerRtt + 1;

  // TODO: If RA channel is wanted to allocate to UT with some other means than randomizing
  // this part of implementation is needed to change
  m_raChRandomIndex = CreateObject<UniformRandomVariable> ();
  m_raChRandomIndex->SetAttribute ("Min", DoubleValue (0));

  // by default we give index 0, even if there is no RA channels configured.
  uint32_t maxIndex = 0;

  if ( m_superframeSeq->GetSuperframeConf (0)->GetRaChannelCount () > 0 )
    {
      maxIndex = m_superframeSeq->GetSuperframeConf (0)->GetRaChannelCount () - 1;
    }

  m_raChRandomIndex->SetAttribute("Max", DoubleValue (maxIndex));
  m_frameHelper = Create<SatFrameHelper> (m_superframeSeq->GetSuperframeConf (0), m_superframeSeq->GetWaveformConf () );

  NS_LOG_LOGIC ("Initialize SatBeamScheduler at " << Simulator::Now ().GetSeconds ());

  Simulator::Schedule (Seconds (m_superframeSeq->GetDurationInSeconds (0)), &SatBeamScheduler::Schedule, this);
}

uint32_t
SatBeamScheduler::AddUt (Address utId, Ptr<SatLowerLayerServiceConf> llsConf)
{
  NS_LOG_FUNCTION (this << utId);

  Ptr<SatDamaEntry> damaEntry = Create<SatDamaEntry> (llsConf);
  Ptr<SatCnoEstimator> cnoEstimator = CreateCnoEstimator ();

  Ptr<SatUtInfo> utInfo = Create<SatUtInfo> (damaEntry, cnoEstimator);

  // TODO: CAC check needed to add

  std::pair<UtInfoMap_t::iterator, bool > result = m_utInfos.insert (std::make_pair (utId, utInfo));

  if (result.second)
    {
      m_utSortedInfos.push_back (std::make_pair (utId, utInfo));
    }
  else
    {
      NS_FATAL_ERROR ("UT (Address: " << utId << ") already added to Beam scheduler.");
    }

  m_superframeSeq->GetSuperframeConf (0)->GetRaChannelCount ();

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
      UpdateDamaEntries ();

      DoPreResourceAllocation ();

      // create TBTP  message
      Ptr<SatTbtpMessage> tbtpMsg = CreateObject<SatTbtpMessage> ();
      tbtpMsg->SetSuperframeCounter (m_superFrameCounter++);

      // schedule time slots according to static configuration 0
      // TODO: algorithms for other configurations
      ScheduleUts (tbtpMsg);
      AddRaChannels (tbtpMsg);

      Send (tbtpMsg);

      NS_LOG_LOGIC ("TBTP sent at: " << Simulator::Now ().GetSeconds ());
    }

  // re-schedule next TBTP sending (call of this function)
  Simulator::Schedule (Seconds (m_superframeSeq->GetDurationInSeconds (0)), &SatBeamScheduler::Schedule, this);
}

void SatBeamScheduler::ScheduleUts (Ptr<SatTbtpMessage> header)
{
  NS_LOG_FUNCTION (this);

  bool UtsOrSlotsLeft = true;

  while ( (UtsOrSlotsLeft) && m_currentUt != m_utSortedInfos.end () )
    {
      UtsOrSlotsLeft = AddUtTimeSlots (header);

      m_currentUt++;
    }
}

void
SatBeamScheduler::AddRaChannels (Ptr<SatTbtpMessage> header)
{
  Ptr<SatSuperframeConf> superFrameConf = m_superframeSeq->GetSuperframeConf (0);

  for (uint32_t i = 0; i < superFrameConf->GetRaChannelCount (); i++)
    {
      uint8_t frameId = superFrameConf->GetRaChannelFrameId (i);
      Ptr<SatFrameConf> frameConf = superFrameConf->GetFrameConf (frameId);
      uint16_t timeSlotCount = frameConf->GetTimeSlotCount() / frameConf->GetCarrierCount();

      header->SetRaChannel (i, superFrameConf->GetRaChannelFrameId (i), timeSlotCount);
    }
}

uint32_t
SatBeamScheduler::AddUtTimeSlots (Ptr<SatTbtpMessage> header)
{
  NS_LOG_FUNCTION (this);

  if ( m_totalSlotsLeft )
    {
      uint32_t timeSlotForUt = m_slotsPerUt;

      if ( m_additionalSlots > 0 )
        {
          m_additionalSlots--;
          timeSlotForUt++;
        }

      m_totalSlotsLeft -= timeSlotForUt;

      while ( timeSlotForUt )
        {
          header->SetDaTimeslot (Mac48Address::ConvertFrom (m_currentUt->first), m_currentFrame, GetNextTimeSlot ());

          timeSlotForUt--;
        }
    }

  return m_totalSlotsLeft;
}

uint16_t
SatBeamScheduler::GetNextTimeSlot ()
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_currentSlot != m_timeSlots.end ());
  NS_ASSERT (m_currentCarrier != m_carrierIds.end ());

  Ptr<SatFrameConf> frameConf = m_superframeSeq->GetSuperframeConf (0)->GetFrameConf (m_currentFrame);

  uint16_t timeSlotId = *m_currentSlot;

  m_currentSlot++;

  if ( m_currentSlot == m_timeSlots.end () )
    {
      m_currentCarrier++;

      if ( m_currentCarrier != m_carrierIds.end () )
        {
          m_timeSlots = frameConf->GetTimeSlotIds (*m_currentCarrier);

          if ( m_timeSlots.size() > 0 )
            {
              m_currentSlot = m_timeSlots.begin ();
            }
          else
            {
              m_currentSlot = m_timeSlots.end ();
            }
        }
    }

  return timeSlotId;
}

void
SatBeamScheduler::UpdateDamaEntries ()
{
  NS_LOG_FUNCTION (this);

  // reset requested bytes per SF for each (RC_index, CC)
  m_craBasedBytes = 0;
  m_rbdcBasedBytes = 0;
  m_vbdcBasedBytes = 0;

  for (UtInfoMap_t::iterator it = m_utInfos.begin (); it != m_utInfos.end (); it ++ )
    {
      // estimation of the C/N0 is done when scheduling UT

      Ptr<SatDamaEntry> damaEntry = it->second->GetDamaEntry ();

      // process received CRs
      it->second->UpdateDamaEntriesFromCrs ();

      // calculate (update) requested bytes per SF for each (RC_index, CC)
      m_craBasedBytes += damaEntry->GetCraBasedBytes (m_superframeSeq->GetDurationInSeconds (0));
      m_rbdcBasedBytes += damaEntry->GetRbdcBasedBytes (m_superframeSeq->GetDurationInSeconds (0));
      m_vbdcBasedBytes += damaEntry->GetVbdcBasedBytes ();
      
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
      std::sort (m_utSortedInfos.begin (), m_utSortedInfos.end (), CompareCno);

      m_currentUt = m_utSortedInfos.begin ();

      for (UtSortedInfoContainer_t::iterator it = m_utSortedInfos.begin (); it != m_utSortedInfos.end (); it++)
        {
          it->second->SetDeallocated ();

          Ptr<SatDamaEntry> damaEntry = it->second->GetDamaEntry ();
          double frameDurationInSeconds = m_superframeSeq->GetSuperframeConf (0)->GetDurationInSeconds ();

          SatFrameHelper::SatFrameAllocReq  allocReq ( damaEntry->GetCraBasedBytes (frameDurationInSeconds),
                                                       damaEntry->GetMinRbdcBasedBytes (frameDurationInSeconds),
                                                       damaEntry->GetRbdcBasedBytes (frameDurationInSeconds),
                                                       allocReq.m_vbdcBytes = damaEntry->GetVbdcBasedBytes () );

          SatFrameHelper::SatFrameAllocResp allocResp;

          if (m_frameHelper->AllocateToFrame (it->second->GetCnoEstimation (), allocReq, allocResp) )
            {
              it->second->SetAllocated (allocResp.m_frameId, allocResp.m_waveformId);
            }
        }

      Ptr<SatFrameConf> frameConf = NULL;

      // find frame for DAMA entries
      for ( uint32_t i = 0; ( (i <  m_superframeSeq->GetSuperframeConf (0)->GetFrameCount () ) && (frameConf == NULL) ); i++ )
        {
          if ( m_superframeSeq->GetSuperframeConf (0)->GetFrameConf (i)->IsRandomAccess () == false )
            {
              frameConf = m_superframeSeq->GetSuperframeConf (0)->GetFrameConf (0);
              m_currentFrame = i;
            }
        }

      // If DAMA entry found, initialize scheduling
      if (frameConf)
        {
          m_totalSlotsLeft = frameConf->GetTimeSlotCount ();

          NS_ASSERT (m_totalSlotsLeft);

          m_carrierIds.clear ();

          for ( uint32_t i = 0; i < frameConf->GetCarrierCount (); i++ )
            {
              m_carrierIds.push_back (i);
            }

          // no full carrier available for every UT
          if ( m_carrierIds.size() < m_utInfos.size () )
            {
              m_slotsPerUt = m_totalSlotsLeft / m_utInfos.size ();
              m_additionalSlots = m_totalSlotsLeft %  m_utInfos.size ();   // how many slot stay over
            }
          else
            {
              m_slotsPerUt = m_totalSlotsLeft / m_carrierIds.size ();
              m_additionalSlots = 0;
            }

          std::random_shuffle (m_carrierIds.begin (), m_carrierIds.end ());

          m_currentSlot = m_timeSlots.end ();
          m_currentCarrier = m_carrierIds.begin ();

          m_timeSlots = frameConf->GetTimeSlotIds (*m_currentCarrier);

          NS_ASSERT (m_timeSlots.size () > 0);

          m_currentSlot = m_timeSlots.begin ();
        }
    }
}

} // namespace ns3
