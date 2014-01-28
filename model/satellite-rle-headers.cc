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
#include "satellite-encap-pdu-status-tag.h"


NS_LOG_COMPONENT_DEFINE ("SatPPduHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPPduHeader);

SatPPduHeader::SatPPduHeader ()
:m_startIndicator (0),
 m_endIndicator (0),
 m_ppduLengthInBytes (0),
 m_fragmentId (0),
 m_totalLengthInBytes (0),
 m_fullPpduHeaderSize (2),
 m_startPpduHeaderSize (4),
 m_endPpduHeaderSize (2),
 m_continuationPpduHeaderSize (2)
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
  if (m_startIndicator)
    {
      // FULL PDU
      if (m_endIndicator)
        {
          return m_fullPpduHeaderSize;
        }
      // START PDU
      else
        {
          return m_startPpduHeaderSize;
        }
    }
  else
    {
      // END PDU
      if (m_endIndicator)
        {
          return m_endPpduHeaderSize;
        }
      // CONTINUATION PDU
      else
        {
          return m_continuationPpduHeaderSize;
        }
    }
  NS_ASSERT (false);
  return 0;
}

void SatPPduHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // FULL PDU
  if (m_startIndicator && m_endIndicator)
    {
      i.WriteU16 ( (m_startIndicator << 15) | (m_endIndicator << 14) | (m_ppduLengthInBytes << 3) );
    }
  // START, CONTINUATION OR END PDU
  else
    {
      // Fragment id
      i.WriteU16 ( (m_startIndicator << 15) | (m_endIndicator << 14) | (m_ppduLengthInBytes << 3) | (m_fragmentId) );
    }

  // START PDU
  if (m_startIndicator && !m_endIndicator)
    {
      i.WriteU16 ( (0x0 << 15 ) | ( m_totalLengthInBytes << 3 ) );
    }

  /**
   * LT, T and C flags and PPDU_Label are not currently used.
   */
}

uint32_t SatPPduHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint16_t field_16;

  // First two bytes
  field_16 = i.ReadU16 ();
  m_startIndicator = field_16 >> 15 & 0x1;
  m_endIndicator = field_16 >> 14 & 0x1;
  m_ppduLengthInBytes = (field_16 & 0x3FF8) >> 3;

  // NOT FULL PDU
  if (!(m_startIndicator && m_endIndicator))
    {
      m_fragmentId = field_16 & 0x0007;
    }

  // START PDU
  if (m_startIndicator && !m_endIndicator)
    {
      field_16 = i.ReadU16 ();
      m_totalLengthInBytes = (field_16 & 0x7FF8) >> 3;
    }

  /**
   * LT, T and C flags and PPDU_Label are not currently used.
   */

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

uint16_t SatPPduHeader::GetPPduLength () const
{
  return m_ppduLengthInBytes;
}

uint8_t SatPPduHeader::GetFragmentId () const
{
  return m_fragmentId;
}

uint16_t SatPPduHeader::GetTotalLength () const
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

void SatPPduHeader::SetPPduLength (uint16_t bytes)
{
  m_ppduLengthInBytes = bytes;
}

void SatPPduHeader::SetFragmentId (uint8_t id)
{
  m_fragmentId = id;
}

void SatPPduHeader::SetTotalLength (uint16_t bytes)
{
  m_totalLengthInBytes = bytes;
}

uint32_t SatPPduHeader::GetHeaderSizeInBytes (uint8_t type) const
{
  uint32_t size (0);
  switch (type)
  {
    case SatEncapPduStatusTag::START_PDU:
      {
        size = m_startPpduHeaderSize;
        break;
      }
    case SatEncapPduStatusTag::CONTINUATION_PDU:
      {
        size = m_continuationPpduHeaderSize;
        break;
      }
    case SatEncapPduStatusTag::END_PDU:
      {
        size = m_endPpduHeaderSize;
        break;
      }
    case SatEncapPduStatusTag::FULL_PDU:
      {
        size = m_fullPpduHeaderSize;
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unsupported SatEncapPduStatusTag!");
        break;
      }
  }
  return size;
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
