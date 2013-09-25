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
SatMacTag::SetAddress (Address dest)
{
  NS_LOG_FUNCTION (this << dest);
  m_macAddress = dest;
}

Address
SatMacTag::GetAddress (void) const
{
  NS_LOG_FUNCTION (this);
  return m_macAddress;
}

uint32_t
SatMacTag::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);

  return ( m_macAddress.GetSerializedSize() );
}
void
SatMacTag::Serialize (TagBuffer i) const
{
  NS_LOG_FUNCTION (this << &i);
  m_macAddress.Serialize (i);
}

void
SatMacTag::Deserialize (TagBuffer i)
{
  NS_LOG_FUNCTION (this << &i);
  m_macAddress.Deserialize (i);
}

void
SatMacTag::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "MacAddress=" << m_macAddress;
}


} // namespace ns3
