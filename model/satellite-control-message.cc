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

#include <map>
#include "ns3/log.h"
#include "ns3/address-utils.h"

#include "satellite-control-message.h"

NS_LOG_COMPONENT_DEFINE ("SatCtrlHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatControlMsgTag);


SatControlMsgTag::SatControlMsgTag ()
 :m_msgType (SAT_NON_CTRL_MSG)
{
  NS_LOG_FUNCTION (this);
}

SatControlMsgTag::~SatControlMsgTag ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SatControlMsgTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatControlMsgTag")
    .SetParent<Tag> ()
    .AddConstructor<SatControlMsgTag> ()
  ;
  return tid;
}
TypeId
SatControlMsgTag::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

void
SatControlMsgTag::SetMsgType (SatControlMsgType_t type)
{
  NS_LOG_FUNCTION (this << type);
  m_msgType = type;
}

SatControlMsgTag::SatControlMsgType_t
SatControlMsgTag::GetMsgType (void) const
{
  NS_LOG_FUNCTION (this);
  return m_msgType;
}

uint32_t
SatControlMsgTag::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);

  return ( sizeof(m_msgType) );
}

void
SatControlMsgTag::Serialize (TagBuffer i) const
{
  NS_LOG_FUNCTION (this << &i);
  i.WriteU32 ( m_msgType );
}

void
SatControlMsgTag::Deserialize (TagBuffer i)
{
  NS_LOG_FUNCTION (this << &i);
  m_msgType = (SatControlMsgType_t) i.ReadU32();
}

void
SatControlMsgTag::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "SatontrolMsgType=" << m_msgType;
}

// TBTP time slot information

SatTbtpHeader::TbtpTimeSlotInfo::TbtpTimeSlotInfo ()
  : m_frameId(0),
    m_timeSlotId(0)
{
  NS_LOG_FUNCTION (this);
}


SatTbtpHeader::TbtpTimeSlotInfo::TbtpTimeSlotInfo (uint8_t frameId, uint16_t timeSlotId)
  : m_frameId(frameId)
{
  NS_LOG_FUNCTION (this);

  if (timeSlotId > maximumTimeSlotId)
    {
      NS_FATAL_ERROR ("Timeslot ID is out or range!!!");
    }

  m_timeSlotId = timeSlotId;
}

SatTbtpHeader::TbtpTimeSlotInfo::~TbtpTimeSlotInfo()
{
  NS_LOG_FUNCTION (this);
}

void
SatTbtpHeader::TbtpTimeSlotInfo::Print (std::ostream &os)  const
{
  os << "Frame ID= " << m_frameId << ", Time Slot ID= " << m_timeSlotId;
}

uint32_t
SatTbtpHeader::TbtpTimeSlotInfo::GetSerializedSize (void) const
{
  return ( sizeof(m_frameId)  + sizeof(m_timeSlotId) );
}

void
SatTbtpHeader::TbtpTimeSlotInfo::Serialize (Buffer::Iterator start) const
{
   start.WriteU8 (m_frameId);
   start.WriteU16 (m_timeSlotId);
}

uint32_t
SatTbtpHeader::TbtpTimeSlotInfo::Deserialize (Buffer::Iterator start)
{
  m_frameId = start.ReadU8 ();
  m_timeSlotId = start.ReadU16 ();

  return GetSerializedSize();
}

// TBTP message header

NS_OBJECT_ENSURE_REGISTERED (SatTbtpHeader);

SatTbtpHeader::SatTbtpHeader ( )
  : m_superframeSeqId (0)
{
  NS_LOG_FUNCTION (this);
}

SatTbtpHeader::SatTbtpHeader ( uint8_t seqId )
 : m_superframeSeqId (seqId)
{
  NS_LOG_FUNCTION (this);
}

SatTbtpHeader::~SatTbtpHeader ()
{
  NS_LOG_FUNCTION (this);

  m_timeSlots.clear();
}

TypeId
SatTbtpHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatTbtpHeader")
    .SetParent<Header> ()
    .AddConstructor<SatTbtpHeader> ()
  ;
  return tid;
}

TypeId
SatTbtpHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

std::vector< Ptr<SatTbtpHeader::TbtpTimeSlotInfo> >
SatTbtpHeader::GetTimeslots (Address utId)
{
  std::pair <TimeSlotMap_t::iterator, TimeSlotMap_t::iterator> timeSlotRange;

  timeSlotRange = m_timeSlots.equal_range (utId);
  std::vector<Ptr<TbtpTimeSlotInfo> > timeSlots;

  for (TimeSlotMap_t::iterator it = timeSlotRange.first; it != timeSlotRange.second; it++)
    {
      timeSlots.push_back ( it->second );
    }

  return timeSlots;
}

