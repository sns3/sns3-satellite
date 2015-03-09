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
#include "satellite-encap-pdu-status-tag.h"

NS_LOG_COMPONENT_DEFINE ("SatEncapPduStatusTag");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SatEncapPduStatusTag);

SatEncapPduStatusTag::SatEncapPduStatusTag ()
  : m_pduStatus (FULL_PDU)
{
}

void
SatEncapPduStatusTag::SetStatus (uint8_t status)
{
  NS_LOG_FUNCTION (this << (uint32_t) status);
  m_pduStatus = status;
}

uint8_t
SatEncapPduStatusTag::GetStatus () const
{
  NS_LOG_FUNCTION (this);
  return m_pduStatus;
}


TypeId
SatEncapPduStatusTag::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SatEncapPduStatusTag")
    .SetParent<Tag> ()
    .AddConstructor<SatEncapPduStatusTag> ()
  ;
  return tid;
}
TypeId
SatEncapPduStatusTag::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
SatEncapPduStatusTag::GetSerializedSize () const
{
  NS_LOG_FUNCTION (this);
  return 1;
}
void
SatEncapPduStatusTag::Serialize (TagBuffer i) const
{
  NS_LOG_FUNCTION (this);
  i.WriteU8 (m_pduStatus);
}
void
SatEncapPduStatusTag::Deserialize (TagBuffer i)
{
  NS_LOG_FUNCTION (this);
  m_pduStatus = i.ReadU8 ();
}
void
SatEncapPduStatusTag::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this);
  os << "PPDU Status=" << (uint32_t) m_pduStatus;
}

}; // namespace ns3
