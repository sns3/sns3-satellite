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
#include "ns3/header.h"
#include "ns3/nstime.h"
#include "ns3/mac48-address.h"
#include "satellite-mac-tag.h"

namespace ns3 {

/**
 * \ingroup satellite
 * \brief This class implements a tag that is used to indentify control messages (packages)
 */
class SatControlMsgTag : public Tag
{
public:

  /*
   * \brief Definition for different types of control messages
   */
  typedef enum
    {
      SAT_NON_CTRL_MSG,
      SAT_TBTP_CTRL_MSG,
      SAT_CR_CTRL_MSG,
      SAT_RA_CTRL_MSG
    }SatControlMsgType_t;

  /*
   * Constructor for SatControlMsgTag
   */
  SatControlMsgTag ();

  /*
   * Destructor for SatControlMsgTag
   */
  ~SatControlMsgTag ();

  /*
   * Set type of the contol message. In construction phase initialized
   * to value SAT_UNKNOWN_CTRL_MSG.
   *
   * \param type The type of the control message
   */
  void SetMsgType (SatControlMsgType_t type);

  /*
   * Get type of the contol message.
   *
   * \return The type of the control message
   */
  SatControlMsgType_t GetMsgType (void) const;

  // methods derived from base classes
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

private:
  SatControlMsgType_t  m_msgType;

};


/**
 * \ingroup satellite
 * \brief The packet header for the Capacity request messages.
 * (Tagged by SatControlMsgTag with type value SAT_CR_CTRL_MSG)
 */

class SatCapacityReqHeader : public Header
{
public:

  /*
   * \brief Definition for different types of Capacity Reqeuest (CR) messages.
   */
  typedef enum
  {
    SAT_UNKNOWN_CR,
    SAT_RBDC_CR,
    SAT_VBDC_CR
  } SatCrRequestType_t;

  /**
   * Constructor for SatCapacityReqHeader
   */
  SatCapacityReqHeader ();

  /**
   * Destructor for SatCapacityReqHeader
   */
  ~SatCapacityReqHeader ();

  /*
   * Get type of the CR message.
   *
   * \return The type of the CR message
   */
  SatCrRequestType_t GetReqType () const;

  /*
   * Set type of the CR message. In construction phase initialized
   * to value SAT_UNKNOWN_CR.
   *
   * \param type Type of the CR message
   */
  void SetReqType (SatCrRequestType_t type);

  /*
   * GSet rate of the CR.
   *
   * \return Rate of the CR
   */
  uint32_t GetRequestedRate () const;

  /*
   * Set rate of the CR.
   *
   * \param rate The rate of the CR.
   */
  void SetRequestedRate (uint32_t rate);

  // methods derived from base classes
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  /**
   * Type of the this Capacity Request
   */
  SatCrRequestType_t m_reqType;

  /**
   * Requested rate.
   */
  uint32_t m_requestedRate;
};

/**
 * \ingroup satellite
 * \brief The packet header for the Terminal Burst Time Plan (TBTP) messages.
 * (Tagged by SatControlMsgTag with type value SAT_TBTP_CTRL_MSG)
 */

class SatTbtpHeader : public Header
{
public:
  /**
   * Time slot info class.
   */
  class TbtpTimeSlotInfo : public SimpleRefCount<SatTbtpHeader::TbtpTimeSlotInfo>
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
     * \param timeSlotId  The id of the timeslot
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

  /**
   * Default constructor for SatTbtpHeader. Set sequence id to 0.
   */
  SatTbtpHeader ();

  /**
   * Constructor for SatTbtpHeader to construct TBTP with given sequence id.
   * \param seqId sequence id
   */
  SatTbtpHeader ( uint8_t seqId );

  /**
   * Destructor for SatTbtpHeader
   */
  ~SatTbtpHeader ();

  /**
   * Set counter of the superframe in this TBTP message.
   *
   * \param counter The superframe counter.
   */
  inline void SetSuperframeCounter (uint32_t counter) { m_superframeCounter = counter; }

  /**
   * Get sequence id of the superframe in this TBTP message.
   *
   * \return The superframe sequency id.
   */
  inline uint8_t GetSuperframeId () {return m_superframeSeqId;}

  /**
   * Get counter of the superframe in this TBTP message.
   *
   * \return The superframe counter.
   */
  inline uint32_t GetSuperframeCounter () {return m_superframeCounter;}

  /**
   * Get the information of the time slots
   *
   * \param utId  id of the UT which time slot information is requested
   *
   * \return vector containing time slot info
   */
  std::vector< Ptr<SatTbtpHeader::TbtpTimeSlotInfo> > GetTimeslots (Address utId);

  /**
   * Set a time slot information
   *
   * \param utId  id of the UT which time slot information is set
   * \param info  information of the time slot info
   */
  void SetTimeslot(Mac48Address utId, Ptr<TbtpTimeSlotInfo> info);

  // methods derived from base classes
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:

  typedef std::multimap<Address, Ptr<TbtpTimeSlotInfo> > TimeSlotMap_t;

  TimeSlotMap_t m_timeSlots;
  uint32_t      m_superframeCounter;
  uint8_t       m_superframeSeqId;
};


} // namespace ns3

#endif // SATELLITE_CONTROL_MESSAGE_H
