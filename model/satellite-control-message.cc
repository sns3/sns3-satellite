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
#include "ns3/enum.h"
#include "ns3/address-utils.h"
#include "satellite-enums.h"

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
  m_daTimeSlots.clear ();
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

const SatTbtpMessage::DaTimeSlotInfoContainer_t&
SatTbtpMessage::GetDaTimeslots (Address utId)
{
  NS_LOG_FUNCTION (this << utId);

  DaTimeSlotMap_t::const_iterator it = m_daTimeSlots.find (utId);

  if ( it != m_daTimeSlots.end () )
    {
      return it->second;
    }

  return m_emptyDaSlotContainer;
}

void
SatTbtpMessage::SetDaTimeslot (Mac48Address utId, uint8_t frameId, Ptr<SatTimeSlotConf> conf)
{
  NS_LOG_FUNCTION (this << utId << frameId << conf);

  // find container for the UT from map
  DaTimeSlotMap_t::iterator it = m_daTimeSlots.find (utId);

  // If not found, add new UT container to map,
  // otherwise use container found from map
  if ( it == m_daTimeSlots.end () )
    {
      std::pair<DaTimeSlotMap_t::iterator, bool> result = m_daTimeSlots.insert (std::make_pair (utId, DaTimeSlotInfoContainer_t()));

      if ( result.second )
        {
          it = result.first;
        }
      else
        {
          it = m_daTimeSlots.end ();
        }
    }

  // container creation for UT has failed, so we need to crash
  if (it == m_daTimeSlots.end ())
    {
      NS_FATAL_ERROR ("Cannot insert slot to container!!!");
    }

  // store time slot info to user specific container
  it->second.push_back (std::make_pair (frameId, conf) );

  // store frame ID to count used frames
  m_frameIds.insert (frameId);
}

const SatTbtpMessage::RaChannelInfoContainer_t
SatTbtpMessage::GetRaChannels () const
{
  NS_LOG_FUNCTION (this);

  SatTbtpMessage::RaChannelInfoContainer_t channels;

  for (RaChannelMap_t::const_iterator it = m_raChannels.begin (); it != m_raChannels.end (); it++)
    {
      channels.insert (it->second);
    }

  return channels;
}

void
SatTbtpMessage::SetRaChannel (uint32_t raChannel, uint8_t frameId, uint16_t timeSlotCount)
{
  NS_LOG_FUNCTION (this << raChannel << frameId << timeSlotCount);

  // find index for the RA channel from map
  RaChannelMap_t::iterator it = m_raChannels.find (raChannel);

  // If not found, add RA channel to map,
  // otherwise raise error
  if ( it == m_raChannels.end () )
    {
      std::pair<RaChannelMap_t::iterator, bool> result = m_raChannels.insert (std::make_pair (raChannel, timeSlotCount));

      if ( result.second == false )
        {
          NS_FATAL_ERROR ("RA channel insertion failed!!!");
        }
    }
  else
    {
      NS_FATAL_ERROR ("RA channel already exists in the container!!!");
    }

  // store frame ID to count used frames
  m_frameIds.insert (frameId);
}


uint32_t SatTbtpMessage::GetSizeInBytes () const
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

  // add size of DA time slots
  for (DaTimeSlotMap_t::const_iterator it = m_daTimeSlots.begin (); it != m_daTimeSlots.end (); it++ )
    {
      sizeInBytes += (it->second.size () * assignmentBodySizeInBytes);
    }

  // add size of RA time slots
  for (RaChannelMap_t::const_iterator it = m_raChannels.begin (); it != m_raChannels.end (); it++ )
    {
      sizeInBytes += (it->second * assignmentBodySizeInBytes);
    }

  return sizeInBytes;

}

void SatTbtpMessage::Dump () const
{
  std::cout << "Superframe counter: " << m_superframeCounter <<
      ", superframe sequence id: " << m_superframeSeqId <<
      ", assignment format: " << m_assignmentFormat << std::endl;

  for (DaTimeSlotMap_t::const_iterator mit = m_daTimeSlots.begin ();
      mit != m_daTimeSlots.end ();
      ++mit)
    {
      std::cout << "UT: " << mit->first << ": ";
      for (DaTimeSlotInfoContainer_t::const_iterator sit = mit->second.begin ();
          sit != mit->second.end ();
          ++sit)
        {
          std::cout << sit->second << " ";
        }
      std::cout << std::endl;
    }

}

NS_OBJECT_ENSURE_REGISTERED (SatCrMessage);

TypeId
SatCrMessage::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatCrMessage")
    .SetParent<SatControlMessage> ()
    .AddConstructor<SatCrMessage> ()
    .AddAttribute ("CrBlockType",
                   "Capacity request control block size type",
                   EnumValue (SatCrMessage::CR_BLOCK_SMALL),
                   MakeEnumAccessor (&SatCrMessage::m_crBlockSizeType),
                   MakeEnumChecker (SatCrMessage::CR_BLOCK_SMALL, "Small",
                                    SatCrMessage::CR_BLOCK_LARGE, "Large"))
  ;
  return tid;
}

