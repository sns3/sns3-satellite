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

#include "satellite-encapsulation-sdu-status-tag.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatEncapSduStatusTag);

SatEncapSduStatusTag::SatEncapSduStatusTag ()
{
}

void
SatEncapSduStatusTag::SetStatus (uint8_t status)
{
  m_sduStatus = status;
}

uint8_t
SatEncapSduStatusTag::GetStatus (void) const
{
  return m_sduStatus;
}

TypeId
SatEncapSduStatusTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatEncapSduStatusTag")
    .SetParent<Tag> ()
    .AddConstructor<SatEncapSduStatusTag> ()
  ;
  return tid;
}
TypeId
SatEncapSduStatusTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
SatEncapSduStatusTag::GetSerializedSize (void) const
{
  return 1;
}
void
SatEncapSduStatusTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_sduStatus);
}
void
SatEncapSduStatusTag::Deserialize (TagBuffer i)
{
  m_sduStatus = i.ReadU8 ();
}
void
SatEncapSduStatusTag::Print (std::ostream &os) const
{
  os << "SDU Status=" << (uint32_t) m_sduStatus;
}

}; // namespace ns3
