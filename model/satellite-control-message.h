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
      SAT_NON_CTRL_MSG, //!< SAT_NON_CTRL_MSG
      SAT_TBTP_CTRL_MSG,//!< SAT_TBTP_CTRL_MSG
      SAT_CR_CTRL_MSG,  //!< SAT_CR_CTRL_MSG
      SAT_RA_CTRL_MSG   //!< SAT_RA_CTRL_MSG
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
  /**
   * Time slot info class used to stored time slots in TBTP.
   */
  class TbtpTimeSlotInfo : public SimpleRefCount<SatTbtpMessage::TbtpTimeSlotInfo>
  {
  public:
    static const uint16_t maximumTimeSlotId = 2047;

    /**
     * Default Constructor for TbtpTimeSlotInfo
     */
    TbtpTimeSlotInfo ();

    /**
     * Constructor for TbtpTimeSlotInfo
     *
     * \param frameId     The id of the frame where time slot belongs to
     * \param timeSlotId  The id of the time slot
     */
    TbtpTimeSlotInfo (uint8_t frameId, uint16_t timeSlotId);

    /**
     * Destructor for TbtpTimeSlotInfo
     */
    ~TbtpTimeSlotInfo ();

    /**
     * Get the id of the frame
     *
     * \return frame id
     */
    inline uint8_t GetFrameId() { return m_frameId; }

    /**
     * Get the id of the time slot
     *
     * \return time slot id
     */
    inline uint16_t GetTimeSlotId() { return m_timeSlotId; }

    // methods derived from base classes
    void Print (std::ostream &os) const;
    uint32_t GetSerializedSize (void) const;
    void Serialize (Buffer::Iterator start) const;
    uint32_t Deserialize (Buffer::Iterator start);

  private:
    uint8_t   m_frameId;
    uint16_t  m_timeSlotId;
  };

  typedef std::vector< Ptr<SatTbtpMessage::TbtpTimeSlotInfo> > TimeSlotInfoContainer_t;

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
   * Get the information of the time slots
   *
   * \param utId  id of the UT which time slot information is requested
   * \return vector containing time slot info
   */
  const TimeSlotInfoContainer_t& GetTimeslots (Address utId);

  /**
   * Set a time slot information
   *
   * \param utId  id of the UT which time slot information is set
   * \param info  information of the time slot info
   */
  void SetTimeslot (Mac48Address utId, Ptr<TbtpTimeSlotInfo> info);

  /**
   * Get real size of the TBTP message, which can be used to e.g. simulate real size.
   *
   * \return Real size of the TBTP message.
   */
  virtual uint32_t GetSizeInBytes () const;

  /**
   * Dump all the contents of the TBTP
   */
  void Dump () const;

private:

  typedef std::map<Address, TimeSlotInfoContainer_t > TimeSlotMap_t;

  TimeSlotMap_t     m_timeSlots;
  uint32_t          m_superframeCounter;
  uint8_t           m_superframeSeqId;
  uint8_t           m_assignmentFormat;
  std::set<uint8_t> m_frameIds;

  /**
   * Empty slot container to be returned if there are not time slots
   */
  const TimeSlotInfoContainer_t m_emptySlotContainer;
};

// TODO: CR is needed to change as packet
/**
 * \ingroup satellite
 * \brief The packet header for the Capacity request messages.
 * (Tagged by SatControlMsgTag with type value SAT_CR_CTRL_MSG)
 */

class SatCrMessage : public SatControlMessage
{
public:

  /**
   * \brief Definition for different types of Capacity Request (CR) messages.
   */
  typedef enum
  {
    SAT_UNKNOWN_CR,
    SAT_RBDC_CR,
    SAT_VBDC_CR
  } SatCrRequestType_t;

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

  /**
   * Get type of the message.
   *
   * \return SatControlMsgTag::SAT_CR_CTRL_MSG
   */
  inline SatControlMsgTag::SatControlMsgType_t GetMsgType () const { return SatControlMsgTag::SAT_CR_CTRL_MSG; }

  /**
   * Get type of the CR message.
   *
   * \return The type of the CR message
   */
  SatCrRequestType_t GetReqType () const;

  /**
   * Set type of the CR message. In construction phase initialized
   * to value SAT_UNKNOWN_CR.
   *
   * \param type Type of the CR message
   */
  void SetReqType (SatCrRequestType_t type);

  /**
   * Get rate of the CR.
   *
   * \return Rate of the CR
   */
  double GetRequestedRate () const;

  /**
   * Set rate of the CR.
   *
   * \param rate The rate of the CR.
   */
  void SetRequestedRate (double rate);

  /**
   * Get C/N0 estimate.
   *
   * \return Estimate of the C/N0.
   */
  double GetCnoEstimate () const;

  /**
   * Set C/N0 estimate.
   *
   * \param cno The estimate of the C/N0.
   */
  void SetCnoEstimate (double cno);

  /**
   * Get real size of the CR message, which can be used to e.g. simulate real size.
   *
   * \return Real size of the CR message.
   */
  virtual uint32_t GetSizeInBytes () const;

private:
  /**
   * Type of the this Capacity Request
   */
  SatCrRequestType_t m_reqType;

  /**
   * Requested rate.
   */
  double m_requestedRate;

  /**
   * C/N0 estimate.
   */
  double m_cno;
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
