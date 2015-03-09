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
#include "ns3/uinteger.h"
#include "satellite-gse-header.h"
#include "satellite-encap-pdu-status-tag.h"


NS_LOG_COMPONENT_DEFINE ("SatGseHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGseHeader);

SatGseHeader::SatGseHeader ()
  : m_startIndicator (0),
    m_endIndicator (0),
    m_gsePduLengthInBytes (0),
    m_fragmentId (0),
    m_totalLengthInBytes (0),
    m_protocolType (0),
    m_labelByte (0),
    m_crc (0),
    m_fullGseHeaderSize (8),
    m_startGseHeaderSize (8),
    m_endGseHeaderSize (8),
    m_continuationGseHeaderSize (3)
{

}


SatGseHeader::~SatGseHeader ()
{

}

TypeId
SatGseHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatGseHeader")
    .SetParent<Header> ()
    .AddConstructor<SatGseHeader> ()
  ;
  return tid;
}

uint32_t SatGseHeader::GetSerializedSize () const
{
  NS_LOG_FUNCTION (this);

  if (m_startIndicator)
    {
      // FULL PDU
      if (m_endIndicator)
        {
          return m_fullGseHeaderSize + m_labelFieldLengthInBytes;
        }
      // START PDU
      else
        {
          return m_startGseHeaderSize + m_labelFieldLengthInBytes;
        }
    }
  else
    {
      // END PDU
      if (m_endIndicator)
        {
          return m_endGseHeaderSize;
        }
      // CONTINUATION PDU
      else
        {
          return m_continuationGseHeaderSize;
        }
    }
  return 0;
}

void SatGseHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this);

  Buffer::Iterator i = start;

  // First two bytes
  i.WriteU16 ( (m_startIndicator << 15) | (m_endIndicator << 14) | (0x0 << 13) | (0x0 << 12) | (m_gsePduLengthInBytes & 0x0FFF) );

  // NOT FULL PDU (START PDU OR CONTINUATION PDU OR END PDU)
  if ( !(m_startIndicator && m_endIndicator) )
    {
      i.WriteU8 (m_fragmentId);
    }

  // START OR FULL PDU
  if (m_startIndicator)
    {
      // START PDU
      if (!m_endIndicator)
        {
          i.WriteU16 (m_totalLengthInBytes);
        }

      // Protocol type
      i.WriteU16 (m_protocolType);

      for (uint32_t c = 0; c < m_labelFieldLengthInBytes; ++c)
        {
          i.WriteU8 (m_labelByte);
        }
    }

  // FULL OR END PDU
  if (m_endIndicator)
    {
      i.WriteU32 (m_crc);
    }
}

uint32_t SatGseHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this);

  Buffer::Iterator i = start;
  uint16_t field_16;

  // First two bytes
  field_16 = i.ReadU16 ();
  m_startIndicator = field_16 >> 15 & 0x1;
  m_endIndicator = field_16 >> 14 & 0x1;
  m_gsePduLengthInBytes = field_16 & 0x0FFF;

  // NOT FULL PDU
  if ( !(m_startIndicator && m_endIndicator) )
    {
      m_fragmentId = i.ReadU8 ();
    }

  // START OR FULL PDU
  if (m_startIndicator)
    {
      // START PDU
      if (!m_endIndicator)
        {
          m_totalLengthInBytes = i.ReadU16 ();
        }
    }

  /**
   * The GSE header includes also other fields, but currently they are not
   * deserialized because they are not used. They are just serialized so
   * that the header size is correct.
   */

  return GetSerializedSize ();
}

void SatGseHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this);

  os << m_startIndicator << " " << m_endIndicator << " "
     << m_gsePduLengthInBytes << " " << m_fragmentId << " "
     << m_totalLengthInBytes << " " << m_protocolType << " "
     << m_labelByte << " " << m_crc << std::endl;
}

TypeId
SatGseHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint8_t SatGseHeader::GetStartIndicator () const
{
  NS_LOG_FUNCTION (this);
  return m_startIndicator;
}

uint8_t SatGseHeader::GetEndIndicator () const
{
  NS_LOG_FUNCTION (this);
  return m_endIndicator;
}

uint32_t SatGseHeader::GetGsePduLength () const
{
  NS_LOG_FUNCTION (this);
  return m_gsePduLengthInBytes;
}

uint32_t SatGseHeader::GetFragmentId () const
{
  NS_LOG_FUNCTION (this);
  return m_fragmentId;
}

uint32_t SatGseHeader::GetTotalLength () const
{
  NS_LOG_FUNCTION (this);
  return m_totalLengthInBytes;
}

void SatGseHeader::SetStartIndicator ()
{
  NS_LOG_FUNCTION (this);
  m_startIndicator = 1;
}

void SatGseHeader::SetEndIndicator ()
{
  NS_LOG_FUNCTION (this);
  m_endIndicator = 1;
}

void SatGseHeader::SetGsePduLength (uint32_t bytes)
{
  NS_LOG_FUNCTION (this << bytes);
  m_gsePduLengthInBytes = bytes;
}

void SatGseHeader::SetFragmentId (uint32_t id)
{
  NS_LOG_FUNCTION (this << id);
  m_fragmentId = id;
}

void SatGseHeader::SetTotalLength (uint32_t bytes)
{
  NS_LOG_FUNCTION (this << bytes);
  m_totalLengthInBytes = bytes;
}

uint32_t SatGseHeader::GetGseHeaderSizeInBytes (uint8_t type) const
{
  NS_LOG_FUNCTION (this << (uint32_t) type);

  uint32_t size (0);
  switch (type)
    {
    case SatEncapPduStatusTag::START_PDU:
      {
        size = m_startGseHeaderSize + m_labelFieldLengthInBytes;
        break;
      }
    case SatEncapPduStatusTag::CONTINUATION_PDU:
      {
        size = m_continuationGseHeaderSize;
        break;
      }
    case SatEncapPduStatusTag::END_PDU:
      {
        size = m_endGseHeaderSize;
        break;
      }
    case SatEncapPduStatusTag::FULL_PDU:
      {
        size = m_fullGseHeaderSize + m_labelFieldLengthInBytes;
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

uint32_t SatGseHeader::GetMaxGseHeaderSizeInBytes () const
{
  NS_LOG_FUNCTION (this);

  return std::max (std::max ( (m_fullGseHeaderSize + m_labelFieldLengthInBytes),
                              (m_startGseHeaderSize + m_labelFieldLengthInBytes) ),
                   (std::max ( m_endGseHeaderSize,
                               m_continuationGseHeaderSize)));
}

}; // namespace ns3
