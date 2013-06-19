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

#include <list>

namespace ns3 {

/**
 * \ingroup lte
 * \brief The packet header for the Control messages used by lower layer of
 * satellite network components.
 */
class SatCtrlHeader : public Header
{
public:

  typedef enum {
    UNDEFINED_MSG = 0,
    TBTP_MSG   = 1,
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

  MsgType GetMsgType () const;
  uint32_t GetSequenceNumber () const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  MsgType   m_msgType;
  uint32_t  m_sequenceNumber;
};

} // namespace ns3

#endif // SAT_CTRL_HEADER_H
