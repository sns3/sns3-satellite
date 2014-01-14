/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd
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
  ;
  return tid;
}

SatBeamScheduler::SatBeamScheduler ()
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

  m_txCallback(packet, Mac48Address::GetBroadcast (), Ipv4L3Protocol::PROT_NUMBER);

  return true;
}

void
SatBeamScheduler::Initialize (uint32_t beamId, SatBeamScheduler::SendCallback cb, Ptr<SatSuperframeSeq> seq)
{
  NS_LOG_FUNCTION (this << &cb);
  m_beamId = beamId;
  m_txCallback = cb;
  m_superframeSeq = seq;
  m_superFrameCounter = 0;

  Ptr<SatFrameConf> frameConf = m_superframeSeq->GetSuperframeConf (0)->GetFrameConf (0);

  for ( uint32_t i = 0; i < frameConf->GetCarrierCount(); i++ )
    {
      m_carrierIds.push_back (i);
    }

  Simulator::Schedule (Seconds (m_superframeSeq->GetDuration_s (0)), &SatBeamScheduler::Schedule, this);
}

void
SatBeamScheduler::AddUt (Address utId, double cra)
{
  NS_LOG_FUNCTION (this << utId);

  UtInfo utInfo;
  utInfo.m_cra = cra;
  utInfo.m_cno = 0.0;

  std::pair<std::map<Address, UtInfo>::iterator, bool > result = m_uts.insert (std::make_pair(utId, utInfo));

  NS_ASSERT (result.second == true);
}

void
SatBeamScheduler::UpdateUtCno (Address utId, double cno)
{
  NS_LOG_FUNCTION (this << utId << cno);

  // check that UT is added to this scheduler.
  std::map<Address, UtInfo>::iterator result = m_uts.find (utId);
  NS_ASSERT (result != m_uts.end());

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
      Ptr<Packet> packet = Create<Packet> ();

      // add TBTP tag to message
      SatControlMsgTag tag;
      tag.SetMsgType (SatControlMsgTag::SAT_TBTP_CTRL_MSG);
      packet->AddPacketTag (tag);

      // add TBTP specific header to message
      SatTbtpHeader header;
      header.SetSuperframeCounter (m_superFrameCounter++);

      // schedule timeslots according to static configuration 0
      // TODO: algorithms for other configurations
      InitializeScheduling ();
      ScheduleUts (header);

      packet->AddHeader (header);
      Send (packet);
    }

  // re-schedule next TBTP sending (call of this function)
  Simulator::Schedule (Seconds (m_superframeSeq->GetDuration_s (0)), &SatBeamScheduler::Schedule, this);
}

void SatBeamScheduler::ScheduleUts (SatTbtpHeader& header)
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
SatBeamScheduler::AddUtTimeSlots (SatTbtpHeader& header)
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
          Ptr<SatTbtpHeader::TbtpTimeSlotInfo > timeSlotInfo = Create<SatTbtpHeader::TbtpTimeSlotInfo> (0, GetNextTimeSlot () );
          header.SetTimeslot (Mac48Address::ConvertFrom (m_currentUt->first), timeSlotInfo);

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

  Ptr<SatFrameConf> frameConf = m_superframeSeq->GetSuperframeConf (0)->GetFrameConf (0);

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

      Ptr<SatFrameConf> frameConf = m_superframeSeq->GetSuperframeConf (0)->GetFrameConf (0);

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
          m_slotsPerUt = m_totalSlotLeft / m_carrierIds.size();
          m_additionalSlots = 0;
        }

      std::random_shuffle (m_carrierIds.begin (), m_carrierIds.end () );

      m_currentSlot = m_timeSlots.end ();
      m_currentCarrier = m_carrierIds.begin ();

      m_timeSlots = frameConf->GetTimeSlotIds (*m_currentCarrier);

      NS_ASSERT (m_timeSlots.size() > 0);

      m_currentSlot = m_timeSlots.begin ();
    }
}

} // namespace ns3
