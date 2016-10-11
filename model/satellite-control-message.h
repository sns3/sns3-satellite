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
 * \brief This class implements a tag that is used to identify
 * control messages (packages).
 */
class SatControlMsgTag : public Tag
{
public:
  /**
   * \brief Definition for different types of control messages
   */
  typedef enum
  {
    SAT_NON_CTRL_MSG,     //!< SAT_NON_CTRL_MSG
    SAT_TBTP_CTRL_MSG,    //!< SAT_TBTP_CTRL_MSG
    SAT_CR_CTRL_MSG,      //!< SAT_CR_CTRL_MSG
    SAT_RA_CTRL_MSG,      //!< SAT_RA_CTRL_MSG
    SAT_ARQ_ACK,          //!< SAT_ARQ_ACK
    SAT_CN0_REPORT        //!< SAT_CN0_REPORT
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
   * \brief Set type of the control message. In construction phase initialized
   * to value SAT_UNKNOWN_CTRL_MSG.
   * \param type The type of the control message
   */
  void SetMsgType (SatControlMsgType_t type);

  /**
   * \brief Get type of the control message.
   * \return The type of the control message
   */
  SatControlMsgType_t GetMsgType (void) const;

  /**
   * \brief Set message type specific identifier.
   * \param msgId Message type specific identifier. May be ignored by some message types.
   */
  virtual void SetMsgId (uint32_t msgId);

  /**
   * \brief Get message type specific identifier.
   * \return Message type specific identifier.
   */
  virtual uint32_t GetMsgId () const;


  /**
   * methods derived from base classes
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Get serialized size of methods
   * \return Serialized size in bytes
   */
  virtual uint32_t GetSerializedSize (void) const;

  /**
   * Serializes information to buffer from this instance of methods
   * \param i Buffer in which the information is serialized
   */
  virtual void Serialize (TagBuffer i) const;

  /**
   * Deserializes information from buffer to this instance of methods
   * \param i Buffer from which the information is deserialized
   */
  virtual void Deserialize (TagBuffer i);

  /**
   * Print time stamp of this instance of methods
   * \param &os Output stream to which tag timestamp is printed.
   */
  virtual void Print (std::ostream &os) const;

private:
  SatControlMsgType_t  m_msgType;
  uint32_t             m_msgId;
};

/**
 * \ingroup satellite
 * \brief Abstract satellite control message class. The real
 * control messages are inherited from it.
 */

class SatControlMessage : public Object
{
public:

  /**
   * methods derived from base classes
   */
  static TypeId GetTypeId (void);

  /**
   * Default constructor for SatControlMessage.
   */
  SatControlMessage ()
  {
  }

  /**
   * Destructor
   */
  ~SatControlMessage ()
  {
  }

  /**
   * Get real size of the control message. This can be used to
   * simulate real packet size.
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
 * However it implements method GetSizeInBytes, which can be used to simulate
 * real TBTP message size.
 */

class SatTbtpMessage : public SatControlMessage
{
public:
  /**
   * Container for time slot configurations in time slot map item DaTimeSlotMapItem_t.
   */
  typedef std::vector< Ptr<SatTimeSlotConf> >  DaTimeSlotConfContainer_t;

  /**
   * Item for DA time slot information.
   *
   * Stored information is pair, which member first holds frame id
   * of the time slots and member second holds container of time slot configurations.
   */
  typedef std::pair<uint8_t, DaTimeSlotConfContainer_t>  DaTimeSlotInfoItem_t;

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


  /**
   * methods derived from base classes
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Default constructor for SatTbtpHeader. Set sequence id to 0.
   */
  SatTbtpMessage ();

  /**
   * Constructor for SatTbtpHeader to construct TBTP with given sequence id.
   * \param seqId sequence id
   */
  SatTbtpMessage (uint8_t seqId);

  /**
   * Destructor for SatTbtpHeader
   */
  ~SatTbtpMessage ();

  /**
   * Get type of the message.
   *
   * \return SatControlMsgTag::SAT_TBTP_CTRL_MSG
   */
  inline SatControlMsgTag::SatControlMsgType_t GetMsgType () const
  {
    return SatControlMsgTag::SAT_TBTP_CTRL_MSG;
  }

  /**
   * Set counter of the super frame in this TBTP message.
   *
   * \param counter The super frame counter.
   */
  inline void SetSuperframeCounter (uint32_t counter)
  {
    m_superframeCounter = counter;
  }

  /**
   * Get sequence id of the super frame in this TBTP message.
   *
   * \return The super frame sequence id.
   */
  inline uint8_t GetSuperframeSeqId ()
  {
    return m_superframeSeqId;
  }

  /**
   * Get counter of the super frame in this TBTP message.
   *
   * \return The super frame counter.
   */
  inline uint32_t GetSuperframeCounter ()
  {
    return m_superframeCounter;
  }

