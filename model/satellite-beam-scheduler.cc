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
#include "ns3/ipv4-l3-protocol.h"
#include "satellite-beam-scheduler.h"

NS_LOG_COMPONENT_DEFINE ("SatBeamScheduler");

namespace ns3 {

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
    m_totalSlotLeft (0),
    m_additionalSlots (0),
    m_slotsPerUt (0)
{
  NS_LOG_FUNCTION (this);
  m_currentUt = m_uts.end ();
  m_firstUt = m_uts.end ();
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
  m_raChRandomIndex->SetAttribute("Min", DoubleValue (0));

  // by default we give index 0, even if there is no RA channels configured.
  uint32_t maxIndex = 0;

  if ( m_superframeSeq->GetSuperframeConf (0)->GetRaChannelCount () > 0 )
    {
      maxIndex = m_superframeSeq->GetSuperframeConf (0)->GetRaChannelCount () - 1;
    }

  m_raChRandomIndex->SetAttribute("Max", DoubleValue (maxIndex));

  NS_LOG_LOGIC ("Initialize SatBeamScheduler at " << Simulator::Now ().GetSeconds ());

  Simulator::Schedule (Seconds (m_superframeSeq->GetDurationInSeconds (0)), &SatBeamScheduler::Schedule, this);
}

uint32_t
SatBeamScheduler::AddUt (Address utId, Ptr<SatLowerLayerServiceConf> llsConf)
{
  NS_LOG_FUNCTION (this << utId);

  UtInfo utInfo;

  Ptr<SatDamaEntry> damaEntry = Create<SatDamaEntry> (llsConf);

  utInfo.m_damaEntry = damaEntry;
  utInfo.m_cno = NAN;

  // TODO: CAC check needed to add

  std::pair<std::map<Address, UtInfo>::iterator, bool > result = m_uts.insert (std::make_pair (utId, utInfo));

  if (result.second == false)
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
  std::map<Address, UtInfo>::iterator result = m_uts.find (utId);
  NS_ASSERT (result != m_uts.end ());

  // TODO: Container for C/N0 values needed, now we just save the latest value.
  m_uts[utId].m_cno = cno;
}

void
SatBeamScheduler::UtCrReceived (Address utId, Ptr<SatCrMessage> crMsg)
{
  NS_LOG_FUNCTION (this << utId << crMsg);

  // check that UT is added to this scheduler.
  std::map<Address, UtInfo>::iterator result = m_uts.find (utId);
  NS_ASSERT (result != m_uts.end ());

  // TODO: Container for C/N0 values needed, now we just save the latest value.
  m_uts[utId].m_crContainer.push_back (crMsg);
}

double
SatBeamScheduler::EstimateUtCno (Address utId)
{
  NS_LOG_FUNCTION (this << utId);

  // TODO: Estimation logic needed to implement. Now we just return the latest value calculated by lower layer (Phy).
  return m_uts[utId].m_cno;
}

void
SatBeamScheduler::Schedule ()
{
  NS_LOG_FUNCTION (this);

  // check that there is UTs to schedule
  if ( m_uts.size() > 0 )
    {
      UpdateDamaEntries ();

      InitializeScheduling ();

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

  // TODO: UT C/N0 estimation utilization missing still
  // EstimateUtCno (m_currentUt->first);

  while (UtsOrSlotsLeft)
    {
      UtsOrSlotsLeft = AddUtTimeSlots (header);

      m_currentUt++;

      // check if we have reached end of the UT list
      if ( m_currentUt == m_uts.end () )
        {
          m_currentUt = m_uts.begin ();
        }

      if ( m_currentUt == m_firstUt )
        {
          UtsOrSlotsLeft = false;
        }
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

  if ( m_totalSlotLeft )
    {
      uint32_t timeSlotForUt = m_slotsPerUt;

      if ( m_additionalSlots > 0 )
        {
          m_additionalSlots--;
          timeSlotForUt++;
        }

      m_totalSlotLeft -= timeSlotForUt;

      while ( timeSlotForUt )
        {
          header->SetDaTimeslot (Mac48Address::ConvertFrom (m_currentUt->first), m_currentFrame, GetNextTimeSlot ());

          timeSlotForUt--;
        }
    }

  return m_totalSlotLeft;
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

  for (UtInfoMap_t::iterator it = m_uts.begin (); it != m_uts.end (); it ++ )
    {
      // estimate C/N0
      EstimateUtCno (it->first);

      // process received CRs
      for ( UtInfo::CrContainer_t::const_iterator crIt = it->second.m_crContainer.begin (); crIt != it->second.m_crContainer.end (); crIt++ )
        {
          // TODO: Update SatDamaEntry (RC_index, CC) when CR message content is implemented
        }
            
      // clear container when CRs processed
      it->second.m_crContainer.clear ();
      
      // TODO: calculate requested bytes per SF for each (RC_index, CC)
    }
}

void SatBeamScheduler::InitializeScheduling ()
{
  NS_LOG_FUNCTION (this);

  m_firstUt = m_uts.end();

  if ( m_uts.size () > 0 )
    {
      if ( m_currentUt == m_uts.end () )
        {
          m_currentUt = m_uts.begin ();
        }

      m_firstUt = m_currentUt;

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

      NS_ASSERT (frameConf);
      m_carrierIds.clear ();

      for ( uint32_t i = 0; i < frameConf->GetCarrierCount (); i++ )
        {
          m_carrierIds.push_back (i);
        }

      m_totalSlotLeft = frameConf->GetTimeSlotCount ();

      NS_ASSERT (m_totalSlotLeft > 0);

      // no full carrier available for every UT
      if ( m_carrierIds.size() < m_uts.size () )
        {
          m_slotsPerUt = m_totalSlotLeft / m_uts.size ();
          m_additionalSlots = m_totalSlotLeft %  m_uts.size ();   // how many slot stay over
        }
      else
        {
          m_slotsPerUt = m_totalSlotLeft / m_carrierIds.size ();
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

} // namespace ns3
