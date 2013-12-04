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

#include "satellite-rle-headers.h"


NS_LOG_COMPONENT_DEFINE ("SatPPduHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPPduHeader);

SatPPduHeader::SatPPduHeader ()
:m_startIndicator (0),
 m_endIndicator (0),
 m_ppduLengthInBytes (0),
 m_fragmentId (0),
 m_totalLengthInBytes (0)
{
}


SatPPduHeader::~SatPPduHeader ()
{

}

TypeId
SatPPduHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatPPduHeader")
    .SetParent<Header> ()
    .AddConstructor<SatPPduHeader> ()
  ;
  return tid;
}

uint32_t SatPPduHeader::GetSerializedSize (void) const
{
  return ( sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t));
}

void SatPPduHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteU8 (m_startIndicator);
  start.WriteU8 (m_endIndicator);
  start.WriteU32 (m_ppduLengthInBytes);
  start.WriteU32 (m_fragmentId);
  start.WriteU32 (m_totalLengthInBytes);
}

uint32_t SatPPduHeader::Deserialize (Buffer::Iterator start)
{
  m_startIndicator = start.ReadU8 ();
  m_endIndicator = start.ReadU8 ();
  m_ppduLengthInBytes = start.ReadU32 ();
  m_fragmentId = start.ReadU32 ();
  m_totalLengthInBytes = start.ReadU32 ();

  return GetSerializedSize();
}

void SatPPduHeader::Print (std::ostream &os) const
{
  os << m_startIndicator << " " << m_endIndicator << " " << m_ppduLengthInBytes << " " << m_fragmentId << " " << m_totalLengthInBytes;
}

TypeId
SatPPduHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint8_t SatPPduHeader::GetStartIndicator () const
{
  return m_startIndicator;
}

uint8_t SatPPduHeader::GetEndIndicator () const
{
  return m_endIndicator;
}

uint32_t SatPPduHeader::GetPPduLength () const
{
  return m_ppduLengthInBytes;
}

uint32_t SatPPduHeader::GetFragmentId () const
{
  return m_fragmentId;
}

uint32_t SatPPduHeader::GetTotalLength () const
{
  return m_totalLengthInBytes;
}

void SatPPduHeader::SetStartIndicator ()
{
  m_startIndicator = 1;
}

void SatPPduHeader::SetEndIndicator ()
{
  m_endIndicator = 1;
}

void SatPPduHeader::SetPPduLength (uint32_t bytes)
{
  m_ppduLengthInBytes = bytes;
}

void SatPPduHeader::SetFragmentId (uint32_t id)
{
  m_fragmentId = id;
}

void SatPPduHeader::SetTotalLength (uint32_t bytes)
{
  m_totalLengthInBytes = bytes;
}


NS_OBJECT_ENSURE_REGISTERED (SatFPduHeader);

SatFPduHeader::SatFPduHeader ()
:m_numPPdus (0)
{
}

SatFPduHeader::~SatFPduHeader ()
{

}

TypeId
SatFPduHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatFPduHeader")
    .SetParent<Header> ()
    .AddConstructor<SatFPduHeader> ()
  ;
  return tid;
}

uint32_t SatFPduHeader::GetSerializedSize (void) const
{
  return ( sizeof(uint8_t) + m_numPPdus * sizeof (uint32_t));
}

void SatFPduHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteU8 (m_numPPdus);
  for (std::vector<uint32_t>::const_iterator cit = m_ppduSizesInBytes.begin ();
      cit != m_ppduSizesInBytes.end ();
      ++cit)
    {
      start.WriteU32 (*cit);
    }
}

uint32_t SatFPduHeader::Deserialize (Buffer::Iterator start)
{
  m_numPPdus = start.ReadU8 ();
  for (uint32_t i = 0; i < m_numPPdus; ++i)
    {
      uint32_t s = start.ReadU32 ();
      m_ppduSizesInBytes.push_back (s);
    }

  return GetSerializedSize();
}

void SatFPduHeader::Print (std::ostream &os) const
{
  os << m_numPPdus;
}

void SatFPduHeader::PushPPduLength (uint32_t size)
{
  m_numPPdus++;
  m_ppduSizesInBytes.push_back (size);
}

TypeId
SatFPduHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint8_t SatFPduHeader::GetNumPPdus () const
{
  return m_numPPdus;
}

uint32_t SatFPduHeader::GetPPduLength (uint32_t index) const
{
  NS_ASSERT(index < m_numPPdus);
  return m_ppduSizesInBytes[index];
}

}; // namespace ns3
