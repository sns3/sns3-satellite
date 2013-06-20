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

#include "satellite-control-header.h"

NS_LOG_COMPONENT_DEFINE ("SatCtrlHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatCtrlHeader);

SatCtrlHeader::SatCtrlHeader ()
  : m_msgType (UNDEFINED_MSG),
    m_sequenceNumber (0xffffffff)
{
}

SatCtrlHeader::~SatCtrlHeader ()
{
  m_msgType = UNDEFINED_MSG;
  m_sequenceNumber = 0xffffffff;
}

void
SatCtrlHeader::SetMsgType (SatCtrlHeader::MsgType type)
{
  m_msgType = type;
}

void
SatCtrlHeader::SetSequenceNumber (uint32_t sequenceNumber)
{
  m_sequenceNumber = sequenceNumber;
}

void
SatCtrlHeader::SetMsgData (double data)
{
  m_data = data;
}

SatCtrlHeader::MsgType
SatCtrlHeader::GetMsgType () const
{
  return m_msgType;
}

uint32_t
SatCtrlHeader::GetSequenceNumber () const
{
  return m_sequenceNumber;
}

double
SatCtrlHeader::GetMsgData () const
{
  return m_data;
}

TypeId
SatCtrlHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatCtrlHeader")
    .SetParent<Header> ()
    .AddConstructor<SatCtrlHeader> ()
  ;
  return tid;
}

TypeId
SatCtrlHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void SatCtrlHeader::Print (std::ostream &os)  const
{
  os << "M Type=" << (uint32_t)m_msgType;
  os << " SN=" << m_sequenceNumber;
}

uint32_t SatCtrlHeader::GetSerializedSize (void) const
{
  return (sizeof(uint32_t) * 2 + sizeof(double));
}

void SatCtrlHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  uint8_t buff[sizeof(double)];
  std::memcpy(buff, &m_data, sizeof(double));

  i.WriteU32( m_msgType );
  i.WriteU32 ( m_sequenceNumber );
  i.Write(buff, sizeof(double));
}

uint32_t SatCtrlHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t buff[sizeof(double)];

  m_msgType = (MsgType) i.ReadU32();
  m_sequenceNumber = i.ReadU32();
  i.Read(buff, sizeof(double));

  std::memcpy( &m_data, buff, sizeof(double));

  return GetSerializedSize ();
}

}; // namespace ns3
