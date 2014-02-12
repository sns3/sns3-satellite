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

#include <map>
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/address-utils.h"

#include "satellite-control-message.h"

NS_LOG_COMPONENT_DEFINE ("SatCtrlMessage");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatControlMsgTag);


SatControlMsgTag::SatControlMsgTag ()
 :m_msgType (SAT_NON_CTRL_MSG),
  m_msgId (0)
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

  return ( sizeof(m_msgType) + sizeof (m_msgId) );
}

void
SatControlMsgTag::Serialize (TagBuffer i) const
{
  NS_LOG_FUNCTION (this << &i);
  i.WriteU32 ( m_msgType );
  i.WriteU32 ( m_msgId );
}

void
SatControlMsgTag::Deserialize (TagBuffer i)
{
  NS_LOG_FUNCTION (this << &i);
  m_msgType = (SatControlMsgType_t) i.ReadU32 ();
  m_msgId = i.ReadU32 ();
}

void
SatControlMsgTag::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "SatControlMsgType=" << m_msgType << m_msgId;
}

void
SatControlMsgTag::SetMsgId (uint32_t msgId)
{
  NS_LOG_FUNCTION (this << m_msgId);
  m_msgId = msgId;
}

uint32_t
SatControlMsgTag::GetMsgId () const
{
  NS_LOG_FUNCTION (this);

  return m_msgId;
}

// TBTP time slot information

SatTbtpMessage::TbtpTimeSlotInfo::TbtpTimeSlotInfo ()
  : m_frameId(0),
    m_timeSlotId(0)
{
  NS_LOG_FUNCTION (this);
}


SatTbtpMessage::TbtpTimeSlotInfo::TbtpTimeSlotInfo (uint8_t frameId, uint16_t timeSlotId)
  : m_frameId(frameId)
{
  NS_LOG_FUNCTION (this);

  if (timeSlotId > maximumTimeSlotId)
    {
      NS_FATAL_ERROR ("Timeslot ID is out or range!!!");
    }

  m_timeSlotId = timeSlotId;
}

SatTbtpMessage::TbtpTimeSlotInfo::~TbtpTimeSlotInfo()
{
  NS_LOG_FUNCTION (this);
}

void
SatTbtpMessage::TbtpTimeSlotInfo::Print (std::ostream &os)  const
{
  os << "Frame ID= " << m_frameId << ", Time Slot ID= " << m_timeSlotId;
}

uint32_t
SatTbtpMessage::TbtpTimeSlotInfo::GetSerializedSize (void) const
{
  return ( sizeof(m_frameId)  + sizeof(m_timeSlotId) );
}

void
SatTbtpMessage::TbtpTimeSlotInfo::Serialize (Buffer::Iterator start) const
{
   start.WriteU8 (m_frameId);
   start.WriteU16 (m_timeSlotId);
}

uint32_t
SatTbtpMessage::TbtpTimeSlotInfo::Deserialize (Buffer::Iterator start)
{
  m_frameId = start.ReadU8 ();
  m_timeSlotId = start.ReadU16 ();

  return GetSerializedSize();
}

// Control message

NS_OBJECT_ENSURE_REGISTERED (SatControlMessage);

TypeId
SatControlMessage::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatControlMessage")
    .SetParent<Object> ()
  ;
  return tid;
}


// TBTP message

NS_OBJECT_ENSURE_REGISTERED (SatTbtpMessage);

SatTbtpMessage::SatTbtpMessage ( )
  : m_superframeCounter (0),
    m_superframeSeqId (0),
    m_assignmentFormat (0)
{
  NS_LOG_FUNCTION (this);
}

SatTbtpMessage::SatTbtpMessage ( uint8_t seqId )
 : m_superframeCounter (0),
   m_superframeSeqId (seqId),
   m_assignmentFormat (0)
{
  NS_LOG_FUNCTION (this);
}

SatTbtpMessage::~SatTbtpMessage ()
{
  NS_LOG_FUNCTION (this);

  m_frameIds.clear ();
  m_timeSlots.clear ();
}

TypeId
SatTbtpMessage::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatTbtpMessage")
    .SetParent<SatControlMessage> ()
    .AddConstructor<SatTbtpMessage> ()
    .AddAttribute ("AssigmentFormat",
                   "Assignment format of assignment IDs in TBTP.)",
                   UintegerValue (0),
                   MakeUintegerAccessor (&SatTbtpMessage::m_assignmentFormat),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}

TypeId
SatTbtpMessage::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

const SatTbtpMessage::TimeSlotInfoContainer_t&
SatTbtpMessage::GetTimeslots (Address utId)
{
  NS_LOG_FUNCTION (this << utId);

  TimeSlotMap_t::const_iterator it = m_timeSlots.find (utId);

  if ( it != m_timeSlots.end () )
    {
      return it->second;
    }

  return m_emptySlotContainer;
}

