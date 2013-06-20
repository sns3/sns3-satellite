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

#ifndef SAT_CTRL_HEADER_H
#define SAT_CTRL_HEADER_H

#include "ns3/header.h"
#include "ns3/nstime.h"
#include "ns3/simple-ref-count.h"

namespace ns3 {

//TODO: these messages are under work not working ones yet

/**
 * \ingroup satellite
 * \brief The packet data for the Control messages used by lower layer of
 * satellite network components. This is just used as interface.
 */

class SatCtrlData : public SimpleRefCount<SatCtrlData>
{
public:
  SatCtrlData() {}
  ~SatCtrlData() {}
};

/**
 * \ingroup satellite
 * \brief The packet header for the Control messages used by lower layer of
 * satellite network components.
 */
class SatCtrlHeader : public Header
{
public:

  typedef enum {
    UNDEFINED_MSG = 0,
    TBTP_MSG = 1,
    CR_MSG = 2,
  } MsgType;

  /**
   * \brief Constructor
   *
   * Creates a null header
   */
  SatCtrlHeader ();
  ~SatCtrlHeader ();

  void SetMsgType (MsgType type);
  void SetSequenceNumber (uint32_t sequenceNumber);
  void SetMsgData(double data);

  MsgType GetMsgType (void) const;
  uint32_t GetSequenceNumber (void) const;
  double GetMsgData(void) const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  MsgType               m_msgType;
  uint32_t              m_sequenceNumber;
  double                m_data;
};

class SatCapacityReqData : public SatCtrlData
{
public:
  typedef enum
  {
    UNDEFINED = 0,
    RBDC = 1,
    VBDC = 2
  } RequestType;

  /**
   * \brief Constructor
   *
   * Creates a null request data
   */
  SatCapacityReqData (){ m_reqType = SatCapacityReqData::UNDEFINED;}

  ~SatCapacityReqData () {}

  inline RequestType GetReqType () const {return m_reqType;}
  inline void SetReqType(RequestType type) {m_reqType = type;}

  uint32_t GetRequestedRate () const {return m_requestedRate;}
  void SetRequestedRate(uint32_t rate) {m_requestedRate = rate;}


private:
  /**
   * Type of the SatCapacityReqData
   */
  RequestType   m_reqType;
  uint32_t      m_requestedRate;

};

class SatTbtpRepData : public SatCtrlData
{
public:
  /**
   * \brief Constructor
   *
   * Creates a null request data
   */
  SatTbtpRepData () {}

  ~SatTbtpRepData (){}

  inline double GetInterval () {return m_interval;}
  void SetInterval(double interval) {m_interval = interval;}


private:
  /**
   * TODO: needed to change, now just inform time to schedule next sending in UT
   */
  double    m_interval;

};


} // namespace ns3

#endif // SAT_CTRL_HEADER_H