void
SatTbtpHeader::SetTimeslot (Mac48Address utId, Ptr<TbtpTimeSlotInfo> info)
{
  m_timeSlots.insert (std::make_pair(utId, info));
}

void SatTbtpHeader::Print (std::ostream &os)  const
{
  os << "TBTP header, Super Frame Seq ID= " << m_superframeSeqId;
}

uint32_t SatTbtpHeader::GetSerializedSize (void) const
{
  uint32_t timeSlotSerializedSize = Address (Mac48Address ()).GetLength () * sizeof(uint8_t);
  timeSlotSerializedSize += TbtpTimeSlotInfo().GetSerializedSize();

  // time slot map items (address + time slot info) + number of items in map
  return ( ( m_timeSlots.size() * timeSlotSerializedSize ) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint8_t));
}

void
SatTbtpHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteU32 ( m_superframeCounter );
  start.WriteU8 ( m_superframeSeqId );

  // write number of time slot info
  start.WriteU32 ( m_timeSlots.size() );

  // write time slots
  for (TimeSlotMap_t::const_iterator it = m_timeSlots.begin(); it != m_timeSlots.end(); it++)
    {
      WriteTo (start, Mac48Address::ConvertFrom(it->first));

      it->second->Serialize(start);
      start.Next( it->second->GetSerializedSize() );
    }
}

uint32_t
SatTbtpHeader::Deserialize (Buffer::Iterator start)
{
  m_superframeCounter = start.ReadU32 ();
  m_superframeSeqId = start.ReadU8 ();

  uint32_t count = start.ReadU32();

  while (count)
    {
      Mac48Address address;
      ReadFrom (start, address);

      Ptr<TbtpTimeSlotInfo> timeSlotInfo = Create<SatTbtpHeader::TbtpTimeSlotInfo> ();
      timeSlotInfo->Deserialize (start);
      start.Next( timeSlotInfo->GetSerializedSize() );

      m_timeSlots.insert (std::make_pair (address, timeSlotInfo) );
      count--;
    }

  return GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED (SatCapacityReqHeader);

TypeId
SatCapacityReqHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatCapacityReqHeader")
    .SetParent<Tag> ()
    .AddConstructor<SatCapacityReqHeader> ()
  ;
  return tid;
}

TypeId
SatCapacityReqHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

SatCapacityReqHeader::SatCapacityReqHeader ()
{
  NS_LOG_FUNCTION (this);
}

SatCapacityReqHeader::~SatCapacityReqHeader ()
{
  NS_LOG_FUNCTION (this);
}

void
SatCapacityReqHeader::SetReqType (SatCrRequestType_t type)
{
  NS_LOG_FUNCTION (this << type);
  m_reqType = type;
}

double
SatCapacityReqHeader::GetRequestedRate (void) const
{
  NS_LOG_FUNCTION (this);
  return m_requestedRate;
}

void
SatCapacityReqHeader::SetRequestedRate (double rate)
{
  NS_LOG_FUNCTION (this << rate);
  m_requestedRate = rate;
}

double
SatCapacityReqHeader::GetCnoEstimate (void) const
{
  NS_LOG_FUNCTION (this);
  return m_cno;
}

void
SatCapacityReqHeader::SetCnoEstimate (double cno)
{
  NS_LOG_FUNCTION (this << cno);
  m_cno = cno;
}

SatCapacityReqHeader::SatCrRequestType_t
SatCapacityReqHeader::GetReqType (void) const
{
  NS_LOG_FUNCTION (this);
  return m_reqType;
}

void SatCapacityReqHeader::Print (std::ostream &os)  const
{
  os << "M Type= CR";
}

uint32_t SatCapacityReqHeader::GetSerializedSize (void) const
{
 return ( sizeof (m_reqType) + sizeof (m_requestedRate) + sizeof (m_cno) );
}

void SatCapacityReqHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteU32 (m_reqType);
  start.Write ((uint8_t const*) &m_requestedRate, sizeof (m_requestedRate));
  start.Write ((uint8_t const*) &m_cno, sizeof (m_cno));
}

uint32_t SatCapacityReqHeader::Deserialize (Buffer::Iterator start)
{
  m_reqType = (SatCrRequestType_t) start.ReadU32();
  start.Read ((uint8_t *) &m_requestedRate, sizeof (m_requestedRate));
  start.Read ((uint8_t *) &m_cno, sizeof (m_cno));

  return GetSerializedSize();
}

}; // namespace ns3