void
SatTbtpMessage::SetTimeslot (Mac48Address utId, Ptr<TbtpTimeSlotInfo> info)
{
  NS_LOG_FUNCTION (this << utId << info);

  // find container for the UT from map
  TimeSlotMap_t::iterator it = m_timeSlots.find (utId);

  // If not found, add new UT container to map,
  // otherwise use container found from map
  if ( it == m_timeSlots.end () )
    {
      TimeSlotInfoContainer_t slotInfos;
      std::pair<TimeSlotMap_t::iterator, bool> result = m_timeSlots.insert (std::make_pair (utId, slotInfos));

      if ( result.second )
        {
          it = result.first;
        }
      else
        {
          it = m_timeSlots.end ();
        }
    }

  // container creation for UT has failed, so we need to crash
  if (it == m_timeSlots.end ())
    {
      NS_FATAL_ERROR ("Cannot insert slot to container!!!");
    }

  // store time slot info to user specific container
  it->second.push_back (info);

  // store frame ID to count used frames
  m_frameIds.insert (info->GetFrameId ());
}

uint32_t SatTbtpMessage::GetSizeinBytes ()
{
  NS_LOG_FUNCTION (this);

  // see definition for TBTP2 from specification ETSI EN 301 545-2 (V1.1.1), chapter 6.4.9

  uint32_t sizeInBytes = m_tbtpBodySizeInBytes + ( m_frameIds.size () * m_tbtpFrameBodySizeInBytes );
  uint32_t assignmentBodySizeInBytes = 0;

  switch (m_assignmentFormat)
  {
    case 0:
      // assignment id 48 bits
      assignmentBodySizeInBytes = 6;
      break;

    case 1:
      // assignment id 8 bits
      assignmentBodySizeInBytes = 1;
      break;

    case 2:
      // assignment id 16 bits
      assignmentBodySizeInBytes = 2;
      break;

    case 3:
      // assignment id 24 bits
      assignmentBodySizeInBytes = 3;
      break;

    case 10:
      // dynamic tx type 8 bits + assignment id 8 bits
      assignmentBodySizeInBytes = 2;
      break;

    case 11:
      // dynamic tx type 8 bits + assignment id 16 bits
      assignmentBodySizeInBytes = 3;
      break;

    case 12:
      // dynamic tx type 8 bits + assignment id 24 bits
      assignmentBodySizeInBytes = 4;
      break;

    default:
      NS_FATAL_ERROR ("Assignment format=" << m_assignmentFormat << " not supported!!!" );
      break;
  }

  sizeInBytes += (m_timeSlots.size () * assignmentBodySizeInBytes);

  return sizeInBytes;

}

void SatTbtpMessage::Dump () const
{
  std::cout << "Superframe counter: " << m_superframeCounter <<
      ", superframe sequence id: " << m_superframeSeqId <<
      ", assignment format: " << m_assignmentFormat << std::endl;

  for (TimeSlotMap_t::const_iterator mit = m_timeSlots.begin ();
      mit != m_timeSlots.end ();
      ++mit)
    {
      std::cout << "UT: " << mit->first << ": ";
      for (TimeSlotInfoContainer_t::const_iterator sit = mit->second.begin ();
          sit != mit->second.end ();
          ++sit)
        {
          std::cout << (*sit)->GetTimeSlotId () << " ";
        }
      std::cout << std::endl;
    }

}

// TBTP message container

SatControlMsgContainer::SatControlMsgContainer ()
 :m_id (0),
  m_maxMsgCount (50)
{
  NS_LOG_FUNCTION (this);
}

SatControlMsgContainer::~SatControlMsgContainer ()
{
  NS_LOG_FUNCTION (this);
}


uint32_t
SatControlMsgContainer::Add (Ptr<SatControlMessage> ctrlMsg)
{
  NS_LOG_FUNCTION (this << ctrlMsg);

  // If limit to store msgs are reached, remove first msg from map before adding the new one
  if ( m_ctrlMsgs.size () >= m_maxMsgCount )
    {
      m_ctrlMsgs.erase (m_ctrlMsgs.begin ()->first );
    }

  std::pair<CtrlMsgMap_t::iterator, bool> result = m_ctrlMsgs.insert (std::make_pair (m_id, ctrlMsg));

  if ( result.second == false )
    {
      NS_FATAL_ERROR ("Control message can't added.");
    }

  return m_id++;
}

Ptr<SatControlMessage>
SatControlMsgContainer::Get (uint32_t id) const
{
  NS_LOG_FUNCTION (this << id);

  Ptr<SatControlMessage> msg = NULL;

  CtrlMsgMap_t::const_iterator it = m_ctrlMsgs.find (id);

  if ( it != m_ctrlMsgs.end () )
    {
      msg = it->second;
    }

  return msg;
}

void SatControlMsgContainer::SetMaxMsgCount (uint32_t maxMsgCount)
{
  NS_LOG_FUNCTION (this << maxMsgCount);

  m_maxMsgCount = maxMsgCount;
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
  NS_LOG_FUNCTION (this);

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
