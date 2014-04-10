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

#ifndef SATELLITE_CONTROL_MESSAGE_H
#define SATELLITE_CONTROL_MESSAGE_H

#include <vector>
#include <map>
#include <set>
#include "ns3/header.h"
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"

#include "satellite-mac-tag.h"
#include "satellite-enums.h"
#include "ns3/satellite-frame-conf.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief This class implements a tag that is used to identify control messages (packages)
 */
class SatControlMsgTag : public Tag
{
public:
  /**
   * \brief Definition for different types of control messages
   */
  typedef enum
    {
      SAT_NON_CTRL_MSG,   //!< SAT_NON_CTRL_MSG
      SAT_TBTP_CTRL_MSG,  //!< SAT_TBTP_CTRL_MSG
      SAT_CR_CTRL_MSG,    //!< SAT_CR_CTRL_MSG
      SAT_RA_CTRL_MSG,    //!< SAT_RA_CTRL_MSG
      SAT_ARQ_ACK,        //!< SAT_ARQ_ACK
      SAT_CN0_REPORT      //!< SAT_CN0_REPORT
    } SatControlMsgType_t;

  /**
   * Constructor for SatControlMsgTag
   */
  SatControlMsgTag ();

  /**
   * Destructor for SatControlMsgTag
   */
  ~SatControlMsgTag ();

  /**
   * Set type of the control message. In construction phase initialized
   * to value SAT_UNKNOWN_CTRL_MSG.
   *
   * \param type The type of the control message
   */
  void SetMsgType (SatControlMsgType_t type);

  /**
   * Get type of the contol message.
   *
   * \return The type of the control message
   */
  SatControlMsgType_t GetMsgType (void) const;

  /**
   * Set message type specific identifier.
   *
   * \param msgId Message type specific identifier. May be ignored by some message types.
   */
  virtual void SetMsgId (uint32_t msgId);

  /**
   * Get message type specific identifier.
   *
   * \return Message type specific identifier.
   */
  virtual uint32_t GetMsgId () const;

  // methods derived from base classes
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

private:
  SatControlMsgType_t  m_msgType;
  uint32_t             m_msgId;
};

/**
 * \ingroup satellite
 * \brief Abstract satellite control message class
 */

class SatControlMessage : public Object
{
public:

  // methods derived from base classes
  static TypeId GetTypeId (void);

  /**
   * Default constructor for SatControlMessage.
   */
  SatControlMessage () {};

  /**
   * Destructor for  Set sequence id to 0.
   */
  ~SatControlMessage () {};

  /**
   * Get real size of the control message, which can be used to e.g. simulate real size.
   *
   * \return Real size of the control message.
   */
  virtual uint32_t GetSizeInBytes () const = 0;

  /**
   * Get message specific type.
   *
   * \return Message specific type
   */
  virtual SatControlMsgTag::SatControlMsgType_t GetMsgType () const = 0;

private:

};

/**
 * \ingroup satellite
 * \brief The packet for the Terminal Burst Time Plan (TBTP) messages.
 * (Tagged by SatControlMsgTag with type value SAT_TBTP_CTRL_MSG)
 * NOTE! Message implementation doesn't follow specification (ETSI EN 301 542-2).
 * However it introduced method GetSizeInBytes which can be used when real size is simulated.
 */

class SatTbtpMessage : public SatControlMessage
{
public:
  static const uint16_t maximumTimeSlotId = 2047;

  /**
   * Container for DA time slot information.
   *
   * Stored information is pair, which member first holds frame id
   * and member second holds time slot id.
   */
  typedef std::vector< std::pair<uint8_t, Ptr<SatTimeSlotConf> > >  DaTimeSlotInfoContainer_t;

  /**
   * Container for RA channel information
   *
   * Stored information is index of the RA channel.
   */
  typedef std::set< uint8_t >  RaChannelInfoContainer_t;

  /**
   * Size of message body without frame info and slot assignment info
   *
   * field                  bits
   * ------------------------------------
   * group id               8
   * superframe sequence    8
   * assignment context     8
   * superframe count       8
   * assignment format (AF) 8
   * frame loop count       8
   */
  static const uint32_t m_tbtpBodySizeInBytes = 6;

  /**
   * Size of the frame body
   *
   * field                  bits
   * ------------------------------------
   * frame number           8
   * assignment offset      16
   * assignment loop count  16
   */
  static const uint32_t m_tbtpFrameBodySizeInBytes = 5;

