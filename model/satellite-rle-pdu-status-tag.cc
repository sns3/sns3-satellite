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

#include "satellite-rle-pdu-status-tag.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatRlePduStatusTag);

SatRlePduStatusTag::SatRlePduStatusTag ()
{
}

void
SatRlePduStatusTag::SetStatus (uint8_t status)
{
  m_ppduStatus = status;
}

uint8_t
SatRlePduStatusTag::GetStatus (void) const
{
  return m_ppduStatus;
}


TypeId
SatRlePduStatusTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SatRlePduStatusTag")
    .SetParent<Tag> ()
    .AddConstructor<SatRlePduStatusTag> ()
  ;
  return tid;
}
TypeId
SatRlePduStatusTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
SatRlePduStatusTag::GetSerializedSize (void) const
{
  return 1;
}
void
SatRlePduStatusTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_ppduStatus);
}
void
SatRlePduStatusTag::Deserialize (TagBuffer i)
{
  m_ppduStatus = i.ReadU8 ();
}
void
SatRlePduStatusTag::Print (std::ostream &os) const
{
  os << "PPDU Status=" << (uint32_t) m_ppduStatus;
}

}; // namespace ns3
