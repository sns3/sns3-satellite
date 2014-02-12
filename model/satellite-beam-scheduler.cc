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
#include "ns3/ipv4-l3-protocol.h"
//#include "ns3/random-variable-stream.h"
//#include "satellite-dama-entry.h"
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
SatBeamScheduler::Send (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  NS_LOG_LOGIC ("p=" << packet );

  m_txCallback (packet, Mac48Address::GetBroadcast (), Ipv4L3Protocol::PROT_NUMBER);

  return true;
}

void
SatBeamScheduler::Initialize (uint32_t beamId, SatBeamScheduler::SendCallback cb, Ptr<SatSuperframeSeq> seq)
{
  NS_LOG_FUNCTION (this << beamId << &cb);
  m_beamId = beamId;
  m_txCallback = cb;
  m_superframeSeq = seq;
  m_superFrameCounter = 0;

  // How many TBTPs is transmitted during RTT?
  uint32_t tbtpsPerRtt = (uint32_t)(ceil (m_rttEstimate.GetSeconds () / m_superframeSeq->GetDurationInSeconds (0)));

  // Scheduling starts after one empty super frame.
  m_superFrameCounter = tbtpsPerRtt + 1;

  NS_LOG_LOGIC ("Initialize SatBeamScheduler at " << Simulator::Now ().GetSeconds ());

  Simulator::Schedule (Seconds (m_superframeSeq->GetDurationInSeconds (0)), &SatBeamScheduler::Schedule, this);
}

void
SatBeamScheduler::AddUt (Address utId, double cra)
{
  NS_LOG_FUNCTION (this << utId);

  UtInfo utInfo;
  utInfo.m_cra = cra;
  utInfo.m_cno = NAN;

  std::pair<std::map<Address, UtInfo>::iterator, bool > result = m_uts.insert (std::make_pair (utId, utInfo));

  NS_ASSERT (result.second == true);
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

  if ( m_uts.size() > 0 )
    {
      // create TBTP  message
      Ptr<SatTbtpMessage> tbtpMsg = CreateObject<SatTbtpMessage> ();
      tbtpMsg->SetSuperframeCounter (m_superFrameCounter++);

      // schedule time slots according to static configuration 0
      // TODO: algorithms for other configurations
      InitializeScheduling ();
      ScheduleUts (tbtpMsg);
      ScheduleRandomSlots (tbtpMsg);

      uint32_t msgId = m_superframeSeq->AddTbtpMessage (m_beamId, tbtpMsg);

      Ptr<Packet> packet = Create<Packet> (tbtpMsg->GetSizeinBytes ());

      // add TBTP tag to message
      SatControlMsgTag tag;
      tag.SetMsgType (SatControlMsgTag::SAT_TBTP_CTRL_MSG);
      tag.SetMsgId (msgId);

      packet->AddPacketTag (tag);

      Send (packet);

      NS_LOG_LOGIC ("TBTP sent at: " << Simulator::Now ().GetSeconds ());
    }

  // re-schedule next TBTP sending (call of this function)
  Simulator::Schedule (Seconds (m_superframeSeq->GetDurationInSeconds (0)), &SatBeamScheduler::Schedule, this);
}

void SatBeamScheduler::ScheduleRandomSlots (Ptr<SatTbtpMessage> header)
{
  NS_LOG_FUNCTION (this);

  Ptr<SatFrameConf> frameConf = NULL;
  uint32_t frameId = 0;

  // find frame for RA entries
  for ( uint32_t i = 0; ( (i <  m_superframeSeq->GetSuperframeConf (0)->GetFrameCount ()) && (frameConf == NULL) ); i++ )
    {
      if ( m_superframeSeq->GetSuperframeConf (0)->GetFrameConf (i)->IsRandomAccess () )
        {
          frameConf = m_superframeSeq->GetSuperframeConf (0)->GetFrameConf (0);
          frameId = i;
        }
   }

  if ( frameConf != NULL )
    {
      SatFrameConf::SatTimeSlotIdList_t timeSlots = frameConf->GetTimeSlotIds (0);

      for (SatFrameConf::SatTimeSlotIdList_t::const_iterator it = timeSlots.begin (); it != timeSlots.end (); it++ )
        {
          Ptr<SatTbtpMessage::TbtpTimeSlotInfo > timeSlotInfo = Create<SatTbtpMessage::TbtpTimeSlotInfo> (frameId, (*it) );
          header->SetTimeslot (Mac48Address::GetBroadcast (), timeSlotInfo);
        }
    }
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
          Ptr<SatTbtpMessage::TbtpTimeSlotInfo > timeSlotInfo = Create<SatTbtpMessage::TbtpTimeSlotInfo> (m_currentFrame, GetNextTimeSlot () );
          header->SetTimeslot (Mac48Address::ConvertFrom (m_currentUt->first), timeSlotInfo);

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