  // methods derived from base classes
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor for SatTbtpHeader. Set sequence id to 0.
   */
  SatTbtpMessage ();

  /**
   * Constructor for SatTbtpHeader to construct TBTP with given sequence id.
   * \param seqId sequence id
   */
  SatTbtpMessage ( uint8_t seqId );

  /**
   * Destructor for SatTbtpHeader
   */
  ~SatTbtpMessage ();

  /**
   * Get type of the message.
   *
   * \return SatControlMsgTag::SAT_TBTP_CTRL_MSG
   */
  inline SatControlMsgTag::SatControlMsgType_t GetMsgType () const { return SatControlMsgTag::SAT_TBTP_CTRL_MSG; }

  /**
   * Set counter of the super frame in this TBTP message.
   *
   * \param counter The super frame counter.
   */
  inline void SetSuperframeCounter (uint32_t counter) { m_superframeCounter = counter; }

  /**
   * Get sequence id of the super frame in this TBTP message.
   *
   * \return The super frame sequence id.
   */
  inline uint8_t GetSuperframeSeqId () {return m_superframeSeqId;}

  /**
   * Get counter of the super frame in this TBTP message.
   *
   * \return The super frame counter.
   */
  inline uint32_t GetSuperframeCounter () {return m_superframeCounter;}

  /**
   * Get the information of the DA time slots.
   *
   * \param utId  id of the UT which time slot information is requested
   * \return vector containing DA time slot info
   */
  const DaTimeSlotInfoContainer_t& GetDaTimeslots (Address utId);

  /**
   * Set a DA time slot information
   *
   * \param utId  id of the UT which time slot information is set
   * \param frameID  Frame ID of the time slot
   * \param timeSlotId Id of the time slot
   */
  void SetDaTimeslot (Mac48Address utId, uint8_t frameId, Ptr<SatTimeSlotConf> conf);

  /**
   * Get the information of the RA channels.
   *
   * \return vector containing RA channels.
   */
  const RaChannelInfoContainer_t GetRaChannels () const;

  /**
   * Set a RA time slot information
   *
   * \param raChannel  raChannel index
   * \param frameID  Frame ID of ra channel
   * \param timeSlotCount Timeslots in channel
   */
  void SetRaChannel (uint32_t raChannel, uint8_t frameId, uint16_t timeSlotCount);

  /**
   * Get real size of the TBTP message, which can be used to e.g. simulate real size.
   *
   * \return Real size of the TBTP message.
   */
  virtual uint32_t GetSizeInBytes () const;

  /**
   * Get size of the time slot in bytes.
   *
   * \return Size of the time slot in bytes.
   */
  uint32_t GetTimeSlotInfoSizeInBytes () const;

  /**
   * Dump all the contents of the TBTP
   */
  void Dump () const;

private:

  typedef std::map <uint8_t, uint16_t >  RaChannelMap_t;
  typedef std::map<Address, DaTimeSlotInfoContainer_t > DaTimeSlotMap_t;

  DaTimeSlotMap_t   m_daTimeSlots;
  RaChannelMap_t    m_raChannels;
  uint32_t          m_superframeCounter;
  uint8_t           m_superframeSeqId;
  uint8_t           m_assignmentFormat;
  std::set<uint8_t> m_frameIds;

  /**
   * Empty DA slot container to be returned if there are not DA time slots
   */
  const DaTimeSlotInfoContainer_t m_emptyDaSlotContainer;
};

/**
 * \ingroup satellite
 * \brief Capacity request message
 * (Tagged by SatControlMsgTag with type value SAT_CR_CTRL_MSG)
 */

class SatCrMessage : public SatControlMessage
{
public:

  typedef enum
  {
    CR_BLOCK_SMALL,
    CR_BLOCK_LARGE
  } SatCrBlockSize_t;

  /**
   * Constructor for SatCrMessage
   */
  SatCrMessage ();

  /**
   * Destructor for SatCrMessage
   */
  ~SatCrMessage ();

  // methods derived from base classes
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  typedef std::pair<uint8_t, SatEnums::SatCapacityAllocationCategory_t> RequestDescriptor_t;
  typedef std::map<RequestDescriptor_t, uint16_t > RequestContainer_t;

