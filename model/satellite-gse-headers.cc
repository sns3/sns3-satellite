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

#include "satellite-gse-headers.h"


NS_LOG_COMPONENT_DEFINE ("SatGseHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatGseHeader);

SatGseHeader::SatGseHeader ()
:m_startIndicator (0),
 m_endIndicator (0),
 m_gsePduLengthInBytes (0),
 m_fragmentId (0),
 m_totalLengthInBytes (0)
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

uint32_t SatGseHeader::GetSerializedSize (void) const
{
  return ( sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t));
}

void SatGseHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteU8 (m_startIndicator);
  start.WriteU8 (m_endIndicator);
  start.WriteU32 (m_gsePduLengthInBytes);
  start.WriteU32 (m_fragmentId);
  start.WriteU32 (m_totalLengthInBytes);
}

uint32_t SatGseHeader::Deserialize (Buffer::Iterator start)
{
  m_startIndicator = start.ReadU8 ();
  m_endIndicator = start.ReadU8 ();
  m_gsePduLengthInBytes = start.ReadU32 ();
  m_fragmentId = start.ReadU32 ();
  m_totalLengthInBytes = start.ReadU32 ();

  return GetSerializedSize();
}

void SatGseHeader::Print (std::ostream &os) const
{
  os << m_startIndicator << " " << m_endIndicator << " " << m_gsePduLengthInBytes << " " << m_fragmentId << " " << m_totalLengthInBytes;
}

TypeId
SatGseHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint8_t SatGseHeader::GetStartIndicator () const
{
  return m_startIndicator;
}

uint8_t SatGseHeader::GetEndIndicator () const
{
  return m_endIndicator;
}

uint32_t SatGseHeader::GetGsePduLength () const
{
  return m_gsePduLengthInBytes;
}

uint32_t SatGseHeader::GetFragmentId () const
{
  return m_fragmentId;
}

uint32_t SatGseHeader::GetTotalLength () const
{
  return m_totalLengthInBytes;
}

void SatGseHeader::SetStartIndicator ()
{
  m_startIndicator = 1;
}

void SatGseHeader::SetEndIndicator ()
{
  m_endIndicator = 1;
}

void SatGseHeader::SetGsePduLength (uint32_t bytes)
{
  m_gsePduLengthInBytes = bytes;
}

void SatGseHeader::SetFragmentId (uint32_t id)
{
  m_fragmentId = id;
}

void SatGseHeader::SetTotalLength (uint32_t bytes)
{
  m_totalLengthInBytes = bytes;
}

}; // namespace ns3
