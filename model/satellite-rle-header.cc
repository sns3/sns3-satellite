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
#include "satellite-rle-header.h"
#include "satellite-encap-pdu-status-tag.h"


NS_LOG_COMPONENT_DEFINE ("SatPPduHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatPPduHeader);

SatPPduHeader::SatPPduHeader ()
  : m_startIndicator (0),
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

uint32_t SatPPduHeader::GetSerializedSize () const
{
  NS_LOG_FUNCTION (this);

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

  return 0;
}

void SatPPduHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this);

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
  NS_LOG_FUNCTION (this);

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

  return GetSerializedSize ();
}

void SatPPduHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this);

  os << m_startIndicator << " " << m_endIndicator << " " << m_ppduLengthInBytes << " " << m_fragmentId << " " << m_totalLengthInBytes;
}

TypeId
SatPPduHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint8_t SatPPduHeader::GetStartIndicator () const
{
  NS_LOG_FUNCTION (this);

  return m_startIndicator;
}

uint8_t SatPPduHeader::GetEndIndicator () const
{
  NS_LOG_FUNCTION (this);

  return m_endIndicator;
}

uint16_t SatPPduHeader::GetPPduLength () const
{
  NS_LOG_FUNCTION (this);

  return m_ppduLengthInBytes;
}

uint8_t SatPPduHeader::GetFragmentId () const
{
  NS_LOG_FUNCTION (this);

  return m_fragmentId;
}

uint16_t SatPPduHeader::GetTotalLength () const
{
  NS_LOG_FUNCTION (this);

  return m_totalLengthInBytes;
}

void SatPPduHeader::SetStartIndicator ()
{
  NS_LOG_FUNCTION (this);

  m_startIndicator = 1;
}

void SatPPduHeader::SetEndIndicator ()
{
  NS_LOG_FUNCTION (this);

  m_endIndicator = 1;
}

void SatPPduHeader::SetPPduLength (uint16_t bytes)
{
  NS_LOG_FUNCTION (this << bytes);

  m_ppduLengthInBytes = bytes;
}

void SatPPduHeader::SetFragmentId (uint8_t id)
{
  NS_LOG_FUNCTION (this << (uint32_t) id);

  m_fragmentId = id;
}

void SatPPduHeader::SetTotalLength (uint16_t bytes)
{
  NS_LOG_FUNCTION (this << bytes);

  m_totalLengthInBytes = bytes;
}

uint32_t SatPPduHeader::GetHeaderSizeInBytes (uint8_t type) const
{
  NS_LOG_FUNCTION (this << (uint32_t) type);

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
        NS_FATAL_ERROR ("Unsupported SatEncapPduStatusTag: " << type);
        break;
      }
    }
  return size;
}


uint32_t SatPPduHeader::GetMaxHeaderSizeInBytes () const
{
  NS_LOG_FUNCTION (this);

  return std::max (std::max (m_startPpduHeaderSize, m_continuationPpduHeaderSize), std::max (m_endPpduHeaderSize, m_fullPpduHeaderSize));
}


}; // namespace ns3