  /**
   * Get type of the message.
   *
   * \return SatControlMsgTag::SAT_CR_CTRL_MSG
   */
  inline SatControlMsgTag::SatControlMsgType_t GetMsgType () const { return SatControlMsgTag::SAT_CR_CTRL_MSG; }

  /**
   * Add a control element to capacity request
   */
  void AddControlElement (uint8_t rcIndex, SatEnums::SatCapacityAllocationCategory_t cac, uint32_t value);

  /**
   * Get the capacity request content
   * \return RequestContainer_t Capacity request container
   */
  const RequestContainer_t GetCapacityRequestContent () const;

  /**
   * The number of capacity request elements
   * \return uint32_t Number of CR elements
   */
  uint32_t GetNumCapacityRequestElements () const;

  /**
   * Get C/N0 estimate.
   * \return Estimate of the C/N0.
   */
  double GetCnoEstimate () const;

  /**
   * Set C/N0 estimate.
   * \param cno The estimate of the C/N0.
   */
  void SetCnoEstimate (double cno);

  /**
   * Get real size of the CR message, which can be used to e.g. simulate real size.
   * \return Real size of the CR message.
   */
  virtual uint32_t GetSizeInBytes () const;

  /**
   * Has the CR non-zero content
   * \return bool Flag to indicate whether the CR has non-zero content
   */
  bool HasNonZeroContent () const;

private:

  RequestContainer_t m_requestData;

  /**
   * Control element size is defined by attribute. Note that according to
   * specifications the valid values are
   * - SMALL = 2 bytes
   * - LARGE = 3 bytes
   */
  SatCrBlockSize_t m_crBlockSizeType;

  /**
   * C/N0 estimate.
   */
  double m_forwardLinkCNo;

  /**
   * The CR could hold zero samples.
   */
  bool m_hasNonZeroContent;

  /**
   * Type field of the CR control element
   */
  static const uint32_t CONTROL_MSG_TYPE_VALUE_SIZE_IN_BYTES = 1;

  /**
   * RCST_status + power headroom = 1 Byte
   * CNI = 1 Byte
   * Least margin transmission mode request = 1 Byte
   */
  static const uint32_t CONTROL_MSG_COMMON_HEADER_SIZE_IN_BYTES = 3;

};


/**
 * \ingroup satellite
 * \brief ARQ ACK message
 */

class SatArqAckMessage : public SatControlMessage
{
public:

  /**
   * Constructor for SatCrMessage
   */
  SatArqAckMessage ();

  /**
   * Destructor for SatCrMessage
   */
  ~SatArqAckMessage ();

  // methods derived from base classes
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Get type of the message.
   *
   * \return SatControlMsgTag::SAT_ARQ_ACK
   */
  inline SatControlMsgTag::SatControlMsgType_t GetMsgType () const { return SatControlMsgTag::SAT_ARQ_ACK; }

  /**
   * Set the sequence number to be ACK'ed
   * \param sn Sequence number
   */
  void SetSequenceNumber (uint8_t sn);

  /**
   * Get the sequence number to be ACK'ed
   * \return uint32_t Sequence number
   */
  uint8_t GetSequenceNumber () const;

  /**
   * Set the flow id to be ACK'ed
   * \param sn Sequence number
   */
  void SetFlowId (uint8_t sn);

  /**
   * Get the sequence number to be ACK'ed
   * \return uint32_t Sequence number
   */
  uint8_t GetFlowId () const;

  /**
   * Get real size of the ACK message, which can be used to e.g. simulate real size.
   * \return Real size of the ARQ ACK
   */
  virtual uint32_t GetSizeInBytes () const;

private:

  uint8_t m_sequenceNumber;
  uint8_t m_flowId;
};


/**
 * \ingroup satellite
 * \brief C/N0 (CNI) estimation report message.
 * (Tagged by SatControlMsgTag with type value SAT_CN0_REPORT)
 *
 * This message is sent periodically by UT to GW.
 */

class SatCnoReportMessage : public SatControlMessage
{
public:

  /**
   * Constructor for SatCnoReportMessage
   */
  SatCnoReportMessage ();

  /**
   * Destructor for SatCnoReportMessage
   */
  ~SatCnoReportMessage ();

  // methods derived from base classes
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Get type of the message.
   *
   * \return SatControlMsgTag::SAT_CN0_REPORT
   */
  inline SatControlMsgTag::SatControlMsgType_t GetMsgType () const { return SatControlMsgTag::SAT_CN0_REPORT; }