  /**
   * Get size of frame info size in this TBTP message.
   *
   * \return Frame info size.
   */
  inline uint32_t GetFrameInfoSize () const
  {
    return m_tbtpFrameBodySizeInBytes;
  }

  /**
   * Get the information of the DA time slots.
   *
   * \param utId  id of the UT which time slot information is requested
   * \return vector containing DA time slot info
   */
  const DaTimeSlotInfoItem_t& GetDaTimeslots (Address utId);

  /**
   * Set a DA time slot information
   *
   * \param utId id of the UT which time slot information is set
   * \param frameId Frame ID of the time slot
   * \param conf Pointer to time slot configuration
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
   * \param raChannel raChannel index
   * \param frameId Frame ID of RA channel
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
  typedef std::map<Address, DaTimeSlotInfoItem_t > DaTimeSlotMap_t;

  DaTimeSlotMap_t   m_daTimeSlots;
  RaChannelMap_t    m_raChannels;
  uint32_t          m_superframeCounter;
  uint8_t           m_superframeSeqId;
  uint8_t           m_assignmentFormat;
  std::set<uint8_t> m_frameIds;

  /**
   * Empty DA slot container to be returned if there are not DA time slots
   */
  const DaTimeSlotInfoItem_t m_emptyDaSlotContainer;
};

/**
 * \ingroup satellite
 * \brief The packet for the Capacity Request (CR) messages.
 * (Tagged by SatControlMsgTag with type value SAT_CR_CTRL_MSG)
 * NOTE! Message implementation doesn't follow specification (ETSI EN 301 542-2).
 * However it implements method GetSizeInBytes, which can be used to simulate
 * real CR message size.
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


  /**
   * methods derived from base classes
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Define type RequestDescriptor_t
   */
  typedef std::pair<uint8_t, SatEnums::SatCapacityAllocationCategory_t> RequestDescriptor_t;

  /**
   * Define type RequestContainer_t
   */
  typedef std::map<RequestDescriptor_t, uint16_t > RequestContainer_t;

  /**
   * \brief Get type of the message.
   * \return SatControlMsgTag::SAT_CR_CTRL_MSG
   */
  inline SatControlMsgTag::SatControlMsgType_t GetMsgType () const
  {
    return SatControlMsgTag::SAT_CR_CTRL_MSG;
  }

  /**
   * \brief Add a control element to capacity request
   */
  void AddControlElement (uint8_t rcIndex, SatEnums::SatCapacityAllocationCategory_t cac, uint32_t value);

  /**
   * \brief Get the capacity request content
   * \return RequestContainer_t Capacity request container
   */
  const RequestContainer_t GetCapacityRequestContent () const;

  /**
   * \brief The number of capacity request elements
   * \return uint32_t Number of CR elements
   */
  uint32_t GetNumCapacityRequestElements () const;

  /**
   * \brief Get C/N0 estimate.
   * \return Estimate of the C/N0.
   */
  double GetCnoEstimate () const;

  /**
   * \brief Set C/N0 estimate.
   * \param cno The estimate of the C/N0.
   */
  void SetCnoEstimate (double cno);

  /**
   * \brief Get real size of the CR message, which can be used to e.g. simulate real size.
   * \return Real size of the CR message.
   */
  virtual uint32_t GetSizeInBytes () const;

  /**
   * \brief Has the CR non-zero content
   * \return bool Flag to indicate whether the CR has non-zero content
   */
  bool IsNotEmpty () const;

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
 * \brief The packet for the Automatic Repeat reQuest (ARQ)
 * acknowledgment (ACK) messages.
 * (Tagged by SatControlMsgTag with type value SAT_ARQ_ACK)
 */

class SatArqAckMessage : public SatControlMessage
{
public:
  /**
   * Constructor for SatArqAckMessage
   */
  SatArqAckMessage ();

  /**
   * Destructor for SatArqAckMessage
   */
  ~SatArqAckMessage ();


  /**
   * methods derived from base classes
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * \brief Get type of the message.
   * \return SatControlMsgTag::SAT_ARQ_ACK
   */
  inline SatControlMsgTag::SatControlMsgType_t GetMsgType () const
  {
    return SatControlMsgTag::SAT_ARQ_ACK;
  }

  /**
   * \brief Set the sequence number to be ACK'ed
   * \param sn Sequence number
   */
  void SetSequenceNumber (uint8_t sn);

  /**
   * \brief Get the sequence number to be ACK'ed
   * \return uint32_t Sequence number
   */
  uint8_t GetSequenceNumber () const;

  /**
   * \brief Set the flow id to be ACK'ed
   * \param sn Sequence number
   */
  void SetFlowId (uint8_t sn);

  /**
   * \brief Get the sequence number to be ACK'ed
   * \return uint32_t Sequence number
   */
  uint8_t GetFlowId () const;

