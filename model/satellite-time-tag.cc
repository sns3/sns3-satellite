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

#include "ns3/tag.h"
#include "ns3/uinteger.h"

#include "satellite-time-tag.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatTimeTag);

SatTimeTag::SatTimeTag ()
  : m_senderTimestamp (Seconds (0))
{
  // Nothing to do here
}


SatTimeTag::SatTimeTag (Time senderTimestamp)
  : m_senderTimestamp (senderTimestamp)

{
  // Nothing to do here
}

TypeId
SatTimeTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatTimeTag")
    .SetParent<Tag> ()
    .AddConstructor<SatTimeTag> ();
  return tid;
}

TypeId
SatTimeTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
SatTimeTag::GetSerializedSize (void) const
{
  return sizeof(Time);
}

void
SatTimeTag::Serialize (TagBuffer i) const
{
  int64_t senderTimestamp = m_senderTimestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *)&senderTimestamp, sizeof(int64_t));
}

void
SatTimeTag::Deserialize (TagBuffer i)
{
  int64_t senderTimestamp;
  i.Read ((uint8_t *)&senderTimestamp, 8);
  m_senderTimestamp   = NanoSeconds (senderTimestamp);
}

void
SatTimeTag::Print (std::ostream &os) const
{
  os << m_senderTimestamp;
}

} // namespace ns3

