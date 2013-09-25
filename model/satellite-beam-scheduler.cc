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
SatBeamScheduler::Initialize (uint32_t beamId, SatBeamScheduler::SendCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_beamId = beamId;
  m_txCallback = cb;

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
  Ptr<Packet> packet = Create<Packet> ();

  // add TBTP tag to message
  SatControlMsgTag tag;
  tag.SetMsgType(SatControlMsgTag::SAT_TBTP_CTRL_MSG);
  packet->AddPacketTag (tag);

  // add TBTP specific header to message
  SatTbtpHeader header;

  // TODO: more realistic scheduling implemented later
  // now just set different some time slot ID for every UTs
  // and frame ID to 0

  uint16_t timeSlotId = 1;

  for ( std::set<Address>::iterator it = m_uts.begin(); it != m_uts.end(); it++ )
    {
      Ptr<SatTbtpHeader::TbtpTimeSlotInfo > timeSlotInfo = Create<SatTbtpHeader::TbtpTimeSlotInfo> (0, timeSlotId++ );
      header.SetTimeslot(Mac48Address::ConvertFrom(*it), timeSlotInfo);
    }

  packet->AddHeader (header);

  Send (packet);

  // re-schedule next TBTP sending (call of this function)
  Simulator::Schedule (Seconds(0.1), &SatBeamScheduler::Schedule, this);
}

} // namespace ns3