  /**
   * \brief Get real size of the ACK message, which can be used to e.g. simulate real size.
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


  /**
   * methods derived from base classes
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * \brief Get type of the message.
   *
   * \return SatControlMsgTag::SAT_CN0_REPORT
   */
  inline SatControlMsgTag::SatControlMsgType_t GetMsgType () const
  {
    return SatControlMsgTag::SAT_CN0_REPORT;
  }

  /**
   * \brief Get C/N0 estimate.
   * \return Estimate of the C/N0.
   */
  double GetCnoEstimate () const;

  /**
   * \brief Set C/N0 estimate.
   * \param cno The estimate of the C/N0.
   */
  void SetCnoEstimate (double cno);

  /**
   * \brief Get real size of the CR message, which can be used to e.g. simulate real size.
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


  /**
   * methods derived from base classes
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Get the type ID of instance
   * \return the object TypeId
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * \brief Get type of the message.
   *
   * \return SatControlMsgTag::SAT_RA_CTRL_MSG
   */
  inline SatControlMsgTag::SatControlMsgType_t GetMsgType () const
  {
    return SatControlMsgTag::SAT_RA_CTRL_MSG;
  }

  /**
   * \brief Get backoff probability
   * \return backoff probability
   */
  uint16_t GetBackoffProbability () const;

  /**
   * \brief Set backoff probability
   * \param backoffProbability Backoff probability
   */
  void SetBackoffProbability (uint16_t backoffProbability);

  /**
   * \brief Get backoff time
   * \return backoff time
   */
  uint16_t GetBackoffTime () const;

  /**
   * \brief Set backoff time
   * \param backoffTime Backoff time
   */
  void SetBackoffTime (uint16_t backoffTime);

  /**
   * \brief Get allocation chanel ID
   * \return Allocation channel ID
   */
  uint8_t GetAllocationChannelId () const;

  /**
   * Set allocation channel ID
   * \param allocationChannel Allocation channel ID
   */
  void SetAllocationChannelId (uint8_t allocationChannel);

  /**
   * \brief Get real size of the random access message, which can be used to e.g. simulate real size.
   * \return Real size of the random access message.
   */
  virtual uint32_t GetSizeInBytes () const;

private:
  /**
   * Common header of the random access element
   */
  static const uint32_t RA_CONTROL_MSG_HEADER_SIZE_IN_BYTES = 5;

  /**
   * Allocation channel ID
   */
  uint8_t m_allocationChannelId;

  /**
   * Backoff probability
   */
  uint16_t m_backoffProbability;

  /**
   * Backoff time
   */
  uint16_t m_backoffTime;
};


/**
 * \ingroup satellite
 * \brief The container to store control messages. Container assigns two sequences of IDs
 * for added messages.
 * - Send/buffered IDs - used during buffering period between ND and MAC.
 * - Receive IDs - used to indicate the receiver the id for the control PDU
 *
 * Message are deleted after set store time expired for a message. Message is deleted already when read,
 * if this functionality is enabled in creation time.
 *
 * Container is needed to store control messages which content are not wanted to simulate inside packet.
 *
 * The reason for two sets of IDs relate to two things:
 * - The SatControlMessage Ptr needs to be stored also during buffering time (before it gets scheduling time)
 * - The SatControlMsgContainer containers assume that the recv IDs are given in FIFO order.
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
   * \brief Reserve an id and store a control message.
   *
   * \param controlMsg Pointer to message to be added.
   * \return Reserved send ID of the created added message.
   */
  uint32_t ReserveIdAndStore (Ptr<SatControlMessage> controlMsg);

  /**
   * \brief Add a control message.
   *
   * \param sendId of the message to add.
   * \return Receive id given by the container.
   */
  uint32_t Send (uint32_t sendId);

  /**
   * \brief Read a control message.
   *
   * \param recvId Id of the message to read.
   * \return Pointer to message.
   */
  Ptr<SatControlMessage> Read (uint32_t recvId);

private:
  /**
   * \brief Erase first item from container. Schedules a new erase call to this function with time left
   * for next item in list (if container is not empty).
   */
  void EraseFirst ();

  /**
   * \brief Do clean up for the Ctrl msg id map. Currently it needs to erase
   * a map entry based on value, which is not very efficient.
   * \param recvId Ctrl msg id
   */
  void CleanUpIdMap (uint32_t recvId);

  typedef std::map<uint32_t, Ptr<SatControlMessage> >   ReservedCtrlMsgMap_t;
  typedef std::map<uint32_t, uint32_t>                  CtrlIdMap_t;
  typedef std::pair<Time, Ptr<SatControlMessage> >      CtrlMsgMapValue_t;
  typedef std::map<uint32_t, CtrlMsgMapValue_t >        CtrlMsgMap_t;

  ReservedCtrlMsgMap_t  m_reservedCtrlMsgs;
  CtrlMsgMap_t          m_ctrlMsgs;
  CtrlIdMap_t                                             m_ctrlIdMap;
  uint32_t              m_sendId;
  uint32_t              m_recvId;
  EventId               m_storeTimeout;

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
