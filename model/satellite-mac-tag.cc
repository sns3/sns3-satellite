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
#include "satellite-mac-tag.h"

NS_LOG_COMPONENT_DEFINE ("SatMacTag");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatMacTag);


SatMacTag::SatMacTag ()
{
  NS_LOG_FUNCTION (this);
}

SatMacTag::~SatMacTag ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SatMacTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatMacTag")
    .SetParent<Tag> ()
    .AddConstructor<SatMacTag> ()
  ;
  return tid;
}
TypeId
SatMacTag::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);

  return GetTypeId ();
}

void
SatMacTag::SetDestAddress (Address dest)
{
  NS_LOG_FUNCTION (this << dest);
  m_destAddress = dest;
}

Address
SatMacTag::GetDestAddress (void) const
{
  NS_LOG_FUNCTION (this);
  return m_destAddress;
}

void
SatMacTag::SetSourceAddress (Address source)
{
  NS_LOG_FUNCTION (this << source);
  m_sourceAddress = source;
}

Address
SatMacTag::GetSourceAddress (void) const
{
  NS_LOG_FUNCTION (this);
  return m_sourceAddress;
}

uint32_t
SatMacTag::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);

  return ( m_destAddress.GetLength() + m_sourceAddress.GetLength() + 2 * sizeof (uint32_t) );
}
void
SatMacTag::Serialize (TagBuffer i) const
{
  NS_LOG_FUNCTION (this << &i);

  uint8_t buff[Address::MAX_SIZE];
  uint32_t len = m_destAddress.CopyTo(buff);

  i.WriteU32 (len);
  i.Write (buff, len);

  len = m_sourceAddress.CopyTo(buff);
  i.WriteU32 (len);

  m_sourceAddress.CopyTo(buff);
  i.Write (buff, len);
}

void
SatMacTag::Deserialize (TagBuffer i)
{
  NS_LOG_FUNCTION (this << &i);

  uint8_t buff[Address::MAX_SIZE];
  uint32_t len = i.ReadU32();

  i.Read (buff, len);
  m_destAddress.CopyFrom(buff, len);

  len = i.ReadU32();
  i.Read (buff, len);
  m_sourceAddress.CopyFrom(buff, len);
}

void
SatMacTag::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "DestAddress=" << m_destAddress << "SourceAddress" << m_sourceAddress;
}


} // namespace ns3