TypeId
SatCrMessage::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

SatCrMessage::SatCrMessage ()
 :m_crBlockSizeType (SatCrMessage::CR_BLOCK_SMALL),
  m_forwardLinkCNo (NAN),
  m_hasNonZeroContent (false)
{
  NS_LOG_FUNCTION (this);
}

SatCrMessage::~SatCrMessage ()
{
  NS_LOG_FUNCTION (this);
}

void
SatCrMessage::AddControlElement (uint8_t rcIndex, SatEnums::SatCapacityAllocationCategory_t cac, uint32_t value)
{
  NS_LOG_FUNCTION (this << rcIndex << cac << value);

  RequestDescriptor_t p = std::make_pair<uint8_t, SatEnums::SatCapacityAllocationCategory_t> (rcIndex, cac);
  m_requestData.insert (std::make_pair<RequestDescriptor_t, uint32_t> (p, value));

  if (value > 0)
    {
      m_hasNonZeroContent = true;
    }
}


const SatCrMessage::RequestContainer_t
SatCrMessage::GetCapacityRequestContent () const
{
  return m_requestData;
}

uint32_t
SatCrMessage::GetNumCapacityRequestElements () const
{

  return m_requestData.size ();
}

double
SatCrMessage::GetCnoEstimate (void) const
{
  NS_LOG_FUNCTION (this);
  return m_forwardLinkCNo;
}

void
SatCrMessage::SetCnoEstimate (double cno)
{
  NS_LOG_FUNCTION (this << cno);
  m_forwardLinkCNo = cno;
}

uint32_t
SatCrMessage::GetSizeInBytes () const
{
  NS_LOG_FUNCTION (this);

  /**
   * CR_BLOCK_SMALL = 2
   * CR_BLOCK_LARGE = 3
   */
  uint32_t crBlockSizeInBytes = (m_crBlockSizeType == SatCrMessage::CR_BLOCK_SMALL ? 2 : 3);

  uint32_t size (CONTROL_MSG_TYPE_VALUE_SIZE_IN_BYTES +
                 CONTROL_MSG_COMMON_HEADER_SIZE_IN_BYTES +
                 m_requestData.size () * crBlockSizeInBytes);
  return size;
}


bool SatCrMessage::HasNonZeroContent () const
{
  return m_hasNonZeroContent;
}

// Control message container

SatControlMsgContainer::SatControlMsgContainer ()
 : m_id (0),
   m_storeTime (MilliSeconds (300)),
   m_deleteOnRead (false)

{
  NS_LOG_FUNCTION (this);
}

SatControlMsgContainer::SatControlMsgContainer (Time storeTime, bool deleteOnRead)
 : m_id (0),
   m_storeTime (storeTime),
   m_deleteOnRead (deleteOnRead)

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

  Time now = Simulator::Now ();

  CtrlMsgMapValue_t mapValue = std::make_pair (now, ctrlMsg);

  std::pair<CtrlMsgMap_t::iterator, bool> result = m_ctrlMsgs.insert (std::make_pair (m_id, mapValue));

  if ( result.second == false )
    {
      NS_FATAL_ERROR ("Control message can't added.");
    }

  if ( m_storeTimeout.IsExpired ()  )
    {
      m_storeTimeout = Simulator::Schedule (m_storeTime, &SatControlMsgContainer::EraseFirst, this);
    }

  return m_id++;
}

Ptr<SatControlMessage>
SatControlMsgContainer::Get (uint32_t id)
{
  NS_LOG_FUNCTION (this << id);

  Ptr<SatControlMessage> msg = NULL;

  CtrlMsgMap_t::iterator it = m_ctrlMsgs.find (id);

  if ( it != m_ctrlMsgs.end () )
    {
      msg = it->second.second;

      if (m_deleteOnRead)
        {
          if ( it == m_ctrlMsgs.begin () )
            {
              if (m_storeTimeout.IsRunning () )
                {
                  m_storeTimeout.Cancel ();
                }

              EraseFirst ();
            }
          else
            {
              m_ctrlMsgs.erase (it);
            }
        }
    }

  return msg;
}

void
SatControlMsgContainer::EraseFirst ()
{
  NS_LOG_FUNCTION (this);

  CtrlMsgMap_t::iterator first = m_ctrlMsgs.begin ();
  m_ctrlMsgs.erase (first);

  first = m_ctrlMsgs.begin ();

  if ( first != m_ctrlMsgs.end () )
    {
      Time storedMoment = first->second.first;
      Time elapsedTime = Simulator::Now () - storedMoment;

      m_storeTimeout = Simulator::Schedule (m_storeTime - elapsedTime, &SatControlMsgContainer::EraseFirst, this);
    }
}

}; // namespace ns3
