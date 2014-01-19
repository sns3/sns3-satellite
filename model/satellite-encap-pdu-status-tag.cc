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

#include "satellite-encap-pdu-status-tag.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatEncapPduStatusTag);

SatEncapPduStatusTag::SatEncapPduStatusTag ()
{
}

void
SatEncapPduStatusTag::SetStatus (uint8_t status)
{
  m_pduStatus = status;
}

uint8_t
SatEncapPduStatusTag::GetStatus (void) const
{
  return m_pduStatus;
}


TypeId
SatEncapPduStatusTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatEncapPduStatusTag")
    .SetParent<Tag> ()
    .AddConstructor<SatEncapPduStatusTag> ()
  ;
  return tid;
}
TypeId
SatEncapPduStatusTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
SatEncapPduStatusTag::GetSerializedSize (void) const
{
  return 1;
}
void
SatEncapPduStatusTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_pduStatus);
}
void
SatEncapPduStatusTag::Deserialize (TagBuffer i)
{
  m_pduStatus = i.ReadU8 ();
}
void
SatEncapPduStatusTag::Print (std::ostream &os) const
{
  os << "PPDU Status=" << (uint32_t) m_pduStatus;
}

}; // namespace ns3
