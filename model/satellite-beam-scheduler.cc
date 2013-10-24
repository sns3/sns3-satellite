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

#include "ns3/log.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/random-variable-stream.h"
//#include "satellite-dama-entry.h"

#include "satellite-control-message.h"
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

  m_txCallback(packet, Mac48Address::GetBroadcast(), Ipv4L3Protocol::PROT_NUMBER);

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

  Simulator::Schedule (Seconds(0.1), &SatBeamScheduler::Schedule, this);
}

void
SatBeamScheduler::AddUt (Address utId)
{
  NS_LOG_FUNCTION (this << utId);
  m_uts.insert(utId);
}

void
SatBeamScheduler::Schedule ()
{
  NS_LOG_FUNCTION (this);

  Ptr<Packet> packet = Create<Packet> ();

  // add TBTP tag to message
  SatControlMsgTag tag;
  tag.SetMsgType(SatControlMsgTag::SAT_TBTP_CTRL_MSG);
  packet->AddPacketTag (tag);

  // add TBTP specific header to message
  SatTbtpHeader header;
  header.SetSuperframeCounter( m_superFrameCounter++ );

  // TODO: more realistic scheduling implemented later
  // now just set two random time slot IDs for every UT assuming that there is two slots in every carrier.
  // Used frame ID is always 0.

  if ( m_uts.size () > 0 )
    {
      uint16_t maxTimeSlotId = m_superframeSeq->GetSuperframeConf(0)->GetFrameConf(0)->GetTimeSlotrCount() - 1;

      std::set<uint16_t> reservedTimeSlot;
      std::set<Address>::iterator startIterator;

      if ( m_currentUtAddress.IsInvalid () )
        {
          startIterator = m_uts.begin ();
          m_currentUtAddress = *startIterator;
        }
      else
        {
          startIterator = m_uts.find (m_currentUtAddress);
        }

      bool utsScheduled = false;
      Ptr<UniformRandomVariable> randTimeSlotId = CreateObject<UniformRandomVariable> ();

      // allocate timeslot until there at least two free timeslots left and UTs left
      while ( ( ( reservedTimeSlot.size() + 1 ) <= maxTimeSlotId ) && !utsScheduled )
        {
          uint16_t timeSlotId1;
          uint16_t timeSlotId2;

          std::set<uint16_t>::iterator foundSlot;

          // get free random slot (first in carrier)
          do
          {
            timeSlotId1 = randTimeSlotId->GetInteger(0, maxTimeSlotId);

            if ( timeSlotId1 % 2 )
              {
                timeSlotId1--;
              }

            foundSlot = reservedTimeSlot.find (timeSlotId1);
          } while (foundSlot != reservedTimeSlot.end ());

          // get free random slot (second in carrier)
          do
          {
            timeSlotId2 = randTimeSlotId->GetInteger(0, maxTimeSlotId);

            if ( !(timeSlotId2 % 2) )
              {
                timeSlotId2++;
              }

            foundSlot = reservedTimeSlot.find (timeSlotId2);
          } while (foundSlot != reservedTimeSlot.end ());

          // reserve timeslots
          reservedTimeSlot.insert (timeSlotId1);
          reservedTimeSlot.insert (timeSlotId2);

          // add timeslot to TBTP message header
          Ptr<SatTbtpHeader::TbtpTimeSlotInfo > timeSlotInfo1 = Create<SatTbtpHeader::TbtpTimeSlotInfo> (0, timeSlotId1 );
          header.SetTimeslot(Mac48Address::ConvertFrom(*startIterator), timeSlotInfo1);

          Ptr<SatTbtpHeader::TbtpTimeSlotInfo > timeSlotInfo2 = Create<SatTbtpHeader::TbtpTimeSlotInfo> (0, timeSlotId2 );
          header.SetTimeslot(Mac48Address::ConvertFrom(*startIterator), timeSlotInfo2);

          startIterator++;

          // check if we have reached end of the UT list
          if ( startIterator == m_uts.end() )
            {
              startIterator = m_uts.begin();
            }

          // check if we have reached the UT where we started
          if ( *startIterator == m_currentUtAddress )
            {
              utsScheduled = true;
            }
        }

      packet->AddHeader (header);

      Send (packet);

    }

  // re-schedule next TBTP sending (call of this function)
  Simulator::Schedule (Seconds(0.1), &SatBeamScheduler::Schedule, this);
}

} // namespace ns3
