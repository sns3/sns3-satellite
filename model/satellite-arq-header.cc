/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Magister Solutions Ltd.
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
 * Author: Jani Puttonen <jani.puttonen@magister.fi>
 */

#include "ns3/log.h"
#include "satellite-arq-header.h"


NS_LOG_COMPONENT_DEFINE ("SatArqHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatArqHeader);

SatArqHeader::SatArqHeader ()
  : m_seqNo (0)
{
}


SatArqHeader::~SatArqHeader ()
{

}

TypeId
SatArqHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatArqHeader")
    .SetParent<Header> ()
    .AddConstructor<SatArqHeader> ()
  ;
  return tid;
}

uint32_t SatArqHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return 1;
}

void SatArqHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this);
  Buffer::Iterator i = start;
  i.WriteU8 (m_seqNo);
}

uint32_t SatArqHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this);
  Buffer::Iterator i = start;

  m_seqNo = i.ReadU8 ();
  return GetSerializedSize ();
}

void SatArqHeader::Print (std::ostream &os) const
{
  os << m_seqNo;
}

TypeId
SatArqHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint8_t SatArqHeader::GetSeqNo () const
{
  NS_LOG_FUNCTION (this);
  return m_seqNo;
}

void SatArqHeader::SetSeqNo (uint8_t seqNo)
{
  NS_LOG_FUNCTION (this << (uint32_t) seqNo);
  m_seqNo = seqNo;
}



}; // namespace ns3
