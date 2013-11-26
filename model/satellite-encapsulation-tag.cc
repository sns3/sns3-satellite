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

#include "satellite-encapsulation-tag.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatEncapTag);

SatEncapTag::SatEncapTag ()
  : m_senderTimestamp (Seconds (0))
{
  // Nothing to do here
}


SatEncapTag::SatEncapTag (Time senderTimestamp)
  : m_senderTimestamp (senderTimestamp)

{
  // Nothing to do here
}

TypeId
SatEncapTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatEncapTag")
    .SetParent<Tag> ()
    .AddConstructor<SatEncapTag> ();
  return tid;
}

TypeId
SatEncapTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
SatEncapTag::GetSerializedSize (void) const
{
  return sizeof(Time);
}

void
SatEncapTag::Serialize (TagBuffer i) const
{
  int64_t senderTimestamp = m_senderTimestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *)&senderTimestamp, sizeof(int64_t));
}

void
SatEncapTag::Deserialize (TagBuffer i)
{
  int64_t senderTimestamp;
  i.Read ((uint8_t *)&senderTimestamp, 8);
  m_senderTimestamp   = NanoSeconds (senderTimestamp);

}

void
SatEncapTag::Print (std::ostream &os) const
{
  os << m_senderTimestamp;
}

} // namespace ns3