  /**
   * Get C/N0 estimate.
   * \return Estimate of the C/N0.
   */
  double GetCnoEstimate () const;

  /**
   * Set C/N0 estimate.
   * \param cno The estimate of the C/N0.
   */
  void SetCnoEstimate (double cno);

  /**
   * Get real size of the CR message, which can be used to e.g. simulate real size.
   * \return Real size of the CR message.
   */
  virtual uint32_t GetSizeInBytes () const;

private:

  /**
   * C/N0 estimate.
   */
  double m_forwardLinkCNo;
};

/**
 * \ingroup satellite
 * \brief Random access load control message
 * (Tagged by SatControlMsgTag with type value SAT_RA_CTRL_MSG)
 */

class SatRaMessage : public SatControlMessage
{
public:

  /**
   * Constructor for SatRaMessage
   */
  SatRaMessage ();

  /**
   * Destructor for SatRaMessage
   */
  ~SatRaMessage ();

  // methods derived from base classes
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Get type of the message.
   *
   * \return SatControlMsgTag::SAT_RA_CTRL_MSG
   */
  inline SatControlMsgTag::SatControlMsgType_t GetMsgType () const { return SatControlMsgTag::SAT_RA_CTRL_MSG; }

  /**
   *
   * \return
   */
  uint16_t GetBackoffProbability () const;

  /**
   *
   * \param backoffProbability
   */
  void SetBackoffProbability (uint16_t backoffProbability);

  /**
   *
   * \return
   */
  uint32_t GetAllocationChannelId () const;

  /**
   *
   * \param allocationChannel
   */
  void SetAllocationChannelId (uint32_t allocationChannel);

  /**
   * Get real size of the random access message, which can be used to e.g. simulate real size.
   * \return Real size of the random access message.
   */
  virtual uint32_t GetSizeInBytes () const;

private:

  /**
   * Type field of the random access control element
   */
  static const uint32_t RA_CONTROL_MSG_TYPE_VALUE_SIZE_IN_BYTES = 1;

  /**
   * Common header of the random access control element
   */
  static const uint32_t RA_CONTROL_MSG_COMMON_HEADER_SIZE_IN_BYTES = 3;

  /**
   *
   */
  uint32_t m_allocationChannelId;

  /**
   *
   */
  uint16_t m_backoffProbability;
};


/**
 * \ingroup satellite
 * \brief The container to store control messages. It assigns ID for added messages.
 * ID is used when message is requested.
 *
 * Message are deleted after set store time expired for a message. Message is deleted already when read,
 * if this functionality is enabled in creation time.
 *
 * Container is needed to store control messages which content are not wanted to simulate inside packet.
 *
 */
class SatControlMsgContainer : public SimpleRefCount<SatControlMsgContainer>
{
public:
  /**
   * Default constructor for SatControlMsgContainer.
   */
  SatControlMsgContainer ();

  /**
   * Default constructor for SatControlMsgContainer.
   */
  SatControlMsgContainer (Time m_storeTime, bool deleteOnRead);

  /**
   * Destructor for SatControlMsgContainer
   */
  ~SatControlMsgContainer ();

  /**
   * Add a control message.
   *
   * \param Id of the message to add.
   * \param Pointer to message to add.
   *
   * \return ID of the created added message.
   */
  uint32_t Add (Ptr<SatControlMessage> controlMsg);

  /**
   * Get a control message.
   *
   * \param Id of the message to get.
   * \return Pointer to message.
   */
  Ptr<SatControlMessage> Get (uint32_t id);

private:

  /**
   * Erase first item from container. Schedules a new erase call to this function with time left
   * for next item in list (if container is not empty).
   */
  void EraseFirst ();

  typedef std::pair<Time, Ptr<SatControlMessage> > CtrlMsgMapValue_t;
  typedef std::map<uint32_t, CtrlMsgMapValue_t > CtrlMsgMap_t;
  CtrlMsgMap_t   m_ctrlMsgs;
  uint32_t       m_id;
  EventId        m_storeTimeout;

  /**
   * Time to store a message in container.
   *
   * If m_deleteOnRead is set false, the message
   * is always deleted only when this time is elapsed.
   */
  Time m_storeTime;

  /**
   * Flag to tell, if message is deleted from container when read (get).
   */
  bool m_deleteOnRead;

};



} // namespace ns3

#endif // SATELLITE_CONTROL_MESSAGE_H
